# Database Seeding Instructions

## Seed 100 Courses

### Option 1: Via PostgreSQL Client (Recommended)

```bash
# Make sure PostgreSQL is running
docker-compose up -d

# Connect to database
psql -h localhost -p 5432 -U postgres -d roadmap

# Run seed script
\i backend/data/seed_courses.sql
```

Or using psql directly:
```bash
psql -h localhost -p 5432 -U postgres -d roadmap -f backend/data/seed_courses.sql
```

### Option 2: Via Backend Import (Existing Method)

The C++ backend automatically imports from `courses.json` on first run if database is empty.

To re-seed:
1. Clear database: `DELETE FROM courses;`
2. Restart backend - it will import from `courses.json`

### Option 3: Docker Exec

```bash
docker exec -i roadmap_postgres psql -U postgres -d roadmap < backend/data/seed_courses.sql
```

## Verify Seeding

```sql
-- Count total courses
SELECT COUNT(*) FROM courses;

-- Count by domain
SELECT domain, COUNT(*) as count
FROM courses
GROUP BY domain
ORDER BY count DESC;

-- View all courses
SELECT id, title, domain, level FROM courses ORDER BY id;
```

## Course Distribution

- Data Science: 20 courses (1-20)
- Web Development: 20 courses (21-40)
- DevOps: 15 courses (41-55)
- Cloud: 10 courses (56-65)
- Mobile: 10 courses (66-75)
- Cybersecurity: 10 courses (76-85)
- Database: 7 courses (86-92)
- AI & ML: 8 courses (93-100)

**Total: 100 courses**

## Domains Covered

1. **Data Science**: Python, ML, Deep Learning, NLP, Computer Vision
2. **Web Development**: Frontend (React, Vue, Angular), Backend (Node, Express)
3. **DevOps**: Docker, Kubernetes, CI/CD, Infrastructure as Code
4. **Cloud**: AWS, Azure, GCP, Serverless
5. **Mobile**: Android, iOS, React Native, Flutter
6. **Cybersecurity**: Ethical Hacking, Network Security, Compliance
7. **Database**: PostgreSQL, SQL, NoSQL, Redis, Optimization
8. **AI & ML**: Generative AI, LLMs, Prompt Engineering, MLOps

## Prerequisite Graph

Courses have proper prerequisite chains:
- Beginner courses have no prerequisites
- Intermediate courses depend on beginner courses
- Advanced courses build on intermediate knowledge
- Cross-domain dependencies exist (e.g., Web Security depends on Web Development)
