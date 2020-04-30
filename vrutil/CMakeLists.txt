set(TARGET_NAME vrutil)
add_library(${TARGET_NAME}
    OverlayApp.cpp    
    OverlayItem.cpp
    D3DManager.cpp
    )
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)    
target_include_directories(${TARGET_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${EXTERNAL_DIR}/GSL/include
    ${EXTERNAL_DIR}/openvr/headers
    )
target_link_directories(${TARGET_NAME} PUBLIC
    ${EXTERNAL_DIR}/openvr/lib/win64
    )
target_link_libraries(${TARGET_NAME} PUBLIC
    openvr_api
    d3d11
    winmm
    )
