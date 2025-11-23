from django.shortcuts import render
from django.http import JsonResponse
from django.conf import settings
import requests
import json


def index(request):
    """Home page with course catalog and profile form"""
    # Fetch courses from backend
    try:
        response = requests.get(f"{settings.BACKEND_API_URL}/courses", timeout=5)
        courses = response.json() if response.status_code == 200 else []
    except Exception as e:
        courses = []
        print(f"Error fetching courses: {e}")

    context = {
        'courses': courses,
        'domains': list(set(c.get('domain', '') for c in courses if c.get('domain'))),
        'levels': ['Beginner', 'Intermediate', 'Advanced']
    }
    return render(request, 'recommendations/index.html', context)


def get_recommendations(request):
    """Generate learning plan recommendations"""
    if request.method == 'POST':
        try:
            # Parse form data
            profile = {
                'userId': int(request.POST.get('userId', 1)),
                'targetDomain': request.POST.get('targetDomain', ''),
                'currentLevel': request.POST.get('currentLevel', 'Beginner'),
                'interests': request.POST.get('interests', '').split(','),
                'hoursPerWeek': int(request.POST.get('hoursPerWeek', 10)),
                'deadlineWeeks': int(request.POST.get('deadlineWeeks', 12))
            }

            # Clean interests
            profile['interests'] = [i.strip() for i in profile['interests'] if i.strip()]

            # Call backend API
            response = requests.post(
                f"{settings.BACKEND_API_URL}/recommendations",
                json={'profile': profile},
                headers={'Content-Type': 'application/json'},
                timeout=10
            )

            if response.status_code == 200:
                plan = response.json()

                # Get course details for the plan
                courses_response = requests.get(f"{settings.BACKEND_API_URL}/courses", timeout=5)
                courses = courses_response.json() if courses_response.status_code == 200 else []
                courses_dict = {c['id']: c for c in courses}

                # Enrich plan with course details
                for step in plan.get('steps', []):
                    course = courses_dict.get(step['courseId'])
                    if course:
                        step['courseTitle'] = course.get('title', 'Unknown Course')
                        step['courseDomain'] = course.get('domain', '')
                        step['courseLevel'] = course.get('level', '')

                context = {
                    'plan': plan,
                    'profile': profile,
                    'success': True
                }
                return render(request, 'recommendations/plan.html', context)
            else:
                error_msg = response.json().get('error', 'Failed to generate recommendations')
                return render(request, 'recommendations/error.html', {'error': error_msg})

        except requests.exceptions.ConnectionError:
            return render(request, 'recommendations/error.html', {
                'error': 'Cannot connect to backend server. Please ensure the C++ backend is running on port 8080.'
            })
        except Exception as e:
            return render(request, 'recommendations/error.html', {'error': str(e)})

    return render(request, 'recommendations/index.html')


def view_plan(request, user_id):
    """View saved learning plan for a user"""
    try:
        response = requests.get(f"{settings.BACKEND_API_URL}/plans/{user_id}", timeout=5)

        if response.status_code == 200:
            plan = response.json()

            # Get course details
            courses_response = requests.get(f"{settings.BACKEND_API_URL}/courses", timeout=5)
            courses = courses_response.json() if courses_response.status_code == 200 else []
            courses_dict = {c['id']: c for c in courses}

            # Enrich plan with course details
            for step in plan.get('steps', []):
                course = courses_dict.get(step['courseId'])
                if course:
                    step['courseTitle'] = course.get('title', 'Unknown Course')
                    step['courseDomain'] = course.get('domain', '')
                    step['courseLevel'] = course.get('level', '')

            context = {
                'plan': plan,
                'user_id': user_id,
                'success': True
            }
            return render(request, 'recommendations/plan.html', context)
        elif response.status_code == 404:
            return render(request, 'recommendations/error.html', {
                'error': f'No plan found for user {user_id}'
            })
        else:
            return render(request, 'recommendations/error.html', {
                'error': 'Failed to load plan'
            })

    except Exception as e:
        return render(request, 'recommendations/error.html', {'error': str(e)})


def courses_catalog(request):
    """Display all available courses"""
    try:
        response = requests.get(f"{settings.BACKEND_API_URL}/courses", timeout=5)
        courses = response.json() if response.status_code == 200 else []

        # Group by domain
        courses_by_domain = {}
        for course in courses:
            domain = course.get('domain', 'Other')
            if domain not in courses_by_domain:
                courses_by_domain[domain] = []
            courses_by_domain[domain].append(course)

        context = {
            'courses_by_domain': courses_by_domain,
            'total_courses': len(courses)
        }
        return render(request, 'recommendations/catalog.html', context)

    except Exception as e:
        return render(request, 'recommendations/error.html', {'error': str(e)})
