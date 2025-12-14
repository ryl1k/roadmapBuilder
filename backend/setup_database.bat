@echo off
echo Creating PostgreSQL database 'roadmap'...
echo.
echo Make sure PostgreSQL is installed and running!
echo Default connection: localhost:5432, user: postgres, password: admin
echo.

REM Find psql.exe in common PostgreSQL installation paths
set PSQL_PATH=

if exist "C:\Program Files\PostgreSQL\16\bin\psql.exe" set PSQL_PATH=C:\Program Files\PostgreSQL\16\bin\psql.exe
if exist "C:\Program Files\PostgreSQL\15\bin\psql.exe" set PSQL_PATH=C:\Program Files\PostgreSQL\15\bin\psql.exe
if exist "C:\Program Files\PostgreSQL\14\bin\psql.exe" set PSQL_PATH=C:\Program Files\PostgreSQL\14\bin\psql.exe

if "%PSQL_PATH%"=="" (
    echo ERROR: Could not find psql.exe
    echo Please install PostgreSQL from https://www.postgresql.org/download/windows/
    echo Or manually create database using pgAdmin
    pause
    exit /b 1
)

echo Found PostgreSQL at: %PSQL_PATH%
echo.
echo You will be prompted for the postgres user password...
echo.

REM Create the database
"%PSQL_PATH%" -U postgres -c "CREATE DATABASE roadmap;"

if %errorlevel% equ 0 (
    echo.
    echo SUCCESS! Database 'roadmap' created successfully.
    echo You can now run the C++ backend server.
    echo.
) else (
    echo.
    echo If database already exists, you can ignore the error above.
    echo.
)

pause
