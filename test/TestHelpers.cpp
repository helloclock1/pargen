#include "TestHelpers.h"

std::unique_ptr<std::istringstream> MakeStream(const std::string &s) {
    return std::move(std::make_unique<std::istringstream>(s));
}