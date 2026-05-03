#include "middlewares.hpp"

#include "ak_acl/utils.hpp"

namespace ak_acl::handlers::middlewares {

void TokenRequired::invoke(const drogon::HttpRequestPtr &request,
                           drogon::MiddlewareNextCallback &&nextCb,
                           drogon::MiddlewareCallback &&mcb) {
  const auto auth = request->getHeader("authorization");

  if (auth.rfind("Bearer ", 0) != 0) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k401Unauthorized);
    response->addHeader("WWW-Authenticate", "Bearer");
    response->setBody("missing bearer token");
    mcb(response);
    return;
  }

  const auto accessToken = auth.substr(7);
  auto client = drogon::app().getDbClient();

  if (client == nullptr) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k500InternalServerError);
    response->setBody("database client unavailable");
    mcb(response);
    return;
  }

  try {
    const auto now = ak_acl::utils::ToIso8601Utc(ak_acl::utils::NowTmUtc());
    const auto result = client->execSqlSync(
        "SELECT access_token, expires_at, revoked FROM auth_token "
        "WHERE access_token = ? AND revoked = 0 AND expires_at > ?",
        accessToken, now);

    if (result.empty()) {
      auto response = drogon::HttpResponse::newHttpResponse();
      response->setStatusCode(drogon::k401Unauthorized);
      response->addHeader("WWW-Authenticate", "Bearer");
      response->setBody("invalid or expired bearer token");
      mcb(response);
      return;
    }

    nextCb(std::move(mcb));
  } catch (const std::exception &e) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k500InternalServerError);
    response->setBody(e.what());
    mcb(response);
  }
}

void LogInRequired::invoke(const drogon::HttpRequestPtr &request,
                           drogon::MiddlewareNextCallback &&nextCb,
                           drogon::MiddlewareCallback &&mcb) {
  const auto sessionCookie = request->getCookie("session");

  if (sessionCookie.empty()) {
    mcb(drogon::HttpResponse::newRedirectionResponse("/ak-acl",
                                                     drogon::k303SeeOther));
    return;
  }

  auto client = drogon::app().getDbClient();

  if (client == nullptr) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k500InternalServerError);
    response->setBody("database client unavailable");
    mcb(response);
    return;
  }

  try {
    const auto now = ak_acl::utils::ToIso8601Utc(ak_acl::utils::NowTmUtc());
    const auto result = client->execSqlSync(
        "SELECT key, username, expires_at FROM auth_session "
        "WHERE key = ? AND expires_at > ?",
        sessionCookie, now);

    if (result.empty()) {
      mcb(drogon::HttpResponse::newRedirectionResponse("/ak-acl",
                                                       drogon::k303SeeOther));
      return;
    }

    nextCb(std::move(mcb));
  } catch (const std::exception &e) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k500InternalServerError);
    response->setBody(e.what());
    mcb(response);
  }
}

} // namespace ak_acl::handlers::middlewares
