#include "PathUtils.h"

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#endif

// This will hold the resolved base folder for all assets
static std::string gBasePath;

std::string getExecutableDir() {
#if defined(__APPLE__)
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        std::string fullPath(path);
        size_t pos = fullPath.find_last_of('/');
        return fullPath.substr(0, pos); // Returns build/bin
    }
#endif
    return ".";
}

// Sets base path
void initBasePath() {
    // We go one level up from "bin" to reach "build"
    gBasePath = getExecutableDir();
}

// Use this for all relative asset paths
std::string resolvePath(const std::string& relative) {
    return gBasePath + "/" + relative;
}

