# 🧠 Build & Setup Instructions

Complete guide for building and running the **Course Recommendation Platform** with C++ backend, Python AI service, and FastAPI frontend.

---

## 🧩 1. Prerequisites

### System Requirements
| Component | Requirement |
|-----------|-------------|
| **OS** | Windows 10/11, Linux, macOS |
| **RAM** | 4GB minimum, 8GB recommended |
| **Disk** | 2GB for dependencies + database |

### Required Software

#### For Backend (C++)
- **Visual Studio 2022** (Windows) with C++ Desktop Development workload
  - OR **CMake 3.16+** + **g++/clang++** with C++20 support (Linux/macOS)
- **vcpkg** (integrated with Visual Studio or standalone)
- **Docker Desktop** (for PostgreSQL)

#### For AI Service & Frontend (Python)
- **Python 3.10+**
- **pip** package manager
- **virtualenv** (recommended)

---

## ⚙️ 2. Database Setup (PostgreSQL)

### Start PostgreSQL with Docker Compose

```powershell
# From project root
docker-compose up -d
```

This will:
1. Pull PostgreSQL 16 Alpine image
2. Create `roadmap` database
3. Auto-run `backend/data/init_db.sql` (creates tables + seeds 100 courses)
4. Expose PostgreSQL on port 5432

###  Verify Database
```powershell
docker exec roadmap_postgres psql -U postgres -d roadmap -c "SELECT COUNT(*) FROM courses;"
```
Expected output: `100`

### View Course Distribution
```powershell
docker exec roadmap_postgres psql -U postgres -d roadmap -c "SELECT domain, COUNT(*) FROM courses GROUP BY domain ORDER BY COUNT(*) DESC;"
```

### Stop PostgreSQL
```powershell
docker-compose down
```

**Note:** To disable auto-seeding after first run, comment out this line in `docker-compose.yml`:
```yaml
# - ./backend/data/init_db.sql:/docker-entrypoint-initdb.d/init_db.sql
```

---

## 🔨 3. Build Backend (C++)

### Windows (Visual Studio 2022)

1. **Open Project**
   ```powershell
   cd backend
   start RoadmapBuilder-Backend.vcxproj
   ```

2. **First Build - Install Dependencies**
   - Visual Studio will automatically install `libpqxx` via vcpkg (takes 3-5 minutes)
   - Progress shown in Output window → vcpkg tab

3. **Build Configuration**
   - Select **Release | x64** for production
   - Select **Debug | x64** for development with debugging symbols

4. **Build**
   - Press `Ctrl+Shift+B` or **Build → Build Solution**
   - Executable location: `backend/x64/Release/RoadmapBuilder-Backend.exe`

5. **Run**
   ```powershell
   cd backend
   x64/Release/RoadmapBuilder-Backend.exe
   ```

### Linux / macOS (CMake)

1. **Install Dependencies**
   ```bash
   # Ubuntu/Debian
   sudo apt install build-essential cmake libpq-dev postgresql-client

   # macOS (Homebrew)
   brew install cmake libpqxx postgresql
   ```

2. **Build**
   ```bash
   cd backend
   mkdir build && cd build
   cmake ..
   make -j4
   ```

3. **Run**
   ```bash
   ./backend
   ```

### Verify Backend Running
Open browser: `http://localhost:8080/api/health`
Expected: `{"status":"ok","version":"1.0"}`

---

## 🤖 4. Setup AI Service (Python Flask)

### Install Dependencies

```powershell
cd ai-service
python -m venv venv

# Windows
venv\Scripts\activate

# Linux/macOS
source venv/bin/activate

pip install -r requirements.txt
```

### Configure Groq API Key

Create `.env` file in `ai-service/`:
```env
GROQ_API_KEY=your_groq_api_key_here
PORT=8081
```

**Get Free API Key:** https://console.groq.com/keys

### Run AI Service

```powershell
python app.py
```

Service starts on `http://localhost:8081`

### Verify AI Service
```powershell
curl http://localhost:8081/health
```
Expected: `{"status":"ok","service":"ai-recommender","version":"1.0"}`

---

## 🌐 5. Setup Frontend (FastAPI)

### Install Dependencies

```powershell
cd webapp
python -m venv venv

# Windows
venv\Scripts\activate

# Linux/macOS
source venv/bin/activate

pip install -r requirements.txt
```

### Configure Environment

Create `.env` file in `webapp/`:
```env
BACKEND_API_URL=http://localhost:8080/api
AI_SERVICE_URL=http://localhost:8081
PORT=3000
```

### Run Frontend

```powershell
python main.py
```

### Access Application
Open browser: `http://localhost:3000`

---

## 🚀 6. Complete Startup Sequence

**Start all services in order:**

```powershell
# Terminal 1 - PostgreSQL
docker-compose up -d

# Terminal 2 - Backend (C++)
cd backend
x64/Release/RoadmapBuilder-Backend.exe

# Terminal 3 - AI Service
cd ai-service
venv\Scripts\activate
python app.py

# Terminal 4 - Frontend
cd webapp
venv\Scripts\activate
python main.py
```

**Access the application:** `http://localhost:3000`

---

## 🧪 7. Testing

### Test Course API
```bash
curl http://localhost:8080/api/courses | jq '. | length'
```
Expected: `100`

