file (GLOB traces RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.trace)
file (GLOB local_traces RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} local/*.trace)

list (APPEND traces ${local_traces})
list (SORT traces)

foreach (trace ${traces})
    if (APITRACE_EXECUTABLE AND APITRACE_SOURCE_DIR)
        add_test(
            NAME trim-stress-${trace}
            COMMAND
            ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/trim_stress_driver.py
                --apitrace ${APITRACE_EXECUTABLE}
                --apitrace-source ${APITRACE_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/${trace}
        )
    endif ()
endforeach (trace)
