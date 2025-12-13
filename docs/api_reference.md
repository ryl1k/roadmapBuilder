# 📡 REST API Reference

Complete API documentation for the **Course Recommendation Platform** backend (C++ Crow server on port 8080) and AI service (Python Flask on port 8081).

---

## 🔗 Base URLs

| Service | URL | Purpose |
|---------|-----|---------|
| **Backend** | `http://localhost:8080/api` | Main business logic, database |
| **AI Service** | `http://localhost:8081` | Natural language processing |

**Content-Type:** `application/json` for all requests/responses

---

## 📚 Backend API (Port 8080)

### 1. Course Catalog

#### `GET /api/courses`
Returns all available courses from PostgreSQL.

**Response:**
```json
[
  {
    "id": 1,
    "title": "Python Fundamentals",
    "domain": "Data Science",
    "level": "Beginner",
    "durationHours": 20,
    "score": 0,
    "tags": ["python", "programming", "basics"],
    "prereqIds": []
  },
  {
    "id": 4,
    "title": "Pandas for Data Analysis",
    "domain": "Data Science",
    "level": "Intermediate",
    "durationHours": 30,
    "tags": ["pandas", "python", "data-analysis"],
    "prereqIds": [3]
  }
]
```

**Status Codes:**
- `200 OK` - Success
- `500 Internal Server Error` - Database connection error

---

### 2. Generate Learning Plan

#### `POST /api/recommendations`
Creates a personalized learning roadmap using the greedy algorithm.

**Request Body:**
```json
{
  "profile": {
    "userId": 1,
    "targetDomain": "Data Science",
    "currentLevel": "Beginner",
    "interests": ["python", "machine-learning"],
    "hoursPerWeek": 10,
    "deadlineWeeks": 12
  }
}
```

**Response:**
```json
{
  "steps": [
    {
      "step": 1,
      "courseId": 1,
      "hours": 20,
      "note": "Foundation course"
    },
    {
      "step": 2,
      "courseId": 3,
      "hours": 15,
      "note": "NumPy essentials"
    },
    {
      "step": 3,
      "courseId": 4,
      "hours": 30,
      "note": "Data analysis with Pandas"
    }
  ],
  "totalHours": 65
}
```

**Algorithm:**
1. Filter courses by `targetDomain`
2. Score each course (domain match 40%, level match 30%, tag overlap 30%)
3. Sort by score descending
4. Greedily select courses that:
   - Match or are below user's `currentLevel`
   - Have prerequisites already completed
   - Fit within `hoursPerWeek * deadlineWeeks` budget

**Status Codes:**
- `200 OK` - Plan generated successfully
- `400 Bad Request` - Invalid JSON or missing fields
- `500 Internal Server Error` - Algorithm error

---

### 3. User Plans

#### `GET /api/plans/<userId>`
Retrieves the saved learning plan for a user.

**Example:** `GET /api/plans/1`

**Response:**
```json
{
  "steps": [...],
  "totalHours": 65
}
```

**Status Codes:**
- `200 OK` - Plan found
- `404 Not Found` - No plan exists for this user

---

#### `POST /api/plans/<userId>`
Manually save a learning plan for a user.

**Request Body:**
```json
{
  "steps": [
    { "step": 1, "courseId": 1, "hours": 20, "note": "Start here" }
  ],
  "totalHours": 20
}
```

**Response:**
```json
{
  "status": "ok"
}
```

**Status Codes:**
- `200 OK` - Plan saved
- `400 Bad Request` - Invalid plan structure

---

#### `DELETE /api/plans/<userId>`
Deletes a user's saved plan.

**Response:**
```json
{
  "status": "deleted"
}
```

**Status Codes:**
- `200 OK` - Plan deleted
- `404 Not Found` - No plan to delete

---

### 4. Authentication

#### `POST /api/auth/register`
Register a new user account.

**Request Body:**
```json
{
  "username": "john_doe",
  "email": "john@example.com",
  "password": "securepassword123"
}
```

**Response:**
```json
{
  "success": true,
  "username": "john_doe",
  "token": "john_doe"
}
```

**Status Codes:**
- `200 OK` - User created
- `400 Bad Request` - Username/email already exists

**Security Note:** Password is hashed with `std::hash` (demo only - use bcrypt in production).

---

#### `POST /api/auth/login`
Authenticate and receive access token.

**Request Body:**
```json
{
  "username": "john_doe",
  "password": "securepassword123"
}
```

**Response:**
```json
{
  "success": true,
  "username": "john_doe",
  "token": "john_doe"
}
```

**Status Codes:**
- `200 OK` - Login successful
- `401 Unauthorized` - Invalid credentials

---

#### `GET /api/auth/me`
Get current user information (requires auth token).

**Headers:**
```
Authorization: Bearer john_doe
```

**Response:**
```json
{
  "id": 1,
  "username": "john_doe",
  "email": "john@example.com",
  "created_at": "2025-12-13T10:00:00Z"
}
```

**Status Codes:**
- `200 OK` - User info returned
- `401 Unauthorized` - Invalid or missing token

---

### 5. Health Check

