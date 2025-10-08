# Course Recommendation Platform — Backend (C++/Crow)

> Інтелектуальна система побудови персональних навчальних планів. Цей репозиторій містить **бекенд** на C++ з REST API (Crow) і модульною архітектурою.

---

## Зміст
- [Ідея та цілі](#ідея-та-цілі)
- [Ключові можливості](#ключові-можливості)
- [Архітектура (огляд)](#архітектура-огляд)
- [Моделі даних](#моделі-даних)
- [REST API](#rest-api)
- [Структура репозиторію](#структура-репозиторію)
- [Вимоги та збірка](#вимоги-та-збірка)
- [Конфігурація](#конфігурація)
- [Сідінг даних](#сідінг-даних)
- [Швидкий старт (curl)](#швидкий-старт-curl)
- [План розвитку](#план-розвитку)

---

## Ідея та цілі
Система допомагає користувачам:
- сформувати **профіль навчання** (галузь, рівень, інтереси, доступний час);
- отримати **персональний план** з курсів з урахуванням пререквізитів;
- **зберігати** та **переглядати** свої плани;
- експериментувати з різними підходами рекомендацій (Greedy / DP тощо).

Цілі архітектури: **модульність**, **заміна реалізацій без змін інтерфейсів**, **проста збірка**, **читабельний код**.

---

## Ключові можливості
- **C++20 + Crow**: легкий HTTP-сервер з REST-маршрутами.
- **Окремі шари**: Domain / Interfaces / Implementations / Services / REST.
- **Джерела даних**: JSON (готово), SQLite (готово як заглушка/скелет).
- **Рекомендації**: базовий Greedy (ранжування + фільтрація + пререквізити).
- **Стійкість до змін**: легко додати DP/ML-рекомендер, інше сховище.

---

## Архітектура (огляд)
- **Domain**: `Course`, `UserProfile`, `Plan`, `PlanStep` — бізнес-сутності.
- **Interfaces**: `ICatalog`, `IStorage`, `IRecommenderStrategy` — контракти.
- **Implementations**: `JsonCatalog`, `SqliteCatalog`, `JsonStorage`, `SqliteStorage`.
- **Services**: `ScoringService` (оцінка релевантності), `PrereqGraph` (перевірка залежностей), `GreedyRecommender`.
- **REST API**: `server.cpp` — маршрути `/api/...` для доступу до логіки.

Докладніше див. `docs/ARCHITECTURE.md` та `docs/STRUCTURE.md`.

---

## Моделі даних

### Course
| Поле | Тип | Опис |
|------|-----|------|
| `id` | int | Унікальний ідентифікатор |
| `title` | string | Назва курсу |
| `domain` | string | Галузь (напр. Programming, Data Science) |
| `level` | string | Рівень (Beginner/Intermediate/Advanced) |
| `durationHours` | int | Тривалість у годинах |
| `score` | double | Базова вага/рейтинг |
| `tags` | list<string> | Ключові слова |
| `prereqIds` | list<int> | ID курсів-передумов |

### UserProfile
| Поле | Тип | Опис |
|------|-----|------|
| `userId` | int | Ідентифікатор користувача |
| `targetDomain` | string | Цільова галузь |
| `currentLevel` | string | Поточний рівень |
| `interests` | list<string> | Інтереси (теги) |
| `hoursPerWeek` | int | Доступний час на тиждень |
| `deadlineWeeks` | int | Дедлайн у тижнях |

### Plan / PlanStep
| Клас | Поле | Тип | Опис |
|------|------|-----|------|
| Plan | `steps` | list<PlanStep> | Послідовність кроків |
| Plan | `totalHours` | int | Загальний час |
| Plan | `errors` | list<string> | Повідомлення валідації |
| PlanStep | `step` | int | Номер кроку |
| PlanStep | `courseId` | int | ID курсу |
| PlanStep | `hours` | int | Години на курс |
| PlanStep | `note` | string | Коментар |

---

## REST API
> Базова адреса за замовчуванням: `http://localhost:8080`

### `GET /api/courses`
Повертає всі курси з каталогу.

**200 OK**
```json
[
  {"id":1, "title":"Intro to Python", "domain":"Programming", ...},
  {"id":2, "title":"ML Basics", "domain":"Data Science", ...}
]
```

### `GET /api/courses/:id`
Повертає один курс.

**200 OK** / **404 Not Found**

### `POST /api/recommendations`
Формує персональний навчальний план.

**Body (JSON)**
```json
{
  "userId": 1,
  "targetDomain": "Data Science",
  "currentLevel": "Beginner",
  "interests": ["python", "ml"],
  "hoursPerWeek": 6,
  "deadlineWeeks": 8
}
```
**200 OK**
```json
{
  "userId": 1,
  "totalHours": 42,
  "steps": [
    { "step": 1, "courseId": 2, "hours": 8, "note": "Start with basics" },
    { "step": 2, "courseId": 4, "hours": 10, "note": "Intermediate topics" }
  ]
}
```
**400 Bad Request** — якщо профіль невалідний або бракує даних.

### `GET /api/plans/:userId`
Отримати збережений план користувача.

### `DELETE /api/plans/:userId`
Видалити збережений план користувача.

---

## Структура репозиторію
```
backend/
├── CMakeLists.txt
├── third_party/
│   ├── crow_all.h
│   └── json.hpp
├── include/
│   ├── models/
│   ├── catalog/
│   ├── storage/
│   ├── recommender/
│   └── services/
├── src/
│   ├── catalog/
│   ├── storage/
│   ├── recommender/
│   ├── services/
│   └── server.cpp
├── data/
│   └── courses.seed.json
└── docs/
    ├── Architecture_Specification.md
    └── Backend_Structure.md
```

---

## Вимоги та збірка
**Вимоги:** `g++ (C++20)`, `CMake >= 3.16`, `pthread`.

**Кроки:**
```bash
cd backend
mkdir build && cd build
cmake ..
make -j4
./backend   # старт Crow-сервера на :8080
```

---

## Конфігурація
- Порт сервера: у `server.cpp` (за замовчуванням `8080`).
- Шлях до куриків: `data/courses.seed.json`.
- Директорія для планів: у `JsonStorage` (наприклад, `data/userplans/`).

---

## Сідінг даних
`data/courses.seed.json` — список початкових курсів. Формат прикладу:
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

## Швидкий старт (curl)
```bash
# 1) Отримати всі курси
curl -s http://localhost:8080/api/courses | jq .

# 2) Запросити рекомендації
curl -s -X POST http://localhost:8080/api/recommendations \
  -H 'Content-Type: application/json' \
  -d '{
    "userId": 1,
    "targetDomain": "Data Science",
    "currentLevel": "Beginner",
    "interests": ["python", "ml"],
    "hoursPerWeek": 6,
    "deadlineWeeks": 8
  }' | jq .
```

---

## План розвитку
- Додати `DPRecommender` (Dynamic Programming) як альтернативну стратегію.
- Впровадити `SqliteCatalog`/`SqliteStorage` повністю (CRUD).
- Авторизація (JWT) та багатокористувацький режим.
- Аналітика: історія рекомендацій, A/B-тести оцінок.

---

> Будь-які імена/підписи у README відсутні навмисно. Док залишено максимально нейтральним і технічним.

