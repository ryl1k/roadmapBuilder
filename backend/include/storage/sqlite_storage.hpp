#pragma once

#include "istorage.hpp"
#include <sqlite3.h>
#include <string>

class SqliteStorage : public IStorage {
	std::string dbPath;
	sqlite3* db;

public:
	explicit SqliteStorage(const std::string& dbPath);
	~SqliteStorage();

	void savePlan(int userId, const Plan& plan) override;
	std::optional<Plan> loadPlan(int userId) override;

private:
	void createTables();
};
