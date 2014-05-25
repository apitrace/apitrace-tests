This is an automated test suite for apitrace.

In addition to apitrace requirements, it also requires:

* GLUT

* GLEW

To run the test suite do on Unices:

    cmake -DAPITRACE_SOURCE_DIR=/path/to/apitrace/tree -DAPITRACE_EXECUTABLE=/path/to/apitrace/build/apitrace -H. -B./build
    export CTEST_OUTPUT_ON_FAILURE=1
    make -C ./build all
    ctest

You can run multiple tests in parallel by specifying the `-j` option.

Or on Windows:

    cmake -G "Visual Studio 10" -H. -B.\build
    cmake --build .\build --target ALL_BUILD
    set CTEST_OUTPUT_ON_FAILURE=1
    cmake --build .\build --target RUN_TESTS

A detailed log will be written to `Testing/Temporary/LastTest.log`.
