# 💻 Frontend Structure (Python / PyQt)

Документ описує структуру десктопного клієнта **Course Recommendation Platform**. Клієнт написаний на **Python 3.11+** з використанням **PyQt5/6** для інтерфейсу та **requests** для REST-запитів до Crow backend.

---

## 1. Загальна архітектура
Фронтенд реалізує **MVC-подібну** архітектуру:
- **Model** — об’єкти `UserProfile`, `Course`, `Plan`.
- **View** — екрани PyQt (`MainWindow`, `ProfileForm`, `PlanView`).
- **Controller / Service** — класи, що виконують запити до REST API.

```
PyQt Application
├── models/           → структури даних
├── ui/               → файли .ui (Qt Designer)
├── services/         → REST-запити, адаптери
├── components/       → елементи GUI (таблиці, графіки)
└── main.py           → точка входу
```

---

## 2. Основні файли та модулі

### 🧩 `models/`
Зберігає Python-класи, що відповідають об’єктам бекенду.
```python
@dataclass
class Course:
    id: int
    title: str
    domain: str
    level: str
    durationHours: int
    score: float

@dataclass
class UserProfile:
    userId: int
    targetDomain: str
    currentLevel: str
    interests: list[str]
    hoursPerWeek: int
    deadlineWeeks: int

@dataclass
class PlanStep:
    step: int
    courseId: int
    hours: int
    note: str

@dataclass
class Plan:
    steps: list[PlanStep]
    totalHours: int
```

---

### 🧠 `services/api_client.py`
Інкапсулює всю взаємодію з REST API.
```python
import requests

class ApiClient:
    def __init__(self, base_url="http://localhost:8080/api"):
        self.base_url = base_url

    def get_courses(self):
        return requests.get(f"{self.base_url}/courses").json()

    def get_plan(self, user_id: int):
        return requests.get(f"{self.base_url}/plans/{user_id}").json()

    def post_recommendations(self, profile: dict):
        return requests.post(f"{self.base_url}/recommendations", json=profile).json()
```

---

### 🪟 `ui/main_window.ui`
Створюється у **Qt Designer**. Основні елементи:
- Вкладки: `Profile`, `Courses`, `Plan`.
- Кнопки: `Generate Plan`, `Save Plan`, `Load Plan`.
- Таблиці для відображення курсів і кроків плану.

---

### 🎛️ `main_window.py`
Зв’язує UI з логікою API.
```python
from PyQt5.QtWidgets import QMainWindow, QApplication, QMessageBox
from services.api_client import ApiClient

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.api = ApiClient()

    def generate_plan(self):
        profile = self.collect_profile_data()
        plan = self.api.post_recommendations(profile)
        self.display_plan(plan)
```

---

### 📊 `components/plan_view.py`
Відповідає за відображення плану користувачу — таблиця або графік.
```python
from PyQt5.QtWidgets import QTableWidget, QTableWidgetItem

def show_plan(table: QTableWidget, plan: dict):
    table.setRowCount(len(plan["steps"]))
    for i, step in enumerate(plan["steps"]):
        table.setItem(i, 0, QTableWidgetItem(str(step["step"])))
        table.setItem(i, 1, QTableWidgetItem(str(step["courseId"])))
        table.setItem(i, 2, QTableWidgetItem(str(step["hours"])))
        table.setItem(i, 3, QTableWidgetItem(step["note"]))
```

---

## 3. Потік взаємодії
1. Користувач відкриває апку → запит `/courses` для відображення каталогу.
2. Вводить свої параметри → `POST /recommendations`.
3. Отримує план → бачить у таблиці → може зберегти.
4. При повторному запуску → `GET /plans/:userId` завантажує старі плани.

---

## 4. Стилі та UX
- Використовуються кастомні стилі через **Qt Stylesheet (QSS)**.
- Можна додати теми: світла / темна.
- Планується додати візуалізацію прогресу (progress bars, charts).

---

## 5. Розширення
- Авторизація (login dialog → JWT token).
- Аналітика (зберігання історії користувача).
- Експорт плану в PDF/CSV.
- AI-помічник (інтеграція LLM для пояснення рекомендацій).