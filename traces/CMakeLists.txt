set (traces
    incomplete-call
    zlib-no-eof
)

foreach (trace ${traces})
    add_test(
        NAME trace_${trace}
        COMMAND
        python ${CMAKE_SOURCE_DIR}/tool_driver.py
            --apitrace ${APITRACE_EXECUTABLE}
            --ref-dump ${CMAKE_CURRENT_SOURCE_DIR}/${trace}.ref.txt
            --
            dump --call-nos=no ${CMAKE_CURRENT_SOURCE_DIR}/${trace}.trace
    )
endforeach (trace)
