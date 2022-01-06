#pragma once

#include <optional>
#include <string>

namespace nfwk {

std::string make_random_salt(std::size_t half_length);
std::string make_sha256_hash(std::string_view password, std::optional<std::string_view> salt);

}
