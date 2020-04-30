set(TARGET_NAME desktopdup)
add_executable(${TARGET_NAME}
    main.cpp
    duplicator.cpp
    quad.cpp
    pointershape.cpp
    compileshader.cpp
    )
target_link_libraries(${TARGET_NAME} PRIVATE
    vrutil
    d3dcompiler
    )
