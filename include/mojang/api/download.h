#pragma once
#include <utils/utils.h>
#include <cpr/cpr.h>
#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
using json = nlohmann::json;

namespace mojang::api {

    struct Rule {
        std::string action;
        json features;
        json os;
        std::string arch;
    };

    struct ArgumentValue {
        std::vector<std::string> values;
    };

    struct Argument {
        std::vector<Rule> rules;
        ArgumentValue value;
    };

    struct Library {
        std::string name;
        std::string url;
        std::string path;

        std::vector<Rule> rules;
    };

    struct JarInfo {
        std::string version;
        std::string jsonurl;
        std::string url;
        std::string type;
        std::string release_time;
        std::string path;
        std::string mainclass;
        std::string mlv; // minimum launcher version
        std::string assetindex;
        std::string assetindexid;

        std::vector<Argument> arguments;
        std::vector<Library> libraries;
    };

    class Downloader {
    public:
        JarInfo get(const std::string& version /*, std::string loader*/,
            bool should_download);

    private:
        std::vector<Library> parseLibraries(const json& libraries);
        std::vector<Argument> parseArguments(const json& arguments);
        bool downloadAssets(const JarInfo& jarInfo);
    };

} // namespace mojang::api
