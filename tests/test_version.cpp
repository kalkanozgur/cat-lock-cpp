#include <string>

#include "core/version.h"
#include "harness.h"

TEST(version_is_set) {
    CHECK(kk::version() != nullptr);
    CHECK_EQ(std::string(kk::version()), std::string("0.0.0"));
}
