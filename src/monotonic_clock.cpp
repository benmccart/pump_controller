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

#include "monotonic_clock.hpp"

#include <Arduino.h>

namespace // anonymous namespace - no extern linkage.
{
    chrono::monotonic_clock_t *instance = nullptr;
}


namespace chrono
{
    monotonic_time_t to_monotonic(time_point_t time) noexcept
    {
        auto const duration = time.time_since_epoch();
        return monotonic_time_t{ std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() };
    }

    time_point_t from_monotonic(monotonic_time_t const &time) noexcept
    {
        std::chrono::milliseconds duration(time.milliseconds);
        return std::chrono::system_clock::time_point(duration);
    }

    monotonic_clock_t::monotonic_clock_t() noexcept
    : last_millis_(0u), time_(start_time)
    {
        if (::instance == nullptr)
            ::instance = this;
    }

    time_point_t monotonic_clock_t::now() noexcept
    {
        update();
        return from_monotonic(time_);
    }

    void monotonic_clock_t::update() noexcept
    {
        uint32_t const current = millis();
        monotonic_time_t::time_t const ellapsed = [&]()
        {
            if (current < last_millis_)
            {
                monotonic_time_t::time_t const remainder = (std::numeric_limits<uint32_t>::max() - last_millis_);
                return remainder + 1ll + static_cast<monotonic_time_t::time_t>(current);
            }
            else
            {
                return static_cast<monotonic_time_t::time_t>(current - last_millis_);
            }
        }();
        time_.milliseconds += ellapsed;
        last_millis_ = current;
    }

}
