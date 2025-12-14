#include "../../include/catalog/postgres_catalog.hpp"
#include "../../third_party/json.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>

using json = nlohmann::json;

PostgresCatalog::PostgresCatalog(const std::string& connectionString)
	: connStr(connectionString) {
	try {
		conn = std::make_unique<pqxx::connection>(connStr);
		if (!conn->is_open()) {
			throw std::runtime_error("Failed to connect to PostgreSQL");
		}
		createTables();
	} catch (const std::exception& e) {
		throw std::runtime_error("PostgreSQL catalog error: " + std::string(e.what()));
	}
}

PostgresCatalog::~PostgresCatalog() {
	if (conn && conn->is_open()) {
		conn->close();
	}
}

void PostgresCatalog::reconnect() {
	if (!conn || !conn->is_open()) {
		conn = std::make_unique<pqxx::connection>(connStr);
	}
}

void PostgresCatalog::createTables() {
	try {
		pqxx::work txn(*conn);

		txn.exec(R"(
			CREATE TABLE IF NOT EXISTS courses (
				id INTEGER PRIMARY KEY,
				title VARCHAR(255) NOT NULL,
				domain VARCHAR(100) NOT NULL,
				level VARCHAR(50) NOT NULL,
				duration_hours INTEGER NOT NULL,
				tags TEXT[] NOT NULL,
				prereq_ids INTEGER[] NOT NULL DEFAULT '{}'
			)
		)");

		txn.exec("CREATE INDEX IF NOT EXISTS idx_courses_domain ON courses(domain)");
		txn.exec("CREATE INDEX IF NOT EXISTS idx_courses_level ON courses(level)");

		txn.commit();
		std::cout << "Courses table created/verified" << std::endl;
	} catch (const std::exception& e) {
		throw std::runtime_error("Failed to create courses table: " + std::string(e.what()));
	}
}

void PostgresCatalog::importFromJson(const std::string& jsonPath) {
	try {
		std::ifstream file(jsonPath);
		if (!file.is_open()) {
			throw std::runtime_error("Cannot open JSON file: " + jsonPath);
		}

		json coursesJson;
		file >> coursesJson;

		reconnect();
		pqxx::work txn(*conn);

		// Clear existing courses
		txn.exec("DELETE FROM courses");

		// Insert courses
		for (const auto& courseJson : coursesJson) {
			int id = courseJson["id"];
			std::string title = courseJson["title"];
			std::string domain = courseJson["domain"];
			std::string level = courseJson["level"];
			int duration = courseJson["durationHours"];

			// Convert tags array to PostgreSQL array format
			std::string tagsArray = "{";
			for (size_t i = 0; i < courseJson["tags"].size(); ++i) {
				if (i > 0) tagsArray += ",";
				tagsArray += "\"" + courseJson["tags"][i].get<std::string>() + "\"";
			}
			tagsArray += "}";

			// Convert prereqIds array
			std::string prereqArray = "{";
			for (size_t i = 0; i < courseJson["prereqIds"].size(); ++i) {
				if (i > 0) prereqArray += ",";
				prereqArray += std::to_string(courseJson["prereqIds"][i].get<int>());
			}
			prereqArray += "}";

			txn.exec(
				"INSERT INTO courses (id, title, domain, level, duration_hours, tags, prereq_ids) VALUES ($1, $2, $3, $4, $5, $6::text[], $7::integer[])",
				pqxx::params(id, title, domain, level, duration, tagsArray, prereqArray)
			);
		}

		txn.commit();
		std::cout << "Imported " << coursesJson.size() << " courses from JSON" << std::endl;
	} catch (const std::exception& e) {
		throw std::runtime_error("Import failed: " + std::string(e.what()));
	}
}

std::vector<Course> PostgresCatalog::getAll() {
	try {
		reconnect();
		pqxx::work txn(*conn);

		auto result = txn.exec("SELECT id, title, domain, level, duration_hours, tags, prereq_ids FROM courses ORDER BY id");

		std::vector<Course> courses;
		for (const auto& row : result) {
			Course course;
			course.setId(row[0].as<int>());
			course.setTitle(row[1].as<std::string>());
			course.setDomain(row[2].as<std::string>());
			course.setLevel(row[3].as<std::string>());
			course.setDurationHours(row[4].as<int>());

			// Parse tags array
			std::string tagsStr = row[5].as<std::string>();
			std::vector<std::string> tags;
			if (!tagsStr.empty() && tagsStr != "{}") {
				tagsStr = tagsStr.substr(1, tagsStr.length() - 2); // Remove {}
				size_t pos = 0;
				while ((pos = tagsStr.find(',')) != std::string::npos) {
					std::string tag = tagsStr.substr(0, pos);
					if (!tag.empty()) {
						if (tag.front() == '"') tag = tag.substr(1, tag.length() - 2);
						tags.push_back(tag);
					}
					tagsStr.erase(0, pos + 1);
				}
				if (!tagsStr.empty()) {
					if (tagsStr.front() == '"') tagsStr = tagsStr.substr(1, tagsStr.length() - 2);
					tags.push_back(tagsStr);
				}
			}
			course.setTags(tags);

			// Parse prereq array
			std::string prereqStr = row[6].as<std::string>();
			std::vector<int> prereqs;
			if (!prereqStr.empty() && prereqStr != "{}") {
				prereqStr = prereqStr.substr(1, prereqStr.length() - 2); // Remove {}
				size_t pos = 0;
				while ((pos = prereqStr.find(',')) != std::string::npos) {
					prereqs.push_back(std::stoi(prereqStr.substr(0, pos)));
					prereqStr.erase(0, pos + 1);
				}
				if (!prereqStr.empty()) {
					prereqs.push_back(std::stoi(prereqStr));
				}
			}
			course.setPrerequisiteCourseIds(prereqs);

			courses.push_back(course);
		}

		return courses;
	} catch (const std::exception& e) {
		throw std::runtime_error("Failed to get courses: " + std::string(e.what()));
	}
}
