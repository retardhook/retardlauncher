#include "api.h"
#include <utils/utils.h>

using namespace mojang::api;

JarInfo Downloader::get(const std::string &version /*, std::string loader*/,
                        bool should_download) {
  cpr::Response r = cpr::Get(cpr::Url{
      "https://piston-meta.mojang.com/mc/game/version_manifest_v2.json"});

  if (r.status_code == 200 && !r.text.empty()) {
    json j = json::parse(r.text);

    if (j.contains("versions")) {
      for (auto &version_info : j["versions"]) {
        if (version_info.contains("id") && version_info["id"] == version &&
            version_info.contains("url") && version_info.contains("type") &&
            version_info.contains("releaseTime")) {

          cpr::Response r =
              cpr::Get(cpr::Url{version_info["url"].get<std::string>()});
          json version = json::parse(r.text);

          JarInfo jarInfo;
          jarInfo.version = version_info["id"].get<std::string>();
          jarInfo.url =
              version["downloads"]["client"]["url"].get<std::string>();
          jarInfo.jsonurl = version_info["url"].get<std::string>();
          jarInfo.mainclass = version["mainClass"].get<std::string>();
          jarInfo.mlv =
              std::to_string(version["minimumLauncherVersion"].get<int>());
          jarInfo.type = version_info["type"].get<std::string>();
          jarInfo.release_time = version_info["releaseTime"].get<std::string>();

          if (version.contains("arguments")) {
            if (version["arguments"].contains("game")) {
              jarInfo.arguments = parseArguments(version["arguments"]["game"]);
            }
            if (version["arguments"].contains("jvm")) {
              auto jvmArgs = parseArguments(version["arguments"]["jvm"]);
              jarInfo.arguments.insert(jarInfo.arguments.end(), jvmArgs.begin(),
                                       jvmArgs.end());
            }
          }

          if (version.contains("libraries")) {
            auto libs = parseLibraries(version["libraries"]);
            jarInfo.libraries.insert(jarInfo.libraries.end(), libs.begin(),
                                     libs.end());
          }

          if (should_download) {
            if (!utils::download(jarInfo.url, "jars/" + jarInfo.version + "/",
                                 "client.jar")) {
              return {"", "", "", "", "", "", "", "", {}, {}};
            }
            jarInfo.path = "jars/" + jarInfo.version + "/client.jar";
            for (auto &lib : jarInfo.libraries) {
              if (utils::checkRules(lib.rules)) {
                std::string filename =
                    lib.path.substr(lib.path.find_last_of('/') + 1);
                std::string libArch = "unknown";
                if (lib.url.find("x86_64") != std::string::npos) {
                  libArch = "x86_64";
                } else if (lib.url.find("x86") != std::string::npos) {
                  libArch = "x86";
                } else if (lib.url.find("arm64") != std::string::npos ||
                           lib.url.find("aarch_64") != std::string::npos) {
                  libArch = "arm64";
                }

                if (libArch == "unknown" || libArch == utils::getArch()) {
                  if (!utils::download(
                          lib.url, "jars/" + jarInfo.version + "/libraries/",
                          filename)) {
                    return {"", "", "", "", "", "", "", "", {}, {}};
                  }
                  lib.path =
                      "jars/" + jarInfo.version + "/libraries/" + filename;
                } else {
                  std::cout << "Skipping library (architecture mismatch): "
                            << lib.name << std::endl;
                }
              }
            }

            return jarInfo;
          }
        }
      }
    } else {
      return {"", "", "", "", "", "", "", "", {}, {}};
    }
  }
  return {"", "", "", "", "", "", "", "", {}, {}};
}

std::vector<Library> Downloader::parseLibraries(const json &libraries) {
  std::vector<Library> libs;
  for (const auto &lib : libraries) {
    Library library;
    if (lib.contains("downloads")) {
      library.url = lib["downloads"]["artifact"]["url"].get<std::string>();
      library.path = lib["downloads"]["artifact"]["path"].get<std::string>();
    }
    if (lib.contains("name")) {
      library.name = lib["name"].get<std::string>();
    }
    if (lib.contains("rules")) {
      for (const auto &rule : lib["rules"]) {
        Rule libraryRule;
        if (rule.contains("action")) {
          libraryRule.action = rule["action"].get<std::string>();
        }
        if (rule.contains("os")) {
          libraryRule.os = rule["os"];
        }
        if (rule.contains("features")) {
          libraryRule.features = rule["features"];
        }
        if (rule.contains("arch")) {
          libraryRule.arch = rule["arch"].get<std::string>();
        }
        library.rules.push_back(libraryRule);
      }
    }
    libs.push_back(library);
  }
  return libs;
}

std::vector<Argument> Downloader::parseArguments(const json &arguments) {
  std::vector<Argument> args;
  for (const auto &arg : arguments) {
    Argument argument;
    if (arg.is_string()) {
      argument.value.values.push_back(arg.get<std::string>());
    } else if (arg.is_object()) {
      if (arg.contains("rules")) {
        for (const auto &rule : arg["rules"]) {
          Rule argumentRule;
          if (rule.contains("action")) {
            argumentRule.action = rule["action"].get<std::string>();
          }
          if (rule.contains("os")) {
            argumentRule.os = rule["os"];
          }
          if (rule.contains("features")) {
            argumentRule.features = rule["features"];
          }
          if (rule.contains("arch")) {
            argumentRule.arch = rule["arch"].get<std::string>();
          }
          argument.rules.push_back(argumentRule);
        }
      }
      if (arg.contains("value")) {
        ArgumentValue argumentValue;
        if (arg["value"].is_array()) {
          for (const auto &value : arg["value"]) {
            argumentValue.values.push_back(value.get<std::string>());
          }
        } else {
          argumentValue.values.push_back(arg["value"].get<std::string>());
        }
        argument.value = argumentValue;
      }
    }
    args.push_back(argument);
  }
  return args;
}
