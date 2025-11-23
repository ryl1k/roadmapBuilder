#include "../../include/catalog/json_catalog.hpp"
#include "../../third_party/json.hpp"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

std::vector<Course> JsonCatalog::getAll() {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open courses file: " + path);
    }

    json j;
    file >> j;

    std::vector<Course> courses;
    for (const auto& item : j) {
        Course course;
        course.setId(item.value("id", 0));
        course.setTitle(item.value("title", ""));
        course.setDomain(item.value("domain", ""));
        course.setLevel(item.value("level", ""));
        course.setDurationHours(item.value("durationHours", 0));
        course.setScore(item.value("score", 1.0));

        if (item.contains("tags")) {
            course.setTags(item["tags"].get<std::vector<std::string>>());
        }

        if (item.contains("prerequisiteCourseIds")) {
            course.setPrerequisiteCourseIds(item["prerequisiteCourseIds"].get<std::vector<int>>());
        }

        courses.push_back(course);
    }

    return courses;
}
