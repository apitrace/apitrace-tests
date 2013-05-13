function (ADD_CLI_TEST)
    cmake_parse_arguments(
        TEST
        # Options
        ""
        # One value args
        "NAME"
        # Multi value args
        ""
        ${ARGN}
    )

    if (APITRACE_EXECUTABLE)
        add_test(
            NAME ${TEST_NAME}
            COMMAND
            ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/cli_driver.py
                --apitrace ${APITRACE_EXECUTABLE}
                --apitrace-source ${APITRACE_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}
        )
    endif ()
endfunction ()

add_cli_test(NAME "cli-diff-images.script")
add_cli_test(NAME "cli-diff-images-mismatch.script")
add_cli_test(NAME "cli-trim-no-side-effects-prune.script")
add_cli_test(NAME "cli-trim-no-side-effects-trim.script")
add_cli_test(NAME "cli-trim-no-side-effects-prune-and-trim.script")
add_cli_test(NAME "cli-trim-single-swapbuffers.script")
add_cli_test(NAME "cli-trim-unused-textures.script")
add_cli_test(NAME "cli-trim-unused-shaders.script")
add_cli_test(NAME "cli-trim-spec-all.script")
add_cli_test(NAME "cli-trim-spec-textures.script")
add_cli_test(NAME "cli-trim-spec-shaders.script")
add_cli_test(NAME "cli-trim-spec-drawing.script")
