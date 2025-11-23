#pragma once

#include "icatalog.hpp"
#include <sqlite3.h>
#include <string>
#include <memory>

class SqliteCatalog : public ICatalog {
	std::string dbPath;
	sqlite3* db;

public:
	explicit SqliteCatalog(const std::string& dbPath);
	~SqliteCatalog();

	std::vector<Course> getAll() override;
	void initializeDatabase();
	void importFromJson(const std::string& jsonPath);

private:
	void createTables();
};
