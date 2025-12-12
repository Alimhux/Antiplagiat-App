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

    // Swagger UI
    server.Get("/docs", [this](const httplib::Request& req, httplib::Response& res) {
        handleDocs(req, res);
    });

    server.Get("/openapi.yaml", [this](const httplib::Request& req, httplib::Response& res) {
        handleOpenApiSpec(req, res);
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

    server.Get(R"(/api/submissions/(\d+)/wordcloud)", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetWordCloud(req, res);
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
    endpoints["GET /api/submissions/{id}/wordcloud"] = "Get word cloud visualization URL";
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
    response["word_cloud_url"] = "/api/submissions/" + std::to_string(submissionId) + "/wordcloud";

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

void GatewayHandlers::handleGetWordCloud(const httplib::Request& req, httplib::Response& res) {
    std::string id = req.matches[1];
    std::cout << "[Gateway] GET /api/submissions/" << id << "/wordcloud" << std::endl;

    auto response = analysisService_.get("/submissions/" + id + "/wordcloud");
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

void GatewayHandlers::handleDocs(const httplib::Request& /*req*/, httplib::Response& res) {
    std::string html = R"(
<!DOCTYPE html>
<html lang="ru">
<head>
    <title>Antiplagiat API</title>
    <meta charset="utf-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" type="text/css" href="https://unpkg.com/swagger-ui-dist@5.9.0/swagger-ui.css" />
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;500;600;700&display=swap" rel="stylesheet">
    <style>
        :root {
            --purple-50: #faf5ff;
            --purple-100: #f3e8ff;
            --purple-200: #e9d5ff;
            --purple-300: #d8b4fe;
            --purple-400: #c084fc;
            --purple-500: #a855f7;
            --purple-600: #9333ea;
            --purple-700: #7c3aed;
            --purple-800: #6b21a8;
            --purple-900: #581c87;
            --white: #ffffff;
            --gray-50: #f9fafb;
            --gray-100: #f3f4f6;
            --gray-200: #e5e7eb;
            --gray-300: #d1d5db;
            --gray-600: #4b5563;
            --gray-700: #374151;
            --gray-800: #1f2937;
        }

        * {
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, sans-serif;
        }

        body {
            margin: 0;
            padding: 0;
            background: var(--gray-50);
        }

        /* Header */
        .custom-header {
            background: linear-gradient(135deg, var(--purple-700) 0%, var(--purple-600) 100%);
            color: var(--white);
            padding: 40px 0;
            text-align: center;
            box-shadow: 0 4px 20px rgba(124, 58, 237, 0.3);
        }

        .custom-header h1 {
            margin: 0 0 8px 0;
            font-size: 32px;
            font-weight: 700;
            letter-spacing: -0.5px;
        }

        .custom-header p {
            margin: 0;
            font-size: 16px;
            opacity: 0.9;
            font-weight: 400;
        }

        .header-badge {
            display: inline-block;
            background: rgba(255,255,255,0.2);
            padding: 6px 14px;
            border-radius: 20px;
            font-size: 13px;
            margin-top: 16px;
            font-weight: 500;
        }

        /* Swagger UI Customization */
        .swagger-ui {
            max-width: 1200px;
            margin: 0 auto;
            padding: 30px 20px;
        }

        .swagger-ui .topbar { display: none; }

        .swagger-ui .info { margin: 0 0 30px 0; }
        .swagger-ui .info hgroup.main { margin: 0; }
        .swagger-ui .info .title { font-size: 0; height: 0; margin: 0; }
        .swagger-ui .info .description { margin-top: 0; }

        .swagger-ui .info .description p,
        .swagger-ui .info .description li {
            font-size: 14px;
            color: var(--gray-600);
            line-height: 1.7;
        }

        .swagger-ui .info .description h2 {
            color: var(--purple-800);
            font-size: 18px;
            font-weight: 600;
            margin: 24px 0 12px 0;
            padding-bottom: 8px;
            border-bottom: 2px solid var(--purple-200);
        }

        /* Operation blocks */
        .swagger-ui .opblock {
            border-radius: 12px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.06);
            border: 1px solid var(--gray-200);
            margin-bottom: 12px;
        }

        .swagger-ui .opblock .opblock-summary {
            border-radius: 12px;
            padding: 12px 16px;
        }

        .swagger-ui .opblock.opblock-get {
            background: var(--purple-50);
            border-color: var(--purple-200);
        }
        .swagger-ui .opblock.opblock-get .opblock-summary {
            border-color: var(--purple-300);
        }
        .swagger-ui .opblock.opblock-get .opblock-summary-method {
            background: var(--purple-600);
        }

        .swagger-ui .opblock.opblock-post {
            background: var(--white);
            border-color: var(--purple-300);
        }
        .swagger-ui .opblock.opblock-post .opblock-summary {
            border-color: var(--purple-400);
        }
        .swagger-ui .opblock.opblock-post .opblock-summary-method {
            background: var(--purple-700);
        }

        .swagger-ui .opblock .opblock-summary-method {
            border-radius: 6px;
            font-weight: 600;
            font-size: 12px;
            padding: 6px 12px;
            min-width: 60px;
        }

        .swagger-ui .opblock .opblock-summary-path {
            font-size: 14px;
            font-weight: 500;
            color: var(--gray-800);
        }

        .swagger-ui .opblock .opblock-summary-description {
            font-size: 13px;
            color: var(--gray-600);
        }

        /* Tags */
        .swagger-ui .opblock-tag {
            font-size: 18px;
            font-weight: 600;
            color: var(--purple-800);
            border-bottom: 2px solid var(--purple-200);
            padding: 16px 0;
            margin: 30px 0 16px 0;
        }

        .swagger-ui .opblock-tag:hover {
            background: transparent;
        }

        .swagger-ui .opblock-tag small {
            font-size: 13px;
            color: var(--gray-600);
            font-weight: 400;
        }

        /* Buttons */
        .swagger-ui .btn {
            border-radius: 8px;
            font-weight: 500;
            font-size: 13px;
            padding: 8px 16px;
            transition: all 0.2s;
        }

        .swagger-ui .btn.execute {
            background: var(--purple-600);
            border-color: var(--purple-600);
        }

        .swagger-ui .btn.execute:hover {
            background: var(--purple-700);
            border-color: var(--purple-700);
        }

        .swagger-ui .btn.try-out__btn {
            border-color: var(--purple-400);
            color: var(--purple-700);
        }

        .swagger-ui .btn.try-out__btn:hover {
            background: var(--purple-50);
        }

        /* Models */
        .swagger-ui section.models {
            border: 1px solid var(--gray-200);
            border-radius: 12px;
        }

        .swagger-ui section.models h4 {
            font-size: 16px;
            color: var(--purple-800);
        }

        /* Responses */
        .swagger-ui .responses-inner {
            padding: 16px;
        }

        .swagger-ui .response-col_status {
            font-weight: 600;
            color: var(--purple-700);
        }

        /* Input fields */
        .swagger-ui input[type=text],
        .swagger-ui textarea {
            border-radius: 8px;
            border: 1px solid var(--gray-300);
            padding: 10px 12px;
            font-size: 14px;
        }

        .swagger-ui input[type=text]:focus,
        .swagger-ui textarea:focus {
            border-color: var(--purple-400);
            outline: none;
            box-shadow: 0 0 0 3px var(--purple-100);
        }

        .swagger-ui select {
            border-radius: 8px;
            border: 1px solid var(--gray-300);
            padding: 8px 12px;
        }

        /* Code blocks */
        .swagger-ui .highlight-code {
            border-radius: 8px;
        }

        /* Footer */
        .custom-footer {
            text-align: center;
            padding: 30px;
            color: var(--gray-600);
            font-size: 13px;
        }

        .custom-footer a {
            color: var(--purple-600);
            text-decoration: none;
        }

        .custom-footer a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <div class="custom-header">
        <h1>Antiplagiat API</h1>
        <p>Система проверки студенческих работ на плагиат</p>
        <div class="header-badge">v1.0.0</div>
    </div>

    <div id="swagger-ui"></div>

    <div class="custom-footer">
        HSE Software Construction | Homework 3
    </div>

    <script src="https://unpkg.com/swagger-ui-dist@5.9.0/swagger-ui-bundle.js"></script>
    <script>
        window.onload = function() {
            SwaggerUIBundle({
                url: "/openapi.yaml",
                dom_id: '#swagger-ui',
                presets: [
                    SwaggerUIBundle.presets.apis,
                    SwaggerUIBundle.SwaggerUIStandalonePreset
                ],
                layout: "BaseLayout",
                deepLinking: true,
                showExtensions: true,
                showCommonExtensions: true,
                defaultModelsExpandDepth: 0,
                docExpansion: "list"
            });
        };
    </script>
</body>
</html>
)";
    res.status = 200;
    res.set_content(html, "text/html");
}

void GatewayHandlers::handleOpenApiSpec(const httplib::Request& /*req*/, httplib::Response& res) {
    std::string yaml = R"(openapi: 3.0.3
info:
  title: Antiplagiat API
  description: |
    Система проверки студенческих работ на плагиат.
    
    ## Архитектура
    - **API Gateway** (порт 8080) — единая точка входа
    - **File Storing Service** (порт 8081) — хранение файлов
    - **File Analysis Service** (порт 8082) — анализ на плагиат
    
    ## Алгоритм определения плагиата
    Система вычисляет SHA-256 хэш содержимого файла. Если существует более ранняя 
    сдача от другого студента с идентичным хэшем — фиксируется плагиат (100% совпадение).
  version: 1.0.0

servers:
  - url: http://localhost:8080
    description: API Gateway

tags:
  - name: submissions
    description: Работа с загруженными работами
  - name: reports
    description: Отчёты о плагиате
  - name: health
    description: Проверка состояния сервисов

paths:
  /health:
    get:
      tags: [health]
      summary: Проверка здоровья API Gateway
      responses:
        '200':
          description: Сервис работает
          content:
            application/json:
              schema:
                type: object
                properties:
                  status:
                    type: string
                    example: ok
                  service:
                    type: string
                    example: api-gateway

  /api/submissions:
    post:
      tags: [submissions]
      summary: Загрузить работу на проверку
      description: Загружает файл и автоматически запускает проверку на плагиат
      requestBody:
        required: true
        content:
          application/json:
            schema:
              type: object
              required: [student_name, task_id, filename, content]
              properties:
                student_name:
                  type: string
                  example: "Иванов Иван"
                task_id:
                  type: string
                  example: "homework-3"
                filename:
                  type: string
                  example: "solution.cpp"
                content:
                  type: string
                  example: "int main() { return 0; }"
      responses:
        '201':
          description: Работа загружена и проанализирована
        '400':
          description: Ошибка валидации

  /api/submissions/{id}:
    get:
      tags: [submissions]
      summary: Получить информацию о работе
      parameters:
        - name: id
          in: path
          required: true
          schema:
            type: integer
      responses:
        '200':
          description: Информация о работе
        '404':
          description: Работа не найдена

  /api/submissions/{id}/report:
    get:
      tags: [reports]
      summary: Получить отчёт о плагиате
      parameters:
        - name: id
          in: path
          required: true
          schema:
            type: integer
      responses:
        '200':
          description: Отчёт о проверке
        '404':
          description: Отчёт не найден

  /api/submissions/{id}/wordcloud:
    get:
      tags: [reports]
      summary: Получить облако слов (Word Cloud)
      description: Возвращает URL для визуализации работы в виде облака слов
      parameters:
        - name: id
          in: path
          required: true
          schema:
            type: integer
      responses:
        '200':
          description: URL облака слов

  /api/tasks/{task_id}/reports:
    get:
      tags: [reports]
      summary: Получить все отчёты по заданию
      parameters:
        - name: task_id
          in: path
          required: true
          schema:
            type: string
          example: homework-3
      responses:
        '200':
          description: Сводка по заданию
)";
    res.status = 200;
    res.set_content(yaml, "text/yaml");
}

}