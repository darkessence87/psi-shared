if(WIN32)
    set(SUB_DIR_LIBS ${CMAKE_BUILD_TYPE}/win64)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std:c++20 -W4 /MP /O2")

    # set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10000000")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
else()
    message("[${projectName}] Only Windows build is supported at the moment.")
    return()
endif()

message("[${projectName}] CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")