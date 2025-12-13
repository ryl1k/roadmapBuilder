#pragma once

#include "istorage.hpp"
#include <pqxx/pqxx>
#include <string>
#include <memory>

class PostgresStorage : public IStorage {
	std::string connStr;
	std::unique_ptr<pqxx::connection> conn;

public:
	explicit PostgresStorage(const std::string& connectionString);
	~PostgresStorage();

	void savePlan(int userId, const Plan& plan) override;
	std::optional<Plan> loadPlan(int userId) override;

	void saveUser(const std::string& username, const std::string& email, const std::string& password) override;
	bool validateUser(const std::string& username, const std::string& password) override;
	std::optional<json> getUser(const std::string& username) override;

private:
	void createTables();
	std::string hashPassword(const std::string& password);
	void reconnect();
};
