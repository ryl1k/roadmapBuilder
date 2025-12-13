#include <optional>
#include <string>
#include "../models/plan.hpp"
#include "../../third_party/json.hpp"

using json = nlohmann::json;

class IStorage {
public:
	virtual void savePlan(int userId, const Plan& plan) = 0;
	virtual std::optional<Plan> loadPlan(int userId) = 0;

	// User auth methods
	virtual void saveUser(const std::string& username, const std::string& email, const std::string& password) = 0;
	virtual bool validateUser(const std::string& username, const std::string& password) = 0;
	virtual std::optional<json> getUser(const std::string& username) = 0;

	virtual ~IStorage() = default;
};