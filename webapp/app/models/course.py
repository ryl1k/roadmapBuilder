from pydantic import BaseModel
from typing import List, Optional

class Course(BaseModel):
    id: int
    title: str
    domain: str
    level: str
    durationHours: int
    tags: List[str]
    prereqIds: List[int] = []

class UserProfile(BaseModel):
    userId: int
    targetDomain: str
    currentLevel: str
    interests: List[str]
    hoursPerWeek: int
    deadlineWeeks: int

class PlanStep(BaseModel):
    step: int
    courseId: int
    hours: int
    note: str = ""

class Plan(BaseModel):
    steps: List[PlanStep]
    totalHours: int
    reasoning: Optional[str] = None
