#pragma once

#include "psi/shared/ISharedMemoryManager.hpp"
#include "psi/shared/ISharedMemoryObject.hpp"

#include <fcntl.h>
#include <iostream>
#ifdef __linux__
#include <sys/mman.h>
#endif

namespace psi {

template <typename C, SynchType S>
ISharedMemoryManager<C, S>::~ISharedMemoryManager()
{
    freeMemory();
}

template <typename C, SynchType S>
std::shared_ptr<ISharedMemoryManager<C, S>> ISharedMemoryManager<C, S>::getInstance(const std::string &name)
{
    static std::map<std::string, std::shared_ptr<ISharedMemoryManager<C, S>>> managers;
    auto itr = managers.find(name);
    if (itr != managers.end()) {
        return itr->second;
    }

    managers.emplace(name, std::shared_ptr<ISharedMemoryManager>(new ISharedMemoryManager(name)));
    return managers[name];
}

template <typename C, SynchType S>
bool ISharedMemoryManager<C, S>::isShared()
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

template <typename C, SynchType S>
void ISharedMemoryManager<C, S>::loadToShared(const C *const object)
{
    std::lock_guard<Mutex> lock(m_mutex);

    createImpl();
    if (object && m_sharedObject) {
        m_sharedObject->lock();
        m_sharedObject->write(object);
        m_sharedObject->unlock();
    }
}

template <typename C, SynchType S>
C *ISharedMemoryManager<C, S>::loadFromShared()
{
    std::lock_guard<Mutex> lock(m_mutex);

    createImpl();
    return m_sharedObject ? m_sharedObject->read() : nullptr;
}

template <typename C, SynchType S>
std::shared_ptr<IBaseSharedMemoryObject<C, S>> ISharedMemoryManager<C, S>::getSharedMemory()
{
    return m_sharedObject;
}

template <typename C, SynchType S>
void ISharedMemoryManager<C, S>::freeMemory()
{
    std::lock_guard<Mutex> lock(m_mutex);
    m_sharedObject.reset();
}

template <typename C, SynchType S>
const std::string ISharedMemoryManager<C, S>::name() const
{
    return m_fileName;
}

template <typename C, SynchType S>
ISharedMemoryManager<C, S>::ISharedMemoryManager(const std::string &name)
    : m_mutex(name + "SMM")
{
    const std::string fName = typeid(C).name();
    auto separator = fName.find_last_of(':');
    if (separator == std::string::npos) {
        separator = fName.find_last_of(' ');
    }

    m_fileName = fName.substr(separator + 1) + "_" + name;
}

template <typename C, SynchType S>
const MappedFile ISharedMemoryManager<C, S>::tryOpenMap(const char *mapName)
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
                                   NULL,                 // default security
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

template <typename C, SynchType S>
void ISharedMemoryManager<C, S>::createImpl()
{
    // create/open shared memory
    const auto &mapFile = tryOpenMap(mapFileName().c_str());
    if (!mapFile.handle) {
        std::cerr << "Could not create/open mapping file: " << m_fileName << std::endl;
        return;
    }

    // std::cout << "size: " << sizeof(C) << ", max_size: " << MAX_OBJECT_SIZE << std::endl;
    auto ptr = std::make_shared<ISharedMemoryObject<C, S>>(MAX_OBJECT_SIZE, mapFile);
    m_sharedObject = std::dynamic_pointer_cast<IBaseSharedMemoryObject<C, S>>(ptr);
}

template <typename C, SynchType S>
std::string ISharedMemoryManager<C, S>::mapFileName() const
{
    return std::string("shmap.") + m_fileName;
}

} // namespace psi
