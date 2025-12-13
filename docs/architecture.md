# 🧩 System Architecture Overview

This document describes the architecture of **Course Recommendation Platform**, a three-tier microservices system for generating personalized learning roadmaps.

---

## 1. High-Level Architecture

The system consists of three independent services:

```text
┌─────────────────────┐
│   Frontend (Web)    │  FastAPI (Python) - Port 3000
│   - Landing page    │  Pure UI layer, no business logic
│   - Chat interface  │
└──────────┬──────────┘
           │ HTTP
           ↓
┌─────────────────────┐
│  Backend (C++)      │  Crow REST API - Port 8080
│  - Business logic   │  All algorithms, database, orchestration
│  - PostgreSQL       │
│  - Auth (JWT)       │
└──────────┬──────────┘
           │ HTTP
           ↓
┌─────────────────────┐
│  AI Service (Flask) │  Python/Groq - Port 8081
│  - Tag extraction   │  Natural language → structured data
│  - Groq LLM API     │
└─────────────────────┘
```

**Design Principle:** Backend-only business logic, frontend-only presentation.

---

## 2. Service Responsibilities

### 🌐 Frontend (FastAPI - Python)
**Port:** 3000
**Purpose:** Pure UI rendering, no business logic

**Routes:**
- `/` - Landing page
- `/recommend` - AI chatbot interface
- `/catalog` - Course browsing
- `/static/*` - CSS, JS, images

**Technology:**
- FastAPI for routing
- Jinja2 templates
- TailwindCSS + Alpine.js
- No database access, no computations

---

### ⚙️ Backend (Crow - C++)
**Port:** 8080
**Purpose:** All business logic, algorithms, database operations

**Core Modules:**
- `PostgresCatalog` - Course data management
- `PostgresStorage` - User plans and authentication
- `GreedyRecommender` - Learning path algorithm
- `ScoringService` - Course relevance calculation

**Database:** PostgreSQL with libpqxx
- `courses` - 100 courses across 8 domains
- `users` - Authentication and profiles
- `plans` - User learning roadmaps
- `plan_steps` - Individual course assignments

**Endpoints:**
- `/api/courses` - Course catalog
- `/api/recommendations` - Generate learning plan
- `/api/plans/:userId` - CRUD for user plans
- `/api/auth/*` - Registration, login, token validation
- `/api/health` - Health check

---

### 🤖 AI Service (Flask - Python)
**Port:** 8081
**Purpose:** Natural language processing with Groq LLM

**Core Function:**
- Extract structured profile from user description
- Uses Groq API (llama-3.3-70b-versatile model)
- Temperature: 0.3 for consistent extraction

**Endpoints:**
- `/api/extract-tags` - User description → structured tags
- `/health` - Service health check

**Input:** "I want to become a data scientist. I know Python basics..."
**Output:**
```json
{
  "targetDomain": "Data Science",
  "currentLevel": "Beginner",
  "tags": ["python", "machine-learning", "pandas"],
  "hoursPerWeek": 10,
  "deadlineWeeks": 12
}
```

---

## 3. Data Flow - Chatbot Recommendation

```text
User types: "I want to learn web development. I know HTML/CSS..."

   ┌─────────────────────────────────────────┐
   │ 1. Frontend sends to AI Service         │
   │    POST /api/extract-tags               │
   └────────────┬────────────────────────────┘
                ↓
   ┌─────────────────────────────────────────┐
   │ 2. AI Service (Groq LLM)                │
   │    Extracts: domain, level, tags, time  │
   └────────────┬────────────────────────────┘
                ↓
   ┌─────────────────────────────────────────┐
   │ 3. Frontend sends to Backend            │
   │    POST /api/recommendations            │
   │    { targetDomain, currentLevel, tags } │
   └────────────┬────────────────────────────┘
                ↓
   ┌─────────────────────────────────────────┐
   │ 4. Backend C++ Greedy Algorithm         │
   │    - Score courses by relevance         │
   │    - Build prerequisite chain           │
   │    - Respect time budget                │
   └────────────┬────────────────────────────┘
                ↓
   ┌─────────────────────────────────────────┐
   │ 5. PostgreSQL saves plan                │
   │    INSERT INTO plans, plan_steps        │
   └────────────┬────────────────────────────┘
                ↓
   ┌─────────────────────────────────────────┐
   │ 6. Frontend displays in chatbot         │
   │    Step-by-step course list with badges │
   └─────────────────────────────────────────┘
```

