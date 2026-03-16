#pragma once
#include <string>

std::string getExecutableDir();
std::string resolvePath(const std::string& relative);
void initBasePath();

