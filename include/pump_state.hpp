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

#ifndef PUMP_STATE_HPP_
#define PUMP_STATE_HPP_

#include <chrono>
#include <cstdint>
#include <optional>
#include <variant>

#include "event_queue.hpp"
#include "logging.hpp"
#include "modbus_io.hpp"
#include "monotonic_clock.hpp"

namespace io
{
    /**
    * Analog input pin.
    */
    struct analog_input_t
    {
        pin_size_t pin = 0u;
    };

    using input_source_t = std::variant<register_t, analog_input_t>;
    using optional_input_t = std::optional<io::input_source_t>;


}

namespace control
{

    struct state_item_t
    {
        io::register_t reg;
        uint16_t current = 0u;
        uint16_t desired = 0u;
    };

    struct state_t
    {
        state_item_t run;
        state_item_t frequency;
    };

    /**
    * A point at which the state of the pump will change, represented by a pressure and a frequency. At the specified pressure
    * there is a state change.
    */
    struct stepper_point_t
    {
        uint16_t pressure;
        uint16_t frequency;
    };

    /**
    * Levels of operation,.
    * When stopped, the pump will start only if the pressure gets into the range [0, start.pressure]
    * When running, start.frequency is used in the range [0, fill.pressure)
    * When running, the fill.frequency is used in the range [fill.pressure, stop.pressure)
    * When running, the pump will stop only if the pressure gets into the range [stop.pressure, std::numeric_limits<uint16_t>::max()]
    */
    struct stepper_levels_t
    {
        stepper_point_t stop;
        stepper_point_t fill;
        stepper_point_t start;
    };

    struct run_args_t
    {
        uint16_t run  = 0x0001u; /**< Defaults to 1u, but could be any bit pattern. */
        uint16_t stop = 0x0000u; /**< Defaults to 0u, but could be any bit pattern. */
    };

    /**
    * Arguments used to initialize the pump controller.  The flood register is optional.  If supplied is means you have a liquid sensor hooked
    * up that when triggered by coming into contact with a liquid will trigger a low state below the specified value, that will indicate
    * a flood condition in which the pump shall be stopped for the specified timeout duration.
    */
    struct args_t
    {
        io::modbus_t *modbus = nullptr;
        io::register_t run_reg;
        io::register_t frequency_reg;
        io::input_source_t pressure;
        io::optional_input_t  flood;

        stepper_levels_t levels;
        run_args_t run_args;

        uint16_t flood_trigger_value;
        chrono::duration_t flood_timeout;
    };

    class pump_t
    {
    public:
        pump_t(io::logger_t&, chrono::monotonic_clock_t&, chrono::event_queue_t&) noexcept;
        void begin(args_t) noexcept;
        void update() noexcept;

    private:
        using optional_value_t = std::optional<uint16_t>;

        [[nodiscard]] optional_value_t read_input(io::input_source_t) const noexcept;
        [[nodiscard]] io::expected_void_t push_state(state_item_t const&, bool = false) const noexcept;
        [[nodiscard]] io::expected_void_t pull_state(state_item_t&) noexcept;
        [[nodiscard]] bool is_running() noexcept;
        void pull_full_state() noexcept;
        void push_full_state() noexcept;
        void handle_pressure_update(optional_value_t) noexcept;
        void handle_flood_condition() noexcept;
        void update_run(uint16_t) noexcept;
        void update_frequency(uint16_t) noexcept;
        void update_flood(uint16_t) noexcept;
        void full_stop() noexcept;

        io::logger_t &logger_;
        chrono::monotonic_clock_t &time_;
        chrono::event_queue_t &events_;

        args_t args_;
        state_t state_;

        uint16_t failed_pressure_;
        bool flooded_;
    };
}

#endif // PUMP_STATE_HPP_