#include "context.h"
#include <filesystem>
#include <iostream>

using namespace std;

int main(int argc, const char** argv)
{
    auto& context = Context::instance();

    // TODO: Allow users to specify on the commandline.
    filesystem::path config_file = getenv("HOME");
    config_file /= ".takp_launcher.conf";

    context.set_config_file(config_file);

    context.start();

    context.wait();

    return 0;
}
