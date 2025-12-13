# ⚙️ Backend Structure (C++ / Crow / PostgreSQL)

This document describes the C++ backend codebase organization for the Course Recommendation Platform. The backend uses **C++20**, **Crow** (REST framework), and **libpqxx** (PostgreSQL client).

---

## 1. Project Structure

```
backend/
├── RoadmapBuilder-Backend.vcxproj  # Visual Studio project
├── vcpkg.json                       # Dependency manifest (libpqxx)
├── docker-compose.yml               # PostgreSQL container
├── include/
│   ├── models/
│   │   ├── course.hpp              # Course data structure
│   │   ├── user_profile.hpp        # User profile/preferences
│   │   └── plan.hpp                # Learning plan steps
│   ├── catalog/
│   │   ├── icatalog.hpp            # Course data interface
│   │   └── postgres_catalog.hpp   # PostgreSQL implementation
│   ├── storage/
│   │   ├── istorage.hpp            # Plan storage interface
│   │   └── postgres_storage.hpp   # PostgreSQL implementation
│   ├── recommender/
│   │   ├── istrategy.hpp           # Recommendation strategy interface
│   │   └── greedy.hpp              # Greedy algorithm
│   ├── services/
│   │   └── scoring.hpp             # Course scoring logic
│   └── utils/
│       └── json_helpers.hpp        # JSON serialization
├── src/
│   ├── server.cpp                  # Main entry point, Crow routes
│   ├── catalog/
│   │   └── postgres_catalog.cpp    # PostgreSQL course queries
│   ├── storage/
│   │   └── postgres_storage.cpp    # PostgreSQL plan/user management
│   ├── recommender/
│   │   └── greedy.cpp              # Greedy recommendation algorithm
│   └── services/
│       └── scoring.cpp             # Course relevance scoring
├── third_party/
│   ├── crow_all.h                  # Crow framework (header-only)
│   └── json.hpp                    # nlohmann/json
└── data/
    ├── init_db.sql                 # Database initialization + seed data
    └── plans/                      # (legacy, now in PostgreSQL)
```

---

## 2. Core Components

### 🧱 2.1 Models (`include/models/`)

Data structures used throughout the system.

#### `Course`
```cpp
class Course {
private:
    int id;
    std::string title;
    std::string domain;
    std::string level;
    int durationHours;
    double score;
    std::vector<std::string> tags;
    std::vector<int> prereqIds;
public:
    // Getters and setters
    int getId() const;
    std::string getTitle() const;
    // ...
};
```

#### `UserProfile`
```cpp
class UserProfile {
private:
    int userId;
    std::string targetDomain;
    std::string currentLevel;
    std::vector<std::string> interests;
    int hoursPerWeek;
    int deadlineWeeks;
public:
    // Getters and setters
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

class Plan {
private:
    std::vector<PlanStep> steps;
    int totalHours;
public:
    void setSteps(const std::vector<PlanStep>& newSteps);
    std::vector<PlanStep> getSteps() const;
    // ...
};
```

---

### 📦 2.2 Catalog Layer (`catalog/`)

Manages course data from PostgreSQL.

#### `ICatalog` (Interface)
```cpp
class ICatalog {
public:
    virtual std::vector<Course> getAll() = 0;
    virtual void importFromJson(const std::string& jsonPath) = 0;
    virtual ~ICatalog() = default;
};
```

#### `PostgresCatalog` (Implementation)
```cpp
class PostgresCatalog : public ICatalog {
private:
    std::string connStr;
    std::unique_ptr<pqxx::connection> conn;

    void createTables();
    void reconnect();

public:
    explicit PostgresCatalog(const std::string& connectionString);
    std::vector<Course> getAll() override;
    void importFromJson(const std::string& jsonPath) override;
};
```

**Key Methods:**
- `createTables()` - Creates `courses` table with indexes
- `getAll()` - `SELECT * FROM courses` with array parsing
- `importFromJson()` - Bulk insert from JSON (for migration)

---

### 💾 2.3 Storage Layer (`storage/`)

