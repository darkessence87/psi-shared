#pragma once

#include <iostream>
#include <string.h>
#ifdef __linux__
#include <sys/mman.h>
#include <unistd.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

#include "psi/shared/IBaseSharedMemoryObject.hpp"

namespace psi {

template <typename C, SynchType S>
IBaseSharedMemoryObject<C, S>::IBaseSharedMemoryObject(size_t maxObjectSize, const MappedFile &mapFile)
    : MAX_OBJECT_SIZE(maxObjectSize)
    , m_mapFile(mapFile)
{
}

template <typename C, SynchType S>
IBaseSharedMemoryObject<C, S>::~IBaseSharedMemoryObject()
{
#ifdef __linux__
    munmap(m_mapFile.handle, MAX_OBJECT_SIZE);
    close(m_mapFile.id);
#endif

#ifdef _WIN32
    UnmapViewOfFile(m_mapFile.handle);
    CloseHandle(m_mapFile.id);
#endif
}

template <typename C, SynchType S>
C *IBaseSharedMemoryObject<C, S>::read() const
{
    return reinterpret_cast<C *>(m_mapFile.handle);
}

template <typename C, SynchType S>
bool IBaseSharedMemoryObject<C, S>::write(const C *const object)
{
    if (!object) {
        std::cerr << "Could not write data to file: " << m_mapFile.name << ". Data is empty" << std::endl;
        return false;
    }

    const size_t size = sizeof(*object);
    if (size > MAX_OBJECT_SIZE) {
        std::cerr << "Could not write data to file: " << m_mapFile.name << ". Size of object:" << size << " is too big"
                  << std::endl;
        return false;
    }

    memcpy(m_mapFile.handle, object, size);

    return true;
}

} // namespace psi
