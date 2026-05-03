#include "auth.hpp"

#include "ak_acl/handlers/auth/utils.hpp"
#include "ak_acl/utils.hpp"

namespace ak_acl::handlers::auth {

void OTokenPost(
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

  const auto auth = request->getHeader("authorization");

  if (auth.rfind("Basic ", 0) != 0) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k401Unauthorized);
    response->addHeader("WWW-Authenticate", "Basic realm=\"ak-acl\"");
    response->setBody("missing basic auth");
    callback(response);
    return;
  }

  const auto decoded = utils::Base64Decode(auth.substr(6));
  const auto separator = decoded.find(':');
  if (separator == std::string::npos) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k400BadRequest);
    response->setBody("invalid authorization header");
    callback(response);
    return;
  }

  const auto clientId = decoded.substr(0, separator);
  const auto clientSecret = decoded.substr(separator + 1);
  const auto grantType = request->getParameter("grant_type");
  const auto scope = request->getParameter("scope");

  if (grantType != "client_credentials") {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k400BadRequest);
    response->setBody("unsupported grant_type");
    callback(response);
    return;
  }

  try {
    const auto result = client->execSqlSync(
        "SELECT client_id, client_secret, grant_type, scope, expires_in FROM "
        "auth_application WHERE client_id = ?",
        clientId);

    if (result.empty()) {
      auto response = drogon::HttpResponse::newHttpResponse();
      response->setStatusCode(drogon::k401Unauthorized);
      response->setBody("invalid client credentials");
      callback(response);
      return;
    }

    const auto row = result[0];
    const auto hashedClientSecret =
        ak_acl::utils::Hmac(clientId + clientSecret);

    if (row["client_secret"].as<std::string>() != hashedClientSecret ||
        row["grant_type"].as<std::string>() != grantType) {
      auto response = drogon::HttpResponse::newHttpResponse();
      response->setStatusCode(drogon::k401Unauthorized);
      response->setBody("invalid client credentials");
      callback(response);
      return;
    }

    if (!scope.empty() &&
        row["scope"].as<std::string>().find(scope) == std::string::npos) {
      auto response = drogon::HttpResponse::newHttpResponse();
      response->setStatusCode(drogon::k400BadRequest);
      response->setBody("invalid scope");
      callback(response);
      return;
    }

    const auto accessToken = ak_acl::utils::RandomStr(48);
    const auto expiresIn = row["expires_in"].as<int>();
    const auto createdAt = ak_acl::utils::NowTmUtc();
    const auto expiresAt = ak_acl::utils::AddSecondsToTm(createdAt, expiresIn);
    const auto tokenScope =
        scope.empty() ? row["scope"].as<std::string>() : scope;

    client->execSqlSync(
        "INSERT INTO auth_token (access_token, expires_at, scope, revoked, "
        "created_at) VALUES (?, ?, ?, 0, ?)",
        accessToken, ak_acl::utils::ToIso8601Utc(expiresAt), tokenScope,
        ak_acl::utils::ToIso8601Utc(createdAt));

    Json::Value json;
    json["access_token"] = accessToken;
    json["token_type"] = "Bearer";
    json["expires_in"] = expiresIn;
    json["scope"] = tokenScope;
    callback(drogon::HttpResponse::newHttpJsonResponse(json));
  } catch (const std::exception &e) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k500InternalServerError);
    response->setBody(e.what());
    callback(response);
  }
}

void SignInGet(
    const drogon::HttpRequestPtr &request,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
  drogon::HttpViewData data;
  data.insertAsString("message", "");
  callback(drogon::HttpResponse::newHttpViewResponse("AuthSignIn", data));
}

void SignInPost(
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

  const auto username = request->getParameter("username");
  const auto password = request->getParameter("password");
  const auto redirectTo = request->getParameter("to");

  if (username.empty() || password.empty() || redirectTo.empty()) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k400BadRequest);
    response->setBody("missing username, password, or to");
    callback(response);
    return;
  }

  try {
    if (!utils::Authenticate(client, username, password)) {
      drogon::HttpViewData data;
      data.insertAsString("message",
                          "Invalid username or password. Please try again.");
      callback(drogon::HttpResponse::newHttpViewResponse("AuthSignIn", data));
      return;
    }

    const auto sessionKey = utils::LogIn(client, username);

    if (sessionKey.empty()) {
      auto response = drogon::HttpResponse::newHttpResponse();
      response->setStatusCode(drogon::k500InternalServerError);
      response->setBody("failed to create session");
      callback(response);
      return;
    }

    auto response = drogon::HttpResponse::newRedirectionResponse(
        redirectTo, drogon::k303SeeOther);

    drogon::Cookie sessionCookie("session", sessionKey);
    sessionCookie.setPath("/");
    sessionCookie.setHttpOnly(true);
    sessionCookie.setSameSite(drogon::Cookie::SameSite::kLax);
    response->addCookie(sessionCookie);

    callback(response);
  } catch (const std::exception &e) {
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k500InternalServerError);
    response->setBody(e.what());
    callback(response);
  }
}

} // namespace ak_acl::handlers::auth
