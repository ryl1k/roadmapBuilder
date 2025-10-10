#include "istorage.hpp"

class JsonStorage : public IStorage {
	std::string path;
public:
	explicit JsonStorage(const std::string& path) : path(path) {}
	void savePlan(int userId, const Plan& plan) override;
	std::optional<Plan> loadPlan(int userId) override;
}