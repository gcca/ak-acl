#include "utils.hpp"

#include <array>
#include <cctype>
#include <string_view>

#include "ak_acl/utils.hpp"

namespace ak_acl::handlers::auth::utils {

std::string Base64Decode(const std::string &input) {
  static constexpr std::string_view alphabet =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  std::array<int, 256> decodeMap{};
  decodeMap.fill(-1);
  for (size_t i = 0; i < alphabet.size(); ++i) {
    decodeMap[static_cast<unsigned char>(alphabet[i])] = static_cast<int>(i);
  }

  std::string output;
  int val = 0;
  int valb = -8;
  for (unsigned char c : input) {
    if (std::isspace(c) != 0 || c == '=') {
      continue;
    }
    const int decoded = decodeMap[c];
    if (decoded < 0) {
      return {};
    }
    val = (val << 6) + decoded;
    valb += 6;
    if (valb >= 0) {
      output.push_back(static_cast<char>((val >> valb) & 0xFF));
      valb -= 8;
    }
  }
  return output;
}

bool Authenticate(const std::shared_ptr<drogon::orm::DbClient> &client,
                  const std::string &username, const std::string &password) {
  const auto result = client->execSqlSync(
      "SELECT username, password FROM auth_user WHERE username = ?", username);

  if (result.empty()) {
    return false;
  }

  const auto row = result[0];
  const auto hashedPassword = ak_acl::utils::Hmac(username + password);

  return row["password"].as<std::string>() == hashedPassword;
}

std::string LogIn(const std::shared_ptr<drogon::orm::DbClient> &client,
                  const std::string &username) {
  const auto sessionKey = ak_acl::utils::RandomStr(40);
  const auto createdAt = ak_acl::utils::NowTmUtc();
  const auto expiresAt = ak_acl::utils::AddSecondsToTm(createdAt, 3600);

  client->execSqlSync(
      "INSERT INTO auth_session (key, username, expires_at, created_at) "
      "VALUES (?, ?, ?, ?)",
      sessionKey, username, ak_acl::utils::ToIso8601Utc(expiresAt),
      ak_acl::utils::ToIso8601Utc(createdAt));

  return sessionKey;
}

} // namespace ak_acl::handlers::auth::utils
