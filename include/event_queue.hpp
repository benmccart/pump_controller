#ifndef EVENT_QUEUE_HPP_
#define EVENT_QUEUE_HPP_

#include <chrono>
#include <cstdint>

#include <etl/priority_queue.h>

#include "monotonic_clock.hpp"

namespace chrono
{
    constexpr std::size_t max_queued_events = 32u;

    using event_handler_t = void (*)(time_point_t, time_point_t);

    struct event_t
    {
        event_t(event_handler_t, time_point_t) noexcept;
        event_t() = delete;
        event_t(event_t const&) noexcept = default;
        event_t(event_t &&) noexcept = default;
        event_t& operator=(event_t const&) noexcept = default;
        event_t& operator=(event_t &&) noexcept = default;

        event_handler_t handler = nullptr;
        time_point_t time;
    };

    constexpr bool operator<(event_t const &lhs, event_t const &rhs) noexcept
    {
        bool const greater = lhs.time > rhs.time; // ETL uses operator< to sort items in descending rather than ascending order... stupid.
        return greater;
    }

    class event_queue_t
    {
    public:
        event_queue_t() noexcept;
        bool schedule(event_t const&) noexcept;
        void process_events(time_point_t) noexcept;

    private:
        etl::priority_queue<event_t,max_queued_events> queue_;
    };
}



#endif // EVENT_QUEUE_HPP_