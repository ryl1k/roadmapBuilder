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

	void saveUser(const std::string& username, const std::string& email, const std::string& password) override;
	bool validateUser(const std::string& username, const std::string& password) override;
	std::optional<json> getUser(const std::string& username) override;

private:
	void createTables();
	std::string hashPassword(const std::string& password);
};
