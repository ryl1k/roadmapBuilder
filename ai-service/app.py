from flask import Flask, request, jsonify
from flask_cors import CORS
from groq import Groq
import os
import json
from dotenv import load_dotenv

load_dotenv()

app = Flask(__name__)
CORS(app)

# Initialize Groq client
groq_client = Groq(api_key=os.getenv('GROQ_API_KEY'))

@app.route('/health', methods=['GET'])
def health():
    return jsonify({'status': 'ok', 'service': 'ai-recommender', 'version': '1.0'})

@app.route('/api/ai-recommendations', methods=['POST'])
def ai_recommendations():
    try:
        data = request.json
        profile = data.get('profile', {})
        courses = data.get('courses', [])

        if not profile or not courses:
            return jsonify({'error': 'Missing profile or courses data'}), 400

        # Build prompt for Groq
        prompt = build_recommendation_prompt(profile, courses)

        # Call Groq API
        response = groq_client.chat.completions.create(
            model="llama-3.3-70b-versatile",
            messages=[
                {
                    "role": "system",
                    "content": "You are an expert learning path advisor. Analyze user profiles and course catalogs to create optimal, personalized learning plans. Return structured JSON with course recommendations."
                },
                {
                    "role": "user",
                    "content": prompt
                }
            ],
            temperature=0.7,
            max_tokens=2000
        )

        # Parse AI response
        ai_content = response.choices[0].message.content

        # Extract JSON from response
        plan = parse_ai_response(ai_content, profile, courses)

        return jsonify(plan), 200

    except Exception as e:
        return jsonify({'error': str(e), 'fallback': True}), 500

def build_recommendation_prompt(profile, courses):
    """Build detailed prompt for Groq AI"""

    courses_info = []
    for c in courses:
        courses_info.append({
            'id': c.get('id'),
            'title': c.get('title'),
            'domain': c.get('domain'),
            'level': c.get('level'),
            'duration': c.get('durationHours'),
            'tags': c.get('tags', []),
            'prereqs': c.get('prereqIds', [])
        })

    prompt = f"""
Create a personalized learning plan based on the following:

USER PROFILE:
- Target Domain: {profile.get('targetDomain')}
- Current Level: {profile.get('currentLevel')}
- Interests: {', '.join(profile.get('interests', []))}
- Available Hours/Week: {profile.get('hoursPerWeek')}
- Deadline: {profile.get('deadlineWeeks')} weeks
- Total Available Hours: {profile.get('hoursPerWeek') * profile.get('deadlineWeeks')}

AVAILABLE COURSES:
{json.dumps(courses_info, indent=2)}

INSTRUCTIONS:
1. Select courses that match the user's target domain and interests
2. Ensure prerequisites are satisfied (courses must be ordered correctly)
3. Match the user's current level (start appropriate, progress logically)
4. Stay within time budget ({profile.get('hoursPerWeek') * profile.get('deadlineWeeks')} total hours)
5. Prioritize courses with relevant tags matching user interests
6. Create a logical progression from fundamentals to advanced topics

Return ONLY a JSON object in this exact format:
{{
  "steps": [
    {{
      "step": 1,
      "courseId": <course_id>,
      "hours": <duration>,
      "note": "<brief explanation why this course>"
    }}
  ],
  "totalHours": <sum_of_all_hours>,
  "reasoning": "<brief explanation of the learning path strategy>"
}}

IMPORTANT: Return ONLY valid JSON, no markdown, no extra text.
"""

    return prompt

def parse_ai_response(ai_content, profile, courses):
    """Parse and validate AI response"""
    try:
        # Try to find JSON in the response
        start_idx = ai_content.find('{')
        end_idx = ai_content.rfind('}') + 1

        if start_idx == -1 or end_idx == 0:
            raise ValueError("No JSON found in AI response")

        json_str = ai_content[start_idx:end_idx]
        plan = json.loads(json_str)

        # Validate structure
        if 'steps' not in plan or not isinstance(plan['steps'], list):
            raise ValueError("Invalid plan structure")

        # Ensure totalHours is present
        if 'totalHours' not in plan:
            plan['totalHours'] = sum(step.get('hours', 0) for step in plan['steps'])

        return plan

    except (json.JSONDecodeError, ValueError) as e:
        # If parsing fails, raise to trigger fallback
        raise Exception(f"Failed to parse AI response: {str(e)}")

if __name__ == '__main__':
    port = int(os.getenv('PORT', 8081))
    print(f"Starting AI Recommender Service on port {port}...")
    app.run(host='0.0.0.0', port=port, debug=True)
