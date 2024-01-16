IN_PROGRESS
# Description
This library is used for communication between processes (IPC) in a scope of single machine using shared memory approach.

It consists of 3 spaces for data transfer:
* [call space](https://github.com/darkessence87/psi-shared/blob/master/psi/include/psi/shared/ipc/space/CallSpace.h): is written by clients, is read by server
* [callback space](https://github.com/darkessence87/psi-shared/blob/master/psi/include/psi/shared/ipc/space/CallbackSpace.h): is read by clients, is written by server
* [event space](https://github.com/darkessence87/psi-shared/blob/master/psi/include/psi/shared/ipc/space/EventSpace.h): is read by clients, is written by server

Synchronization is done using semaphores on Linux and mutexes on Windows.
IPCServer and IPCClient have to implement same interface (see example below).

Acceptable function definitions ([details](https://github.com/darkessence87/psi-shared/blob/master/psi/include/psi/shared/ipc/server/MemberFnWrapper.h)):
* void call ( ) *// without arguments*
* void call ( serializable_param... ) *// with any number of serializable arguments*
* void call ( serializable_callback ) *// with only serializable callback*
* void call ( serializable_param... , serializable_callback ) *// with any number of serializable arguments and serializable callback in the end*
* serializable_param call ( ) *// serializable return value, without arguments*
* serializable_param call ( serializable_param... ) *// serializable return value,  with any number of serializable arguments*
> serializable_param: any object for which 'serialization' template specialized
> 
> serializable_callback: function with arguments of any object(s) for which 'serialization' template specialized

# Usage examples
- [1.0_Simple_SharedMemory](https://github.com/darkessence87/psi-shared/tree/master/psi/examples/1.0_Simple_SharedMemory)
