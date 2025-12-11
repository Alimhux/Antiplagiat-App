#include "reportrepository.h"

namespace repository {

ReportRepository::ReportRepository(db::Database& database)
    : db_(database)
{}

int ReportRepository::create(const models::Report& report) {
    pqxx::work txn(db_.connection());

    std::string origIdValue = report.originalSubmissionId
        ? std::to_string(*report.originalSubmissionId)
        : "NULL";

    std::string query =
        "INSERT INTO reports (submission_id, task_id, student_name, is_plagiarism, "
        "similarity_percent, original_submission_id, status, completed_at) "
        "VALUES (" + std::to_string(report.submissionId) + ", "
                   + txn.quote(report.taskId) + ", "
                   + txn.quote(report.studentName) + ", "
                   + (report.isPlagiarism ? "true" : "false") + ", "
                   + std::to_string(report.similarityPercent) + ", "
                   + origIdValue + ", "
                   + txn.quote(report.status) + ", NOW()) "
        "RETURNING id";

    pqxx::result result = txn.exec(query);
    txn.commit();

    return result[0][0].as<int>();
}

std::optional<models::Report> ReportRepository::findBySubmissionId(int submissionId) {
    pqxx::work txn(db_.connection());

    std::string query =
        "SELECT id, submission_id, task_id, student_name, is_plagiarism, "
        "similarity_percent, original_submission_id, status, created_at, completed_at "
        "FROM reports WHERE submission_id = " + std::to_string(submissionId);

    pqxx::result result = txn.exec(query);
    txn.commit();

    if (result.empty()) {
        return std::nullopt;
    }

    return rowToReport(result[0]);
}

std::vector<models::Report> ReportRepository::findByTaskId(const std::string& taskId) {
    pqxx::work txn(db_.connection());

    std::string query =
        "SELECT id, submission_id, task_id, student_name, is_plagiarism, "
        "similarity_percent, original_submission_id, status, created_at, completed_at "
        "FROM reports WHERE task_id = " + txn.quote(taskId) + " "
        "ORDER BY created_at DESC";

    pqxx::result result = txn.exec(query);
    txn.commit();

    std::vector<models::Report> reports;
    reports.reserve(result.size());

    for (const auto& row : result) {
        reports.push_back(rowToReport(row));
    }

    return reports;
}

models::Report ReportRepository::rowToReport(const pqxx::row& row) {
    models::Report r;
    r.id = row[0].as<int>();
    r.submissionId = row[1].as<int>();
    r.taskId = row[2].as<std::string>();
    r.studentName = row[3].as<std::string>();
    r.isPlagiarism = row[4].as<bool>();
    r.similarityPercent = row[5].as<double>();

    if (!row[6].is_null()) {
        r.originalSubmissionId = row[6].as<int>();
    }

    r.status = row[7].as<std::string>();
    r.createdAt = row[8].as<std::string>();

    if (!row[9].is_null()) {
        r.completedAt = row[9].as<std::string>();
    }

    return r;
}

}