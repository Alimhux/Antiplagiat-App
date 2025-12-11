#ifndef ANALYSISHANDLERS_H
#define ANALYSISHANDLERS_H

#include "../service/analysisservice.h"
#include "httplib.h"

namespace handlers {

class AnalysisHandlers {
public:
  explicit AnalysisHandlers(service::AnalysisService& analysisService);

  void registerRoutes(httplib::Server& server);

private:
  void handleHealth(const httplib::Request& req, httplib::Response& res);
  void handleAnalyze(const httplib::Request& req, httplib::Response& res);
  void handleGetReport(const httplib::Request& req, httplib::Response& res);
  void handleGetTaskReports(const httplib::Request& req, httplib::Response& res);

  void sendError(httplib::Response& res, int status, const std::string& message);
  void sendJson(httplib::Response& res, int status, const std::string& json);

  service::AnalysisService& analysisService_;
};

}



#endif //ANALYSISHANDLERS_H
