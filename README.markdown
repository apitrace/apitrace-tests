This is an attempt to write an automated test suite for apitrace.

It is still work in progress, and has a lot unfinished code/ideas.

To run the test suite do

    cmake -DAPITRACE_BINARY_DIR=/path/to/apitrace/build/ .
    export CTEST_OUTPUT_ON_FAILURE=1
    make all test

Detailed log will be written to Testing/Temporary/LastTest.log
