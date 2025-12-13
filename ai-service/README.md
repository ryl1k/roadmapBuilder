# AI Recommendation Service

AI-powered course recommendation service using Groq LLM.

## Setup

1. Install dependencies:
```bash
pip install -r requirements.txt
```

2. Create `.env` file:
```bash
cp .env.example .env
```

3. Add your Groq API key to `.env`:
```
GROQ_API_KEY=your_actual_key
```

## Run

```bash
python app.py
```

Service runs on port 8081 by default.

## API Endpoints

### POST /api/ai-recommendations
Generate AI-powered learning plan.

Request:
```json
{
  "profile": {
    "userId": 1,
    "targetDomain": "Data Science",
    "currentLevel": "Beginner",
    "interests": ["python", "ml"],
    "hoursPerWeek": 10,
    "deadlineWeeks": 12
  },
  "courses": [...]
}
```

Response:
```json
{
  "steps": [...],
  "totalHours": 80,
  "reasoning": "..."
}
```

### GET /health
Health check endpoint.
