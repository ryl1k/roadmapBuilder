#include "../../include/storage/sqlite_storage.hpp"
#include "../../third_party/json.hpp"
#include <stdexcept>
#include <sstream>
#include <functional>

using json = nlohmann::json;

SqliteStorage::SqliteStorage(const std::string& dbPath) : dbPath(dbPath), db(nullptr) {
	int rc = sqlite3_open(dbPath.c_str(), &db);
	if (rc != SQLITE_OK) {
		throw std::runtime_error("Cannot open database: " + std::string(sqlite3_errmsg(db)));
	}
	createTables();
}

SqliteStorage::~SqliteStorage() {
	if (db) {
		sqlite3_close(db);
	}
}

void SqliteStorage::createTables() {
	const char* sql = R"(
		CREATE TABLE IF NOT EXISTS users (
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			username TEXT UNIQUE NOT NULL,
			email TEXT UNIQUE NOT NULL,
			password_hash TEXT NOT NULL,
			created_at DATETIME DEFAULT CURRENT_TIMESTAMP
		);

		CREATE TABLE IF NOT EXISTS plans (
			user_id INTEGER PRIMARY KEY,
			total_hours INTEGER NOT NULL,
			created_at DATETIME DEFAULT CURRENT_TIMESTAMP
		);

		CREATE TABLE IF NOT EXISTS plan_steps (
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			user_id INTEGER,
			step INTEGER NOT NULL,
			course_id INTEGER NOT NULL,
			hours INTEGER NOT NULL,
			note TEXT,
			FOREIGN KEY (user_id) REFERENCES plans(user_id)
		);
	)";

	char* errMsg = nullptr;
	int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
	if (rc != SQLITE_OK) {
		std::string error = errMsg;
		sqlite3_free(errMsg);
		throw std::runtime_error("SQL error: " + error);
	}
}

void SqliteStorage::savePlan(int userId, const Plan& plan) {
	// Begin transaction
	sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

	// Delete existing plan
	std::string deletePlan = "DELETE FROM plans WHERE user_id = " + std::to_string(userId);
	std::string deleteSteps = "DELETE FROM plan_steps WHERE user_id = " + std::to_string(userId);
	sqlite3_exec(db, deletePlan.c_str(), nullptr, nullptr, nullptr);
	sqlite3_exec(db, deleteSteps.c_str(), nullptr, nullptr, nullptr);

	// Insert plan
	const char* planSql = "INSERT INTO plans (user_id, total_hours) VALUES (?, ?)";
	sqlite3_stmt* stmt;

	if (sqlite3_prepare_v2(db, planSql, -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_int(stmt, 1, userId);
		sqlite3_bind_int(stmt, 2, plan.getTotalHours());
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}

	// Insert steps
	for (const auto& step : plan.getSteps()) {
		const char* stepSql = "INSERT INTO plan_steps (user_id, step, course_id, hours, note) VALUES (?, ?, ?, ?, ?)";
		sqlite3_stmt* stepStmt;

		if (sqlite3_prepare_v2(db, stepSql, -1, &stepStmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stepStmt, 1, userId);
			sqlite3_bind_int(stepStmt, 2, step.step);
			sqlite3_bind_int(stepStmt, 3, step.courseId);
			sqlite3_bind_int(stepStmt, 4, step.hours);
			sqlite3_bind_text(stepStmt, 5, step.note.c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_step(stepStmt);
			sqlite3_finalize(stepStmt);
		}
	}

	// Commit transaction
	sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
}

std::optional<Plan> SqliteStorage::loadPlan(int userId) {
	const char* sql = "SELECT total_hours FROM plans WHERE user_id = ?";
	sqlite3_stmt* stmt;

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return std::nullopt;
	}

	sqlite3_bind_int(stmt, 1, userId);

	if (sqlite3_step(stmt) != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return std::nullopt;
	}

	Plan plan;
	plan.setTotalHours(sqlite3_column_int(stmt, 0));
	sqlite3_finalize(stmt);

	// Get steps
	const char* stepsSql = "SELECT step, course_id, hours, note FROM plan_steps WHERE user_id = ? ORDER BY step";
	sqlite3_stmt* stepsStmt;

	if (sqlite3_prepare_v2(db, stepsSql, -1, &stepsStmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_int(stepsStmt, 1, userId);

		std::vector<PlanStep> steps;
		while (sqlite3_step(stepsStmt) == SQLITE_ROW) {
			PlanStep step;
			step.step = sqlite3_column_int(stepsStmt, 0);
			step.courseId = sqlite3_column_int(stepsStmt, 1);
			step.hours = sqlite3_column_int(stepsStmt, 2);
			step.note = reinterpret_cast<const char*>(sqlite3_column_text(stepsStmt, 3));
			steps.push_back(step);
		}
		plan.setSteps(steps);
		sqlite3_finalize(stepsStmt);
	}

	return plan;
}

std::string SqliteStorage::hashPassword(const std::string& password) {
	// Simple hash for demo (use bcrypt in production)
	std::hash<std::string> hasher;
	return std::to_string(hasher(password));
}

void SqliteStorage::saveUser(const std::string& username, const std::string& email, const std::string& password) {
	const char* sql = "INSERT INTO users (username, email, password_hash) VALUES (?, ?, ?)";
	sqlite3_stmt* stmt;

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
		sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
		std::string hash = hashPassword(password);
		sqlite3_bind_text(stmt, 3, hash.c_str(), -1, SQLITE_TRANSIENT);

		if (sqlite3_step(stmt) != SQLITE_DONE) {
			sqlite3_finalize(stmt);
			throw std::runtime_error("Failed to create user");
		}
		sqlite3_finalize(stmt);
	} else {
		throw std::runtime_error("Failed to prepare statement");
	}
}

bool SqliteStorage::validateUser(const std::string& username, const std::string& password) {
	const char* sql = "SELECT password_hash FROM users WHERE username = ?";
	sqlite3_stmt* stmt;

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return false;
	}

	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

	if (sqlite3_step(stmt) != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return false;
	}

	std::string storedHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
	sqlite3_finalize(stmt);

	return storedHash == hashPassword(password);
}

std::optional<json> SqliteStorage::getUser(const std::string& username) {
	const char* sql = "SELECT id, username, email FROM users WHERE username = ?";
	sqlite3_stmt* stmt;

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		return std::nullopt;
	}

	sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

	if (sqlite3_step(stmt) != SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return std::nullopt;
	}

	json user = {
		{"id", sqlite3_column_int(stmt, 0)},
		{"username", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))},
		{"email", reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))}
	};

	sqlite3_finalize(stmt);
	return user;
}
