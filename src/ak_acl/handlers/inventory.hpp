#pragma once

#include <drogon/drogon.h>

namespace ak_acl::handlers::inventory {

void EmployeeList(
    const drogon::HttpRequestPtr &request,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback);

} // namespace ak_acl::handlers::inventory
