add_custom_command (
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/tri_vs.h
    COMMAND ${DirectX_FXC_EXECUTABLE} /nologo /T vs_4_0 /E VS /Fh ${CMAKE_CURRENT_SOURCE_DIR}/tri_vs.h ${CMAKE_CURRENT_SOURCE_DIR}/tri.fx
    DEPENDS tri.fx
)

add_custom_command (
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/tri_ps.h
    COMMAND ${DirectX_FXC_EXECUTABLE} /nologo /T ps_4_0 /E PS /Fh ${CMAKE_CURRENT_SOURCE_DIR}/tri_ps.h ${CMAKE_CURRENT_SOURCE_DIR}/tri.fx
    DEPENDS tri.fx
)

add_custom_target (hlsl DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tri_vs.h ${CMAKE_CURRENT_SOURCE_DIR}/tri_ps.h)
