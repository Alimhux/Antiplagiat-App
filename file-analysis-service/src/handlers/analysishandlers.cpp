#include "analysishandlers.h"
#include "json.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

using json = nlohmann::json;

namespace handlers {

AnalysisHandlers::AnalysisHandlers(service::AnalysisService& analysisService,
                                   clients::FileServiceClient& fileClient)
    : analysisService_(analysisService)
    , fileClient_(fileClient)
{}

void AnalysisHandlers::registerRoutes(httplib::Server& server) {
    server.Get("/health", [this](const httplib::Request& req, httplib::Response& res) {
        handleHealth(req, res);
    });

    server.Post("/analyze", [this](const httplib::Request& req, httplib::Response& res) {
        handleAnalyze(req, res);
    });

    server.Get(R"(/reports/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetReport(req, res);
    });

    server.Get(R"(/tasks/([^/]+)/reports)", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetTaskReports(req, res);
    });

    // Word Cloud endpoint
    server.Get(R"(/submissions/(\d+)/wordcloud)", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetWordCloud(req, res);
    });
}

void AnalysisHandlers::handleHealth(const httplib::Request& /*req*/, httplib::Response& res) {
    json response;
    response["status"] = "ok";
    response["service"] = "file-analysis-service";
    sendJson(res, 200, response.dump());
}

void AnalysisHandlers::handleAnalyze(const httplib::Request& req, httplib::Response& res) {
    std::cout << "[AnalysisHandlers] POST /analyze" << std::endl;

    try {
        if (req.body.empty()) {
            sendError(res, 400, "Empty request body");
            return;
        }

        json body;
        try {
            body = json::parse(req.body);
        } catch (const std::exception& e) {
            sendError(res, 400, "Invalid JSON");
            return;
        }

        service::AnalyzeRequest analyzeReq;
        analyzeReq.submissionId = body["submission_id"];
        analyzeReq.taskId = body["task_id"];
        analyzeReq.studentName = body["student_name"];
        analyzeReq.fileHash = body["file_hash"];

        auto result = analysisService_.analyze(analyzeReq);

        json response;
        response["report_id"] = result.reportId;
        response["submission_id"] = result.submissionId;
        response["is_plagiarism"] = result.isPlagiarism;
        response["similarity_percent"] = result.similarityPercent;

        if (result.originalSubmissionId) {
            response["original_submission_id"] = *result.originalSubmissionId;
        } else {
            response["original_submission_id"] = nullptr;
        }

        response["status"] = result.status;

        sendJson(res, 201, response.dump());

    } catch (const std::exception& e) {
        std::cerr << "[AnalysisHandlers] Error in handleAnalyze: " << e.what() << std::endl;
        sendError(res, 500, std::string("Server error: ") + e.what());
    }
}

void AnalysisHandlers::handleGetReport(const httplib::Request& req, httplib::Response& res) {
    try {
        int submissionId = std::stoi(req.matches[1]);
        std::cout << "[AnalysisHandlers] GET /reports/" << submissionId << std::endl;

        auto report = analysisService_.getReport(submissionId);
        if (!report) {
            sendError(res, 404, "Report not found");
            return;
        }

        json response;
        response["report_id"] = report->id;
        response["submission_id"] = report->submissionId;
        response["task_id"] = report->taskId;
        response["student_name"] = report->studentName;
        response["is_plagiarism"] = report->isPlagiarism;
        response["similarity_percent"] = report->similarityPercent;

        if (report->originalSubmissionId) {
            response["original_submission_id"] = *report->originalSubmissionId;
        } else {
            response["original_submission_id"] = nullptr;
        }

        response["status"] = report->status;
        response["created_at"] = report->createdAt;

        if (!report->completedAt.empty()) {
            response["completed_at"] = report->completedAt;
        } else {
            response["completed_at"] = nullptr;
        }

        // Добавляем ссылку на Word Cloud
        response["word_cloud_url"] = "/submissions/" + std::to_string(submissionId) + "/wordcloud";

        sendJson(res, 200, response.dump());

    } catch (const std::exception& e) {
        std::cerr << "[AnalysisHandlers] Error in handleGetReport: " << e.what() << std::endl;
        sendError(res, 500, std::string("Server error: ") + e.what());
    }
}

void AnalysisHandlers::handleGetTaskReports(const httplib::Request& req, httplib::Response& res) {
    try {
        std::string taskId = req.matches[1];
        std::cout << "[AnalysisHandlers] GET /tasks/" << taskId << "/reports" << std::endl;

        auto reports = analysisService_.getReportsByTask(taskId);

        json reportsJson = json::array();
        int plagiarismCount = 0;

        for (const auto& r : reports) {
            if (r.isPlagiarism) plagiarismCount++;

            json report;
            report["report_id"] = r.id;
            report["submission_id"] = r.submissionId;
            report["student_name"] = r.studentName;
            report["is_plagiarism"] = r.isPlagiarism;
            report["similarity_percent"] = r.similarityPercent;
            report["status"] = r.status;
            report["created_at"] = r.createdAt;
            report["word_cloud_url"] = "/submissions/" + std::to_string(r.submissionId) + "/wordcloud";
            reportsJson.push_back(report);
        }

        json response;
        response["task_id"] = taskId;
        response["total_submissions"] = reports.size();
        response["plagiarism_count"] = plagiarismCount;
        response["reports"] = reportsJson;

        sendJson(res, 200, response.dump());

    } catch (const std::exception& e) {
        std::cerr << "[AnalysisHandlers] Error in handleGetTaskReports: " << e.what() << std::endl;
        sendError(res, 500, std::string("Server error: ") + e.what());
    }
}

void AnalysisHandlers::handleGetWordCloud(const httplib::Request& req, httplib::Response& res) {
    try {
        int submissionId = std::stoi(req.matches[1]);
        std::cout << "[AnalysisHandlers] GET /submissions/" << submissionId << "/wordcloud" << std::endl;

        // Получаем содержимое файла через FileServiceClient
        std::string content = fileClient_.getFileContent(submissionId);

        if (content.empty()) {
            sendError(res, 404, "File content not found");
            return;
        }

        // URL-encode содержимого для QuickChart API
        std::ostringstream escaped;
        escaped.fill('0');
        escaped << std::hex;

        for (char c : content) {
            if (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~' || c == ' ') {
                if (c == ' ') {
                    escaped << '+';
                } else {
                    escaped << c;
                }
            } else {
                escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
            }
        }

        // Формируем URL для QuickChart Word Cloud API
        std::string wordCloudUrl = "https://quickchart.io/wordcloud?text=" + escaped.str() +
                                   "&width=800&height=600&fontScale=15&rotation=0";

        json response;
        response["submission_id"] = submissionId;
        response["word_cloud_url"] = wordCloudUrl;
        response["message"] = "Open the URL in browser to view the word cloud";

        sendJson(res, 200, response.dump());

    } catch (const std::exception& e) {
        std::cerr << "[AnalysisHandlers] Error in handleGetWordCloud: " << e.what() << std::endl;
        sendError(res, 500, std::string("Server error: ") + e.what());
    }
}

void AnalysisHandlers::sendError(httplib::Response& res, int status, const std::string& message) {
    json error;
    error["error"] = message;
    res.status = status;
    res.set_content(error.dump(), "application/json");
}

void AnalysisHandlers::sendJson(httplib::Response& res, int status, const std::string& jsonStr) {
    res.status = status;
    res.set_content(jsonStr, "application/json");
}

}