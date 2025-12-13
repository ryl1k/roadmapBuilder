# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Course Recommendation Platform is a personalized learning plan generator consisting of:
- **Backend**: C++20 REST API using Crow framework (runs on port 8080)
- **AI Service**: Python Flask service using Groq LLM (runs on port 8081)
- **Frontend**: Django web application (runs on port 8000)
- **Architecture**: Microservices with interface-based design and dependency injection pattern

The system allows users to input their learning goals, interests, level, and available time, then generates personalized course recommendations using AI-powered analysis (via Groq) with greedy algorithm fallback.

## Build & Run Commands

### Backend (C++ / Crow)

**Build on Windows (MSVC):**
```bash
# Open RoadmapBuilder-Backend.vcxproj in Visual Studio and build
# Or use MSBuild from command line
```

**Build on Linux/macOS:**
```bash
cd backend
mkdir build && cd build
cmake ..
make -j4
./backend
```

**Run:**
- Server starts on port 8080 by default
- Uses `backend/data/courses.json` for course catalog
- Saves user plans to `backend/data/plans/` directory

### AI Service (Python Flask)

```bash
cd ai-service
pip install -r requirements.txt
# Create .env file with GROQ_API_KEY
python app.py
```

The AI service runs on `http://localhost:8081/`

### Frontend (Django Web Application)

```bash
cd frontend
python -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate
pip install -r requirements.txt
python manage.py migrate
python manage.py runserver
```

The web application runs on `http://localhost:8000/`

**Important**: Start both the C++ backend (port 8080) and AI service (port 8081) before accessing the frontend.

### Testing REST API

```bash
# Get all courses
curl http://localhost:8080/api/courses

# Generate recommendations
curl -X POST http://localhost:8080/api/recommendations \
  -H 'Content-Type: application/json' \
  -d '{
    "profile": {
      "userId": 1,
      "targetDomain": "Data Science",
      "currentLevel": "Beginner",
      "interests": ["python", "ml"],
      "hoursPerWeek": 6,
      "deadlineWeeks": 8
    }
  }'
```

## Architecture & Code Structure

### Core Design Pattern: Interface-Based Dependency Injection

The system is built around abstract interfaces with concrete implementations, allowing easy extension and testing:

**ICatalog** (`backend/include/catalog/icatalog.hpp`)
- Purpose: Abstracts course data sources
- Implementations: `JsonCatalog` (reads from JSON file)
- Future: `SqliteCatalog` for database storage

**IStorage** (`backend/include/storage/istorage.hpp`)
- Purpose: Abstracts plan persistence
- Implementations: `JsonStorage` (saves plans as JSON files)
- Future: `SqliteStorage` for database storage

**IRecommenderStrategy** (`backend/include/recommender/istrategy.hpp`)
- Purpose: Defines recommendation algorithms
- Implementations: `GreedyRecommender` (greedy algorithm with prerequisite handling)
- Future: `DPRecommender`, ML-based recommender

**ScoringService** (`backend/include/services/scoring.hpp`)
- Purpose: Scores courses against user profiles
- Calculates match based on tags, interests, level, and duration

### Data Models

Located in `backend/include/models/`:
- `Course`: id, title, domain, level, durationHours, score, tags, prereqIds
- `UserProfile`: userId, targetDomain, currentLevel, interests, hoursPerWeek, deadlineWeeks
- `Plan`: steps (vector of PlanStep), totalHours
- `PlanStep`: step number, courseId, hours, note

### Request Flow

1. Client sends POST to `/api/recommendations` with UserProfile JSON
2. `server.cpp` parses request body into UserProfile struct
3. Calls `catalog.getAll()` to retrieve available courses
4. Calls `recommender.makePlan(profile, courses)` to generate plan
5. Calls `storage.savePlan(userId, plan)` to persist result
6. Returns Plan as JSON response

### Implementation Status

**Backend (Complete):**
- All interface definitions and implementations
- GreedyRecommender with prerequisite-aware greedy selection
- ScoringService with domain, level, and interest matching (40/30/30 weighting)
- JsonCatalog and JsonStorage with full JSON serialization
- Complete REST API with error handling
- 20 seed courses across multiple domains (Data Science, Web Development, DevOps, Cloud, etc.)

**AI Service (Complete):**
- Flask REST API using Groq LLM (llama-3.3-70b-versatile)
- Intelligent course recommendation with context-aware analysis
- Structured JSON output with reasoning explanations
- Error handling with graceful fallback support

**Frontend (Complete):**
- Django web application with modern, responsive UI
- Dark mode toggle with persistent theme preference
- Algorithm selector (AI vs Greedy) on recommendation form
- Home page with profile form for generating recommendations
- Course catalog page with domain-grouped course listings
- Learning plan visualization with step-by-step course sequence
- AI reasoning display when using AI-powered recommendations
- Error handling with user-friendly messages
- Integration with both backend REST API and AI service

