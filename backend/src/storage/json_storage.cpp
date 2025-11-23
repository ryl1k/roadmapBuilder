#include "../../include/storage/json_storage.hpp"
#include "../../third_party/json.hpp"
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

void JsonStorage::savePlan(int userId, const Plan& plan) {
    // Ensure directory exists
    std::filesystem::create_directories(path);

    json j;
    j["userId"] = userId;
    j["totalHours"] = plan.getTotalHours();

    json stepsArray = json::array();
    for (const auto& step : plan.getSteps()) {
        json stepJson;
        stepJson["step"] = step.step;
        stepJson["courseId"] = step.courseId;
        stepJson["hours"] = step.hours;
        stepJson["note"] = step.note;
        stepsArray.push_back(stepJson);
    }
    j["steps"] = stepsArray;

    std::string filename = path + "/plan_" + std::to_string(userId) + ".json";
    std::ofstream file(filename);
    file << j.dump(2);
}

std::optional<Plan> JsonStorage::loadPlan(int userId) {
    std::string filename = path + "/plan_" + std::to_string(userId) + ".json";
    std::ifstream file(filename);

    if (!file.is_open()) {
        return std::nullopt;
    }

    json j;
    file >> j;

    Plan plan;
    std::vector<PlanStep> steps;

    if (j.contains("steps")) {
        for (const auto& stepJson : j["steps"]) {
            PlanStep step;
            step.step = stepJson.value("step", 0);
            step.courseId = stepJson.value("courseId", 0);
            step.hours = stepJson.value("hours", 0);
            step.note = stepJson.value("note", "");
            steps.push_back(step);
        }
    }

    plan.setSteps(steps);
    plan.setTotalHours(j.value("totalHours", 0));

    return plan;
}
