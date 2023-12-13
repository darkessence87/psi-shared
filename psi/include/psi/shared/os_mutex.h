#pragma once

#ifdef __linux__
#include <semaphore.h>
using Semaphore = sem_t*;
#elif _WIN32
#include <windows.h>
using Semaphore = HANDLE;
#endif

#include <string>

namespace psi {

class Mutex
{
public:
    Mutex(const std::string &name = "_Unnamed");
    ~Mutex();

    void lock();
    void unlock();

private:
    Semaphore m_handle = nullptr;
    const std::string m_name;

    Mutex(const Mutex &other) = delete;
    Mutex &operator=(const Mutex &other) = delete;
};

} // namespace psi
