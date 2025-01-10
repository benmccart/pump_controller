/**
 * Copyright (c) <year> <copyright holders>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
 * to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
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