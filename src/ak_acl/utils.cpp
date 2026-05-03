#include "utils.hpp"

#include <openssl/evp.h>
#include <openssl/hmac.h>

namespace ak_acl::utils {

[[nodiscard]] std::string Hmac(std::span<const std::byte> data) {
  static constexpr char secret[] =
      "7kR~9mZ_xQ2.wP-5vN8~yH3.jL6_tB4-gF1~cD0.sA9_eW7-oK5~iM2.uX8_qY3-nV6~hJ1";

  unsigned char digest[EVP_MAX_MD_SIZE];
  unsigned int digest_len = 0;

  HMAC(EVP_sha256(), secret, sizeof(secret) - 1,
       reinterpret_cast<const unsigned char *>(data.data()), data.size(),
       digest, &digest_len);

  static constexpr char hex[] = "0123456789abcdef";
  std::string output;
  output.reserve(digest_len * 2);
  for (unsigned int i = 0; i < digest_len; ++i) {
    output.push_back(hex[(digest[i] >> 4) & 0x0F]);
    output.push_back(hex[digest[i] & 0x0F]);
  }
  return output;
}

} // namespace ak_acl::utils
