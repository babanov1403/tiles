#include "libhello/hello.h"

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/check.h"
#include "absl/log/globals.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"

#include <iostream>
#include <string>

ABSL_FLAG(std::string, user, "", "username");

int main(int argc, char** argv)
{
    absl::SetStderrThreshold(absl::LogSeverityAtLeast::kInfo);
    absl::ParseCommandLine(argc, argv);
    absl::InitializeLog();

    std::string user = absl::GetFlag(FLAGS_user);
    CHECK(!user.empty());

    LOG(INFO) << "user=" << user;

    std::cout << libhello::hello(user) << std::endl;

    return 0;
}
