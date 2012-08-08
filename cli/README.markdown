This directory tests high-level functionality of apitrace command-line
interface (cli). See also the neighboring "traces" directory which
also tests the cli, but in ways that can only be verified by doing
comparisons of resulting dump files.

For writing new tests, if the functionality can be exercised by a
sequence of apitrace command invocations, and then a final comparison
of "apitrace dump" output, then it's likely simplest to write a new
test in the traces directory. Otherwise, a new test program can be
written in this directory.

The tests in this directory are found in files with names matching
*.script. The scripts are simple line-based commands with the
following meanings based on the first word of each line:

  apitrace:     Execute the current apitrace executable being tested
  		with the given arguments. If apitrace returns a
  		non-zero status, the test will fail.

  expect:       Compare the results of the previously-executed command
                with the given (json-quoted) string. If the strings
                are not identical, the test will fail.

  rm_and_mkdir: Remove any existing directory of the given name and
                then create it. The directory name is always
                interpreted locally. If this fails for any reason
                other than "file does not exist" the test will fail.

If none of the commands in the script cause the test to fail, then the
test will pass.

