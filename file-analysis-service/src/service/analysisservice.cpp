#include "analysisservice.h"
#include <iostream>

namespace service {

AnalysisService::AnalysisService(repository::ReportRepository& repo,
                                   clients::FileServiceClient& fileClient)
    : repo_(repo)
    , fileClient_(fileClient)
{}

AnalyzeResult AnalysisService::analyze(const AnalyzeRequest& request) {
    std::cout << "[AnalysisService] Analyzing submission " << request.submissionId
              << " with hash " << request.fileHash << std::endl;

    // Ищем файлы с таким же хэшем
    auto filesWithSameHash = fileClient_.findByHash(request.fileHash);

    bool isPlagiarism = false;
    std::optional<int> originalSubmissionId;
    double similarityPercent = 0.0;

    // Проверяем: есть ли более ранние сдачи от других студентов
    for (const auto& file : filesWithSameHash) {
        // Если файл загружен раньше и это другой студент
        if (file.id < request.submissionId && file.studentName != request.studentName) {
            isPlagiarism = true;
            originalSubmissionId = file.id;
            similarityPercent = 100.0;  // Полное совпадение хэша = 100%

            std::cout << "[AnalysisService] PLAGIARISM DETECTED! Original submission: "
                      << file.id << " by " << file.studentName << std::endl;
            break;
        }
    }

    // Создаём отчёт
    models::Report report;
    report.submissionId = request.submissionId;
    report.taskId = request.taskId;
    report.studentName = request.studentName;
    report.isPlagiarism = isPlagiarism;
    report.similarityPercent = similarityPercent;
    report.originalSubmissionId = originalSubmissionId;
    report.status = "completed";

    int reportId = repo_.create(report);

    // Формируем результат
    AnalyzeResult result;
    result.reportId = reportId;
    result.submissionId = request.submissionId;
    result.isPlagiarism = isPlagiarism;
    result.similarityPercent = similarityPercent;
    result.originalSubmissionId = originalSubmissionId;
    result.status = "completed";

    return result;
}

std::optional<models::Report> AnalysisService::getReport(int submissionId) {
    return repo_.findBySubmissionId(submissionId);
}

std::vector<models::Report> AnalysisService::getReportsByTask(const std::string& taskId) {
    return repo_.findByTaskId(taskId);
}

}