# Course Recommendation Platform - Django Frontend

Django web application that provides a user interface for the Course Recommendation Platform.

## Setup

1. **Create virtual environment:**
   ```bash
   python -m venv venv
   source venv/bin/activate  # On Windows: venv\Scripts\activate
   ```

2. **Install dependencies:**
   ```bash
   pip install -r requirements.txt
   ```

3. **Run migrations:**
   ```bash
   python manage.py migrate
   ```

4. **Start the development server:**
   ```bash
   python manage.py runserver
   ```

   The web application will be available at `http://localhost:8000/`

## Important

**The C++ backend must be running on port 8080** for the frontend to work properly. Start the backend before accessing the web interface.

## Features

- **Home Page**: Create personalized learning plans by filling out a profile form
- **Course Catalog**: Browse all available courses organized by domain
- **Learning Plans**: View generated personalized course recommendations
- **Responsive Design**: Clean, modern interface with gradient headers and card layouts

## Pages

- `/` - Home page with profile form
- `/recommend/` - Generate recommendations (POST endpoint)
- `/catalog/` - Browse all courses
- `/plan/<user_id>/` - View saved plan for a specific user

## Configuration

Backend API URL is configured in `roadmap_web/settings.py`:
```python
BACKEND_API_URL = 'http://localhost:8080/api'
```

Change this if your C++ backend runs on a different host/port.
