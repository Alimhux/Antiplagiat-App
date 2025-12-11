#include "gatewayhandlers.h"

#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

namespace handlers {

GatewayHandlers::GatewayHandlers(clients::ServiceClient& fileService,
                                   clients::ServiceClient& analysisService)
    : fileService_(fileService)
    , analysisService_(analysisService)
{}

void GatewayHandlers::registerRoutes(httplib::Server& server) {
    server.Get("/health", [this](const httplib::Request& req, httplib::Response& res) {
        handleHealth(req, res);
    });

    server.Get("/", [this](const httplib::Request& req, httplib::Response& res) {
        handleRoot(req, res);
    });

    server.Post("/api/submissions", [this](const httplib::Request& req, httplib::Response& res) {
        handleCreateSubmission(req, res);
    });

    server.Get(R"(/api/submissions/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetSubmission(req, res);
    });

    server.Get(R"(/api/submissions/(\d+)/report)", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetSubmissionReport(req, res);
    });

    server.Get(R"(/api/tasks/([^/]+)/reports)", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetTaskReports(req, res);
    });
}

void GatewayHandlers::handleHealth(const httplib::Request& /*req*/, httplib::Response& res) {
    json response;
    response["status"] = "ok";
    response["service"] = "api-gateway";
    sendJson(res, 200, response.dump());
}

void GatewayHandlers::handleRoot(const httplib::Request& /*req*/, httplib::Response& res) {
    json response;
    response["message"] = "Antiplagiat API Gateway";
    response["version"] = "1.0.0";

    json endpoints;
    endpoints["POST /api/submissions"] = "Upload a submission for plagiarism check";
    endpoints["GET /api/submissions/{id}"] = "Get submission info";
    endpoints["GET /api/submissions/{id}/report"] = "Get plagiarism report for submission";
    endpoints["GET /api/tasks/{task_id}/reports"] = "Get all reports for a task";
    response["endpoints"] = endpoints;

    sendJson(res, 200, response.dump(2));
}

void GatewayHandlers::handleCreateSubmission(const httplib::Request& req, httplib::Response& res) {
    std::cout << "[Gateway] POST /api/submissions" << std::endl;

    // 1. Валидация
    if (req.body.empty()) {
        sendError(res, 400, "Empty request body");
        return;
    }

    json body;
    try {
        body = json::parse(req.body);
    } catch (const std::exception& e) {
        sendError(res, 400, "Invalid JSON: " + std::string(e.what()));
        return;
    }

    if (!body.contains("content") || body["content"].get<std::string>().empty()) {
        sendError(res, 400, "Field 'content' is required");
        return;
    }

    // 2. Загружаем файл
    auto fileResponse = fileService_.post("/files", req.body);
    if (!fileResponse.success) {
        sendJson(res, 503, fileResponse.body);
        return;
    }

    if (fileResponse.status != 201) {
        sendJson(res, fileResponse.status, fileResponse.body);
        return;
    }

    // 3. Парсим ответ
    json fileData;
    try {
        fileData = json::parse(fileResponse.body);
    } catch (const std::exception& e) {
        sendError(res, 500, "Invalid response from file service");
        return;
    }

    int submissionId = fileData["id"];
    std::string taskId = fileData["task_id"];
    std::string studentName = fileData["student_name"];
    std::string fileHash = fileData["file_hash"];

    std::cout << "[Gateway] File uploaded, id=" << submissionId << ", hash=" << fileHash << std::endl;

    // 4. Запускаем анализ
    json analyzeRequest;
    analyzeRequest["submission_id"] = submissionId;
    analyzeRequest["task_id"] = taskId;
    analyzeRequest["student_name"] = studentName;
    analyzeRequest["file_hash"] = fileHash;

    auto analysisResponse = analysisService_.post("/analyze", analyzeRequest.dump());

    if (!analysisResponse.success) {
        // Файл загружен, но анализ не удался
        json response;
        response["submission"] = fileData;
        response["analysis"] = nullptr;
        response["warning"] = "File uploaded but analysis service unavailable";
        sendJson(res, 207, response.dump());
        return;
    }

    if (analysisResponse.status != 201) {
        json response;
        response["submission"] = fileData;
        try {
            response["analysis_error"] = json::parse(analysisResponse.body);
        } catch (...) {
            response["analysis_error"] = analysisResponse.body;
        }
        sendJson(res, 207, response.dump());
        return;
    }

    // 5. Успех
    json analysisData = json::parse(analysisResponse.body);

    json response;
    response["submission"] = fileData;
    response["analysis"] = analysisData;

    std::cout << "[Gateway] Analysis complete, plagiarism="
              << (analysisData["is_plagiarism"].get<bool>() ? "YES" : "NO") << std::endl;

    sendJson(res, 201, response.dump());
}

void GatewayHandlers::handleGetSubmission(const httplib::Request& req, httplib::Response& res) {
    std::string id = req.matches[1];
    std::cout << "[Gateway] GET /api/submissions/" << id << std::endl;

    auto response = fileService_.get("/files/" + id);
    sendJson(res, response.status, response.body);
}

void GatewayHandlers::handleGetSubmissionReport(const httplib::Request& req, httplib::Response& res) {
    std::string id = req.matches[1];
    std::cout << "[Gateway] GET /api/submissions/" << id << "/report" << std::endl;

    auto response = analysisService_.get("/reports/" + id);
    sendJson(res, response.status, response.body);
}

void GatewayHandlers::handleGetTaskReports(const httplib::Request& req, httplib::Response& res) {
    std::string taskId = req.matches[1];
    std::cout << "[Gateway] GET /api/tasks/" << taskId << "/reports" << std::endl;

    auto response = analysisService_.get("/tasks/" + taskId + "/reports");
    sendJson(res, response.status, response.body);
}

void GatewayHandlers::sendError(httplib::Response& res, int status, const std::string& message) {
    json error;
    error["error"] = message;
    res.status = status;
    res.set_content(error.dump(), "application/json");
}

void GatewayHandlers::sendJson(httplib::Response& res, int status, const std::string& jsonStr) {
    res.status = status;
    res.set_content(jsonStr, "application/json");
}

}