## Development Notes

### File Organization

**Backend:**
- `backend/include/`: All header files (.hpp) organized by module
  - `models/`: Course, UserProfile, Plan data structures
  - `catalog/`, `storage/`, `recommender/`, `services/`: Core modules
  - `utils/`: JSON serialization helpers
- `backend/src/`: Implementation files (.cpp) mirror include structure
- `backend/third_party/`: Contains crow_all.h and json.hpp (nlohmann/json)
- `backend/data/`: Course catalog (`courses.json`) and saved plans (`plans/`)
- `backend/docs/`: Comprehensive documentation (architecture, API reference, build instructions)

**AI Service:**
- `ai-service/app.py`: Flask application with Groq integration
- `ai-service/requirements.txt`: Python dependencies (flask, groq, flask-cors)
- `ai-service/.env.example`: Environment variable template

**Frontend:**
- `frontend/roadmap_web/`: Django project settings
- `frontend/recommendations/`: Django app with views, URLs, templates
- `frontend/recommendations/templates/`: HTML templates (base, index, plan, catalog, error)
- Features: Dark mode, AI/Greedy selector, enhanced UI with animations

### Code Conventions

**Backend (C++):**
- C++20 standard used throughout
- Interface classes prefixed with 'I' (ICatalog, IStorage, IRecommenderStrategy)
- Pure virtual destructors for all interfaces
- Uses nlohmann/json for JSON serialization/deserialization
- Models use private members with getters/setters

**AI Service (Flask):**
- Flask 3.0+ with Python 3.10+
- Groq SDK for LLM integration (llama-3.3-70b-versatile)
- CORS enabled for cross-origin requests
- Environment-based configuration (.env)

**Frontend (Django):**
- Django 4.2+ with Python 3.10+
- Views use requests library to communicate with backend API and AI service
- Templates extend `base.html` for consistent styling
- Dual service integration: `BACKEND_API_URL` (C++) and `AI_SERVICE_URL` (Python)
- AI-first with greedy fallback strategy

### REST API Endpoints

**Backend (C++) - Base URL: `http://localhost:8080/api`**

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/courses` | Returns all available courses |
| POST | `/api/recommendations` | Generates learning plan using greedy algorithm |
| GET | `/api/plans/<userId>` | Retrieves saved plan for user |
| POST | `/api/plans/<userId>` | Saves plan for user |
| DELETE | `/api/plans/<userId>` | Deletes user's plan |
| GET | `/api/health` | Health check endpoint |

**AI Service (Python) - Base URL: `http://localhost:8081`**

| Method | Endpoint | Description |
|--------|----------|-------------|
| POST | `/api/ai-recommendations` | Generates AI-powered learning plan using Groq |
| GET | `/health` | Health check endpoint |

All requests/responses use `application/json` content type.

### Django Frontend Routes

Base URL: `http://localhost:8000/`

| URL | View | Description |
|-----|------|-------------|
| `/` | index | Home page with profile form |
| `/recommend/` | get_recommendations | Generate plan (POST) |
| `/catalog/` | courses_catalog | Browse all courses |
| `/plan/<user_id>/` | view_plan | View saved user plan |

## Extension Points

The architecture is designed for easy extension:

1. **New recommendation algorithms**: Implement `IRecommenderStrategy` interface (e.g., DPRecommender for dynamic programming approach)
2. **Database support**: Implement `ICatalog` and `IStorage` with SQL backend (SQLite or PostgreSQL)
3. **Authentication**: Add JWT service layer in Django and C++ backend routes
4. **ML integration**: Create ML model microservice or integrate sklearn/TensorFlow in Django
5. **Mobile app**: Build React Native/Flutter app consuming the same REST API
6. **User dashboard**: Add progress tracking, completed courses, and learning analytics

## Dependencies

**Backend:**
- Crow (header-only C++ web framework) - included in `third_party/`
- nlohmann/json (JSON library) - included in `third_party/`
- C++20 compatible compiler (MSVC v143 / g++ / clang++)
- CMake 3.16+ (for non-Windows builds)

**AI Service:**
- Python 3.10+
- Flask 3.0+
- Groq SDK 0.4.0+
- flask-cors
- python-dotenv

**Frontend:**
- Python 3.10+
- Django 4.2+
- requests library
- django-cors-headers (if API CORS needed)

## Documentation

Comprehensive documentation available in `docs/`:
- `architecture.md` - System design and component interaction
- `backend_structure.md` - Detailed C++ code organization
- `api_reference.md` - REST endpoint specifications
- `build_instructions.md` - Setup and deployment guide
- `frontend_structure.md` - Python client design (planned)

All documentation is in Ukrainian.
