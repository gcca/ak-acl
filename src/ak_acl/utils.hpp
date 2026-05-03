#pragma once

#include <chrono>
#include <cstddef>
#include <ctime>
#include <iomanip>
#include <optional>
#include <random>
#include <span>
#include <sstream>
#include <string>
#include <string_view>

namespace ak_acl::utils {

[[nodiscard]] inline std::tm NowTmUtc() {
  using namespace std::chrono;
  const auto now = system_clock::now();
  const std::time_t tt = system_clock::to_time_t(now);
  std::tm tm{};
  gmtime_r(&tt, &tm);
  return tm;
}

[[nodiscard]] inline std::tm AddSecondsToTm(const std::tm &value, int seconds) {
  std::tm adjusted = value;
  const std::time_t tt = timegm(&adjusted);
  const auto timePoint = std::chrono::system_clock::from_time_t(tt) +
                         std::chrono::seconds(seconds);
  const std::time_t adjustedTime =
      std::chrono::system_clock::to_time_t(timePoint);
  std::tm tm{};
  gmtime_r(&adjustedTime, &tm);
  return tm;
}

[[nodiscard]] inline std::string ToIso8601Utc(const std::tm &value) {
  std::ostringstream out;
  out << std::put_time(&value, "%FT%TZ");
  return out.str();
}

[[nodiscard]] inline std::optional<std::tm>
ParseIso8601Utc(const std::string &value) {
  std::tm tm{};
  std::istringstream in(value);
  in >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
  if (in.fail()) {
    return std::nullopt;
  }
  return tm;
}

[[nodiscard]] inline bool IsBeforeOrEqualUtc(const std::tm &lhs,
                                             const std::tm &rhs) {
  auto lhsCopy = lhs;
  auto rhsCopy = rhs;
  return timegm(&lhsCopy) <= timegm(&rhsCopy);
}

[[nodiscard]] inline std::string RandomStr(std::size_t length) {
  static constexpr char alphabet[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_";
  static thread_local std::mt19937_64 rng{std::random_device{}()};
  static thread_local std::uniform_int_distribution<std::size_t> dist(
      0, sizeof(alphabet) - 2);

  std::string value;
  value.reserve(length);
  for (std::size_t i = 0; i < length; ++i) {
    value.push_back(alphabet[dist(rng)]);
  }
  return value;
}

[[nodiscard]] std::string Hmac(std::span<const std::byte> data);

[[nodiscard]] inline std::string Hmac(std::string_view value) {
  return Hmac(std::as_bytes(std::span(value.data(), value.size())));
}

} // namespace ak_acl::utils
