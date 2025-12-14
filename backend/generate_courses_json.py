#!/usr/bin/env python3
"""
Generate courses.json from init_db.sql
"""

import json
import re

def parse_sql_to_json(sql_file, output_file):
    """Parse SQL INSERT statements and convert to JSON"""

    with open(sql_file, 'r', encoding='utf-8') as f:
        sql_content = f.read()

    # Extract all course data from INSERT VALUES
    # Pattern: (id, 'title', 'domain', 'level', duration, '{"tag1", "tag2"}', '{prereq1, prereq2}')
    pattern = r'\((\d+),\s*\'([^\']+)\',\s*\'([^\']+)\',\s*\'([^\']+)\',\s*(\d+),\s*\'\{([^}]*)\}\',\s*\'\{([^}]*)\}\'\)'

    matches = re.findall(pattern, sql_content)

    courses = []

    for match in matches:
        course_id, title, domain, level, duration, tags_str, prereqs_str = match

        # Parse tags
        tags = []
        if tags_str.strip():
            # Extract quoted strings
            tag_matches = re.findall(r'"([^"]+)"', tags_str)
            tags = tag_matches

        # Parse prerequisites
        prereq_ids = []
        if prereqs_str.strip():
            # Extract numbers
            prereq_matches = re.findall(r'\d+', prereqs_str)
            prereq_ids = [int(p) for p in prereq_matches]

        course = {
            "id": int(course_id),
            "title": title,
            "domain": domain,
            "level": level,
            "durationHours": int(duration),
            "tags": tags,
            "prereqIds": prereq_ids
        }

        courses.append(course)

    # Write JSON file
    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(courses, f, indent=2, ensure_ascii=False)

    print(f"[OK] Successfully generated {output_file}")
    print(f"[OK] Total courses: {len(courses)}")

    # Show domain breakdown
    domain_counts = {}
    for course in courses:
        domain = course['domain']
        domain_counts[domain] = domain_counts.get(domain, 0) + 1

    print("\nCourses by domain:")
    for domain, count in sorted(domain_counts.items()):
        print(f"  - {domain}: {count} courses")

if __name__ == '__main__':
    parse_sql_to_json('data/init_db.sql', 'data/courses.json')
