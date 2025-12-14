-- PostgreSQL Database Setup Script for Course Recommendation Platform
-- Run this script to create the database

-- Create database (if you're logged in as postgres user)
CREATE DATABASE roadmap;

-- Connect to the roadmap database
\c roadmap

-- The tables will be created automatically by the C++ backend
-- when you run the application for the first time.
-- The backend will create:
--   - courses (course catalog)
--   - users (user accounts)
--   - plans (learning plans)
--   - plan_steps (individual steps in learning plans)

-- Grant necessary permissions (adjust if using different user)
GRANT ALL PRIVILEGES ON DATABASE roadmap TO postgres;
