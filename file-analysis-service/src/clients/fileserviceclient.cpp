#include "fileserviceclient.h"
#include "httplib.h"
#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

namespace clients {

FileServiceClient::FileServiceClient(const std::string& baseUrl) {
    auto [h, p] = parseUrl(baseUrl);
    host_ = h;
    port_ = p;
}

std::vector<FileInfo> FileServiceClient::findByHash(const std::string& hash) {
    httplib::Client client(host_, port_);
    client.set_connection_timeout(5);
    client.set_read_timeout(5);

    auto response = client.Get(("/files/hash/" + hash).c_str());

    std::vector<FileInfo> result;

    if (!response) {
        std::cerr << "[FileServiceClient] Failed to connect to file service" << std::endl;
        return result;
    }

    if (response->status != 200) {
        std::cerr << "[FileServiceClient] Error response: " << response->status << std::endl;
        return result;
    }

    try {
        auto data = json::parse(response->body);
        auto files = data["files"];

        for (const auto& file : files) {
            FileInfo info;
            info.id = file["id"];
            info.studentName = file["student_name"];
            info.taskId = file["task_id"];
            info.filename = file["filename"];
            info.uploadedAt = file["uploaded_at"];
            result.push_back(info);
        }
    } catch (const std::exception& e) {
        std::cerr << "[FileServiceClient] Failed to parse response: " << e.what() << std::endl;
    }

    return result;
}

std::string FileServiceClient::getFileContent(int submissionId) {
    httplib::Client client(host_, port_);
    client.set_connection_timeout(5);
    client.set_read_timeout(10);

    std::string path = "/files/" + std::to_string(submissionId) + "/content";
    auto response = client.Get(path.c_str());

    if (!response) {
        std::cerr << "[FileServiceClient] Failed to get file content" << std::endl;
        return "";
    }

    if (response->status != 200) {
        std::cerr << "[FileServiceClient] Error getting content: " << response->status << std::endl;
        return "";
    }

    return response->body;
}

std::pair<std::string, int> FileServiceClient::parseUrl(const std::string& url) {
    std::string host = "localhost";
    int port = 80;

    std::string cleanUrl = url;
    if (cleanUrl.substr(0, 7) == "http://") {
        cleanUrl = cleanUrl.substr(7);
    }

    size_t colonPos = cleanUrl.find(':');
    if (colonPos != std::string::npos) {
        host = cleanUrl.substr(0, colonPos);
        port = std::stoi(cleanUrl.substr(colonPos + 1));
    } else {
        host = cleanUrl;
    }

    return {host, port};
}

}