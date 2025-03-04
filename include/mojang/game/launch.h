#pragma once
#include <mojang/api/api.h>
#include <mojang/auth/offline.h>
#include <string>

namespace mojang::game {

class Launcher {
public:
  bool launch(const mojang::auth::UserInfo &userInfo,
              const mojang::api::JarInfo &jarInfo,
              const std::string &javaPath = "java");
};
} // namespace mojang::game
