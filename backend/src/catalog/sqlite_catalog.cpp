#include "../../include/catalog/sqlite_catalog.hpp"
#include "../../third_party/json.hpp"
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <map>

using json = nlohmann::json;

SqliteCatalog::SqliteCatalog(const std::string& dbPath) : dbPath(dbPath), db(nullptr) {
	int rc = sqlite3_open(dbPath.c_str(), &db);
	if (rc != SQLITE_OK) {
		throw std::runtime_error("Cannot open database: " + std::string(sqlite3_errmsg(db)));
	}
	createTables();
}

SqliteCatalog::~SqliteCatalog() {
	if (db) {
		sqlite3_close(db);
	}
}

void SqliteCatalog::createTables() {
	const char* sql = R"(
		CREATE TABLE IF NOT EXISTS courses (
			id INTEGER PRIMARY KEY,
			title TEXT NOT NULL,
			domain TEXT NOT NULL,
			level TEXT NOT NULL,
			duration_hours INTEGER NOT NULL,
			score REAL NOT NULL
		);

		CREATE TABLE IF NOT EXISTS course_tags (
			course_id INTEGER,
			tag TEXT,
			FOREIGN KEY (course_id) REFERENCES courses(id)
		);

		CREATE TABLE IF NOT EXISTS course_prerequisites (
			course_id INTEGER,
			prerequisite_id INTEGER,
			FOREIGN KEY (course_id) REFERENCES courses(id),
			FOREIGN KEY (prerequisite_id) REFERENCES courses(id)
		);

		CREATE INDEX IF NOT EXISTS idx_course_domain ON courses(domain);
		CREATE INDEX IF NOT EXISTS idx_course_level ON courses(level);
		CREATE INDEX IF NOT EXISTS idx_course_tags_course_id ON course_tags(course_id);
		CREATE INDEX IF NOT EXISTS idx_course_prerequisites_course_id ON course_prerequisites(course_id);
	)";

	char* errMsg = nullptr;
	int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
	if (rc != SQLITE_OK) {
		std::string error = errMsg;
		sqlite3_free(errMsg);
		throw std::runtime_error("SQL error: " + error);
	}
}

std::vector<Course> SqliteCatalog::getAll() {
	std::vector<Course> courses;
	std::map<int, std::vector<std::string>> tagsMap;
	std::map<int, std::vector<int>> prereqsMap;

	// Query 1: Get all courses
	const char* sql = "SELECT id, title, domain, level, duration_hours, score FROM courses";
	sqlite3_stmt* stmt;

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
		throw std::runtime_error("Failed to prepare statement");
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		Course course;
		course.setId(sqlite3_column_int(stmt, 0));
		course.setTitle(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
		course.setDomain(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
		course.setLevel(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
		course.setDurationHours(sqlite3_column_int(stmt, 4));
		course.setScore(sqlite3_column_double(stmt, 5));
		courses.push_back(course);
	}
	sqlite3_finalize(stmt);

	// Query 2: Get all tags in one query
	const char* tagSql = "SELECT course_id, tag FROM course_tags ORDER BY course_id";
	sqlite3_stmt* tagStmt;
	if (sqlite3_prepare_v2(db, tagSql, -1, &tagStmt, nullptr) == SQLITE_OK) {
		while (sqlite3_step(tagStmt) == SQLITE_ROW) {
			int courseId = sqlite3_column_int(tagStmt, 0);
			std::string tag = reinterpret_cast<const char*>(sqlite3_column_text(tagStmt, 1));
			tagsMap[courseId].push_back(tag);
		}
		sqlite3_finalize(tagStmt);
	}

	// Query 3: Get all prerequisites in one query
	const char* prereqSql = "SELECT course_id, prerequisite_id FROM course_prerequisites ORDER BY course_id";
	sqlite3_stmt* prereqStmt;
	if (sqlite3_prepare_v2(db, prereqSql, -1, &prereqStmt, nullptr) == SQLITE_OK) {
		while (sqlite3_step(prereqStmt) == SQLITE_ROW) {
			int courseId = sqlite3_column_int(prereqStmt, 0);
			int prereqId = sqlite3_column_int(prereqStmt, 1);
			prereqsMap[courseId].push_back(prereqId);
		}
		sqlite3_finalize(prereqStmt);
	}

	// Attach tags and prerequisites to courses
	for (auto& course : courses) {
		int id = course.getId();
		if (tagsMap.find(id) != tagsMap.end()) {
			course.setTags(tagsMap[id]);
		}
		if (prereqsMap.find(id) != prereqsMap.end()) {
			course.setPrerequisiteCourseIds(prereqsMap[id]);
		}
	}

	return courses;
}

void SqliteCatalog::importFromJson(const std::string& jsonPath) {
	std::ifstream file(jsonPath);
	if (!file.is_open()) {
		throw std::runtime_error("Cannot open JSON file: " + jsonPath);
	}

	json j;
	file >> j;

	// Begin transaction
	sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);

	for (const auto& item : j) {
		// Insert course
		const char* sql = "INSERT OR REPLACE INTO courses (id, title, domain, level, duration_hours, score) VALUES (?, ?, ?, ?, ?, ?)";
		sqlite3_stmt* stmt;

		if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
			sqlite3_bind_int(stmt, 1, item["id"]);
			sqlite3_bind_text(stmt, 2, item["title"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 3, item["domain"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 4, item["level"].get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 5, item["durationHours"]);
			sqlite3_bind_double(stmt, 6, item["score"]);

			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
		}

		int courseId = item["id"];

		// Delete old tags and prerequisites
		std::string deleteTags = "DELETE FROM course_tags WHERE course_id = " + std::to_string(courseId);
		std::string deletePrereqs = "DELETE FROM course_prerequisites WHERE course_id = " + std::to_string(courseId);
		sqlite3_exec(db, deleteTags.c_str(), nullptr, nullptr, nullptr);
		sqlite3_exec(db, deletePrereqs.c_str(), nullptr, nullptr, nullptr);

		// Insert tags
		if (item.contains("tags")) {
			for (const auto& tag : item["tags"]) {
				const char* tagSql = "INSERT INTO course_tags (course_id, tag) VALUES (?, ?)";
				sqlite3_stmt* tagStmt;
				if (sqlite3_prepare_v2(db, tagSql, -1, &tagStmt, nullptr) == SQLITE_OK) {
					sqlite3_bind_int(tagStmt, 1, courseId);
					sqlite3_bind_text(tagStmt, 2, tag.get<std::string>().c_str(), -1, SQLITE_TRANSIENT);
					sqlite3_step(tagStmt);
					sqlite3_finalize(tagStmt);
				}
			}
		}

		// Insert prerequisites
		if (item.contains("prerequisiteCourseIds")) {
			for (const auto& prereqId : item["prerequisiteCourseIds"]) {
				const char* prereqSql = "INSERT INTO course_prerequisites (course_id, prerequisite_id) VALUES (?, ?)";
				sqlite3_stmt* prereqStmt;
				if (sqlite3_prepare_v2(db, prereqSql, -1, &prereqStmt, nullptr) == SQLITE_OK) {
					sqlite3_bind_int(prereqStmt, 1, courseId);
					sqlite3_bind_int(prereqStmt, 2, prereqId);
					sqlite3_step(prereqStmt);
					sqlite3_finalize(prereqStmt);
				}
			}
		}
	}

	// Commit transaction
	sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
}
