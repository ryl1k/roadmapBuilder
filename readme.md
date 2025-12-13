# Course Recommendation Platform

> AI-powered personalized learning roadmap generator with natural language interface

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![Python](https://img.shields.io/badge/Python-3.10%2B-yellow.svg)](https://www.python.org/)
[![PostgreSQL](https://img.shields.io/badge/PostgreSQL-16-blue.svg)](https://www.postgresql.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

---

## 🎯 Overview

**Course Recommendation Platform** helps users build personalized learning paths through AI-powered analysis of natural language goals. The system uses a three-tier microservices architecture combining C++ performance, Python AI capabilities, and modern web UX.

### Key Features
- 🤖 **AI Chatbot Interface** - Describe goals in plain English
- 📚 **100+ Courses** - Across 8 domains (Data Science, Web Dev, DevOps, Cloud, Mobile, Cybersecurity, Database, AI/ML)
- 🧠 **Smart Recommendations** - Greedy algorithm with prerequisite handling
- 🔐 **User Authentication** - Register, login, save learning plans
- ⚡ **High Performance** - C++ backend handles 1000+ req/s
- 🗃️ **Production Database** - PostgreSQL with ACID compliance

---

## 📐 Architecture

```text
┌──────────────────┐
│  FastAPI (3000)  │  Pure UI - Landing page, chatbot
│  Frontend        │
└────────┬─────────┘
         │ HTTP
         ↓
┌──────────────────┐
│  Crow (8080)     │  Business logic, database, algorithms
│  C++ Backend     │
└────────┬─────────┘
         │ HTTP
         ↓
┌──────────────────┐
│  Flask (8081)    │  Groq LLM - Tag extraction from NL
│  AI Service      │
└──────────────────┘
         │
         ↓
┌──────────────────┐
│  PostgreSQL      │  100 courses, users, plans
│  (5432)          │
└──────────────────┘
```

**Design Principle:** Backend-only business logic, frontend-only presentation

---

##  Quick Start

### 1. Start PostgreSQL
```powershell
docker-compose up -d
```
Auto-creates database with 100 seeded courses.

### 2. Build & Run Backend (C++)
```powershell
cd backend
start RoadmapBuilder-Backend.vcxproj  # Visual Studio
# Build (Ctrl+Shift+B), then run
x64/Release/RoadmapBuilder-Backend.exe
```

### 3. Run AI Service
```powershell
cd ai-service
python -m venv venv && venv\Scripts\activate
pip install -r requirements.txt
# Create .env with GROQ_API_KEY=your_key
python app.py
```

### 4. Run Frontend
```powershell
cd webapp
python -m venv venv && venv\Scripts\activate
pip install -r requirements.txt
python main.py
```

### 5. Access Application
Open browser: **http://localhost:3000**

---

## 📚 Documentation

| Document | Description |
|----------|-------------|
| [Architecture](docs/architecture.md) | System design, data flow, tech stack |
| [Backend Structure](docs/backend_structure.md) | C++ code organization, PostgreSQL integration |
| [API Reference](docs/api_reference.md) | Complete REST API documentation |
| [Build Instructions](docs/build_instructions.md) | Setup, build, deploy guide |

---

## 🛠️ Tech Stack

### Backend (C++)
- **Framework:** Crow (REST API)
- **Database:** libpqxx (PostgreSQL client)
- **JSON:** nlohmann/json
- **Build:** Visual Studio 2022 + vcpkg

### AI Service (Python)
- **Framework:** Flask
- **LLM:** Groq API (llama-3.3-70b-versatile)
- **NLP:** Natural language → structured tags

### Frontend (Python)
- **Framework:** FastAPI
- **Templates:** Jinja2
- **Styling:** TailwindCSS
- **Interactivity:** Alpine.js

### Database
- **RDBMS:** PostgreSQL 16
- **Deployment:** Docker Compose
- **Schema:** Courses, Users, Plans, Plan Steps

---

## 🔑 Key Components

### Backend Modules (C++)
| Module | Purpose |
|--------|---------|
| `PostgresCatalog` | Course data management |
| `PostgresStorage` | User auth, plan persistence |
| `GreedyRecommender` | Learning path algorithm |
| `ScoringService` | Course relevance calculation |

### API Endpoints
- `GET /api/courses` - Course catalog (100 courses)
- `POST /api/recommendations` - Generate learning plan
- `POST /api/auth/register` - User registration
- `POST /api/auth/login` - Authentication
- `POST /api/extract-tags` - AI tag extraction (via AI service)

### Database Schema
```sql
courses (id, title, domain, level, duration_hours, tags[], prereq_ids[])
users (id, username, email, password_hash, created_at)
plans (user_id, total_hours, created_at)
plan_steps (id, user_id, step, course_id, hours, note)
```

---

## 🎨 User Experience

### 1. Natural Language Input
```
User types: "I want to become a data scientist. I know Python basics but I'm
new to machine learning. I can dedicate 10 hours per week and want to finish
in 3 months."
```

### 2. AI Extraction
```json
{
  "targetDomain": "Data Science",
  "currentLevel": "Beginner",
  "tags": ["python", "machine-learning", "data-analysis"],
  "hoursPerWeek": 10,
  "deadlineWeeks": 12
}
```

### 3. Greedy Algorithm
1. Filter courses by domain (Data Science)
2. Score each course (domain 40%, level 30%, tags 30%)
3. Sort by relevance
4. Select courses with satisfied prerequisites
5. Fit within time budget (120 hours)

### 4. Personalized Roadmap
```
Step 1: Python Fundamentals (20h) - Foundation course
Step 2: NumPy Essentials (15h) - Array manipulation
Step 3: Pandas for Data Analysis (30h) - Data wrangling
Step 4: Machine Learning Basics (40h) - Core algorithms
...
Total: 105 hours
```

---

## 🧪 Testing

```bash
# Health check
curl http://localhost:8080/api/health

# Get all courses
curl http://localhost:8080/api/courses | jq '. | length'

# AI tag extraction
curl -X POST http://localhost:8081/api/extract-tags \
  -H 'Content-Type: application/json' \
  -d '{"description": "I want to learn React and build web apps"}' | jq .

# Generate recommendation
curl -X POST http://localhost:8080/api/recommendations \
  -H 'Content-Type: application/json' \
  -d '{
    "profile": {
      "userId": 1,
      "targetDomain": "Web Development",
      "currentLevel": "Beginner",
      "interests": ["javascript", "react"],
      "hoursPerWeek": 15,
      "deadlineWeeks": 8
    }
  }' | jq .
```

---

## 📊 Course Catalog

**100 courses across 8 domains:**

- **Data Science** (20): Python, ML, Deep Learning, NLP, Computer Vision
- **Web Development** (20): React, Vue, Angular, Node.js, Next.js
- **DevOps** (15): Docker, Kubernetes, CI/CD, Terraform, ArgoCD
- **Cloud** (10): AWS, Azure, GCP, Serverless
- **Mobile** (10): Android, iOS, React Native, Flutter
- **Cybersecurity** (10): Ethical Hacking, Security, Compliance
- **Database** (7): PostgreSQL, SQL, NoSQL, Redis
- **AI & ML** (8): Generative AI, LLMs, RAG, MLOps

---

## 🚀 Roadmap

- [ ] Dynamic Programming recommender (optimal path)
- [ ] ML-based recommendation (collaborative filtering)
- [ ] Progress tracking and analytics
- [ ] Mobile app (React Native/Flutter)
- [ ] Multi-language support
- [ ] Course content integration (videos, exercises)
- [ ] Social features (share plans, leaderboards)

---

## 🤝 Contributing

This is an educational project demonstrating:
- Clean architecture with C++20
- Microservices design patterns
- AI integration with LLMs
- Production-grade database design
- Modern web UX with chatbot interface

Contributions welcome! Please read the documentation before submitting PRs.

---

## 📄 License

MIT License - See LICENSE file for details

---

## 🙏 Acknowledgments

- **Crow** - Lightweight C++ web framework
- **Groq** - Fast LLM inference
- **libpqxx** - PostgreSQL C++ client
- **TailwindCSS** - Utility-first CSS framework

---

<p align="center">
  <strong>Course Recommendation Platform</strong><br>
  AI-powered learning path generator | C++ Backend | Python AI Service | FastAPI Frontend
</p>