### Test AI Tag Extraction
```bash
curl -X POST http://localhost:8081/api/extract-tags \
  -H 'Content-Type: application/json' \
  -d '{"description": "I want to learn Python for data science"}' | jq .
```

### Test Full Recommendation Flow
```bash
curl -X POST http://localhost:8080/api/recommendations \
  -H 'Content-Type: application/json' \
  -d '{
    "profile": {
      "userId": 1,
      "targetDomain": "Data Science",
      "currentLevel": "Beginner",
      "interests": ["python", "ml"],
      "hoursPerWeek": 10,
      "deadlineWeeks": 12
    }
  }' | jq .
```

### Test User Registration
```bash
curl -X POST http://localhost:8080/api/auth/register \
  -H 'Content-Type: application/json' \
  -d '{
    "username": "testuser",
    "email": "test@example.com",
    "password": "password123"
  }' | jq .
```

---

## 🧰 8. Troubleshooting

### Backend Won't Start

**Error:** `relation "courses" does not exist`
**Solution:**
```powershell
docker-compose down
docker volume rm roadmapbuilder_postgres_data
docker-compose up -d
```
Wait 10 seconds for initialization.

**Error:** `libpqxx.lib not found`
**Solution:** Rebuild project in Visual Studio. vcpkg will install libpqxx automatically.

**Error:** `Cannot open database: unable to open database file`
**Solution:** You're running an old build. Rebuild the project with the new PostgreSQL code.

---

### AI Service Errors

**Error:** `GROQ_API_KEY not set`
**Solution:** Create `.env` file with your Groq API key.

**Error:** `Module 'groq' not found`
**Solution:**
```powershell
cd ai-service
pip install -r requirements.txt
```

**Error:** Groq API rate limit
**Solution:** AI service has keyword-based fallback. Check response for `"fallback": true` field.

---

### Frontend Issues

**Error:** `Connection refused to localhost:8080`
**Solution:** Start the C++ backend first.

**Error:** `Template not found`
**Solution:** Run frontend from `webapp/` directory, not project root.

**Error:** CORS error in browser console
**Solution:** Check backend CORS config allows `http://localhost:3000`.

---

### PostgreSQL Issues

**Error:** `port 5432 already in use`
**Solution:** Another PostgreSQL instance is running. Stop it or change port in `docker-compose.yml`.

**Error:** `password authentication failed`
**Solution:** Reset database:
```powershell
docker-compose down -v
docker-compose up -d
```

---

## 🐳 9. Docker Build (Optional)

### Backend Dockerfile (Windows)
```dockerfile
FROM mcr.microsoft.com/windows/servercore:ltsc2022
# Copy precompiled backend executable
COPY x64/Release/RoadmapBuilder-Backend.exe /app/
EXPOSE 8080
CMD ["\\app\\RoadmapBuilder-Backend.exe"]
```

### AI Service Dockerfile
```dockerfile
FROM python:3.11-slim
WORKDIR /app
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt
COPY . .
EXPOSE 8081
CMD ["python", "app.py"]
```

### Frontend Dockerfile
```dockerfile
FROM python:3.11-slim
WORKDIR /app
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt
COPY . .
EXPOSE 3000
CMD ["python", "main.py"]
```

---

## 🌍 10. Production Deployment

### Backend
1. Build in **Release** mode with optimizations (`/O2`)
2. Use systemd/Windows Service for process management
3. Configure reverse proxy (nginx) for HTTPS
4. Use environment variables for PostgreSQL connection string

### Database
- Use managed PostgreSQL (AWS RDS, Azure Database, Google Cloud SQL)
- Enable SSL connections
- Configure backups and point-in-time recovery
- Set up connection pooling (PgBouncer)

### AI Service
- Store `GROQ_API_KEY` in secure secret manager
- Use gunicorn with multiple workers: `gunicorn -w 4 -b 0.0.0.0:8081 app:app`
- Implement request rate limiting
- Set up monitoring for API quota usage

### Frontend
- Use uvicorn with multiple workers: `uvicorn main:app --host 0.0.0.0 --port 3000 --workers 4`
- Serve static files via nginx CDN
- Enable gzip compression
- Configure HTTPS certificates (Let's Encrypt)

---

## ✅ 11. Verification Checklist

After setup, verify all components:

- [ ] PostgreSQL running with 100 courses
- [ ] Backend responds to `/api/health`
- [ ] Backend returns courses from `/api/courses`
- [ ] AI Service responds to `/health`
- [ ] AI Service extracts tags from natural language
- [ ] Frontend loads at `http://localhost:3000`
- [ ] Chatbot interface functional
- [ ] Can generate learning plan end-to-end
- [ ] User registration works
- [ ] User login works

---

## 📚 12. Development Workflow

### Making Changes

**Backend (C++):**
1. Edit code in Visual Studio
2. Build (`Ctrl+Shift+B`)
3. Stop running backend
4. Run new executable

**AI Service/Frontend (Python):**
1. Edit code in your IDE
2. Save changes
3. Flask auto-reloads (if `debug=True`)
4. Refresh browser

### Database Schema Changes
1. Modify `backend/data/init_db.sql`
2. Reset database:
   ```powershell
   docker-compose down -v
   docker-compose up -d
   ```
3. Restart backend to recreate tables

---

This guide covers the complete setup from zero to a running application with all three services communicating properly.
