# FastAPI Web Application

Modern, fast Python frontend for the Course Recommendation Platform.

## Features

- **FastAPI** - Lightning-fast async Python web framework
- **TailwindCSS** - Modern utility-first CSS
- **HTMX & Alpine.js** - Dynamic interactions without heavy JavaScript
- **Auth System** - Login/register integrated with C++ backend
- **Rich Pages** - Landing, profile, recommendation form, results, catalog
- **Instant Route Loading** - No Django overhead

## Setup

1. Install dependencies:
```bash
pip install -r requirements.txt
```

2. Create `.env`:
```bash
cp .env.example .env
```

3. Run the server:
```bash
python main.py
```

Server runs on `http://localhost:3000`

## Architecture

This is a **UI-only** frontend. All business logic runs in the C++ backend:
- Auth endpoints: `/api/auth/*`
- Course data: `/api/courses`
- Recommendations: `/api/recommendations`
- AI recommendations: via AI service on port 8081

The Python frontend only:
- Serves HTML templates
- Makes API calls to C++ backend
- No database access
- No business logic

## Pages

- `/` - Landing page with hero and features
- `/login` - Login form
- `/register` - Registration form
- `/profile` - User profile
- `/recommend` - Create learning plan
- `/plan` - View generated plan
- `/catalog` - Browse all courses
