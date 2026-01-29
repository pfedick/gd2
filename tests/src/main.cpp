
#define PPL7TESTSUITEMAIN
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <ppl7.h>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    if (ppl7::HaveArgv(argc, argv, "-h") || ppl7::HaveArgv(argc, argv, "--help")) return 0;

    try {
        return RUN_ALL_TESTS();
    }
    catch (const ppl7::Exception& e) {
        printf("ppl7::Exception: %s\n", e.what());
    }
    catch (...) {
        printf("Unbekannte Exception\n");
    }

    return 1;
}
