#include "filehandlers.h"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

namespace handlers {

FileHandlers::FileHandlers(service::FileService& fileService)
    : fileService_(fileService)
{}

void FileHandlers::registerRoutes(httplib::Server& server) {
    server.Get("/health", [this](const httplib::Request& req, httplib::Response& res) {
        handleHealth(req, res);
    });

    server.Post("/files", [this](const httplib::Request& req, httplib::Response& res) {
        handleUpload(req, res);
    });

    server.Get(R"(/files/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetFile(req, res);
    });

    server.Get(R"(/files/(\d+)/content)", [this](const httplib::Request& req, httplib::Response& res) {
        handleDownload(req, res);
    });

    server.Get(R"(/files/hash/([a-f0-9]+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleFindByHash(req, res);
    });
}

void FileHandlers::handleHealth(const httplib::Request& /*req*/, httplib::Response& res) {
    json response;
    response["status"] = "ok";
    response["service"] = "file-storing-service";
    sendJson(res, 200, response.dump());
}

void FileHandlers::handleUpload(const httplib::Request& req, httplib::Response& res) {
    std::cout << "[FileHandlers] POST /files" << std::endl;

    try {
        // Парсим JSON
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

        // Формируем запрос к сервису
        service::UploadRequest uploadReq;
        uploadReq.studentName = body.value("student_name", "unknown");
        uploadReq.taskId = body.value("task_id", "unknown");
        uploadReq.filename = body.value("filename", "uploaded_file.txt");
        uploadReq.content = body.value("content", "");

        if (uploadReq.content.empty()) {
            sendError(res, 400, "Field 'content' is required and cannot be empty");
            return;
        }

        // Загружаем файл
        auto result = fileService_.uploadFile(uploadReq);

        // Формируем ответ
        json response;
        response["id"] = result.id;
        response["student_name"] = result.studentName;
        response["task_id"] = result.taskId;
        response["filename"] = result.filename;
        response["file_hash"] = result.fileHash;
        response["file_size"] = result.fileSize;
        response["message"] = "File uploaded successfully";

        sendJson(res, 201, response.dump());

    } catch (const std::invalid_argument& e) {
        sendError(res, 400, e.what());
    } catch (const std::exception& e) {
        std::cerr << "[FileHandlers] Error in handleUpload: " << e.what() << std::endl;
        sendError(res, 500, std::string("Server error: ") + e.what());
    }
}

void FileHandlers::handleGetFile(const httplib::Request& req, httplib::Response& res) {
    try {
        int id = std::stoi(req.matches[1]);
        std::cout << "[FileHandlers] GET /files/" << id << std::endl;

        auto submission = fileService_.getSubmission(id);
        if (!submission) {
            sendError(res, 404, "Submission not found");
            return;
        }

        json response;
        response["id"] = submission->id;
        response["student_name"] = submission->studentName;
        response["task_id"] = submission->taskId;
        response["filename"] = submission->filename;
        response["file_hash"] = submission->fileHash;
        response["file_size"] = submission->fileSize;
        response["uploaded_at"] = submission->uploadedAt;

        sendJson(res, 200, response.dump());

    } catch (const std::exception& e) {
        std::cerr << "[FileHandlers] Error in handleGetFile: " << e.what() << std::endl;
        sendError(res, 500, std::string("Server error: ") + e.what());
    }
}

void FileHandlers::handleDownload(const httplib::Request& req, httplib::Response& res) {
    try {
        int id = std::stoi(req.matches[1]);
        std::cout << "[FileHandlers] GET /files/" << id << "/content" << std::endl;

        auto submission = fileService_.getSubmission(id);
        if (!submission) {
            sendError(res, 404, "File not found");
            return;
        }

        std::string content = fileService_.getFileContent(id);

        res.set_header("Content-Disposition", "attachment; filename=\"" + submission->filename + "\"");
        res.set_content(content, "application/octet-stream");

    } catch (const std::runtime_error& e) {
        sendError(res, 404, e.what());
    } catch (const std::exception& e) {
        std::cerr << "[FileHandlers] Error in handleDownload: " << e.what() << std::endl;
        sendError(res, 500, std::string("Server error: ") + e.what());
    }
}

void FileHandlers::handleFindByHash(const httplib::Request& req, httplib::Response& res) {
    try {
        std::string hash = req.matches[1];
        std::cout << "[FileHandlers] GET /files/hash/" << hash << std::endl;

        auto submissions = fileService_.findByHash(hash);

        json files = json::array();
        for (const auto& s : submissions) {
            json file;
            file["id"] = s.id;
            file["student_name"] = s.studentName;
            file["task_id"] = s.taskId;
            file["filename"] = s.filename;
            file["uploaded_at"] = s.uploadedAt;
            files.push_back(file);
        }

        json response;
        response["files"] = files;
        response["count"] = files.size();

        sendJson(res, 200, response.dump());

    } catch (const std::exception& e) {
        std::cerr << "[FileHandlers] Error in handleFindByHash: " << e.what() << std::endl;
        sendError(res, 500, std::string("Server error: ") + e.what());
    }
}

void FileHandlers::sendError(httplib::Response& res, int status, const std::string& message) {
    json error;
    error["error"] = message;
    res.status = status;
    res.set_content(error.dump(), "application/json");
}

void FileHandlers::sendJson(httplib::Response& res, int status, const std::string& jsonStr) {
    res.status = status;
    res.set_content(jsonStr, "application/json");
}

}