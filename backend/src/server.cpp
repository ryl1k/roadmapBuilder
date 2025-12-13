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
#include "../include/catalog/postgres_catalog.hpp"
#include "../include/storage/postgres_storage.hpp"
#include "../include/recommender/greedy.hpp"
#include "../include/utils/json_helpers.hpp"
#include <iostream>

using json = nlohmann::json;

int main() {
	try {
		crow::App<crow::CORSHandler> app;

		std::cout << "Starting Course Recommendation Platform..." << std::endl;

		// PostgreSQL connection string
		std::string connStr = "host=localhost port=5432 dbname=roadmap user=postgres password=admin";

		// Initialize PostgreSQL database
		std::cout << "Connecting to PostgreSQL..." << std::endl;
		PostgresCatalog catalog(connStr);
		PostgresStorage storage(connStr);

		// Import courses from JSON on first run
		std::cout << "Checking courses in database..." << std::endl;
		try {
			auto courses = catalog.getAll();
			if (courses.empty()) {
				std::cout << "Database empty, importing from courses.json..." << std::endl;
				catalog.importFromJson("data/courses.json");
				std::cout << "Successfully imported " << catalog.getAll().size() << " courses" << std::endl;
			} else {
				std::cout << "Found " << courses.size() << " courses in database" << std::endl;
			}
		} catch (const std::exception& e) {
			std::cerr << "Error loading courses: " << e.what() << std::endl;
			std::cout << "Attempting to import from courses.json..." << std::endl;
			catalog.importFromJson("data/courses.json");
			std::cout << "Database initialized from courses.json" << std::endl;
		}

		GreedyRecommender recommender;

	// Cache courses in memory for better performance
	std::cout << "Loading courses into cache..." << std::endl;
	auto cachedCourses = catalog.getAll();
	std::cout << "Cached " << cachedCourses.size() << " courses" << std::endl;

	// Define HTTP method constants to avoid macro conflicts
	constexpr auto HTTP_GET = crow::HTTPMethod::Get;
	constexpr auto HTTP_POST = crow::HTTPMethod::Post;
	constexpr auto HTTP_DELETE = crow::HTTPMethod::Delete;

	// Enable CORS for all routes
	auto& cors = app.get_middleware<crow::CORSHandler>();
	cors
		.global()
		.origin("http://localhost:8000")
		.methods(HTTP_GET, HTTP_POST, HTTP_DELETE)
		.allow_credentials();

	// GET all courses
	CROW_ROUTE(app, "/api/courses").methods(HTTP_GET)
		([&]() {
			json response = coursesToJson(cachedCourses);
			return crow::response(response.dump());
		});

	// POST recommendation request
	CROW_ROUTE(app, "/api/recommendations").methods(HTTP_POST)
		([&](const crow::request& req) {
			try {
				auto data = json::parse(req.body);
				UserProfile profile = jsonToProfile(data["profile"]);
				auto plan = recommender.makePlan(profile, cachedCourses);
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

	// Auth endpoints
	CROW_ROUTE(app, "/api/auth/register").methods(HTTP_POST)
		([&](const crow::request& req) {
			try {
				auto data = json::parse(req.body);
				std::string username = data["username"];
				std::string email = data["email"];
				std::string password = data["password"];

				// Simple auth - store in database
				storage.saveUser(username, email, password);

				json response = {
					{"success", true},
					{"username", username},
					{"token", username} // Simple token for demo
				};
				return crow::response(200, response.dump());
			} catch (const std::exception& e) {
				json error = {{"error", e.what()}};
				return crow::response(400, error.dump());
			}
		});

	CROW_ROUTE(app, "/api/auth/login").methods(HTTP_POST)
		([&](const crow::request& req) {
			try {
				auto data = json::parse(req.body);
				std::string username = data["username"];
				std::string password = data["password"];

				// Simple auth - validate from database
				bool valid = storage.validateUser(username, password);

				if (valid) {
					json response = {
						{"success", true},
						{"username", username},
						{"token", username}
					};
					return crow::response(200, response.dump());
				} else {
					json error = {{"error", "Invalid credentials"}};
					return crow::response(401, error.dump());
				}
			} catch (const std::exception& e) {
				json error = {{"error", e.what()}};
				return crow::response(400, error.dump());
			}
		});

	CROW_ROUTE(app, "/api/auth/me").methods(HTTP_GET)
		([&](const crow::request& req) {
			auto token = req.get_header_value("Authorization");
			if (token.empty()) {
				json error = {{"error", "No token provided"}};
				return crow::response(401, error.dump());
			}

			// Simple validation
			std::string username = token.substr(token.find(" ") + 1);
			auto user = storage.getUser(username);

			if (user.has_value()) {
				return crow::response(200, user.value().dump());
			} else {
				json error = {{"error", "Invalid token"}};
				return crow::response(401, error.dump());
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

	} catch (const std::exception& e) {
		std::cerr << "FATAL ERROR: " << e.what() << std::endl;
		std::cerr << "Press Enter to exit..." << std::endl;
		std::cin.get();
		return 1;
	} catch (...) {
		std::cerr << "FATAL ERROR: Unknown exception" << std::endl;
		std::cerr << "Press Enter to exit..." << std::endl;
		std::cin.get();
		return 2;
	}

	return 0;
}