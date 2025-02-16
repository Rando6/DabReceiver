#include "MainController.h"

#include <fmt/printf.h>

#include <iostream>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fmt::println("Please pass exactly one argument to this program; the file path of a raw IQ file where I and Q are of the type uint8_t.");
        return -1;
    }

    auto file_path{ std::string(argv[1]) };
    auto mainController{ MainController() };
    mainController.run(file_path);

    return 0;
}
