#ifndef GATEWAYHANDLERS_H
#define GATEWAYHANDLERS_H

#include "../clients/serviceclient.h"
#include "httplib.h"

namespace handlers {

class GatewayHandlers {
public:
  GatewayHandlers(clients::ServiceClient& fileService,
                  clients::ServiceClient& analysisService);

  void registerRoutes(httplib::Server& server);

private:
  // Health & Info
  void handleHealth(const httplib::Request& req, httplib::Response& res);
  void handleRoot(const httplib::Request& req, httplib::Response& res);

  // Submissions
  void handleCreateSubmission(const httplib::Request& req, httplib::Response& res);
  void handleGetSubmission(const httplib::Request& req, httplib::Response& res);
  void handleGetSubmissionReport(const httplib::Request& req, httplib::Response& res);

  // Tasks
  void handleGetTaskReports(const httplib::Request& req, httplib::Response& res);

  // Utils
  void sendError(httplib::Response& res, int status, const std::string& message);
  void sendJson(httplib::Response& res, int status, const std::string& json);

  clients::ServiceClient& fileService_;
  clients::ServiceClient& analysisService_;
};

}



#endif //GATEWAYHANDLERS_H
