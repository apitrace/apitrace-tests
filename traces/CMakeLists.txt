file (GLOB scripts RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.script)

list (SORT scripts)

foreach (script ${scripts})
    add_test(
        NAME ${script}
        COMMAND
        python ${CMAKE_SOURCE_DIR}/tool_driver.py
            --apitrace ${APITRACE_EXECUTABLE}
            --apitrace-source ${APITRACE_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/${script}
    )
endforeach (script)
