#include "fileservice.h"
#include "../utils/hashutils.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace service {

FileService::FileService(repository::FileRepository& repo, const std::string& uploadPath)
    : repo_(repo)
    , uploadPath_(uploadPath)
{}

UploadResult FileService::uploadFile(const UploadRequest& request) {
    // Валидация
    if (request.content.empty()) {
        throw std::invalid_argument("Content cannot be empty");
    }
    if (request.studentName.empty()) {
        throw std::invalid_argument("Student name cannot be empty");
    }
    if (request.taskId.empty()) {
        throw std::invalid_argument("Task ID cannot be empty");
    }

    // Вычисляем хэш
    std::string fileHash = utils::HashUtils::sha256(request.content);

    // Формируем путь для сохранения
    std::string filePath = uploadPath_ + fileHash + "_" + request.filename;

    // Сохраняем на диск
    saveToFile(filePath, request.content);

    // Сохраняем в БД
    models::Submission submission;
    submission.studentName = request.studentName;
    submission.taskId = request.taskId;
    submission.filename = request.filename;
    submission.filePath = filePath;
    submission.fileHash = fileHash;
    submission.fileSize = static_cast<int64_t>(request.content.size());

    int id = repo_.create(submission);

    // Формируем результат
    UploadResult result;
    result.id = id;
    result.studentName = request.studentName;
    result.taskId = request.taskId;
    result.filename = request.filename;
    result.fileHash = fileHash;
    result.fileSize = submission.fileSize;

    return result;
}

std::optional<models::Submission> FileService::getSubmission(int id) {
    return repo_.findById(id);
}

std::string FileService::getFileContent(int id) {
    auto submission = repo_.findById(id);
    if (!submission) {
        throw std::runtime_error("Submission not found");
    }

    return readFromFile(submission->filePath);
}

std::vector<models::Submission> FileService::findByHash(const std::string& hash) {
    return repo_.findByHash(hash);
}

std::vector<models::Submission> FileService::findByTaskId(const std::string& taskId) {
    return repo_.findByTaskId(taskId);
}

void FileService::saveToFile(const std::string& path, const std::string& content) {
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Failed to save file to disk: " + path);
    }
    ofs.write(content.c_str(), content.size());
    ofs.close();
}

std::string FileService::readFromFile(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("File not found on disk: " + path);
    }

    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return buffer.str();
}

}