#### `GET /api/health`
Check if backend server is running.

**Response:**
```json
{
  "status": "ok",
  "version": "1.0"
}
```

**Status Codes:**
- `200 OK` - Server operational

---

## 🤖 AI Service API (Port 8081)

### 1. Extract Tags from Natural Language

#### `POST /api/extract-tags`
Parse user description into structured profile using Groq LLM.

**Request Body:**
```json
{
  "description": "I want to become a data scientist. I know some Python basics but I'm new to machine learning. I can dedicate 10 hours per week and want to finish in 3 months."
}
```

**Response:**
```json
{
  "targetDomain": "Data Science",
  "currentLevel": "Beginner",
  "tags": ["python", "machine-learning", "data-analysis"],
  "hoursPerWeek": 10,
  "deadlineWeeks": 12
}
```

**LLM Configuration:**
- Model: `llama-3.3-70b-versatile`
- Temperature: `0.3` (deterministic extraction)
- Max Tokens: `500`

**Fallback Behavior:**
If Groq API fails, uses keyword matching:
```json
{
  "targetDomain": "General",
  "currentLevel": "Beginner",
  "tags": ["programming"],
  "hoursPerWeek": 10,
  "deadlineWeeks": 12,
  "fallback": true
}
```

**Status Codes:**
- `200 OK` - Tags extracted (check `fallback` field)
- `400 Bad Request` - Missing description

---

### 2. Health Check

#### `GET /health`
Verify AI service is running.

**Response:**
```json
{
  "status": "ok",
  "service": "ai-recommender",
  "version": "1.0"
}
```

---

## 📊 Complete User Journey Flow

```text
1. User visits chatbot page
   Frontend: GET http://localhost:3000/recommend

2. User types: "I want to learn web development. I know HTML basics..."
   Frontend → AI Service: POST /api/extract-tags

3. AI Service extracts structured data
   Response: { domain: "Web Development", level: "Beginner", tags: [...] }

4. Frontend sends extracted profile to backend
   Frontend → Backend: POST /api/recommendations

5. Backend generates learning plan
   - Queries PostgreSQL for courses
   - Runs greedy algorithm
   - Saves plan to database
   Response: { steps: [...], totalHours: 120 }

6. Frontend displays plan in chatbot
   User sees: Step 1: HTML5 Fundamentals (15 hours)
              Step 2: CSS3 Styling (20 hours)
              ...
```

---

## 🔐 CORS Configuration

Backend allows cross-origin requests from:
- `http://localhost:8000` (Django frontend - legacy)
- `http://localhost:3000` (FastAPI frontend - current)

**Allowed Methods:** `GET`, `POST`, `DELETE`
**Credentials:** Allowed

---

## 🛠️ Error Handling

All endpoints return consistent error format:

```json
{
  "error": "Detailed error message"
}
```

**Common Error Codes:**
- `400` - Bad Request (malformed JSON, missing fields)
- `401` - Unauthorized (auth required)
- `404` - Not Found (resource doesn't exist)
- `500` - Internal Server Error (database, algorithm failure)

---

## 📋 Database Schema Reference

### Courses Table
```sql
courses (
  id INTEGER PRIMARY KEY,
  title VARCHAR(255),
  domain VARCHAR(100),
  level VARCHAR(50),
  duration_hours INTEGER,
  tags TEXT[],
  prereq_ids INTEGER[]
)
```

**Indexes:**
- `idx_courses_domain` on `domain`
- `idx_courses_level` on `level`

---

### Users Table
```sql
users (
  id SERIAL PRIMARY KEY,
  username VARCHAR(255) UNIQUE,
  email VARCHAR(255) UNIQUE,
  password_hash VARCHAR(255),
  created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
)
```

---

### Plans & Plan Steps
```sql
plans (
  user_id INTEGER PRIMARY KEY,
  total_hours INTEGER,
  created_at TIMESTAMP,
  FOREIGN KEY (user_id) REFERENCES users(id)
)

plan_steps (
  id SERIAL PRIMARY KEY,
  user_id INTEGER,
  step INTEGER,
  course_id INTEGER,
  hours INTEGER,
  note TEXT,
  FOREIGN KEY (user_id) REFERENCES users(id)
)
```

**Indexes:**
- `idx_plan_steps_user_id` on `plan_steps(user_id)`
- `idx_plans_user_id` on `plans(user_id)`

---

## 🧪 Testing Examples

### Get All Courses
```bash
curl http://localhost:8080/api/courses | jq .
```

### Generate Recommendation
```bash
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

### Extract Tags with AI
```bash
curl -X POST http://localhost:8081/api/extract-tags \
  -H 'Content-Type: application/json' \
  -d '{
    "description": "I want to build mobile apps with React Native"
  }' | jq .
```

### Register User
```bash
curl -X POST http://localhost:8080/api/auth/register \
  -H 'Content-Type: application/json' \
  -d '{
    "username": "test_user",
    "email": "test@example.com",
    "password": "password123"
  }' | jq .
```

---

This API reference covers all endpoints for the complete learning path recommendation system with AI-powered natural language understanding.
