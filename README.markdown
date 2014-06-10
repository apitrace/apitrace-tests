This is an automated test suite for apitrace.

In addition to apitrace requirements, it also requires:

* GLEW

To run the test suite do on Unices:

    cmake -DAPITRACE_SOURCE_DIR=/path/to/apitrace/tree -DAPITRACE_EXECUTABLE=/path/to/apitrace/build/apitrace -H. -B./build
    export CTEST_OUTPUT_ON_FAILURE=1
    make -C ./build all test

You can run multiple tests in parallel by specifying `CTEST_PARALLEL_LEVEL` environment variable.

Or on Windows:

    cmake -G "Visual Studio 10" -H. -B.\build
    set CTEST_OUTPUT_ON_FAILURE=1
    cmake --build .\build --target ALL_BUILD
    cmake --build .\build --target RUN_TESTS

A detailed log will be written to `Testing/Temporary/LastTest.log`.
