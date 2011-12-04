set (traces
    incomplete-call
    zlib-no-eof
)

foreach (trace ${traces})
    add_test(
        NAME trace_${trace}
        COMMAND
        python ${CMAKE_SOURCE_DIR}/driver.py
            --build ${APITRACE_BINARY_DIR}
            --ref-dump ${CMAKE_CURRENT_SOURCE_DIR}/${trace}.ref.txt
            ${trace}.trace
    )
endforeach (trace)
