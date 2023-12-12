
#pragma once

#include <ostream>

namespace psi::sm {

enum class ProcessResult
{
    UnknownContext,     /// returned if BaseState was not created by BaseContext
    UnknownState,       /// returned if no default state set by context

    UnconsumedEvent,    /// returned if no reaction is implemented by state for specified event
    DeferredEvent,      /// returned if event should be postponed and processed in another state
    DiscardedEvent,     /// returned if event is finally processed
    PostedEvent,        /// returned if event has to be processed immediately after current reaction
    TransitState,       /// returned if new state is created by context
};

inline std::ostream &operator<<(std::ostream &os, const ProcessResult &rs)
{
    os << "ProcessResult::";

    switch (rs) {
    case ProcessResult::UnknownContext:
        os << "UnknownContext";
        break;
    case ProcessResult::UnknownState:
        os << "UnknownState";
        break;
    case ProcessResult::UnconsumedEvent:
        os << "UnconsumedEvent";
        break;
    case ProcessResult::DeferredEvent:
        os << "DeferredEvent";
        break;
    case ProcessResult::DiscardedEvent:
        os << "DiscardedEvent";
        break;
    case ProcessResult::PostedEvent:
        os << "PostedEvent";
        break;
    case ProcessResult::TransitState:
        os << "TransitState";
        break;
    }

    os << std::endl;

    return os;
}

} // namespace psi::sm
