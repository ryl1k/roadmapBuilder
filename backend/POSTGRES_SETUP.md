# PostgreSQL Setup for C++ Backend

## Prerequisites

### 1. Install PostgreSQL C++ Library (libpqxx)

**Windows (vcpkg):**
```bash
# Install vcpkg if not already installed
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install libpqxx
.\vcpkg install libpqxx:x64-windows

# Integrate with Visual Studio
.\vcpkg integrate install
```

**Linux:**
```bash
sudo apt-get install libpqxx-dev
```

**macOS:**
```bash
brew install libpqxx
```

### 2. Start PostgreSQL with Docker

```bash
# From project root
docker-compose up -d

# Check status
docker-compose ps

# View logs
docker-compose logs postgres
```

This starts PostgreSQL on `localhost:5432` with:
- Database: `roadmap`
- User: `roadmap_user`
- Password: `roadmap_pass`

### 3. Update Visual Studio Project

**Add to RoadmapBuilder-Backend.vcxproj:**

1. Open project properties
2. C/C++ → General → Additional Include Directories:
   - Add: `$(VcpkgRoot)installed\x64-windows\include`

3. Linker → General → Additional Library Directories:
   - Add: `$(VcpkgRoot)installed\x64-windows\lib`

4. Linker → Input → Additional Dependencies:
   - Add: `libpqxx.lib;pq.lib`

Or use vcpkg integration (recommended):
```bash
vcpkg integrate install
```

### 4. Build and Run

```bash
# Rebuild the project in Visual Studio
# Press Ctrl+Shift+B

# Run from backend directory
cd backend
x64/Release/RoadmapBuilder-Backend.exe
```

## Connection String

Default connection in `server.cpp`:
```cpp
host=localhost port=5432 dbname=roadmap user=roadmap_user password=roadmap_pass
```

## Database Schema

Tables automatically created on startup:
- `users` - User accounts with auth
- `plans` - User learning plans
- `plan_steps` - Individual course steps
- `courses` - Course catalog

## Troubleshooting

**Connection refused:**
```bash
# Check if PostgreSQL is running
docker-compose ps

# Restart PostgreSQL
docker-compose restart postgres
```

**Build errors:**
```bash
# Verify vcpkg installation
vcpkg list | grep pqxx

# Reinstall if needed
vcpkg remove libpqxx:x64-windows
vcpkg install libpqxx:x64-windows
vcpkg integrate install
```

## Stop PostgreSQL

```bash
docker-compose down

# To remove data volume
docker-compose down -v
```
