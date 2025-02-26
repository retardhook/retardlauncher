#pragma once
#include <cpr/cpr.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace mojang::game {
    struct ArgumentRule {
        std::string action;
        json features;
        json os;
        std::string arch;
    };

    struct ArgumentValue {
        std::vector<std::string> values;
    };

    struct Argument {
        std::vector<ArgumentRule> rules;
        ArgumentValue value;
    };

    struct JarInfo {
        std::string version;
        std::string jsonurl;
        std::string url;
        std::string type;
        std::string release_time;
        std::string path;
        std::string main_class;

        // vector
        std::vector<Argument> arguments;
        std::vector<std::string> library_paths;
    };

    class Download {
    public:
        JarInfo get(const std::string& version, bool should_download) {
            cpr::Response r = cpr::Get(cpr::Url{
                "https://piston-meta.mojang.com/mc/game/version_manifest_v2.json" });

            if (r.status_code == 200 && !r.text.empty()) {
                json j = json::parse(r.text);

                if (j.contains("versions")) {
                    for (auto& version_info : j["versions"]) {
                        if (version_info.contains("id") && version_info["id"] == version &&
                            version_info.contains("url") && version_info.contains("type") &&
                            version_info.contains("releaseTime")) {

                            cpr::Response r = cpr::Get(cpr::Url{ version_info["url"].get<std::string>() });
                            json version = json::parse(r.text);

                            JarInfo jar_info;
                            jar_info.version = version_info["id"].get<std::string>();
                            jar_info.url = version["downloads"]["client"]["url"].get<std::string>();
                            jar_info.jsonurl = version_info["url"].get<std::string>();
                            jar_info.type = version_info["type"].get<std::string>();
                            jar_info.release_time = version_info["releaseTime"].get<std::string>();
                            jar_info.main_class = version["mainClass"].get<std::string>();

                            if (version.contains("arguments")) {
                                if (version["arguments"].contains("game")) {
                                    jar_info.arguments = parse_arguments(version["arguments"]["game"]);
                                }
                                if (version["arguments"].contains("jvm")) {
                                    auto jvm_args = parse_arguments(version["arguments"]["jvm"]);
                                    jar_info.arguments.insert(jar_info.arguments.end(), jvm_args.begin(), jvm_args.end());
                                }
                            }

                            std::filesystem::path version_path = "jars/" + jar_info.version;
                            if (should_download) {
                                if (!download(jar_info.url, version_path.string(), jar_info.version + ".jar")) {
                                    return { "", "", "", "", {} };
                                }
                                jar_info.path = version_path.string() + "/" + jar_info.version + ".jar";
                            }

                            if (version.contains("libraries")) {
                                download_libraries(version["libraries"], version_path, jar_info.library_paths);
                            }

                            return jar_info;
                        }
                    }
                }
            }
            return { "", "", "", "", {} };
        }

    private:
        void download_libraries(const json& libraries, const std::filesystem::path& version_path, std::vector<std::string>& library_paths) {
            std::filesystem::path libraries_path = version_path / "libraries";
            for (const auto& library : libraries) {
                if (library.contains("downloads") && library["downloads"].contains("artifact")) {
                    auto library_info = library["downloads"]["artifact"];
                    std::string library_url = library_info["url"].get<std::string>();
                    std::string library_path = library_info["path"].get<std::string>();
                    std::filesystem::path library_dir = libraries_path / std::filesystem::path(library_path).parent_path();
                    if (!std::filesystem::exists(library_dir)) {
                        std::filesystem::create_directories(library_dir);
                    }
                    if (download(library_url, library_dir.string(), std::filesystem::path(library_path).filename().string())) {
                        library_paths.push_back(library_path);
                    }
                }
            }
        }

        std::vector<Argument> parse_arguments(const json& arguments) {
            std::vector<Argument> args;
            for (const auto& arg : arguments) {
                Argument argument;
                if (!arg.contains("rules") && !arg.contains("value")) {
                    argument.value.values.push_back(arg);
                }
                if (arg.contains("rules")) {
                    for (const auto& rule : arg["rules"]) {
                        ArgumentRule argument_rule;
                        if (rule.contains("action")) {
                            argument_rule.action = rule["action"].get<std::string>();
                        }
                        if (rule.contains("os")) {
                            argument_rule.os = rule["os"];
                        }
                        if (rule.contains("features")) {
                            argument_rule.features = rule["features"];
                        }
                        if (rule.contains("arch")) {
                            argument_rule.arch = rule["arch"].get<std::string>();
                        }
                        argument.rules.push_back(argument_rule);
                    }
                }
                if (arg.contains("value")) {
                    ArgumentValue argument_value;
                    if (arg["value"].is_array()) {
                        for (const auto& value : arg["value"]) {
                            argument_value.values.push_back(value.get<std::string>());
                        }
                    }
                    else {
                        argument_value.values.push_back(arg["value"].get<std::string>());
                    }
                    argument.value = argument_value;
                }
                args.push_back(argument);
            }
            return args;
        }

        bool download(const std::string& url, const std::string& path, const std::string& filename) {
            if (std::filesystem::exists(path + "/" + filename)) {
                std::cout << filename << " already exists\n";
                return true;
            }
            cpr::Response r = cpr::Get(cpr::Url{ url });
            if (r.status_code == 200) {
                std::filesystem::create_directories(path);
                std::ofstream file(path + "/" + filename, std::ios::binary);
                file << r.text;
                file.close();
                std::cout << filename << " downloaded\n";
                return true;
            }
            else {
                std::cout << "Failed to download " << filename << std::endl;
                return false;
            }
        }
    };
} // namespace mojang::game