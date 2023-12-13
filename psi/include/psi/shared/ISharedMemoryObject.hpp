#pragma once

#include <mutex>

#include "IBaseSharedMemoryObject.hpp"
#include "os_mutex.h"

namespace psi {

template <typename C, SynchType S>
class ISharedMemoryObject;

/// -------------------------------------------------------------------------------------------

template <typename C>
class ISharedMemoryObject<C, SynchType::InterProcess> final : public IBaseSharedMemoryObject<C, SynchType::InterProcess>
{
public:
    ISharedMemoryObject(size_t maxObjectSize, const MappedFile &mapFile);

    void lock() override;
    void unlock() override;

private:
    Mutex m_mutex;
};

/// -------------------------------------------------------------------------------------------

template <typename C>
class ISharedMemoryObject<C, SynchType::InterThread> final : public IBaseSharedMemoryObject<C, SynchType::InterThread>
{
public:
    ISharedMemoryObject(size_t maxObjectSize, const MappedFile &mapFile);

    void lock() override;
    void unlock() override;

private:
    std::mutex m_mutex;
};

} // namespace psi
