#include "management.hpp"

namespace ak_acl::handlers::management {

void EmployeeControlGet(
    const drogon::HttpRequestPtr &request,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
  drogon::HttpViewData data;
  callback(drogon::HttpResponse::newHttpViewResponse(
      "ManagementEmployeeControl", data));
}

void EmployeeControlListGet(
    const drogon::HttpRequestPtr &request,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
  auto client = drogon::app().getDbClient();

  if (client == nullptr) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k500InternalServerError);
    response->setBody("database client unavailable");
    callback(response);
    return;
  }

  try {
    const int pageSize = 15;
    const auto pageParam = request->getParameter("page");
    const int page = pageParam.empty() ? 1 : std::stoi(pageParam);
    const int offset = (page - 1) * pageSize;
    const auto searchParam = request->getParameter("search");

    int totalEmployees;
    drogon::orm::Result result{nullptr};

    if (searchParam.empty()) {
      const auto countResult = client->execSqlSync(
          "SELECT COUNT(*) as total FROM inventory_employee");
      totalEmployees = countResult[0]["total"].as<int>();
      result = client->execSqlSync(
          "SELECT id, name, plate, document_type, document_number "
          "FROM inventory_employee ORDER BY id LIMIT ? OFFSET ?",
          pageSize, offset);
    } else {
      const auto searchPattern = "%" + searchParam + "%";
      const auto countResult = client->execSqlSync(
          "SELECT COUNT(*) as total FROM inventory_employee "
          "WHERE name LIKE ? OR plate LIKE ? OR document_number LIKE ?",
          searchPattern, searchPattern, searchPattern);
      totalEmployees = countResult[0]["total"].as<int>();
      result = client->execSqlSync(
          "SELECT id, name, plate, document_type, document_number "
          "FROM inventory_employee "
          "WHERE name LIKE ? OR plate LIKE ? OR document_number LIKE ? "
          "ORDER BY id LIMIT ? OFFSET ?",
          searchPattern, searchPattern, searchPattern, pageSize, offset);
    }

    const int totalPages = (totalEmployees + pageSize - 1) / pageSize;

    drogon::HttpViewData data;

    std::vector<std::map<std::string, std::string>> employees;
    for (const auto &row : result) {
      std::map<std::string, std::string> employee;
      employee["id"] = std::to_string(row["id"].as<int>());
      employee["name"] = row["name"].as<std::string>();
      employee["plate"] = row["plate"].as<std::string>();
      employee["document_type"] = row["document_type"].as<std::string>();
      employee["document_number"] = row["document_number"].as<std::string>();
      employees.push_back(employee);
    }

    data.insert("employees", employees);
    data.insert("currentPage", page);
    data.insert("totalPages", totalPages);
    data.insert("pageSize", pageSize);
    data.insert("totalEmployees", totalEmployees);
    data.insertAsString("search", searchParam);

    callback(drogon::HttpResponse::newHttpViewResponse(
        "ManagementEmployeeControlList", data));
  } catch (const std::exception &e) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k500InternalServerError);
    response->setBody(e.what());
    callback(response);
  }
}

void EmployeeControlCreateGet(
    const drogon::HttpRequestPtr &request,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
  drogon::HttpViewData data;
  callback(drogon::HttpResponse::newHttpViewResponse(
      "ManagementEmployeeControlCreate", data));
}

void EmployeeControlCreatePost(
    const drogon::HttpRequestPtr &request,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
  auto client = drogon::app().getDbClient();

  if (client == nullptr) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k500InternalServerError);
    response->setBody("database client unavailable");
    callback(response);
    return;
  }

  const auto name = request->getParameter("name");
  const auto plate = request->getParameter("plate");
  const auto documentType = request->getParameter("document_type");
  const auto documentNumber = request->getParameter("document_number");

  if (name.empty() || plate.empty() || documentType.empty() ||
      documentNumber.empty()) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k400BadRequest);
    response->setBody("missing required fields");
    callback(response);
    return;
  }

  try {
    client->execSqlSync("INSERT INTO inventory_employee (name, plate, "
                        "document_type, document_number) VALUES (?, ?, ?, ?)",
                        name, plate, documentType, documentNumber);

    EmployeeControlListGet(request, std::move(callback));
  } catch (const std::exception &e) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k500InternalServerError);
    response->setBody(e.what());
    callback(response);
  }
}

} // namespace ak_acl::handlers::management
