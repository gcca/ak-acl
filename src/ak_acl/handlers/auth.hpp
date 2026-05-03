#pragma once

#include <drogon/drogon.h>
#include <string>

namespace ak_acl::handlers::auth {

[[nodiscard]] std::string Base64Decode(const std::string &input);

void OTokenPost(
    const drogon::HttpRequestPtr &request,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback);

void SignInGet(const drogon::HttpRequestPtr &request,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback);

void SignInPost(
    const drogon::HttpRequestPtr &request,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback);

} // namespace ak_acl::handlers::auth
