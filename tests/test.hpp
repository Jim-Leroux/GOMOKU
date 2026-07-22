#pragma once

#include <iostream>
#include <string>

namespace Test
{

    inline int g_passed = 0;
    inline int g_failed = 0;

    inline void check(bool cond, const char* expr, const char* file, int line)
    {
        if (cond)
        {
            ++g_passed;
            return;
        }
        ++g_failed;
        std::cerr << "Test failed: " << expr << " in " << file << ":" << line << std::endl;
    }

    inline int summary()
    {
        std::cout << std::endl << "Tests summary: " << g_passed << " passed, " << g_failed << " failed" << std::endl;
        return g_failed > 0 ? 1 : 0;
    }

}

#define CHECK(cond) Test::check(cond, #cond, __FILE__, __LINE__)
