This is an automated test suite for apitrace.

In addition to apitrace requirements, it also requires:

* GLUT

* GLEW

To run the test suite do:

    cmake -DAPITRACE_BINARY_DIR=/path/to/apitrace/build/ .
    export CTEST_OUTPUT_ON_FAILURE=1
    make all test

Detailed log will be written to `Testing/Temporary/LastTest.log`.
