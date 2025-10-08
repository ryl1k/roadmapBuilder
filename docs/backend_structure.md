# ⚙️ Backend Structure (C++ / Crow)

Документ описує організацію коду бекенду системи Course Recommendation Platform. Бекенд реалізовано на **C++20** з використанням **Crow** як легкого REST-фреймворку та **nlohmann/json** для роботи з JSON.

---

## 1. Основна структура проекту
```
backend/
├── CMakeLists.txt
├── third_party/
│   ├── crow_all.h
│   └── json.hpp
├── include/
│   ├── models/
│   │   ├── course.hpp
│   │   ├── user_profile.hpp
│   │   └── plan.hpp
│   ├── catalog/
│   │   ├── icatalog.hpp
│   │   └── json_catalog.hpp
│   ├── storage/
│   │   ├── istorage.hpp
│   │   └── json_storage.hpp
│   ├── recommender/
│   │   ├── istrategy.hpp
│   │   └── greedy.hpp
│   └── services/
│       └── scoring.hpp
├── src/
│   ├── catalog/json_catalog.cpp
│   ├── storage/json_storage.cpp
│   ├── recommender/greedy.cpp
│   ├── services/scoring.cpp
│   └── server.cpp
├── data/
│   └── courses.seed.json
└── docs/
    └── Backend_Structure.md
```

---

## 2. Ключові компоненти

### 🧱 2.1 Models (`include/models/`)
Описують структуру даних, які використовуються системою.

#### `Course`
```cpp
struct Course {
    int id;
    std::string title;
    std::string domain;
    std::string level;
    int durationHours;
    double score;
    std::vector<std::string> tags;
    std::vector<int> prereqIds;
};
```

#### `UserProfile`
```cpp
struct UserProfile {
    int userId;
    std::string targetDomain;
    std::string currentLevel;
    std::vector<std::string> interests;
    int hoursPerWeek;
    int deadlineWeeks;
};
```

#### `Plan` / `PlanStep`
```cpp
struct PlanStep {
    int step;
    int courseId;
    int hours;
    std::string note;
};

struct Plan {
    std::vector<PlanStep> steps;
    int totalHours;
};
```

---

### 📦 2.2 Catalog Layer
Інтерфейс доступу до джерел даних курсів.

#### `ICatalog`
```cpp
class ICatalog {
public:
    virtual std::vector<Course> getAll() = 0;
    virtual ~ICatalog() = default;
};
```

#### `JsonCatalog`
```cpp
class JsonCatalog : public ICatalog {
private:
    std::string path;
public:
    explicit JsonCatalog(const std::string& jsonPath);
    std::vector<Course> getAll() override;
};
```

---

### 💾 2.3 Storage Layer
Робота з навчальними планами користувачів.

#### `IStorage`
```cpp
class IStorage {
public:
    virtual void savePlan(int userId, const Plan& plan) = 0;
    virtual std::optional<Plan> loadPlan(int userId) = 0;
    virtual ~IStorage() = default;
};
```

#### `JsonStorage`
```cpp
class JsonStorage : public IStorage {
private:
    std::string path;
public:
    explicit JsonStorage(const std::string& folderPath);
    void savePlan(int userId, const Plan& plan) override;
    std::optional<Plan> loadPlan(int userId) override;
};
```

---

### 🧮 2.4 Recommender Layer
Містить алгоритми побудови планів навчання.

#### `IRecommenderStrategy`
```cpp
class IRecommenderStrategy {
public:
    virtual Plan makePlan(const UserProfile& profile, const std::vector<Course>& allCourses) = 0;
    virtual ~IRecommenderStrategy() = default;
};
```

#### `GreedyRecommender`
```cpp
class GreedyRecommender : public IRecommenderStrategy {
private:
    ScoringService scorer;
public:
    Plan makePlan(const UserProfile& profile, const std::vector<Course>& allCourses) override;
};
```

---

### 📊 2.5 Scoring Service
```cpp
class ScoringService {
public:
    double matchScore(const Course& course, const UserProfile& profile);
};
```
Метод `matchScore` оцінює релевантність курсу користувачу (за тегами, рівнем, годинами).

---

### 🌐 2.6 Server Entry Point (`src/server.cpp`)
```cpp
#include "crow_all.h"
#include "json.hpp"
#include "catalog/json_catalog.hpp"
#include "storage/json_storage.hpp"
#include "recommender/greedy.hpp"

int main() {
    crow::SimpleApp app;
    JsonCatalog catalog("data/courses.seed.json");
    JsonStorage storage("data/plans");
    GreedyRecommender recommender;

    CROW_ROUTE(app, "/api/courses").methods(crow::HTTPMethod::GET)
    ([&](){ return crow::response(to_json(catalog.getAll())); });

    CROW_ROUTE(app, "/api/recommendations").methods(crow::HTTPMethod::POST)
    ([&](const crow::request& req){
        auto data = nlohmann::json::parse(req.body);
        UserProfile profile = parseProfile(data);
        auto plan = recommender.makePlan(profile, catalog.getAll());
        storage.savePlan(profile.userId, plan);
        return crow::response(to_json(plan));
    });

    app.port(8080).multithreaded().run();
}
```

---

## 3. Потік виконання
1. Сервер Crow запускає REST маршрути.
2. Клієнт (PyQt) надсилає JSON-запит.
3. `JsonCatalog` завантажує курси.
4. `GreedyRecommender` створює навчальний план.
5. `JsonStorage` зберігає результат у файл.

---

## 4. Розширення
- Додати `SqliteCatalog` / `SqliteStorage` для БД.
- Реалізувати `DPRecommender` для складних планів.
- Додати `AuthService` для токенів JWT.
- Інтегрувати `Logger` для журналювання дій.