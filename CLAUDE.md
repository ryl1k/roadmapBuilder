# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Course Recommendation Platform is a personalized learning plan generator consisting of:
- **Backend**: C++20 REST API using Crow framework (runs on port 8080)
- **Frontend**: Django web application (runs on port 8000)
- **Architecture**: Interface-based design with dependency injection pattern

The system allows users to input their learning goals, interests, level, and available time, then generates personalized course recommendations using pluggable recommendation strategies.

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

**Important**: Start the C++ backend first before accessing the frontend.

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

**Frontend (Complete):**
- Django web application with modern, responsive UI
- Home page with profile form for generating recommendations
- Course catalog page with domain-grouped course listings
- Learning plan visualization with step-by-step course sequence
- Error handling with user-friendly messages
- Integration with backend REST API

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

**Frontend:**
- `frontend/roadmap_web/`: Django project settings
- `frontend/recommendations/`: Django app with views, URLs, templates
- `frontend/recommendations/templates/`: HTML templates (base, index, plan, catalog, error)

### Code Conventions

**Backend (C++):**
- C++20 standard used throughout
- Interface classes prefixed with 'I' (ICatalog, IStorage, IRecommenderStrategy)
- Pure virtual destructors for all interfaces
- Uses nlohmann/json for JSON serialization/deserialization
- Models use private members with getters/setters

**Frontend (Django):**
- Django 4.2+ with Python 3.10+
- Views use requests library to communicate with backend API
- Templates extend `base.html` for consistent styling
- Backend API URL configured in settings: `BACKEND_API_URL`

### REST API Endpoints

Base URL: `http://localhost:8080/api`

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/courses` | Returns all available courses |
| POST | `/api/recommendations` | Generates personalized learning plan |
| GET | `/api/plans/<userId>` | Retrieves saved plan for user |
| POST | `/api/plans/<userId>` | Saves plan for user |
| DELETE | `/api/plans/<userId>` | Deletes user's plan |
| GET | `/api/health` | Health check endpoint |

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
