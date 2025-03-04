#include "api.h"
#include <filesystem>
#include <future>
// #include <iostream>
#include <tuple>
#include <utils/utils.h>

using namespace mojang::api;

JarInfo Downloader::get(const std::string &version /*, std::string loader*/,
                        bool should_download) {
  std::vector<std::tuple<std::string, std::string>> failed;
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
          jarInfo.assetindex = version["assetIndex"]["url"].get<std::string>();
          jarInfo.assetindexid = version["assetIndex"]["id"].get<std::string>();
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
          } else {
            if (version.contains("minecraftArguments")) {
              std::string word = "";
              Argument argument;
              for (const auto &chara : version["minecraftArguments"]) {
                if (chara == ' ') {
                  if (!word.empty()) {
                    argument.value.values.push_back(word);
                    word.clear();
                  }
                } else {
                  word += chara;
                }
              }
              if (!word.empty()) {
                argument.value.values.push_back(word);
                word.clear();
              }

              jarInfo.arguments = {argument};
            }
          }

          if (version.contains("libraries")) {
            auto libs = parseLibraries(version["libraries"]);
            jarInfo.libraries.insert(jarInfo.libraries.end(), libs.begin(),
                                     libs.end());
          }
          auto paths = utils::getPaths();

          if (should_download) {
            if (!utils::download(jarInfo.url,
                                 paths.root + "/versions/" + jarInfo.version,
                                 "/client.jar")) {
              return {"", "", "", "", "", "", "", "", {}, {}, {}};
            }
            if (!downloadAssets(jarInfo)) {
              return {"", "", "", "", "", "", "", "", {}, {}, {}};
            }
            jarInfo.path =
                paths.root + "/versions/" + jarInfo.version + "/client.jar";
            std::vector<std::future<bool>> futures;
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
                  futures.push_back(std::async(std::launch::async, [paths, lib,
                                                                    jarInfo,
                                                                    filename,
                                                                    &failed]() {
                    if (!utils::download(
                            lib.url,
                            paths.libraries + "/" +
                                lib.path.substr(0, lib.path.find_last_of('/')),
                            filename)) {
                      failed.push_back(std::make_tuple(
                          lib.url, paths.libraries + "/" + lib.path));
                      return false;
                    }
                    return true;
                  }));
                  lib.path = paths.libraries + "/" + lib.path;
                } else {
                  // std::cout << "Skipping library (architecture mismatch): "
                  // << lib.name << std::endl;
                  continue;
                }
              }
            }

            Downloader::failedurls.insert(failedurls.end(), failed.begin(),
                                          failed.end());
            jarInfo.failed = Downloader::failedurls;
            return jarInfo;
          }
        }
      }
    } else {
      return {"", "", "", "", "", "", "", "", {}, {}, {}};
    }
  }
  return {"", "", "", "", "", "", "", "", {}, {}, {}};
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

bool Downloader::downloadAssets(const JarInfo &jarInfo) {
  auto paths = utils::getPaths();
  auto indexdir = paths.assets + "/indexes";
  if (!std::filesystem::exists(indexdir)) {
    std::filesystem::create_directories(indexdir);
  }
  auto ofs = std::ofstream(
      std::filesystem::path(
          indexdir + "/" +
          jarInfo.assetindex.substr(jarInfo.assetindex.find_last_of('/') + 1))
          .make_preferred()
          .string());
  auto ses = cpr::Session();
  ses.SetUrl(cpr::Url{jarInfo.assetindex});
  ses.Download(ofs);
  auto r = ses.Get();
  // std::cout << "Downloading assets: " << jarInfo.assetindex << std::endl;
  // std::cout << "Response: " << json::parse(r.text) << std::endl;

  auto data = json::parse(r.text);
  std::vector<std::future<bool>> futures;
  std::vector<std::tuple<std::string, std::string>> failed;

  for (const auto &asset : data["objects"]) {
    // std::cout << "Candidate asset: " << asset << std::endl;

    std::string hash = asset["hash"].get<std::string>();
    std::string path = paths.assets + "/objects/" + hash.substr(0, 2) + "/";
    // std::cout << "Checking asset: " << path << std::endl;
    if (!std::filesystem::exists(path + hash)) {
      std::string url = "https://resources.download.minecraft.net/" +
                        hash.substr(0, 2) + "/" + hash;
      futures.push_back(
          std::async(std::launch::async, [url, path, hash, &failed]() {
            if (!utils::download(url, path, hash)) {
              failed.push_back(std::make_tuple(url, path));
              return false;
            }
            return true;
          }));
    }
  }

  Downloader::failedurls.insert(failedurls.end(), failed.begin(), failed.end());
  return true;
}
