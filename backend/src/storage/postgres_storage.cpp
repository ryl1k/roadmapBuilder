#include "../../include/storage/postgres_storage.hpp"
#include "../../third_party/json.hpp"
#include <stdexcept>
#include <functional>
#include <iostream>

using json = nlohmann::json;

PostgresStorage::PostgresStorage(const std::string& connectionString)
	: connStr(connectionString) {
	try {
		conn = std::make_unique<pqxx::connection>(connStr);
		if (!conn->is_open()) {
			throw std::runtime_error("Failed to connect to PostgreSQL");
		}
		std::cout << "Connected to PostgreSQL: " << conn->dbname() << std::endl;
		createTables();
	} catch (const std::exception& e) {
		throw std::runtime_error("PostgreSQL connection error: " + std::string(e.what()));
	}
}

PostgresStorage::~PostgresStorage() {
	if (conn && conn->is_open()) {
		conn->close();
	}
}

void PostgresStorage::reconnect() {
	if (!conn || !conn->is_open()) {
		conn = std::make_unique<pqxx::connection>(connStr);
	}
}

void PostgresStorage::createTables() {
	try {
		pqxx::work txn(*conn);

		// Users table
		txn.exec(R"(
			CREATE TABLE IF NOT EXISTS users (
				id SERIAL PRIMARY KEY,
				username VARCHAR(255) UNIQUE NOT NULL,
				email VARCHAR(255) UNIQUE NOT NULL,
				password_hash VARCHAR(255) NOT NULL,
				created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
			)
		)");

		// Plans table
		txn.exec(R"(
			CREATE TABLE IF NOT EXISTS plans (
				user_id INTEGER PRIMARY KEY,
				total_hours INTEGER NOT NULL,
				created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
				FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
			)
		)");

		// Plan steps table
		txn.exec(R"(
			CREATE TABLE IF NOT EXISTS plan_steps (
				id SERIAL PRIMARY KEY,
				user_id INTEGER NOT NULL,
				step INTEGER NOT NULL,
				course_id INTEGER NOT NULL,
				hours INTEGER NOT NULL,
				note TEXT,
				FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
			)
		)");

		// Create indexes for better performance
		txn.exec("CREATE INDEX IF NOT EXISTS idx_plan_steps_user_id ON plan_steps(user_id)");
		txn.exec("CREATE INDEX IF NOT EXISTS idx_plans_user_id ON plans(user_id)");

		txn.commit();
		std::cout << "Database tables created/verified" << std::endl;
	} catch (const std::exception& e) {
		throw std::runtime_error("Failed to create tables: " + std::string(e.what()));
	}
}

std::string PostgresStorage::hashPassword(const std::string& password) {
	// Simple hash for demo (use bcrypt in production)
	std::hash<std::string> hasher;
	return std::to_string(hasher(password));
}

void PostgresStorage::saveUser(const std::string& username, const std::string& email, const std::string& password) {
	try {
		reconnect();
		pqxx::work txn(*conn);

		std::string hash = hashPassword(password);
		txn.exec_params(
			"INSERT INTO users (username, email, password_hash) VALUES ($1, $2, $3)",
			username, email, hash
		);

		txn.commit();
	} catch (const pqxx::unique_violation& e) {
		throw std::runtime_error("Username or email already exists");
	} catch (const std::exception& e) {
		throw std::runtime_error("Failed to create user: " + std::string(e.what()));
	}
}

bool PostgresStorage::validateUser(const std::string& username, const std::string& password) {
	try {
		reconnect();
		pqxx::work txn(*conn);

		auto result = txn.exec_params(
			"SELECT password_hash FROM users WHERE username = $1",
			username
		);

		if (result.empty()) {
			return false;
		}

		std::string storedHash = result[0][0].as<std::string>();
		return storedHash == hashPassword(password);
	} catch (const std::exception& e) {
		std::cerr << "Validation error: " << e.what() << std::endl;
		return false;
	}
}

std::optional<json> PostgresStorage::getUser(const std::string& username) {
	try {
		reconnect();
		pqxx::work txn(*conn);

		auto result = txn.exec_params(
			"SELECT id, username, email FROM users WHERE username = $1",
			username
		);

		if (result.empty()) {
			return std::nullopt;
		}

		json user = {
			{"id", result[0][0].as<int>()},
			{"username", result[0][1].as<std::string>()},
			{"email", result[0][2].as<std::string>()}
		};

		return user;
	} catch (const std::exception& e) {
		std::cerr << "Get user error: " << e.what() << std::endl;
		return std::nullopt;
	}
}

void PostgresStorage::savePlan(int userId, const Plan& plan) {
	try {
		reconnect();
		pqxx::work txn(*conn);

		// Delete existing plan
		txn.exec_params("DELETE FROM plan_steps WHERE user_id = $1", userId);
		txn.exec_params("DELETE FROM plans WHERE user_id = $1", userId);

		// Insert new plan
		txn.exec_params(
			"INSERT INTO plans (user_id, total_hours) VALUES ($1, $2)",
			userId, plan.getTotalHours()
		);

		// Insert steps
		for (const auto& step : plan.getSteps()) {
			txn.exec_params(
				"INSERT INTO plan_steps (user_id, step, course_id, hours, note) VALUES ($1, $2, $3, $4, $5)",
				userId, step.step, step.courseId, step.hours, step.note
			);
		}

		txn.commit();
	} catch (const std::exception& e) {
		throw std::runtime_error("Failed to save plan: " + std::string(e.what()));
	}
}

std::optional<Plan> PostgresStorage::loadPlan(int userId) {
	try {
		reconnect();
		pqxx::work txn(*conn);

		// Get plan
		auto planResult = txn.exec_params(
			"SELECT total_hours FROM plans WHERE user_id = $1",
			userId
		);

		if (planResult.empty()) {
			return std::nullopt;
		}

		Plan plan;
		plan.setTotalHours(planResult[0][0].as<int>());

		// Get steps
		auto stepsResult = txn.exec_params(
			"SELECT step, course_id, hours, note FROM plan_steps WHERE user_id = $1 ORDER BY step",
			userId
		);

		std::vector<PlanStep> steps;
		for (const auto& row : stepsResult) {
			PlanStep step;
			step.step = row[0].as<int>();
			step.courseId = row[1].as<int>();
			step.hours = row[2].as<int>();
			step.note = row[3].as<std::string>();
			steps.push_back(step);
		}

		plan.setSteps(steps);
		return plan;
	} catch (const std::exception& e) {
		std::cerr << "Load plan error: " << e.what() << std::endl;
		return std::nullopt;
	}
}
