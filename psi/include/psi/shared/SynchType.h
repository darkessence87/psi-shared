#pragma once

namespace psi {

enum class SynchType
{
    InterProcess,
    InterThread,
};

inline const char *asString(SynchType st)
{
    switch (st) {
    case SynchType::InterProcess:
        return "InterProcess";
    case SynchType::InterThread:
        return "InterThread";
    }

    return "Unknown";
}

} // namespace psi
