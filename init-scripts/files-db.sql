-- Таблица для хранения информации о загруженных работах
CREATE TABLE IF NOT EXISTS submissions (
    id SERIAL PRIMARY KEY,
    student_name VARCHAR(255) NOT NULL,
    task_id VARCHAR(100) NOT NULL,
    filename VARCHAR(255) NOT NULL,
    file_path VARCHAR(500) NOT NULL,
    file_hash VARCHAR(64) NOT NULL,
    file_size BIGINT NOT NULL,
    uploaded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );

CREATE INDEX idx_submissions_hash ON submissions(file_hash);

CREATE INDEX idx_submissions_task ON submissions(task_id);

CREATE INDEX idx_submissions_student ON submissions(student_name);