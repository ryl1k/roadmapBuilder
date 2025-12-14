@echo off
echo Populating database with 100 courses...
echo.

REM Find psql.exe in common PostgreSQL installation paths
set PSQL_PATH=

if exist "C:\Program Files\PostgreSQL\16\bin\psql.exe" set PSQL_PATH=C:\Program Files\PostgreSQL\16\bin\psql.exe
if exist "C:\Program Files\PostgreSQL\15\bin\psql.exe" set PSQL_PATH=C:\Program Files\PostgreSQL\15\bin\psql.exe
if exist "C:\Program Files\PostgreSQL\14\bin\psql.exe" set PSQL_PATH=C:\Program Files\PostgreSQL\14\bin\psql.exe

if "%PSQL_PATH%"=="" (
    echo ERROR: Could not find psql.exe
    echo Please install PostgreSQL from https://www.postgresql.org/download/windows/
    pause
    exit /b 1
)

echo Found PostgreSQL at: %PSQL_PATH%
echo.
echo Populating 'roadmap' database with courses...
echo.

REM Run the SQL file
"%PSQL_PATH%" -U postgres -d roadmap -f data\init_db.sql

if %errorlevel% equ 0 (
    echo.
    echo SUCCESS! Database populated with 100 courses.
    echo You can now run the C++ backend server.
    echo.
) else (
    echo.
    echo ERROR: Failed to populate database.
    echo Make sure the 'roadmap' database exists.
    echo.
)

pause
