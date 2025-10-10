#include <optional>
#include "../models/plan.hpp"

class IStorage {
public:
	virtual void savePlan(int userId, const Plan& plan) = 0;
	virtual std::optional<Plan> loadPlan(int userId) = 0;

	virtual ~IStorage() = default;
};