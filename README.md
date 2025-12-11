# 3. Загрузка первого файла через Gateway

curl -X POST http://localhost:8080/api/submissions
-H "Content-Type: application/json"
-d '{
"student_name": "ff Иван",
"task_id": "homework-3",
"filename": "solution.cpp",
"content": "int afafaffaafaffafafaf main() { return 0; }"
}'

# 4. Загрузка копии (должен обнаружить плагиат)

curl -X POST http://localhost:8080/api/submissions 
-H "Content-Type: application/json" 
-d '{
"student_name": "Петров Пётр",
"task_id": "homework-3",
"filename": "my_solution.cpp",
"content": "int main() { return 0; }"
}'

# 5. Получить информацию о submission

curl http://localhost:8080/api/submissions/1

# 6. Получить отчёт о плагиате

curl http://localhost:8080/api/submissions/2/report

# 7. Все отчёты по заданию

curl http://localhost:8080/api/tasks/homework-3/reports


# 3. Загрузка первого файла через Gateway

curl -X POST http://localhost:8080/api/submissions

-H "Content-Type: application/json"
-d '{
"student_name": "андрей бобуа",
"task_id": "hom66",
"filename": "solution.cpp",
"content": "int main() { return 44-98444; }"
}'

# 4. Загрузка копии (должен обнаружить плагиат)

curl -X POST http://localhost:8080/api/submissions
-H "Content-Type: application/json"
-d '{
"student_name": "бабулья бобуа",
"task_id": "hom66",
"filename": "solution.cpp",
"content": "int main() { return -98444; }"
}'

# 5. Получить информацию о submission

curl http://localhost:8080/api/submissions/1

# 6. Получить отчёт о плагиате

curl http://localhost:8080/api/submissions/2/report

# 7. Все отчёты по заданию

curl http://localhost:8080/api/tasks/homework-3/reports
