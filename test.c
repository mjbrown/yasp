#include "unity_fixture.h"
#include "yasp.h"

// Define our main test group name
TEST_GROUP(YASP);

// This is run BEFORE each test
TEST_SETUP(YASP)
{
}

// This is run AFTER each test
TEST_TEAR_DOWN(YASP)
{
}

// A stub test that always fails
TEST(YASP, ExampleFailure)
{
    TEST_FAIL_MESSAGE("Not Implemented");
}

// Add all test cases to a test group
TEST_GROUP_RUNNER(YASP)
{
    RUN_TEST_CASE(YASP, ExampleFailure);
}

// Run all test groups (only 1 for now)
static void
run_all_tests(void)
{
    RUN_TEST_GROUP(YASP);
}

// Main program passes execution to Unity
int main(int argc, char *argv[])
{
    return UnityMain(argc, argv, run_all_tests);
}
