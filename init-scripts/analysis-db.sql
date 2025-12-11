-- Таблица для хранения отчётов анализа
CREATE TABLE IF NOT EXISTS reports (
    id SERIAL PRIMARY KEY,
    submission_id INTEGER NOT NULL,
    task_id VARCHAR(100) NOT NULL,
    student_name VARCHAR(255) NOT NULL,
    is_plagiarism BOOLEAN DEFAULT FALSE,
    similarity_percent DECIMAL(5,2) DEFAULT 0.00,
    original_submission_id INTEGER,
    status VARCHAR(50) DEFAULT 'pending',
    report_path VARCHAR(500),
    word_cloud_url VARCHAR(500),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    completed_at TIMESTAMP
    );

CREATE INDEX IF NOT EXISTS idx_reports_submission ON reports(submission_id);
CREATE INDEX IF NOT EXISTS idx_reports_task ON reports(task_id);
CREATE INDEX IF NOT EXISTS idx_reports_plagiarism ON reports(is_plagiarism);