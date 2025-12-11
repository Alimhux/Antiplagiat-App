#include "filerepository.h"
#include <pqxx/pqxx>

namespace repository {


FileRepository::FileRepository(db::Database& database)
    : db_(database)
{}

int FileRepository::create(const models::Submission& submission) {
    pqxx::work txn(db_.connection());

    std::string query =
        "INSERT INTO submissions (student_name, task_id, filename, file_path, file_hash, file_size) "
        "VALUES (" + txn.quote(submission.studentName) + ", "
                   + txn.quote(submission.taskId) + ", "
                   + txn.quote(submission.filename) + ", "
                   + txn.quote(submission.filePath) + ", "
                   + txn.quote(submission.fileHash) + ", "
                   + std::to_string(submission.fileSize) + ") "
        "RETURNING id";

    pqxx::result result = txn.exec(query);
    txn.commit();

    return result[0][0].as<int>();
}

std::optional<models::Submission> FileRepository::findById(int id) {
    pqxx::work txn(db_.connection());

    std::string query =
        "SELECT id, student_name, task_id, filename, file_path, file_hash, file_size, uploaded_at "
        "FROM submissions WHERE id = " + std::to_string(id);

    pqxx::result result = txn.exec(query);
    txn.commit();

    if (result.empty()) {
        return std::nullopt;
    }

    return rowToSubmission(result[0]);
}

std::vector<models::Submission> FileRepository::findByHash(const std::string& hash) {
    pqxx::work txn(db_.connection());

    std::string query =
        "SELECT id, student_name, task_id, filename, file_path, file_hash, file_size, uploaded_at "
        "FROM submissions WHERE file_hash = " + txn.quote(hash) + " "
        "ORDER BY uploaded_at ASC";

    pqxx::result result = txn.exec(query);
    txn.commit();

    std::vector<models::Submission> submissions;
    submissions.reserve(result.size());

    for (const auto& row : result) {
        submissions.push_back(rowToSubmission(row));
    }

    return submissions;
}

std::vector<models::Submission> FileRepository::findByTaskId(const std::string& taskId) {
    pqxx::work txn(db_.connection());

    std::string query =
        "SELECT id, student_name, task_id, filename, file_path, file_hash, file_size, uploaded_at "
        "FROM submissions WHERE task_id = " + txn.quote(taskId) + " "
        "ORDER BY uploaded_at ASC";

    pqxx::result result = txn.exec(query);
    txn.commit();

    std::vector<models::Submission> submissions;
    submissions.reserve(result.size());

    for (const auto& row : result) {
        submissions.push_back(rowToSubmission(row));
    }

    return submissions;
}

models::Submission FileRepository::rowToSubmission(const pqxx::row& row) {
    models::Submission s;
    s.id = row[0].as<int>();
    s.studentName = row[1].as<std::string>();
    s.taskId = row[2].as<std::string>();
    s.filename = row[3].as<std::string>();
    s.filePath = row[4].as<std::string>();
    s.fileHash = row[5].as<std::string>();
    s.fileSize = row[6].as<int64_t>();
    s.uploadedAt = row[7].as<std::string>();
    return s;
}

}