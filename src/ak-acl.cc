#include <drogon/drogon.h>

#include "ak_acl/handlers/auth.hpp"
#include "ak_acl/handlers/inventory.hpp"
#include "ak_acl/handlers/management.hpp"
#include "ak_acl/handlers/middlewares.hpp"

int main(int argc, char *argv[]) {
  drogon::orm::Sqlite3Config config;
  config.connectionNumber = 1;
  config.filename = "./ak-acl.db";
  config.name = "default";
  config.timeout = -1.0;

  drogon::app().addDbClient(config);

  if (!drogon::app().areAllDbClientsAvailable()) {
    return EXIT_FAILURE;
  }

  drogon::app().registerHandler(
      "/ak-acl/o/token", &ak_acl::handlers::auth::OTokenPost, {drogon::Post});

  drogon::app().registerHandler(
      "/ak-acl/auth/signin", &ak_acl::handlers::auth::SignInGet, {drogon::Get});

  drogon::app().registerHandler("/ak-acl/auth/signin",
                                &ak_acl::handlers::auth::SignInPost,
                                {drogon::Post});

  drogon::app().registerHandler(
      "/ak-acl/inventory/api/v1/employees",
      &ak_acl::handlers::inventory::EmployeeList,
      {drogon::Get,
       drogon::internal::HttpConstraint(
           ak_acl::handlers::middlewares::TokenRequired::classTypeName())});

  drogon::app().registerHandler(
      "/ak-acl/management/employee/control",
      &ak_acl::handlers::management::EmployeeControlGet,
      {drogon::Get,
       drogon::internal::HttpConstraint(
           ak_acl::handlers::middlewares::LogInRequired::classTypeName())});

  drogon::app().registerHandler(
      "/ak-acl/management/employee/control/list",
      &ak_acl::handlers::management::EmployeeControlListGet,
      {drogon::Get,
       drogon::internal::HttpConstraint(
           ak_acl::handlers::middlewares::LogInRequired::classTypeName())});

  drogon::app().registerHandler(
      "/ak-acl/management/employee/control/create",
      &ak_acl::handlers::management::EmployeeControlCreateGet,
      {drogon::Get,
       drogon::internal::HttpConstraint(
           ak_acl::handlers::middlewares::LogInRequired::classTypeName())});

  drogon::app().registerHandler(
      "/ak-acl/management/employee/control/create",
      &ak_acl::handlers::management::EmployeeControlCreatePost,
      {drogon::Post,
       drogon::internal::HttpConstraint(
           ak_acl::handlers::middlewares::LogInRequired::classTypeName())});

  drogon::app().registerHandler(
      "/ak-acl",
      [](const drogon::HttpRequestPtr &,
         std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
        drogon::HttpViewData data;
        data.insertAsString("message", "");
        callback(drogon::HttpResponse::newHttpViewResponse("AuthSignIn", data));
      },
      {drogon::Get});

  drogon::app().addListener("0.0.0.0", 8000);
  drogon::app().run();

  return EXIT_SUCCESS;
}
