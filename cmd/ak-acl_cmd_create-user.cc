#include <sqlite3.h>

#include <print>

#define CLI11_HAS_CODECVT 0
#include "CLI11.hpp"
#include "ak_acl/utils.hpp"

namespace {

void ThrowSqliteError(sqlite3 *db, int code, const char *context) {
  throw std::runtime_error(
      std::format("{}: {} ({})", context, sqlite3_errmsg(db), code));
}

} // namespace

int main(int argc, char **argv) {
  CLI::App app{"Create a new user in the ak-acl database"};

  std::string username;
  std::string password;

  app.add_option("-u,--username", username, "Username for the new user")
      ->required();
  app.add_option("-p,--password", password, "Password for the new user")
      ->required();

  CLI11_PARSE(app, argc, argv);

  sqlite3 *db = nullptr;
  if (sqlite3_open("./ak-acl.db", &db) != SQLITE_OK) {
    const std::string message = db ? sqlite3_errmsg(db) : "unknown error";
    if (db != nullptr) {
      sqlite3_close(db);
    }
    throw std::runtime_error("failed to open ./ak-acl.db: " + message);
  }

  const auto createdAt = ak_acl::utils::NowTmUtc();
  const auto hashedPassword = ak_acl::utils::Hmac(username + password);
  const char *sql =
      "INSERT INTO auth_user (username, password, created_at) VALUES (?, ?, ?)";

  sqlite3_stmt *stmt = nullptr;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    ThrowSqliteError(db, sqlite3_errcode(db), "failed to prepare insert");
  }

  if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT) !=
          SQLITE_OK ||
      sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1,
                        SQLITE_TRANSIENT) != SQLITE_OK ||
      sqlite3_bind_text(stmt, 3, ak_acl::utils::ToIso8601Utc(createdAt).c_str(),
                        -1, SQLITE_TRANSIENT) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    ThrowSqliteError(db, sqlite3_errcode(db), "failed to bind parameters");
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    sqlite3_finalize(stmt);
    ThrowSqliteError(db, sqlite3_errcode(db), "failed to insert user");
  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  std::println("User created successfully:");
  std::println("  username: {}", username);
  std::println("  created_at: {}", ak_acl::utils::ToIso8601Utc(createdAt));

  return 0;
}
