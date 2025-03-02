#pragma once
#include <string>
#include <vector>

namespace mojang::auth {
struct UserInfo {
  std::string access_token;
  std::string session_token;
  std::string username;
  std::string uuid;
};

class Offline {
public:
  UserInfo Login(const std::string &username);

private:
  std::vector<unsigned char> ComputeMD5(const std::string &input);
  std::string GenerateUUID(const std::string &username);
  std::string GenerateRandomString(std::size_t length);
};
} // namespace mojang::auth
