//
// Created by Mpho Mbotho on 2020-11-05.
//
#include <iostream>

#include "code.scc.hpp"

int main(int argc, char* argv[])
{
    demo::scc::Demo d(demo::scc::User{"Carter", 31}, demo::scc::User{"Marge", 30});
    demo::DebugPrinter dbg(std::cout);
    dbg << d;

    return 0;
}