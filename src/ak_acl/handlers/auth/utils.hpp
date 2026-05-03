#pragma once

#include <drogon/drogon.h>
#include <memory>
#include <string>

namespace ak_acl::handlers::auth::utils {

[[nodiscard]] std::string Base64Decode(const std::string &input);

[[nodiscard]] bool
Authenticate(const std::shared_ptr<drogon::orm::DbClient> &client,
             const std::string &username, const std::string &password);

[[nodiscard]] std::string
LogIn(const std::shared_ptr<drogon::orm::DbClient> &client,
      const std::string &username);

} // namespace ak_acl::handlers::auth::utils
