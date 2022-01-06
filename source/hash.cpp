#include "hash.hpp"
#include "io.hpp"

#include <openssl/sha.h>

#include <random>

namespace nfwk {

std::string make_random_salt(std::size_t half_length) {
	std::string salt;
	for (std::size_t i{ 0 }; i < half_length; i++) {
		salt += std::format("{:02X}", static_cast<std::uint8_t>(std::random_device{}() % 256));
	}
	return salt;
}

std::string make_sha256_hash(std::string_view password, std::optional<std::string_view> salt) {
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, password.data(), password.size());
	if (salt.has_value()) {
		SHA256_Update(&sha256, salt->data(), salt->size());
	}
	unsigned char binary_hash[SHA256_DIGEST_LENGTH]{};
	SHA256_Final(binary_hash, &sha256);
	std::string hash;
	for (int i{ 0 }; i < SHA256_DIGEST_LENGTH; i++) {
		hash += std::format("{:02X}", binary_hash[i]);
	}
	return hash;
}


}
