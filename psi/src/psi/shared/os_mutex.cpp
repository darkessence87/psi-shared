#include "psi/shared/os_mutex.h"

#ifdef __linux__
#include <fcntl.h>
#include <sys/stat.h>
#endif

#include <iostream>

namespace psi {

Mutex::Mutex(const std::string &name)
    : m_name(name)
{
#ifdef __linux__
    m_handle = sem_open(m_name.c_str(), 0, S_IRUSR | S_IWUSR, 1);
    if (m_handle == SEM_FAILED) {
        m_handle = sem_open(m_name.c_str(), O_CREAT, S_IRUSR | S_IWUSR, 1);
    }
    if (m_handle == SEM_FAILED) {
        m_handle = nullptr;
        std::cerr << "[psi::Mutex] sem_open failed for '" << m_name << "'\n";
    }
#elif _WIN32
    m_handle = CreateMutex(nullptr, 0, m_name.c_str());
    if (m_handle == nullptr) {
        std::cerr << "[psi::Mutex] CreateMutex failed for '" << m_name << "' error=" << GetLastError() << "\n";
    }
#endif
}

Mutex::~Mutex()
{
#ifdef __linux__
    if (m_handle != nullptr) {
        sem_close(m_handle);
    }
#elif _WIN32
    if (m_handle != nullptr) {
        CloseHandle(m_handle);
    }
#endif
}

void Mutex::lock()
{
    if (m_handle == nullptr) {
        return;
    }
#ifdef __linux__
    sem_wait(m_handle);
#elif _WIN32
    WaitForSingleObject(m_handle, INFINITE);
#endif
}

void Mutex::unlock()
{
    if (m_handle == nullptr) {
        return;
    }
#ifdef __linux__
    sem_post(m_handle);
#elif _WIN32
    ReleaseMutex(m_handle);
#endif
}

} // namespace psi
