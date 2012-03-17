file (GLOB scripts RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.script)

foreach (script ${scripts})
    add_test(
        NAME ${script}
        COMMAND
        python ${CMAKE_SOURCE_DIR}/tool_driver.py
            --apitrace ${APITRACE_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/${script}
    )
endforeach (script)
