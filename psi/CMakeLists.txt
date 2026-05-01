set(SOURCES
    src/psi/shared/os_mutex.cpp
    src/psi/shared/ipc/client/ClientIPC.cpp
    src/psi/shared/ipc/server/ServerIPCBase.cpp
    src/psi/shared/ipc/IPCTemplates.cpp
    src/psi/shared/sm_managers.cpp
)

set (target_lib "psi-shared")

add_library(${target_lib} STATIC ${SOURCES})
add_library(psi::shared ALIAS ${target_lib})

target_include_directories(${target_lib}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

psi_config_target(${target_lib})

set(TEST_SRC
    tests/CallSpace_Tests.cpp
    tests/CallbackSpace_Tests.cpp
    tests/EventRingBuffer_Tests.cpp
    tests/EventSpace_Tests.cpp
    tests/IPCCall_Tests.cpp
    tests/IPCClientServer_Tests.cpp
    tests/SerializeDeserialize_Tests.cpp
)
psi_make_tests("Shared" "${TEST_SRC}" "${target_lib}")

set(EXAMPLE_SRC_1.0
    examples/1.0_Simple_SharedMemory/EntryPoint.cpp
)
psi_make_examples("1.0_Simple_SharedMemory" "${EXAMPLE_SRC_1.0}" "${target_lib}")