Manages user authentication and learning plans.

#### `IStorage` (Interface)
```cpp
class IStorage {
public:
    virtual void savePlan(int userId, const Plan& plan) = 0;
    virtual std::optional<Plan> loadPlan(int userId) = 0;
    virtual void saveUser(const std::string& username,
                         const std::string& email,
                         const std::string& password) = 0;
    virtual bool validateUser(const std::string& username,
                             const std::string& password) = 0;
    virtual std::optional<json> getUser(const std::string& username) = 0;
    virtual ~IStorage() = default;
};
```

#### `PostgresStorage` (Implementation)
```cpp
class PostgresStorage : public IStorage {
private:
    std::string connStr;
    std::unique_ptr<pqxx::connection> conn;

    void createTables();
    std::string hashPassword(const std::string& password);

public:
    explicit PostgresStorage(const std::string& connectionString);

    // Plan management
    void savePlan(int userId, const Plan& plan) override;
    std::optional<Plan> loadPlan(int userId) override;

    // User management
    void saveUser(const std::string& username,
                  const std::string& email,
                  const std::string& password) override;
    bool validateUser(const std::string& username,
                      const std::string& password) override;
    std::optional<json> getUser(const std::string& username) override;
};
```

**Database Tables:**
- `users` - Authentication (username, email, password_hash)
- `plans` - Learning plan metadata (user_id, total_hours)
- `plan_steps` - Individual course steps (step, course_id, hours, note)

**Security:**
- Uses prepared statements (`exec_params`) to prevent SQL injection
- Password hashing with `std::hash` (demo - use bcrypt in production)

---

### 🧮 2.4 Recommender Layer (`recommender/`)

Algorithms for building learning paths.

#### `IRecommenderStrategy` (Interface)
```cpp
class IRecommenderStrategy {
public:
    virtual Plan makePlan(const UserProfile& profile,
                         const std::vector<Course>& allCourses) = 0;
    virtual ~IRecommenderStrategy() = default;
};
```

#### `GreedyRecommender` (Implementation)
```cpp
class GreedyRecommender : public IRecommenderStrategy {
private:
    ScoringService scorer;

    bool canTakeCourse(const Course& course,
                       const std::set<int>& completedIds);

public:
    Plan makePlan(const UserProfile& profile,
                  const std::vector<Course>& allCourses) override;
};
```

**Algorithm:**
1. Filter courses by target domain
2. Score each course using `ScoringService`
3. Sort by score (descending)
4. Greedily select courses that:
   - Match user level
   - Have prerequisites satisfied
   - Fit within time budget
5. Return ordered plan with total hours

---

### 📊 2.5 Scoring Service (`services/`)

Calculates course relevance to user profile.

```cpp
class ScoringService {
public:
    double matchScore(const Course& course, const UserProfile& profile);
};
```

**Scoring Factors:**
- **Domain match** (40%): Course domain == target domain
- **Level match** (30%): Beginner/Intermediate/Advanced alignment
- **Interest overlap** (30%): Tag intersection with user interests

**Example:**
```cpp
Course: { domain: "Data Science", level: "Beginner", tags: ["python", "ml"] }
Profile: { domain: "Data Science", level: "Beginner", interests: ["python"] }
Score: 0.4 (domain) + 0.3 (level) + 0.15 (1/2 tags) = 0.85
```

---

### 🌐 2.6 Server Entry Point (`src/server.cpp`)

Main Crow application with REST routes.

