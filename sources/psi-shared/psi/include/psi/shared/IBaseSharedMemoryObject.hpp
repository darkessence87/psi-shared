#pragma once

#include <string.h>

#include "SynchType.h"

namespace psi {

#ifdef __linux__
struct MappedFile {
    int id = -1;
    std::string name = "Unknown";
    void *handle = nullptr;
};
#elif __arm__
struct MappedFile {
    int id = -1;
    std::string name = "Unknown";
    void *handle = nullptr;
};
#elif _WIN32
struct MappedFile {
    void *id = NULL;
    std::string name = "Unknown";
    void *handle = nullptr;
};
#endif

template <typename C, SynchType S>
class IBaseSharedMemoryObject
{
public:
    IBaseSharedMemoryObject(size_t maxObjectSize, const MappedFile &mapFile);
    virtual ~IBaseSharedMemoryObject();

    virtual void lock() = 0;
    virtual void unlock() = 0;

    C *read() const;
    bool write(const C *const object);

protected:
    const size_t MAX_OBJECT_SIZE;
    const MappedFile m_mapFile;

private:
    IBaseSharedMemoryObject(const IBaseSharedMemoryObject &) = delete;
    IBaseSharedMemoryObject &operator=(const IBaseSharedMemoryObject &) = delete;
};

} // namespace psi
