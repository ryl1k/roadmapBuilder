# System Architecture: Course Recommendation Platform

## 1. Overview
Система призначена для рекомендації навчальних курсів користувачам на основі їхніх інтересів, цілей і доступного часу. Архітектура побудована з урахуванням принципів **модульності**, **розширюваності** та **чистого розділення обов’язків** між рівнями:
- **Domain** — сутності (Course, Plan, UserProfile тощо), які описують основну модель даних.
- **Interfaces** — контракти для каталогів, сховищ та стратегій рекомендацій.
- **Implementations** — реалізації інтерфейсів (Json/SQLite варіанти).
- **Services** — логіка обробки, оцінювання, побудови графів залежностей.
- **REST API (Crow)** — точка входу, що об’єднує всі рівні через HTTP.

---

## 2. Layered Architecture
| Layer | Purpose | Examples |
|--------|----------|-----------|
| **Domain** | Опис бізнес-сутностей і їхньої поведінки | `Course`, `Plan`, `PlanStep`, `UserProfile` |
| **Interfaces** | Абстрактні контракти для даних і логіки | `ICatalog`, `IStorage`, `IRecommenderStrategy` |
| **Implementations** | Конкретні реалізації інтерфейсів | `JsonCatalog`, `SqliteCatalog`, `JsonStorage`, `SqliteStorage` |
| **Services** | Додаткова логіка (оцінка, графи, алгоритми) | `ScoringService`, `PrereqGraph`, `GreedyRecommender`, `DPRecommender` |
| **REST API** | Взаємодія з клієнтом (Crow HTTP Server) | `ApiServer` |

---

## 3. Entities & Classes

### Course (Domain)
**Призначення:** Модель одного курсу у каталозі.

**Поля:**
| Name | Type | Description |
|------|------|--------------|
| id | int | Унікальний ідентифікатор курсу |
| title | string | Назва курсу |
| domain | string | Тематична галузь |
| level | string | Рівень складності |
| durationHours | int | Тривалість у годинах |
| score | double | Вага / рейтинг курсу |
| tags | list<string> | Ключові теги |
| prereqIds | list<int> | Ідентифікатори курсів-передумов |

**Методи:**
| Method | Returns | Description |
|---------|----------|-------------|
| toDict() | dict | Повертає словник для серіалізації в JSON |
| hasPrereq(id:int) | bool | Перевіряє наявність передумови |

**Зв’язки:**
- Використовується у `Plan`, `IRecommenderStrategy`, `ScoringService`.

---

### UserProfile (Domain)
**Призначення:** Представляє дані користувача для персоналізації рекомендацій.

**Поля:**
| Name | Type | Description |
|------|------|-------------|
| userId | int | Ідентифікатор користувача |
| targetDomain | string | Цільова галузь знань |
| currentLevel | string | Поточний рівень |
| interests | list<string> | Список інтересів |
| hoursPerWeek | int | Скільки годин користувач готовий витрачати на навчання |
| deadlineWeeks | int | Термін у тижнях |

**Методи:**
| Method | Returns | Description |
|---------|----------|-------------|
| totalTimeBudget() | int | Обчислює загальний доступний час |
| likes(tag:string) | bool | Перевіряє, чи тег збігається з інтересами користувача |

**Зв’язки:**
- Використовується у `IRecommenderStrategy`, `ScoringService`.

---

### Plan & PlanStep (Domain)
**Призначення:** Описують навчальний план користувача.

**Поля:**
| Class | Name | Type | Description |
|--------|------|------|-------------|
| Plan | steps | list<PlanStep> | Кроки навчання |
| Plan | totalHours | int | Загальна тривалість |
| Plan | errors | list<string> | Повідомлення про помилки |
| PlanStep | step | int | Номер кроку |
| PlanStep | courseId | int | Курс, що входить до кроку |
| PlanStep | hours | int | Години на курс |
| PlanStep | note | string | Коментар до кроку |

**Методи:**
| Method | Returns | Description |
|---------|----------|-------------|
| addStep(step:PlanStep) | void | Додає крок у план |
| validate(pg:PrereqGraph) | bool | Перевіряє коректність залежностей |
| toJson() | string | Серіалізація в JSON |

