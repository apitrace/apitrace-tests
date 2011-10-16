zlib-no-eof.trace is a short, zlib compressed trace, with an unexpected end of
file because the application terminated abnormally (which is actual very
normal).

Depending on how zlib handles the unexpected end of file, it may fail to read
any data from it.
