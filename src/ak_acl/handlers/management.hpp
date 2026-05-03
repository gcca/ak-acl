#pragma once

#include <drogon/drogon.h>

namespace ak_acl::handlers::management {

void EmployeeControlGet(
    const drogon::HttpRequestPtr &request,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback);

void EmployeeControlListGet(
    const drogon::HttpRequestPtr &request,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback);

void EmployeeControlCreateGet(
    const drogon::HttpRequestPtr &request,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback);

void EmployeeControlCreatePost(
    const drogon::HttpRequestPtr &request,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback);

} // namespace ak_acl::handlers::management
