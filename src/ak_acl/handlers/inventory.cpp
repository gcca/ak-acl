#include "inventory.hpp"

namespace ak_acl::handlers::inventory {

void EmployeeList(
    const drogon::HttpRequestPtr &request,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
  const auto page = std::max(1, request->getParameter("page").empty()
                                    ? 1
                                    : std::stoi(request->getParameter("page")));
  const auto limit = 100;
  const auto offset = (page - 1) * limit;

  auto client = drogon::app().getDbClient();

  if (client == nullptr) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k500InternalServerError);
    response->setBody("database client unavailable");
    callback(response);
    return;
  }

  try {
    const auto countResult =
        client->execSqlSync("SELECT COUNT(*) AS total FROM inventory_employee");
    const auto total = countResult[0]["total"].as<long long>();

    const auto result =
        client->execSqlSync("SELECT name, plate FROM inventory_employee ORDER "
                            "BY id LIMIT ? OFFSET ?",
                            limit, offset);

    Json::Value json;
    json["data"] = Json::arrayValue;

    for (const auto &row : result) {
      Json::Value employee;
      employee["name"] = row["name"].as<std::string>();
      employee["plate"] = row["plate"].as<std::string>();
      json["data"].append(employee);
    }

    json["meta"]["total"] = static_cast<Json::Int64>(total);
    json["meta"]["page"] = page;
    json["meta"]["limit"] = limit;

    callback(drogon::HttpResponse::newHttpJsonResponse(json));
  } catch (const std::exception &e) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k500InternalServerError);
    response->setBody(e.what());
    callback(response);
  }
}

} // namespace ak_acl::handlers::inventory
