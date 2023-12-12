#pragma once

#include <map>
#include <memory>

#include "os_mutex.h"

#include "IBaseSharedMemoryObject.hpp"

namespace psi {

template <typename C, SynchType S>
class ISharedMemoryManager final
{
public:
    ~ISharedMemoryManager();

    /// Singleton
    static std::shared_ptr<ISharedMemoryManager> getInstance(const std::string &name = "");

    void freeMemory();

    bool isShared();
    void loadToShared(const C *const object);
    C *loadFromShared();

    std::shared_ptr<IBaseSharedMemoryObject<C, S>> getSharedMemory();
    const std::string name() const;

private:
    ISharedMemoryManager(const std::string &name = "");

    const MappedFile tryOpenMap(const char *mapName);
    void createImpl();
    std::string mapFileName() const;

private:
    static constexpr size_t MAX_OBJECT_SIZE = sizeof(C);
    std::string m_fileName;
    Mutex m_mutex;

    std::shared_ptr<IBaseSharedMemoryObject<C, S>> m_sharedObject;

private:
    ISharedMemoryManager(const ISharedMemoryManager &) = delete;
    ISharedMemoryManager &operator=(const ISharedMemoryManager &) = delete;
};

} // namespace psi
