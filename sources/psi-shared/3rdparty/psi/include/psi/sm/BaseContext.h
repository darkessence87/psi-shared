
#pragma once

#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>

#ifdef PSI_LOGGER
#include "psi/logger/Logger.h"
#else
#include <iostream>
#include <sstream>
#define LOG_TRACE(x)                                                                                                   \
    do {                                                                                                               \
        std::ostringstream os;                                                                                         \
        os << x;                                                                                                       \
        std::cout << os.str() << std::endl;                                                                            \
    } while (0)
#endif

#include "BaseState.h"
#include "ProcessResult.h"

namespace psi::sm {

/**
 * @brief BaseContext class is a thread-safe state machine.
 * The concept is:
 * - only one state @BaseState<IState> exists at any time
 * - events can be anything fast copyable
 * - posted events are always processed before deferred events
 * - deferred events are re-processed only after state is changed
 * - IState is a state interface with at least one 'ProcessResult react(const T &event)' method:
 *      virtual ProcessResult react(const EvExample&) { return ProcessResult::UnconsumedEvent; }
 * 
 * @todo requires small optimizations in events passing through sequences
 * 
 * @tparam IState 
 */
template <typename IState>
class BaseContext
{
public:
    virtual ~BaseContext() = default;

    /// @brief In fact this is a type of functions which are processed by queue. These will capture events to be processed later.
    using Func = std::function<ProcessResult()>;

    /// @brief Short alias to base state type.
    using IBaseState = typename BaseState<IState>;

    /**
     * @brief Processes event in accordence with current state.
     * 
     * @tparam T type of event
     * @param ev evnt object
     */
    template <typename T>
    void process_event(const T &ev)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        auto rs = process_event_impl(ev);

        switch (rs) {
        case ProcessResult::UnknownState:
            // LOG_TRACE("Unknown state");
            break;

        case ProcessResult::DeferredEvent:
            m_deferred.emplace_back([this, ev]() {
                // LOG_TRACE("[" << m_state->name() << "] process deferred " << tools::objName(ev) << ". Queue size: " << queueSize());
                return process_event_impl(ev);
            });
            // LOG_TRACE("[" << m_state->name() << "] defer " << tools::objName(ev) << ". Queue size: " << queueSize());
            break;

        case ProcessResult::TransitState:
            // LOG_TRACE("[" << m_state->name() << "] end transit by " << tools::objName(ev) << ". Queue size: " << queueSize());
            process_queue();
            break;

        case ProcessResult::PostedEvent:
            m_posted.emplace_back([this, ev]() {
                // LOG_TRACE("[" << m_state->name() << "] process posted " << tools::objName(ev) << ". Queue size: " << queueSize());
                return process_event_impl(ev);
            });
            // LOG_TRACE("[" << m_state->name() << "] posted " << tools::objName(ev) << ". Queue size: " << queueSize());
            process_queue();
            break;

        case ProcessResult::UnconsumedEvent:
            // LOG_TRACE("[" << m_state->name() << "] unconsumed " << tools::objName(ev) << ". Queue size: " << queueSize());
            break;

        case ProcessResult::DiscardedEvent:
            // LOG_TRACE("[" << m_state->name() << "] discarded " << tools::objName(ev) << ". Queue size: " << queueSize());
            break;
        }
    }

    /**
     * @brief Posts event to be processed with high priority.
     * Operation is thread-safe.
     * 
     * @tparam T type of event
     * @param ev event object
     */
    template <typename T>
    void post_event(const T &ev)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        m_posted.emplace_back([this, ev]() {
            // LOG_TRACE("[" << m_state->name() << "] process posted " << tools::objName(ev) << ". Queue size: " << queueSize());
            return process_event_impl(ev);
        });
        // LOG_TRACE("[" << m_state->name() << "] post " << tools::objName(ev) << ". Queue size: " << queueSize());
    }

    /**
     * @brief Performs transition from old state (if exists) to new state.
     * Operation is thread-safe.
     * 
     * @tparam NextState type of new state
     */
    template <typename NextState>
    void transit()
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        transit_impl<NextState>();
        process_queue();
    }

    /**
     * @brief Returns pointer to current state if it exists.
     * 
     * @return std::optional<IState *> 
     */
    std::optional<IState *> currentState() const
    {
        return m_state ? std::make_optional<IState *>(m_state.get()) : std::nullopt;
    }

