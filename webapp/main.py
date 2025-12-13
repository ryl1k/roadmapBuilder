from fastapi import FastAPI, Request
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from fastapi.responses import HTMLResponse
import uvicorn
import os

app = FastAPI(title="Course Recommendation Platform")

# Static files and templates
app.mount("/static", StaticFiles(directory="static"), name="static")
templates = Jinja2Templates(directory="templates")

# Backend URLs
BACKEND_URL = os.getenv("BACKEND_API_URL", "http://localhost:8080/api")
AI_SERVICE_URL = os.getenv("AI_SERVICE_URL", "http://localhost:8081")

@app.get("/", response_class=HTMLResponse)
async def home(request: Request):
    return templates.TemplateResponse("index.html", {
        "request": request,
        "backend_url": BACKEND_URL,
        "ai_url": AI_SERVICE_URL
    })

@app.get("/login", response_class=HTMLResponse)
async def login(request: Request):
    return templates.TemplateResponse("login.html", {"request": request, "backend_url": BACKEND_URL})

@app.get("/register", response_class=HTMLResponse)
async def register(request: Request):
    return templates.TemplateResponse("register.html", {"request": request, "backend_url": BACKEND_URL})

@app.get("/profile", response_class=HTMLResponse)
async def profile(request: Request):
    return templates.TemplateResponse("profile.html", {"request": request, "backend_url": BACKEND_URL})

@app.get("/recommend", response_class=HTMLResponse)
async def recommend(request: Request):
    return templates.TemplateResponse("recommend.html", {
        "request": request,
        "backend_url": BACKEND_URL,
        "ai_url": AI_SERVICE_URL
    })

@app.get("/plan", response_class=HTMLResponse)
async def plan(request: Request):
    return templates.TemplateResponse("plan.html", {"request": request, "backend_url": BACKEND_URL})

@app.get("/catalog", response_class=HTMLResponse)
async def catalog(request: Request):
    return templates.TemplateResponse("catalog.html", {"request": request, "backend_url": BACKEND_URL})

if __name__ == "__main__":
    uvicorn.run("main:app", host="0.0.0.0", port=3000, reload=True)
