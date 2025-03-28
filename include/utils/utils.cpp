#include "utils.h"
#include <cpr/api.h>
#include <filesystem>
#include <iostream>
#include <mojang/api/api.h>
#include <string>

using namespace utils;

std::string utils::getOS() {
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) ||                \
    defined(__TOS_WIN__) || defined(__WINDOWS__)
  return "windows";
#elif defined(macintosh) || defined(Macintosh) ||                              \
    (defined(__APPLE__) && defined(__MACH__))
  return "osx";
#elif defined(linux) || defined(__linux) || defined(__linux__) ||              \
    defined(__gnu_linux__)
  return "linux";
#else
  return "unknown";
#endif
}

std::string utils::getArch() {
#if defined(__x86_64__) || defined(_M_X64)
  return "x86_64";
#elif defined(__i386__) || defined(_M_IX86) || defined(i386) || defined(__i386)
  return "x86";
#elif defined(_M_ARM64) || defined(__aarch64__)
  return "arm64";
#else
  return "unknown";
#endif
}

bool utils::checkRules(const std::vector<mojang::api::Rule> &rules) {
  std::string OS_NAME = getOS();
  std::string ARCH_NAME = getArch();
  if (OS_NAME == "unknown" || ARCH_NAME == "unknown") {
    return false;
  }
  for (const auto &rule : rules) {
    if (rule.action == "allow") {
      if (rule.os.contains("name") && rule.os["name"] != OS_NAME) {
        return false;
      }
      if (rule.os.contains("arch")) {
        std::string arch = rule.os["arch"].get<std::string>();
        if (arch == "x86" && ARCH_NAME != "x86") {
          return false;
        } else if (arch == "x86_64" && ARCH_NAME != "x86_64") {
          return false;
        } else if (arch == "arm64" && ARCH_NAME != "arm64") {
          return false;
        }
      }
    }
  }
  return true;
}

bool utils::download(const std::string &url, const std::string &path,
                     const std::string &filename) {
  std::string fullPath = path + filename;
  if (std::filesystem::exists(fullPath)) {
    // std::cout << filename << " already exists\n";
    return true;
  }
  cpr::Response r = cpr::Get(cpr::Url{url});
  // std::cout << r.status_code << std::endl;

  if (r.status_code == 200) {
    std::filesystem::create_directories(path);
    std::ofstream file(fullPath, std::ios::binary);
    file.write(r.text.data(), r.text.size());
    file.close();
    std::cout << url << " downloaded\n";
    return true;
  } else {
    std::cout << "Failed to download " << url << std::endl;
    return false;
  }
}
