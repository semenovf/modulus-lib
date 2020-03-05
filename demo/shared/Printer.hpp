#pragma once
#include <string>
#include <cstdio>

struct Printer
{
    void print (std::string const & s)
    {
        puts(s.c_str());
    }
};

