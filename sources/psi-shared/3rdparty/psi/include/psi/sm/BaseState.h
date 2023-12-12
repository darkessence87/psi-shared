
#pragma once

#include "ProcessResult.h"

namespace psi::sm {

template <typename T>
class BaseContext;

/**
 * @brief BaseState is class responsible for react on events and providing result of reaction.
 * 
 * @tparam IState 
 */
template <typename IState>
class BaseState : public IState
{
public:
    /// @brief Short alias to base context.
    using Context = typename BaseContext<IState>;

    /// @brief Returns name of state.
    /// @return name of state
    virtual const std::string &name() const
    {
        return m_name;
    }

protected:
    /**
     * @brief Construct a new BaseState object.
     * Context will be set by creator of object (BaseContext).
     * 
     * @param name name of state
     */
    BaseState(const std::string &name)
        : m_name(name)
        , m_context(nullptr)
    {
    }

    /**
     * @brief Destroy the Base State object
     * 
     */
    virtual ~BaseState() = default;

    /**
     * @brief Performs transition of context to new state.
     * Lifetime of 'this' ends before return from function.
     * 
     * @tparam NextState type of new state to be created
     * @param postEvent bool value, set it to 'true' if currently processed event is required to be processed in a new state.
     * Default value: 'false'
     * @return ProcessResult result of processing. Might be either 'PostedEvent', either 'TransitState'
     */
    template <typename NextState>
    ProcessResult transit(bool postEvent = false)
    {
        if (m_context) {
            m_context->transit<NextState>();
            return postEvent ? ProcessResult::PostedEvent : ProcessResult::TransitState;
        }

        return ProcessResult::UnknownContext;
    }

    /**
     * @brief Currently processed event is required to be deferred.
     * 
     * @return ProcessResult DeferredEvent
     */
    ProcessResult defer_event()
    {
        return ProcessResult::DeferredEvent;
    }

    /**
     * @brief Currently processed event is no longer required or ignored.
     * 
     * @return ProcessResult DiscardedEvent
     */
    ProcessResult discard_event()
    {
        return ProcessResult::DiscardedEvent;
    }

    /**
     * @brief Posts event to be processed by context with high priority.
     * 
     * @tparam T type of event
     * @param ev event object
     */
    template <typename T>
    void post_event(const T &ev)
    {
        if (m_context) {
            m_context->post_event(ev);
        }
    }

    /**
     * @brief Gives access to context in scope of state.
     * 
     * @tparam DerivedContext type of context
     * @return DerivedContext* pointer to context object
     */
    template <typename DerivedContext>
    DerivedContext *context()
    {
        return static_cast<DerivedContext *>(m_context);
    }

protected:
    /// @brief state's name
    const std::string m_name;

    /// @brief pointer to context (state's parent)
    Context *m_context;
    friend class Context;

private:
    BaseState(const BaseState &) = delete;
    BaseState &operator=(const BaseState &) = delete;
};

} // namespace psi::sm