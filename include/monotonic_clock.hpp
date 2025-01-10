/**
 * Copyright (c) 2025 Ben McCart
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

#ifndef TIME_UTIL_HPP_
#define TIME_UTIL_HPP_

#include <chrono>
#include <cstdint>

namespace chrono
{
    struct monotonic_time_t
    {
        using time_t = int64_t;
        time_t milliseconds;
    };

    constexpr monotonic_time_t start_time{ 31536000000 }; // 1 year.
    using time_point_t = std::chrono::system_clock::time_point;
    using duration_t = std::chrono::system_clock::duration;

    monotonic_time_t to_monotonic(time_point_t) noexcept;
    time_point_t from_monotonic(monotonic_time_t const &) noexcept;

    using timer_callback_t = void (*)(time_point_t);

    class monotonic_clock_t
    {
    public:
        monotonic_clock_t() noexcept;
        time_point_t now() noexcept;
        
    private:
       void update() noexcept;

        uint32_t last_millis_;
        monotonic_time_t time_;
    };










}

#endif // TIME_UTIL_HPP_