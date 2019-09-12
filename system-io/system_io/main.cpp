#include <unistd.h>
#include <iostream>
#include "File.h"

using namespace sysio;

int main()
{
    File f("/etc/hosts");

    // expect not equal to 1
    std::cout << f.fd() << std::endl;

    char buf = 'x';
    // expect equal to 1
    std::cout << ::read(f.fd(), &buf, 1) << std::endl;

    f.close();
    // expect f.fd() equal to -1
    std::cout << f.fd() << std::endl;

    std::cout << "Hello, World!" << std::endl;
    return 0;
}