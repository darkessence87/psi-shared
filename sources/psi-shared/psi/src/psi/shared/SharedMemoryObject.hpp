#pragma once

#include <string.h>

#include "psi/shared/ISharedMemoryObject.hpp"

namespace psi {

template <typename C>
ISharedMemoryObject<C, SynchType::InterProcess>::ISharedMemoryObject(size_t maxObjectSize, const MappedFile &mapFile)
    : IBaseSharedMemoryObject<C, SynchType::InterProcess>(maxObjectSize, mapFile)
    , m_mutex(std::string("mutex_") + mapFile.name.c_str())
{
}

template <typename C>
void ISharedMemoryObject<C, SynchType::InterProcess>::lock()
{
    m_mutex.lock();
}

template <typename C>
void ISharedMemoryObject<C, SynchType::InterProcess>::unlock()
{
    m_mutex.unlock();
}

/// -------------------------------------------------------------------------------------------

template <typename C>
ISharedMemoryObject<C, SynchType::InterThread>::ISharedMemoryObject(size_t maxObjectSize, const MappedFile &mapFile)
    : IBaseSharedMemoryObject<C, SynchType::InterThread>(maxObjectSize, mapFile)
{
}

template <typename C>
void ISharedMemoryObject<C, SynchType::InterThread>::lock()
{
    m_mutex.lock();
}

template <typename C>
void ISharedMemoryObject<C, SynchType::InterThread>::unlock()
{
    m_mutex.unlock();
}

} // namespace psi
