include_directories (
    ${CMAKE_CURRENT_SOURCE_DIR}/../d3dcommon
    ${DirectX_D3D9_INCLUDE_DIR}
)

link_libraries (
    ${DirectX_D3D9_LIBRARY}
)

set (api d3d9)

set (targets
    freelibrary
    tri
    tri_pp
    tex_sysmem
    map_readonly
)

foreach (target ${targets})
    add_executable (${api}_${target} ${target}.cpp)
    set_target_properties (${api}_${target} PROPERTIES OUTPUT_NAME ${target})
endforeach (target)

add_app_tests ()
