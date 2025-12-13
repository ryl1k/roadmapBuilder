#pragma once

#include "icatalog.hpp"
#include <pqxx/pqxx>
#include <memory>

class PostgresCatalog : public ICatalog {
	std::string connStr;
	std::unique_ptr<pqxx::connection> conn;

public:
	explicit PostgresCatalog(const std::string& connectionString);
	~PostgresCatalog();

	std::vector<Course> getAll() override;
	void importFromJson(const std::string& jsonPath);

private:
	void createTables();
	void reconnect();
};
