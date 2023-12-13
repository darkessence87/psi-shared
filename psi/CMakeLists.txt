
include_directories(
    ${3rdPARTY_DIR}/psi/include
    ${psi_comm_dir}/psi/include
    ${psi_thread_dir}/psi/include
    ${psi_tools_dir}/psi/include
    include
    src
)

set(SOURCES
    src/psi/shared/os_mutex.cpp
    src/psi/shared/ipc/client/ClientIPC.cpp
    src/psi/shared/ipc/server/ServerIPCBase.cpp
    src/psi/shared/ipc/IPCTemplates.cpp
)

link_directories(
    ${3rdPARTY_DIR}/psi/lib/${SUB_DIR_LIBS}
    ${psi_thread_dir}/build/bin/${CMAKE_BUILD_TYPE}
)

add_library(psi-shared STATIC ${SOURCES})
target_link_libraries(psi-shared
    psi-thread
)

# set(TEST_SRC
#     tests/TimerTests.cpp
# )
# psi_make_tests("Shared" "${TEST_SRC}" "psi-shared")

set(EXAMPLE_SRC_1.0 examples/1.0_Simple_SharedMemory/EntryPoint.cpp)
psi_make_examples("1.0_Simple_SharedMemory" "${EXAMPLE_SRC_1.0}" "psi-shared")