---

## 4. Technology Stack

| Layer | Technology | Purpose |
|-------|------------|---------|
| **Frontend** | FastAPI, Jinja2, TailwindCSS | Rendering only |
| **Backend** | C++20, Crow, libpqxx | Business logic |
| **Database** | PostgreSQL 16 | Production persistence |
| **AI** | Python Flask, Groq API | NLP processing |
| **Deployment** | Docker Compose | Local development |

---

## 5. Database Schema (PostgreSQL)

### `courses`
```sql
CREATE TABLE courses (
    id INTEGER PRIMARY KEY,
    title VARCHAR(255) NOT NULL,
    domain VARCHAR(100) NOT NULL,
    level VARCHAR(50) NOT NULL,
    duration_hours INTEGER NOT NULL,
    tags TEXT[] NOT NULL,
    prereq_ids INTEGER[] NOT NULL DEFAULT '{}'
);
```

### `users`
```sql
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(255) UNIQUE NOT NULL,
    email VARCHAR(255) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### `plans` & `plan_steps`
```sql
CREATE TABLE plans (
    user_id INTEGER PRIMARY KEY,
    total_hours INTEGER NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id)
);

CREATE TABLE plan_steps (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL,
    step INTEGER NOT NULL,
    course_id INTEGER NOT NULL,
    hours INTEGER NOT NULL,
    note TEXT,
    FOREIGN KEY (user_id) REFERENCES users(id)
);
```

---

## 6. Deployment Architecture

### Development (Docker Compose)
```yaml
services:
  postgres:
    image: postgres:16-alpine
    ports: ["5432:5432"]
    volumes:
      - postgres_data:/var/lib/postgresql/data
      - ./backend/data/init_db.sql:/docker-entrypoint-initdb.d/init_db.sql

  # Backend - compiled C++ executable
  # AI Service - python ai-service/app.py
  # Frontend - python webapp/main.py
```

### Production Considerations
- **Backend:** Compile with `-O3`, run as systemd service
- **Database:** Managed PostgreSQL (AWS RDS, Azure Database)
- **AI Service:** API key in environment variables
- **Frontend:** Nginx reverse proxy, HTTPS

---

## 7. Key Design Decisions

### Why 3 Services?
- **Separation of Concerns:** UI vs Logic vs AI
- **Independent Scaling:** Each service can scale separately
- **Technology Fit:** C++ for speed, Python for AI

### Why PostgreSQL?
- ACID compliance for production
- Array types for tags/prereqs
- Better concurrency than SQLite
- Industry-standard for web apps

### Why Groq?
- Fast inference (llama-3.3-70b)
- Free tier for development
- Consistent JSON extraction
- Fallback to keyword matching if API fails

### Why FastAPI for Frontend?
- Pure Python (user requirement)
- Fast async performance
- Jinja2 templating
- No business logic temptation (separation)

---

## 8. Extension Points

1. **Authentication:** JWT tokens already implemented in C++ backend
2. **Analytics:** Add `/api/stats/:userId` for learning progress
3. **ML Recommender:** Replace GreedyRecommender with trained model
4. **Multi-language:** i18n in frontend templates
5. **Mobile App:** Consume same REST API from React Native/Flutter

---

## 9. Non-Functional Requirements

- **Performance:** C++ backend handles 1000+ req/s
- **Scalability:** Stateless services, horizontal scaling
- **Reliability:** PostgreSQL ACID, transaction safety
- **Security:** Password hashing, SQL injection prevention (prepared statements)
- **Maintainability:** Interface-based design, dependency injection

---

This architecture balances performance (C++), AI capabilities (Groq), and user experience (modern web UI) while maintaining clean separation of concerns.
