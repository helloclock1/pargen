#pragma once

#include <memory>
#include <sstream>
#include <string>

std::unique_ptr<std::istringstream> MakeStream(const std::string &s);
