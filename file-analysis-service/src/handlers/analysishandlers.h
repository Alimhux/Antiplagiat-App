#ifndef ANALYSISHANDLERS_H
#define ANALYSISHANDLERS_H

#include "../service/analysisservice.h"
#include "../clients/fileserviceclient.h"
#include "httplib.h"

namespace handlers {

class AnalysisHandlers {
public:
  AnalysisHandlers(service::AnalysisService& analysisService,
                   clients::FileServiceClient& fileClient);

  void registerRoutes(httplib::Server& server);

private:
  void handleHealth(const httplib::Request& req, httplib::Response& res);
  void handleAnalyze(const httplib::Request& req, httplib::Response& res);
  void handleGetReport(const httplib::Request& req, httplib::Response& res);
  void handleGetTaskReports(const httplib::Request& req, httplib::Response& res);

  // Word Cloud endpoint
  void handleGetWordCloud(const httplib::Request& req, httplib::Response& res);

  void sendError(httplib::Response& res, int status, const std::string& message);
  void sendJson(httplib::Response& res, int status, const std::string& json);

  service::AnalysisService& analysisService_;
  clients::FileServiceClient& fileClient_;
};

}

#endif //ANALYSISHANDLERS_H