#pragma once
#include <string>
#include <vector>

namespace mojang {
namespace api {
struct Rule;
} // namespace api
} // namespace mojang

namespace utils {
std::string getOS();
std::string getArch();
bool checkRules(const std::vector<mojang::api::Rule> &rules);
bool download(const std::string &url, const std::string &path,
              const std::string &filename);
} // namespace utils
