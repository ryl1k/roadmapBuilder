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
		.origin("http://localhost:3000")
		.methods(HTTP_GET, HTTP_POST, HTTP_DELETE, crow::HTTPMethod::Options)
		.headers("Content-Type", "Authorization")
		.allow_credentials();

	std::cout << "CORS enabled for: http://localhost:3000" << std::endl;

	// GET all courses
	CROW_ROUTE(app, "/api/courses").methods(HTTP_GET)
		([&]() {
			std::cout << "\n[REQUEST] GET /api/courses" << std::endl;
			json response = coursesToJson(cachedCourses);
			std::string responseStr = response.dump();
			std::cout << "[RESPONSE] 200 OK - " << cachedCourses.size() << " courses, "
			          << responseStr.length() << " bytes" << std::endl;
			std::cout << "[SAMPLE] " << responseStr.substr(0, 200) << "..." << std::endl;

			crow::response res(200, responseStr);
			res.set_header("Content-Type", "application/json");
			res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
			res.set_header("Access-Control-Allow-Credentials", "true");
			return res;
		});

	// GET all unique tags from courses
	CROW_ROUTE(app, "/api/tags").methods(HTTP_GET)
		([&]() {
			std::cout << "\n[REQUEST] GET /api/tags" << std::endl;
			std::set<std::string> uniqueTags;
			for (const auto& course : cachedCourses) {
				const auto& tags = course.getTags();
				uniqueTags.insert(tags.begin(), tags.end());
			}
			json tagsArray = json::array();
			for (const auto& tag : uniqueTags) {
				tagsArray.push_back(tag);
			}
			std::string responseStr = tagsArray.dump();
			std::cout << "[RESPONSE] 200 OK - " << uniqueTags.size() << " unique tags" << std::endl;
			std::cout << "[SAMPLE] " << responseStr.substr(0, 200) << "..." << std::endl;

			crow::response res(200, responseStr);
			res.set_header("Content-Type", "application/json");
			res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
			res.set_header("Access-Control-Allow-Credentials", "true");
			return res;
		});

	// POST recommendation request
	CROW_ROUTE(app, "/api/recommendations").methods(HTTP_POST)
		([&](const crow::request& req) {
			std::cout << "\n[REQUEST] POST /api/recommendations" << std::endl;
			std::cout << "[BODY] " << req.body.substr(0, 500) << std::endl;
			try {
				auto data = json::parse(req.body);
				UserProfile profile = jsonToProfile(data["profile"]);
				std::cout << "[PROFILE] User " << profile.getUserId()
				          << ", Domain: " << profile.getTargetDomain()
				          << ", Level: " << profile.getCurrentLevel() << std::endl;
				auto plan = recommender.makePlan(profile, cachedCourses);
				std::cout << "[PLAN] Generated " << plan.getSteps().size()
				          << " steps, " << plan.getTotalHours() << " hours" << std::endl;
				storage.savePlan(profile.getUserId(), plan);

				// Enrich plan with full course details
				json enrichedPlan;
				enrichedPlan["totalHours"] = plan.getTotalHours();
				json stepsArray = json::array();

				for (const auto& step : plan.getSteps()) {
					json stepJson;
					stepJson["step"] = step.step;
					stepJson["courseId"] = step.courseId;
					stepJson["hours"] = step.hours;
					stepJson["note"] = step.note;

					// Find and add full course details
					for (const auto& course : cachedCourses) {
						if (course.getId() == step.courseId) {
							stepJson["courseTitle"] = course.getTitle();
							stepJson["courseDomain"] = course.getDomain();
							stepJson["courseLevel"] = course.getLevel();
							stepJson["courseTags"] = course.getTags();
							break;
						}
					}
					stepsArray.push_back(stepJson);
				}
				enrichedPlan["steps"] = stepsArray;

				std::string responseStr = enrichedPlan.dump();
				std::cout << "[RESPONSE] 200 OK - " << responseStr.length() << " bytes (enriched)" << std::endl;
				crow::response res(200, responseStr);
				res.set_header("Content-Type", "application/json");
				res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
				res.set_header("Access-Control-Allow-Credentials", "true");
				return res;
			} catch (const std::exception& e) {
				std::cerr << "[ERROR] " << e.what() << std::endl;
				json error = {{"error", e.what()}};
				crow::response res(400, error.dump());
				res.set_header("Content-Type", "application/json");
				res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
				return res;
			}
		});

	// GET plan by userId
	CROW_ROUTE(app, "/api/plans/<int>").methods(HTTP_GET)
		([&](int userId) {
			std::cout << "\n[REQUEST] GET /api/plans/" << userId << std::endl;
			auto plan = storage.loadPlan(userId);
			if (plan.has_value()) {
				std::cout << "[PLAN] Found plan with " << plan.value().getSteps().size()
				          << " steps, " << plan.value().getTotalHours() << " hours" << std::endl;

				// Enrich plan with full course details (same as POST /recommendations)
				json enrichedPlan;
				enrichedPlan["totalHours"] = plan.value().getTotalHours();
				json stepsArray = json::array();

				for (const auto& step : plan.value().getSteps()) {
					json stepJson;
					stepJson["step"] = step.step;
					stepJson["courseId"] = step.courseId;
					stepJson["hours"] = step.hours;
					stepJson["note"] = step.note;

					// Find and add full course details
					for (const auto& course : cachedCourses) {
						if (course.getId() == step.courseId) {
							stepJson["courseTitle"] = course.getTitle();
							stepJson["courseDomain"] = course.getDomain();
							stepJson["courseLevel"] = course.getLevel();
							stepJson["courseTags"] = course.getTags();
							break;
						}
					}
					stepsArray.push_back(stepJson);
				}
				enrichedPlan["steps"] = stepsArray;

				std::string responseStr = enrichedPlan.dump();
				std::cout << "[RESPONSE] 200 OK - " << responseStr.length() << " bytes (enriched)" << std::endl;

				crow::response res(200, responseStr);
				res.set_header("Content-Type", "application/json");
				res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
				res.set_header("Access-Control-Allow-Credentials", "true");
				return res;
			} else {
				std::cout << "[RESPONSE] 404 Not Found - No plan for user " << userId << std::endl;
				json error = {{"error", "Plan not found"}};
				crow::response res(404, error.dump());
				res.set_header("Content-Type", "application/json");
				res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
				return res;
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
			std::cout << "\n[REQUEST] POST /api/auth/register" << std::endl;
			try {
				auto data = json::parse(req.body);
				std::string username = data["username"];
				std::string email = data["email"];
				std::string password = data["password"];
				std::cout << "[AUTH] Registering user: " << username << " (" << email << ")" << std::endl;

				// Simple auth - store in database
				storage.saveUser(username, email, password);

				json response = {
					{"success", true},
					{"username", username},
					{"token", username} // Simple token for demo
				};
				std::string responseStr = response.dump();
				std::cout << "[RESPONSE] 200 OK - User registered" << std::endl;
				crow::response res(200, responseStr);
				res.set_header("Content-Type", "application/json");
				res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
				res.set_header("Access-Control-Allow-Credentials", "true");
				return res;
			} catch (const std::exception& e) {
				std::cerr << "[ERROR] Registration failed: " << e.what() << std::endl;
				json error = {{"error", e.what()}};
				crow::response res(400, error.dump());
				res.set_header("Content-Type", "application/json");
				res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
				return res;
			}
		});

	CROW_ROUTE(app, "/api/auth/login").methods(HTTP_POST)
		([&](const crow::request& req) {
			std::cout << "\n[REQUEST] POST /api/auth/login" << std::endl;
			try {
				auto data = json::parse(req.body);
				std::string username = data["username"];
				std::string password = data["password"];
				std::cout << "[AUTH] Login attempt for user: " << username << std::endl;

				// Simple auth - validate from database
				bool valid = storage.validateUser(username, password);

				if (valid) {
					json response = {
						{"success", true},
						{"username", username},
						{"token", username}
					};
					std::string responseStr = response.dump();
					std::cout << "[RESPONSE] 200 OK - Login successful" << std::endl;
					crow::response res(200, responseStr);
					res.set_header("Content-Type", "application/json");
					res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
					res.set_header("Access-Control-Allow-Credentials", "true");
					return res;
				} else {
					std::cout << "[RESPONSE] 401 Unauthorized - Invalid credentials" << std::endl;
					json error = {{"error", "Invalid credentials"}};
					crow::response res(401, error.dump());
					res.set_header("Content-Type", "application/json");
					res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
					return res;
				}
			} catch (const std::exception& e) {
				std::cerr << "[ERROR] Login failed: " << e.what() << std::endl;
				json error = {{"error", e.what()}};
				crow::response res(400, error.dump());
				res.set_header("Content-Type", "application/json");
				res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
				return res;
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
			std::cout << "\n[REQUEST] GET /api/health" << std::endl;
			json response = {{"status", "ok"}, {"version", "1.0"}};
			std::cout << "[RESPONSE] 200 OK - Health check passed" << std::endl;
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