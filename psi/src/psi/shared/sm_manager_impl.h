
#pragma once

#include <assert.h>
#include <map>
#include <mutex>

#include "psi/shared/i_sm_manager.h"

#include "sm_object_impl.h"

namespace psi {

template <typename C>
class sm_manager_impl final : public i_typed_sm_manager<C>
{
public:
    sm_manager_impl(const std::string &name)
        : m_mutex(name + "SMM")
    {
        const std::string fName = typeid(C).name();
        auto separator = fName.find_last_of(':');
        if (separator == std::string::npos) {
            separator = fName.find_last_of(' ');
        }

        m_fileName = fName.substr(separator + 1) + "_" + name;
    }

    ~sm_manager_impl() override
    {
        freeMemory();
    }

    static std::shared_ptr<sm_manager_impl> create(const std::string &name = "")
    {
        // Static mutex protects concurrent first-call initialization.
        static std::mutex s_createMutex;
        static auto *managers = new std::map<std::string, std::shared_ptr<sm_manager_impl<C>>>();

        std::lock_guard<std::mutex> guard(s_createMutex);
        auto itr = managers->find(name);
        if (itr != managers->end()) {
            return itr->second;
        }

        auto mgr = std::make_shared<sm_manager_impl<C>>(name);
        auto [it, _] = managers->emplace(name, mgr);
        return it->second;
    }

    bool isShared() override
    {
        std::lock_guard<Mutex> lock(m_mutex);

        // open existing shared object
#ifdef __linux__
        int objectId = shm_open(mapFileName().c_str(), O_EXCL | O_RDWR, 0);
        if (objectId != -1) {
            return true;
        }
#endif

#ifdef _WIN32
        const auto objectId = OpenFileMapping(FILE_MAP_ALL_ACCESS,    // read/write access
                                              FALSE,                  // do not inherit the name
                                              mapFileName().c_str()); // name of mapping object
        if (objectId != nullptr && GetLastError() == ERROR_ALREADY_EXISTS) {
            return true;
        }
#endif

        return false;
    }

    void loadToShared(const void *object, size_t object_sz) override
    {
        static_assert(std::is_trivially_copyable_v<C>, "Shared memory object must be trivially copyable");

        if (!object || object_sz != sizeof(C)) {
            assert(false && "Shared memory write: invalid object or size");
            return;
        }

        std::lock_guard<Mutex> lock(m_mutex);

        createImpl();
        if (object && m_sharedObject) {
            m_sharedObject->lock();
            m_sharedObject->write(static_cast<const C *>(object));
            m_sharedObject->unlock();
        }
    }

    void *loadFromShared() override
    {
        std::lock_guard<Mutex> lock(m_mutex);

        createImpl();
        return m_sharedObject ? m_sharedObject->read() : nullptr;
    }

    std::shared_ptr<i_sm_object<C>> getSharedMemory() override
    {
        return m_sharedObject;
    }

    std::string name() const override
    {
        return m_fileName;
    }

private:
    void freeMemory()
    {
        std::lock_guard<Mutex> lock(m_mutex);
        m_sharedObject.reset();
    }

    const MappedFile tryOpenMap(const char *mapName)
    {
        MappedFile mapFile;
        mapFile.name = mapName;

        // open existing shared object
#ifdef __linux__
        mapFile.id = shm_open(mapName, O_EXCL | O_RDWR, 0);
        if (mapFile.id != -1) {
            // load shared object into memory space
            mapFile.handle = mmap(NULL, MAX_OBJECT_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mapFile.id, 0);
            return mapFile;
        }
#endif

#ifdef _WIN32
        mapFile.id = OpenFileMapping(FILE_MAP_ALL_ACCESS, // read/write access
                                     FALSE,               // do not inherit the name
                                     mapName);            // name of mapping object
        if (mapFile.id != nullptr) {
            // load shared object into memory space
            mapFile.handle = MapViewOfFile(mapFile.id,          // handle to map object
                                           FILE_MAP_ALL_ACCESS, // read/write permission
                                           0,
                                           0,
                                           MAX_OBJECT_SIZE);
            return mapFile;
        }
#endif

        // create shared object
#ifdef __linux__
        mapFile.id = shm_open(mapName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        if (mapFile.id == -1) {
            std::cerr << "Could not create/open shared object: " << m_fileName << std::endl;
            return mapFile;
        }

        // init shared object
        if (ftruncate(mapFile.id, MAX_OBJECT_SIZE) == -1) {
            std::cerr << "Could not init shared object: " << m_fileName << std::endl;
            return mapFile;
        }
#endif

#ifdef _WIN32
        mapFile.id = CreateFileMapping(INVALID_HANDLE_VALUE, // use paging file
                                       nullptr,              // default security
                                       PAGE_READWRITE,       // read/write access
                                       0,                    // maximum object size (high-order DWORD)
                                       MAX_OBJECT_SIZE,      // maximum object size (low-order DWORD)
                                       mapName);             // name of mapping object
        if (mapFile.id == nullptr) {
            std::cerr << "Could not create/open shared object: " << m_fileName << std::endl;
            return mapFile;
        }
#endif

        // load shared object into memory space
#ifdef __linux__
        mapFile.handle = mmap(NULL, MAX_OBJECT_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mapFile.id, 0);
#endif

#ifdef _WIN32
        mapFile.handle = MapViewOfFile(mapFile.id,          // handle to map object
                                       FILE_MAP_ALL_ACCESS, // read/write permission
                                       0,
                                       0,
                                       MAX_OBJECT_SIZE);
#endif

        return mapFile;
    }

    void createImpl()
    {
        // create/open shared memory
        const auto &mapFile = tryOpenMap(mapFileName().c_str());
        if (!mapFile.handle) {
            std::cerr << "Could not create/open mapping file: " << m_fileName << std::endl;
            return;
        }

        // std::cout << "size: " << sizeof(C) << ", max_size: " << MAX_OBJECT_SIZE << std::endl;
        m_sharedObject = std::make_shared<sm_object_impl<C>>(MAX_OBJECT_SIZE, mapFile);
    }

    std::string mapFileName() const
    {
        return std::string("shmap.") + m_fileName;
    }

private:
    static constexpr size_t MAX_OBJECT_SIZE = sizeof(C);
    std::string m_fileName;
    Mutex m_mutex;
    std::shared_ptr<sm_object_impl<C>> m_sharedObject;

private:
    sm_manager_impl(const sm_manager_impl &) = delete;
    sm_manager_impl &operator=(const sm_manager_impl &) = delete;
};

} // namespace psi
