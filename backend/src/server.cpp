#include "../third_party/crow_all.h"
#include "../third_party/json.hpp"
#include "../include/catalog/json_catalog.hpp"
#include "../include/storage/json_storage.hpp"
#include "../include/recommender/greedy.hpp"

int main() {
	crow::SimpleApp app;
	JsonCatalog catalog("data/courses.json");
	JsonStorage storage("data/plans/");
	GreedyRecommender recommender(catalog);

	CROW_ROUTE(app, "api/courses").methods(crow::HTTPMethod::GET)
		([&]() {return crow::response(to_json(catalog.getAll())); });

	CROW_ROUTE(app, "api/recommendations").methods(crow::HTTPMethod::POST)
		([&](const crow::request& req) {
		auto data = nlohmann::json::parse(req.body);
		UserProfile profile = data["profile"];
		auto plan = recommender.makePlan(profile, catalog.getAll());
		storage.savePlan(profile.userId, plan);
		return crow::response(to_json(plan));
			});

	app.port(8080).multithreaded().run();
}