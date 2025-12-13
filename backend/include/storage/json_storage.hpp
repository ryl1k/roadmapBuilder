#include "istorage.hpp"

class JsonStorage : public IStorage {
	std::string path;
public:
	explicit JsonStorage(const std::string& path) : path(path) {}
	void savePlan(int userId, const Plan& plan) override;
	std::optional<Plan> loadPlan(int userId) override;

	void saveUser(const std::string& username, const std::string& email, const std::string& password) override {
		// Not implemented for JSON storage
	}
	bool validateUser(const std::string& username, const std::string& password) override {
		return false;
	}
	std::optional<json> getUser(const std::string& username) override {
		return std::nullopt;
	}
};