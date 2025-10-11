# Course Recommendation Platform

> Intelligent system for generating personalized learning plans. Includes a **C++ Crow REST backend** and a **Python PyQt frontend**.

---

<p align="center">
  <a href="docs/architecture.md">🧩 Architecture</a> •
  <a href="docs/backend_structure.md">⚙️ Backend</a> •
  <a href="docs/frontend_structure.md">💻 Frontend</a> •
  <a href="docs/api_reference.md">📡 API</a> •
  <a href="docs/build_instructions.md">🧠 Build</a>
</p>

---

## 🎯 Project Overview
**Course Recommendation Platform** — це система, яка допомагає користувачам формувати персоналізовані навчальні плани на основі їх інтересів, часу, рівня та цілей.

Складається з двох основних компонентів:
- **Backend:** високопродуктивний сервер на **C++ (Crow)**.
- **Frontend:** десктопний клієнт на **Python (PyQt)**.

---

## 🧩 Architecture Overview
Система використовує чіткий поділ за рівнями:
```
Frontend (PyQt GUI)
  ↓
Crow REST API (C++)
  ↓
Recommendation Engine (C++)
  ↓
JSON / SQLite Storage
```

- Взаємодія відбувається через REST запити (`/api/...`).
- Дані зберігаються у форматі JSON або в майбутньому SQLite.
- Рекомендаційна логіка — модульна (Greedy / DP / ML).

Докладніше: [Architecture.md](docs/Architecture.md)

---

## ⚙️ Backend Summary
Бекенд реалізовано на **C++20** із фреймворком Crow.
- Шари: `catalog`, `storage`, `recommender`, `services`.
- Дані: `data/courses.seed.json`, `data/plans/`.
- API: `/api/courses`, `/api/recommendations`, `/api/plans/...`.

Докладніше: [Backend_Structure.md](docs/Backend_Structure.md)

---

## 💻 Frontend Summary
Фронтенд — десктопна аплікація на **Python (PyQt)**.
- Інтерфейс із вкладками: `Profile`, `Courses`, `Plan`.
- Підключення до API через `requests`.
- Відображення плану у вигляді таблиці.

Докладніше: [Frontend_Structure.md](docs/Frontend_Structure.md)

---

## 📡 REST API
Основні маршрути:
| Метод | Endpoint | Опис |
|--------|-----------|------|
| `GET` | `/api/courses` | Каталог курсів |
| `POST` | `/api/recommendations` | Створення плану |
| `GET` | `/api/plans/:userId` | Завантаження плану |
| `POST` | `/api/plans/:userId` | Збереження плану |
| `DELETE` | `/api/plans/:userId` | Видалення плану |

Докладніше: [API_Reference.md](docs/API_Reference.md)

---

## 🧠 Build Instructions
**Backend:**
```bash
cd backend
mkdir build && cd build
cmake ..
make -j4
./backend
```

**Frontend:**
```bash
cd frontend
python -m venv venv
source venv/bin/activate  # або venv\\Scripts\\activate
pip install -r requirements.txt
python main.py
```

Повний гайд: [Build_Instructions.md](docs/Build_Instructions.md)

---

## 🚀 Roadmap
- [ ] Додати `DPRecommender`.
- [ ] SQLite-підтримка.
- [ ] JWT авторизація.
- [ ] Інтеграція AI-пояснень.
- [ ] Темна/світла тема в PyQt.

---

<p align="center"><b>Course Recommendation Platform © 2025</b><br>
<small>Developed as a modular, educational-grade AI-backed recommender system with C++ Crow backend and Python PyQt frontend. Supports REST API, JSON/SQLite persistence, and extendable ML recommendation logic.</small></p>

