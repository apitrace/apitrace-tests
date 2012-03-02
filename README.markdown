This is an automated test suite for apitrace.

In addition to apitrace requirements, it also requires:

* GLUT

* GLEW

To run the test suite do on Unices:

    cmake -DAPITRACE_EXECUTABLE=/path/to/apitrace -H. -B./build
    export CTEST_OUTPUT_ON_FAILURE=1
    make -C ./build all test

Or on Windows:

    cmake -G "Visual Studio 10" -H. -B.\build
    cmake --build .\build --target ALL_BUILD
    cmake --build .\build --target RUN_TESTS

A detailed log will be written to `Testing/Temporary/LastTest.log`.
