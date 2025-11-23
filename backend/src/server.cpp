// Define before any Windows headers to prevent macro pollution
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "../third_party/crow_all.h"

// Undefine Windows HTTP macros that conflict with Crow
#if defined(GET)
#undef GET
#endif
#if defined(POST)
#undef POST
#endif
#if defined(PUT)
#undef PUT
#endif
#if defined(DELETE)
#undef DELETE
#endif
#if defined(PATCH)
#undef PATCH
#endif

#include "../third_party/json.hpp"
#include "../include/catalog/json_catalog.hpp"
#include "../include/storage/json_storage.hpp"
#include "../include/recommender/greedy.hpp"
#include "../include/utils/json_helpers.hpp"
#include <iostream>

using json = nlohmann::json;

int main() {
	crow::SimpleApp app;
	JsonCatalog catalog("data/courses.json");
	JsonStorage storage("data/plans/");
	GreedyRecommender recommender;

	// Define HTTP method constants to avoid macro conflicts
	constexpr auto HTTP_GET = crow::HTTPMethod::Get;
	constexpr auto HTTP_POST = crow::HTTPMethod::Post;
	constexpr auto HTTP_DELETE = crow::HTTPMethod::Delete;

	// GET all courses
	CROW_ROUTE(app, "/api/courses").methods(HTTP_GET)
		([&]() {
			auto courses = catalog.getAll();
			json response = coursesToJson(courses);
			return crow::response(response.dump());
		});

	// POST recommendation request
	CROW_ROUTE(app, "/api/recommendations").methods(HTTP_POST)
		([&](const crow::request& req) {
			try {
				auto data = json::parse(req.body);
				UserProfile profile = jsonToProfile(data["profile"]);
				auto plan = recommender.makePlan(profile, catalog.getAll());
				storage.savePlan(profile.getUserId(), plan);
				json response = planToJson(plan);
				return crow::response(200, response.dump());
			} catch (const std::exception& e) {
				json error = {{"error", e.what()}};
				return crow::response(400, error.dump());
			}
		});

	// GET plan by userId
	CROW_ROUTE(app, "/api/plans/<int>").methods(HTTP_GET)
		([&](int userId) {
			auto plan = storage.loadPlan(userId);
			if (plan.has_value()) {
				json response = planToJson(plan.value());
				return crow::response(200, response.dump());
			} else {
				json error = {{"error", "Plan not found"}};
				return crow::response(404, error.dump());
			}
		});

	// POST save plan
	CROW_ROUTE(app, "/api/plans/<int>").methods(HTTP_POST)
		([&](const crow::request& req, int userId) {
			try {
				auto data = json::parse(req.body);
				Plan plan;
				std::vector<PlanStep> steps;

				for (const auto& stepJson : data["steps"]) {
					PlanStep step;
					step.step = stepJson["step"];
					step.courseId = stepJson["courseId"];
					step.hours = stepJson["hours"];
					step.note = stepJson["note"];
					steps.push_back(step);
				}

				plan.setSteps(steps);
				plan.setTotalHours(data["totalHours"]);

				storage.savePlan(userId, plan);
				json response = {{"status", "ok"}};
				return crow::response(200, response.dump());
			} catch (const std::exception& e) {
				json error = {{"error", e.what()}};
				return crow::response(400, error.dump());
			}
		});

	// DELETE plan
	CROW_ROUTE(app, "/api/plans/<int>").methods(HTTP_DELETE)
		([&](int userId) {
			std::string filename = "data/plans/plan_" + std::to_string(userId) + ".json";
			if (std::remove(filename.c_str()) == 0) {
				json response = {{"status", "deleted"}};
				return crow::response(200, response.dump());
			} else {
				json error = {{"error", "Plan not found"}};
				return crow::response(404, error.dump());
			}
		});

	// Health check
	CROW_ROUTE(app, "/api/health").methods(HTTP_GET)
		([]() {
			json response = {{"status", "ok"}, {"version", "1.0"}};
			return crow::response(200, response.dump());
		});

	std::cout << "Server starting on port 8080..." << std::endl;
	app.port(8080).multithreaded().run();
}