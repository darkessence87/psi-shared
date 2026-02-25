
#pragma once

#include "psi/shared/i_sm_object.h"

#ifdef __linux__
#include <sys/mman.h>
#include <unistd.h>
#endif
#ifdef _WIN32
#include <Windows.h>
#endif

#include <iostream>
#include <mutex>
#include <string>

#include "os_mutex.h"

namespace psi {

#ifdef __linux__
struct MappedFile {
    int id = -1;
    std::string name = "Unknown";
    void *handle = nullptr;
};
#elif _WIN32
struct MappedFile {
    void *id = nullptr;
    std::string name = "Unknown";
    void *handle = nullptr;
};
#endif

template <typename C>
class sm_object_impl : public i_sm_object<C>
{
public:
    sm_object_impl(size_t maxObjectSize, const MappedFile &mapFile)
        : MAX_OBJECT_SIZE(maxObjectSize)
        , m_mapFile(mapFile)
        , m_mutex(std::string("mutex_") + mapFile.name.c_str())
    {
    }

    ~sm_object_impl() override
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

    void lock() override
    {
        m_mutex.lock();
    }

    void unlock() override
    {
        m_mutex.unlock();
    }

    C *read() const override
    {
        return reinterpret_cast<C *>(m_mapFile.handle);
    }

    bool write(const C *const object) override
    {
        if (!object) {
            std::cerr << "Could not write data to file: " << m_mapFile.name << ". Data is empty" << std::endl;
            return false;
        }

        const size_t size = sizeof(*object);
        if (size > MAX_OBJECT_SIZE) {
            std::cerr << "Could not write data to file: " << m_mapFile.name << ". Size of object:" << size
                      << " is too big" << std::endl;
            return false;
        }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage-in-libc-call"
        std::memcpy(m_mapFile.handle, object, size);
#pragma clang diagnostic pop

        return true;
    }

private:
    const size_t MAX_OBJECT_SIZE;
    const MappedFile m_mapFile;
    Mutex m_mutex;

private:
    sm_object_impl(const sm_object_impl &) = delete;
    sm_object_impl &operator=(const sm_object_impl &) = delete;
};

} // namespace psi