protected:
    /**
     * @brief Performs transition from old state (if exists) to new state.
     * 
     * @tparam NewState type of new state
     */
    template <typename NewState>
    void transit_impl()
    {
        IBaseState *st = new NewState();
        st->m_context = this;

        const auto newSt = st->name();
        if (m_state) {
            const auto oldSt = m_state->name();
            LOG_TRACE("[" << oldSt << "] => [" << newSt << "] transition");
        } else {
            LOG_TRACE("[" << newSt << "] Initialize state");
        }

        m_state.reset(st);
    }

    /**
     * @brief Sends event to current state
     * 
     * @tparam EventType event type to be reacted by state
     * @param ev event object
     * @return ProcessResult result of state's reaction on event
     */
    template <typename EventType>
    ProcessResult process_event_impl(const EventType &ev)
    {
        // if (m_state) {
        //     LOG_TRACE("[" << m_state->name() << "] react " << tools::objName(ev) << ". Queue size: " << queueSize());
        // }
        return m_state ? m_state->react(ev) : ProcessResult::UnknownState;
    }

    /**
     * @brief Prioritizes and processing events created during state's reaction on event.
     * Posted events are processed first, then deferred.
     * Any posted event or state change re-starts the processing.
     * Deferred events are processed only once per cycle in a context of single state.
     */
    virtual void process_queue()
    {
        // LOG_TRACE("m_posted: " << m_posted.size() << ", m_deferred: " << m_deferred.size()
        //                        << ", m_queue: " << m_queue.size());

        m_queue.insert(m_queue.begin(), m_posted.begin(), m_posted.end());
        m_posted.clear();

        m_queue.insert(m_queue.end(), m_deferred.begin(), m_deferred.end());
        m_deferred.clear();

        if (m_queue.empty()) {
            return;
        }

        bool transitionFound = false;
        while (!m_queue.empty()) {
            auto fn = m_queue.front();
            m_queue.pop_front();

            auto rs = fn();

            switch (rs) {
            case ProcessResult::DeferredEvent:
                m_deferred.emplace_back(fn);
                // LOG_TRACE("[" << m_state->name() << "] re-defer event. Queue size: " << queueSize());
                break;

            case ProcessResult::TransitState:
                transitionFound = true;
                break;

            case ProcessResult::PostedEvent:
                m_posted.emplace_back(fn);
                // LOG_TRACE("[" << m_state->name() << "] post event. Queue size: " << queueSize());
                transitionFound = true;
                break;

            case ProcessResult::DiscardedEvent:
                // LOG_TRACE("[" << m_state->name() << "] discard event. Queue size: " << queueSize());
                break;
            }

            if (transitionFound) {
                break;
            }
        }

        if (transitionFound) {
            process_queue();
        }
    }

    size_t queueSize() const
    {
        return m_deferred.size() + m_posted.size() + m_queue.size();
    }

protected:
    /// @brief recursive mutex is used because state may also call context's methods during event processing
    std::recursive_mutex m_mutex;

    /// @brief state's lifetime is limited and managed by context
    std::unique_ptr<IState> m_state;

    /// @brief queue of posted events, is processed with highest priority
    std::deque<Func> m_posted;

    /// @brief queue of deferred events, is processed with lowest priority
    std::deque<Func> m_deferred;

    /// @brief main processing events queue
    std::deque<Func> m_queue;

private:
    // BaseContext(const BaseContext &) = delete;
    BaseContext &operator=(const BaseContext &) = delete;
};

} // namespace psi::sm