```cpp
#include "../third_party/crow_all.h"
#include "../third_party/json.hpp"
#include "../include/catalog/postgres_catalog.hpp"
#include "../include/storage/postgres_storage.hpp"
#include "../include/recommender/greedy.hpp"

int main() {
    crow::App<crow::CORSHandler> app;

    // PostgreSQL connection
    std::string connStr = "host=localhost port=5432 dbname=roadmap user=postgres password=admin";

    PostgresCatalog catalog(connStr);
    PostgresStorage storage(connStr);
    GreedyRecommender recommender;

    // Cache courses for performance
    auto cachedCourses = catalog.getAll();

    // CORS configuration
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global()
        .origin("http://localhost:8000")
        .methods(HTTP_GET, HTTP_POST, HTTP_DELETE);

    // Routes
    CROW_ROUTE(app, "/api/courses").methods(HTTP_GET)
    ([&]() {
        json response = coursesToJson(cachedCourses);
        return crow::response(response.dump());
    });

    CROW_ROUTE(app, "/api/recommendations").methods(HTTP_POST)
    ([&](const crow::request& req) {
        auto data = json::parse(req.body);
        UserProfile profile = jsonToProfile(data["profile"]);
        auto plan = recommender.makePlan(profile, cachedCourses);
        storage.savePlan(profile.getUserId(), plan);
        return crow::response(200, planToJson(plan).dump());
    });

    // Auth routes: /api/auth/register, /api/auth/login, /api/auth/me
    // Plan CRUD: GET/POST/DELETE /api/plans/<userId>

    app.port(8080).multithreaded().run();
}
```

---

## 3. Execution Flow

### Course Recommendation Request

```text
1. Client POST /api/recommendations
   ↓
2. Parse JSON → UserProfile
   ↓
3. catalog.getAll() → std::vector<Course> (from cache)
   ↓
4. recommender.makePlan() → Plan
   ├─ Filter by domain
   ├─ Score each course
   ├─ Sort by score
   ├─ Greedy selection with prereqs
   └─ Build PlanStep sequence
   ↓
5. storage.savePlan() → PostgreSQL INSERT
   ↓
6. Return Plan as JSON
```

---

## 4. Database Connection

**Connection String:**
```cpp
"host=localhost port=5432 dbname=roadmap user=postgres password=admin"
```

**Connection Management:**
- Created in constructor: `conn = std::make_unique<pqxx::connection>(connStr)`
- Auto-reconnect on disconnect: `reconnect()` method
- RAII cleanup: Destructor closes connection

**Query Execution:**
```cpp
pqxx::work txn(*conn);
pqxx::result res = txn.exec_params(
    "SELECT * FROM courses WHERE domain = $1",
    domain
);
txn.commit();
```

---

## 5. Extension Points

### Add New Recommender
```cpp
class DPRecommender : public IRecommenderStrategy {
public:
    Plan makePlan(const UserProfile& profile,
                  const std::vector<Course>& allCourses) override {
        // Dynamic programming implementation
    }
};
```

### Add Caching Layer
```cpp
class CachedCatalog : public ICatalog {
private:
    std::unique_ptr<ICatalog> wrapped;
    std::vector<Course> cache;
public:
    std::vector<Course> getAll() override {
        if (cache.empty()) cache = wrapped->getAll();
        return cache;
    }
};
```

### Add Logging
```cpp
class LoggingStorage : public IStorage {
private:
    std::unique_ptr<IStorage> wrapped;
    Logger logger;
public:
    void savePlan(int userId, const Plan& plan) override {
        logger.info("Saving plan for user " + std::to_string(userId));
        wrapped->savePlan(userId, plan);
    }
};
```

---

## 6. Build Configuration

**vcpkg.json:**
```json
{
  "name": "roadmap-builder-backend",
  "version": "1.0.0",
  "dependencies": ["libpqxx"]
}
```

**Visual Studio Project:**
- Platform Toolset: v143 (MSVC)
- C++ Standard: C++20 (`/std:c++20`)
- vcpkg Manifest Mode: Enabled
- Preprocessor: `ASIO_STANDALONE`, `_WIN32_WINNT=0x0601`

---

## 7. Performance Considerations

- **Course Caching:** Courses loaded once at startup, cached in memory
- **Prepared Statements:** SQL injection prevention + query optimization
- **Multithreading:** Crow runs in multithreaded mode
- **Connection Pooling:** Single connection per instance (can extend to pool)

---

This backend design prioritizes clean architecture (interfaces), performance (C++), and production-readiness (PostgreSQL + prepared statements).
