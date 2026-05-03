#pragma once

#include <drogon/drogon.h>

namespace ak_acl::handlers::middlewares {

class TokenRequired : public drogon::HttpMiddleware<TokenRequired> {
public:
  void invoke(const drogon::HttpRequestPtr &request,
              drogon::MiddlewareNextCallback &&nextCb,
              drogon::MiddlewareCallback &&mcb) override;
};

class LogInRequired : public drogon::HttpMiddleware<LogInRequired> {
public:
  void invoke(const drogon::HttpRequestPtr &request,
              drogon::MiddlewareNextCallback &&nextCb,
              drogon::MiddlewareCallback &&mcb) override;
};

} // namespace ak_acl::handlers::middlewares
