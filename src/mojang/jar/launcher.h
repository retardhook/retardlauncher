#pragma once
#include "../auth/offline.h"
#include "../jar/download.h"
#include <cstdlib>

namespace mojang::game {
    class Launcher {
    public:
        bool LaunchGame(mojang::auth::UserInfo& user, const JarInfo& jarInfo,
            const std::string& javapath = "java") {
            std::string command = javapath + " -jar " + jarInfo.path;

            for (const auto& arg : jarInfo.arguments) {
                if (arg.rules.empty() || evaluateRules(arg.rules)) {
                    for (const auto& value : arg.value.values) {
                        command += " " + value;
                    }
                }
            }

            std::cout << "Launching Minecraft with command: " << command << std::endl;
            int result = system(command.c_str());

            if (result != 0) {
                return false;
            }

            return true;
        }

    private:
        bool evaluateRules(const std::vector<ArgumentRule>& rules) {
            for (const auto& rule : rules) {
                if (rule.action == "allow") {
                    if (rule.os.contains("name") && rule.os["name"] != "linux") {
                        return false;
                    }
                    if (rule.arch == "x86" && sizeof(void*) != 4) {
                        return false;
                    }
                }
                else if (rule.action == "disallow") {
                    if (rule.os.contains("name") && rule.os["name"] == "linux") {
                        return false;
                    }
                }
            }
            return true;
        }
    };
} // namespace mojang::game
