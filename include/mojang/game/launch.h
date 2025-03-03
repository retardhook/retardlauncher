#pragma once
#include <mojang/api/download.h>
#include <mojang/auth/offline.h>
#include <string>

namespace mojang::game {

    class Launcher {
    public:
        bool launch(const mojang::auth::UserInfo& userInfo,
            const mojang::api::JarInfo& jarInfo,
            const std::string& javaPath = "java");

    private:
        std::string get_game_dir();
    };
} // namespace mojang::game