**Зв’язки:**
- Композиція: `Plan` містить `PlanStep`.
- Використовується у `IStorage` і `IRecommenderStrategy`.

---

### ICatalog / IStorage / IRecommenderStrategy (Interfaces)
**Призначення:** Контракти для основних функцій системи.

| Interface | Responsibility | Methods |
|------------|----------------|----------|
| ICatalog | Джерело курсів | `getAll()`, `byId()`, `search()`, `filter()` |
| IStorage | Збереження навчальних планів | `savePlan()`, `loadPlan()`, `listPlans()`, `deletePlan()` |
| IRecommenderStrategy | Побудова плану навчання | `makePlan(profile, courses)` |

**Зв’язки:**
- `JsonCatalog` і `SqliteCatalog` реалізують `ICatalog`.
- `JsonStorage` і `SqliteStorage` реалізують `IStorage`.
- `GreedyRecommender` і `DPRecommender` реалізують `IRecommenderStrategy`.

---

### Implementations
**JsonCatalog / SqliteCatalog:** читають курси з JSON або SQLite бази.  
**JsonStorage / SqliteStorage:** зберігають користувацькі плани в JSON або SQLite.

**Ключові методи:**
- `getAll()` — повертає всі курси.
- `savePlan(userId, plan)` — записує навчальний план користувача.
- `loadPlan(userId)` — отримує попередній план.

---

### Services: ScoringService, PrereqGraph, Recommenders
**ScoringService:** оцінює релевантність курсів користувачу за тегами, рівнем, напрямом.  
**PrereqGraph:** перевіряє логічність послідовності курсів (від простих до складних).  
**GreedyRecommender / DPRecommender:** формують навчальний план.

**Основні методи:**
| Class | Method | Description |
|--------|---------|-------------|
| ScoringService | rank() | Сортує курси за релевантністю |
| PrereqGraph | build(), validSequence() | Створює та перевіряє граф залежностей |
| Recommender | makePlan() | Генерує оптимальний набір курсів |

**Зв’язки:**
- `Recommender` використовує `ScoringService` і `PrereqGraph`.

---

### ApiServer (Crow REST)
**Призначення:** точка входу для користувача. Обробляє HTTP-запити.

**Основні маршрути:**
| Endpoint | Method | Description |
|-----------|---------|-------------|
| `/api/courses` | GET | Отримати всі курси |
| `/api/recommendations` | POST | Згенерувати навчальний план |
| `/api/plans` | GET | Переглянути збережені плани користувача |
| `/api/plans` | POST | Зберегти план |

**Зв’язки:**
- Викликає `ICatalog`, `IRecommenderStrategy`, `IStorage`.

---

## 4. Data Flow Example
1. **UI** надсилає POST `/api/recommendations` з профілем користувача.
2. **ApiServer** звертається до `ICatalog` для отримання курсів.
3. Викликає `IRecommenderStrategy.makePlan(profile, courses)`.
4. Стратегія використовує `ScoringService` для оцінки курсів.
5. Використовується `PrereqGraph` для перевірки залежностей.
6. План зберігається через `IStorage.savePlan()`.
7. Повертається JSON-відповідь із оптимальним планом.

---

## 5. Dependencies Summary
| Source | Depends On | Type |
|---------|-------------|------|
| Plan | PlanStep | composition |
| IRecommenderStrategy | Course, UserProfile | logic |
| Recommender | ScoringService, PrereqGraph | service dependency |
| ApiServer | ICatalog, IStorage, IRecommenderStrategy | control |

---

## 6. Extension Points
- Додавання нових стратегій рекомендацій (напр. ML-моделі).
- Заміна сховища (Redis, PostgreSQL, тощо).
- Розширення API додатковими маршрутами аналітики.
- Модуль авторизації / JWT.

---
**Автор:** Ruslan Shevchuk  
**Призначення:** документація для захисту курсової з ООП  
**Технології:** C++ (Crow), Python (client), SQLite, JSON, REST