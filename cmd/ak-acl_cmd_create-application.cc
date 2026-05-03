#include <sqlite3.h>

#include <print>

#include "ak_acl/utils.hpp"

namespace {

void ThrowSqliteError(sqlite3 *db, int code, const char *context) {
  throw std::runtime_error(
      std::format("{}: {} ({})", context, sqlite3_errmsg(db), code));
}

} // namespace

int main() {
  sqlite3 *db = nullptr;
  if (sqlite3_open("./ak-acl.db", &db) != SQLITE_OK) {
    const std::string message = db ? sqlite3_errmsg(db) : "unknown error";
    if (db != nullptr) {
      sqlite3_close(db);
    }
    throw std::runtime_error("failed to open ./ak-acl.db: " + message);
  }

  const auto clientId = ak_acl::utils::RandomStr(40);
  const auto clientSecret = ak_acl::utils::RandomStr(48);
  std::println("client_id={}", clientId);
  std::println("client_secret={}", clientSecret);
  const auto createdAt = ak_acl::utils::NowTmUtc();
  const auto hashedSecret = ak_acl::utils::Hmac(clientId + clientSecret);
  const char *sql =
      "INSERT INTO auth_application "
      "(client_id, client_secret, grant_type, scope, expires_in, created_at) "
      "VALUES (?, ?, 'client_credentials', 'read:inventory-employee', 900, ?)";

  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    ThrowSqliteError(db, sqlite3_errcode(db), "failed to prepare insert");
  }

  if (sqlite3_bind_text(stmt, 1, clientId.c_str(), -1, SQLITE_TRANSIENT) !=
          SQLITE_OK ||
      sqlite3_bind_text(stmt, 2, hashedSecret.c_str(), -1, SQLITE_TRANSIENT) !=
          SQLITE_OK ||
      sqlite3_bind_text(stmt, 3, ak_acl::utils::ToIso8601Utc(createdAt).c_str(),
                        -1, SQLITE_TRANSIENT) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    ThrowSqliteError(db, sqlite3_errcode(db), "failed to bind parameters");
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    sqlite3_finalize(stmt);
    ThrowSqliteError(db, sqlite3_errcode(db), "failed to insert application");
  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  std::println("grant_type=client_credentials");
  std::println("scope=read:inventory-employee");
  return 0;
}
