# Backend Folder Structure (Crow + JSON)

Цей документ описує архітектуру та зміст бекенд-директорії **Course Recommendation Platform**, побудованої на **C++ (Crow)**, з підтримкою REST API, JSON та модульного дизайну.

---

## 📁 Загальна структура
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
└── build_instructions.md
```

---

## ⚙️ 1. CMakeLists.txt
**Призначення:** файл збірки CMake, який визначає правила компіляції, шляхи до заголовків і вихідний виконуваний файл.

**Типовий вміст:**
```cmake
cmake_minimum_required(VERSION 3.16)
project(CourseRecommender)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)

include_directories(include third_party)

add_executable(backend
    src/server.cpp
    src/catalog/json_catalog.cpp
    src/storage/json_storage.cpp
    src/recommender/greedy.cpp
    src/services/scoring.cpp
)

target_link_libraries(backend pthread)
```

**Функції:**
- Компілює проєкт у один виконуваний файл `backend`.
- Підключає Crow і nlohmann/json (через `third_party/`).
- Підтримує C++20 для більш чистого синтаксису.

---

## 📚 2. third_party/
Тут зберігаються **зовнішні бібліотеки у вигляді single-header файлів**.

### crow_all.h
- Веб-фреймворк **Crow** (аналог Flask, але для C++).
- Забезпечує створення REST API ендпоінтів.

### json.hpp
- Бібліотека **nlohmann/json** для серіалізації та десеріалізації об’єктів у JSON.
- Використовується у `JsonCatalog`, `JsonStorage` і при відповіді API.

---

## 🧩 3. include/
Містить **усі заголовкові файли (.hpp)** з оголошеннями класів і інтерфейсів. 
Розбито по логічних модулях.

### models/
Описує доменні сутності системи.

#### course.hpp
Модель навчального курсу.
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

#### user_profile.hpp
Профіль користувача з обмеженнями часу, інтересами, рівнем знань.

#### plan.hpp
Опис навчального плану (`Plan` + `PlanStep`).
- Містить методи валідації через `PrereqGraph`.

---

### catalog/
Модуль для доступу до джерел даних курсів.

#### icatalog.hpp
Інтерфейс каталогу:
```cpp
class ICatalog {
public:
    virtual std::vector<Course> getAll() const = 0;
    virtual std::optional<Course> byId(int id) const = 0;
    virtual std::vector<Course> search(const std::string& query) const = 0;
    virtual ~ICatalog() = default;
};
```

#### json_catalog.hpp
Реалізація каталогу на базі JSON-файлу (`data/courses.seed.json`).

---

### storage/
Відповідає за збереження користувацьких навчальних планів.

#### istorage.hpp
Інтерфейс для сховища:
```cpp
class IStorage {
public:
    virtual void savePlan(int userId, const Plan& plan) = 0;
    virtual std::optional<Plan> loadPlan(int userId) = 0;
    virtual ~IStorage() = default;
};
```

#### json_storage.hpp
Реалізація збереження у JSON-файли (у `data/`).

---

### recommender/
Реалізація стратегій побудови навчального плану.

#### istrategy.hpp
Інтерфейс стратегії:
```cpp
class IRecommenderStrategy {
public:
    virtual Plan makePlan(const UserProfile&, const std::vector<Course>&) = 0;
    virtual ~IRecommenderStrategy() = default;
};
```

#### greedy.hpp
Реалізація жадібного алгоритму (Greedy) для вибору курсів.
- Використовує `ScoringService`.
- Може бути замінена на DP/ML реалізацію.

---

### services/
Допоміжні сервіси.

#### scoring.hpp
Обчислення релевантності курсів для профілю користувача.
- Методи: `relevance()`, `jaccard()`, `cosine()`, `rank()`.
- Використовується у Recommender.

---

## 🔩 4. src/
Реалізація методів, оголошених у `include/`.

| Файл | Призначення |
|-------|--------------|
| `catalog/json_catalog.cpp` | Завантажує курси з JSON |
| `storage/json_storage.cpp` | Зберігає плани користувачів |
| `recommender/greedy.cpp` | Логіка жадібного алгоритму |
| `services/scoring.cpp` | Реалізація функцій оцінки |
| `server.cpp` | Головна точка запуску Crow-сервера |

---

### server.cpp
Crow REST API сервер.
```cpp
#include "crow_all.h"
#include "json_catalog.hpp"
#include "greedy.hpp"
#include "json_storage.hpp"

int main() {
    crow::SimpleApp app;

    JsonCatalog catalog("data/courses.seed.json");
    JsonStorage storage("data/userplans/");
    GreedyRecommender recommender;

    CROW_ROUTE(app, "/api/courses").methods("GET"_method)([&]() {
        auto courses = catalog.getAll();
        nlohmann::json res = courses;
        return crow::response{res.dump()};
    });

    app.port(8080).multithreaded().run();
}
```

---

## 📂 5. data/
Містить тестові дані.

### courses.seed.json
Приклад:
```json
[
  {
    "id": 1,
    "title": "Intro to Python",
    "domain": "Programming",
    "level": "Beginner",
    "durationHours": 8,
    "score": 0.95,
    "tags": ["python", "basics"],
    "prereqIds": []
  }
]
```

---

## 🧠 6. build_instructions.md
**Документ для швидкої збірки:**
```
mkdir build && cd build
cmake ..
make -j4
./backend
```

---

### 🔗 Потік запиту (end-to-end):
1. Користувач надсилає `POST /api/recommendations` із профілем.
2. `server.cpp` → викликає `JsonCatalog.getAll()`.
3. Передає курси у `GreedyRecommender.makePlan()`.
4. `ScoringService.rank()` визначає релевантність.
5. `JsonStorage.savePlan()` записує план у `data/`.
6. Повертається JSON-відповідь.

---

**Підсумок:**  
Архітектура повністю модульна, легко тестується і розширюється.  
Можна замінити будь-який компонент (JSON → SQLite, Greedy → ML) без зміни решти системи.

