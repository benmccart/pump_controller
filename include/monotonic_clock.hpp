#ifndef TIME_UTIL_HPP_
#define TIME_UTIL_HPP_

#include <chrono>
#include <cstdint>

#include <RTC.h>

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