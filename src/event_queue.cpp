#include "event_queue.hpp"

#include <etl/vector.h>

namespace chrono
{

    event_t::event_t(event_handler_t hndlr, time_point_t tm) noexcept
    : handler(hndlr), time(tm)
    {}

    event_queue_t::event_queue_t() noexcept
    {}

    bool event_queue_t::schedule(event_t const &event) noexcept
    {
        if (queue_.full() || event.handler == nullptr)
            return false;

        queue_.push(event);
    }

    template <class T>
    auto begin(etl::vector<T,max_queued_events> &cont) noexcept { return cont.begin();  }

    template <class T>
    auto end(etl::vector<T,max_queued_events> &cont) noexcept   { return cont.end();    }

    void event_queue_t::process_events(time_point_t now) noexcept
    {
        etl::vector<event_t,max_queued_events> events;
        while (!queue_.empty() && queue_.top().time <= now)
        {
            events.push_back(queue_.top());
            queue_.pop();
        }

        for (event_t const &event : events)
        {
            event.handler(event.time, now);
        }
    }

}