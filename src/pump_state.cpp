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

#include <array>
#include <chrono>
#include <string_view>

#include "logging.hpp"
#include "pump_state.hpp"

namespace
{
    control::pump_t *instance = nullptr;
}

// Visitor for lambdas that handle visiting the types.
template <typename... Ts>
struct lambda_visitor : Ts... 
{
    using Ts::operator()...;
};

// Deduction guide.
template <typename... Ts>
lambda_visitor(Ts...) -> lambda_visitor<Ts...>;

namespace control
{
    constexpr io::register_t reg_flood{ 0x0C19 };
    constexpr io::register_t reg_pressure{ 0x0C1A };

    pump_t::pump_t(io::logger_t &lggr, chrono::monotonic_clock_t &tm, chrono::event_queue_t &vnts) noexcept
    : logger_(lggr), time_(tm), events_(vnts), failed_pressure_(0u), flooded_(false)
    {
        if (::instance == nullptr)
            ::instance = this;
    }

    void pump_t::begin(args_t args) noexcept
    {
        args_ = args;
        state_ = state_t
        {
             .run = state_item_t
            {
                args_.run_reg,
                0u,
                0u
            },
            .frequency = state_item_t
            {
                args_.frequency_reg,
                0u,
                0u
            }
        };

        full_stop();
        auto expected = push_state(state_.run, true);
        logger_.log_on_error(expected);
        expected = push_state(state_.frequency, true);
        logger_.log_on_error(expected);

        auto expected_pressure = read_input(args_.pressure);
        if (expected_pressure)
            logger_.log("pressure: " , *expected_pressure);

        if (args_.flood)
        {
            auto expected_flood = read_input(*args_.flood);
            if (expected_flood)
            logger_.log("flood: ", *expected_flood);
        }
    }

    void pump_t::update() noexcept
    {   
        pull_full_state();

        // Apply logic to current input state.
        auto expected_pressure = read_input(args_.pressure);
        handle_pressure_update(expected_pressure);
        handle_flood_condition();

        push_full_state();
        logger_.log(io::value_msg_t{ "run: ", state_.run.reg, state_.run.desired });
        logger_.log(io::value_msg_t{ "frequency: ", state_.frequency.reg, state_.frequency.desired });
    }

    [[nodiscard]] pump_t::optional_value_t pump_t::read_input(io::input_source_t input) const noexcept
    {
        auto visitor = lambda_visitor
        {
            [&](io::register_t reg) 
            {
                auto expected = args_.modbus->read_holding_register(reg);
                logger_.log_on_error(expected);
                if (expected)
                    return optional_value_t{ expected.value() };
                else
                    return optional_value_t{};
                
            },
            [&](io::analog_input_t ai)
            {
                int value = analogRead(ai.pin);
                return optional_value_t{ static_cast<uint16_t>(value) };
            }
        };
        return std::visit(visitor, input);
    }

    [[nodiscard]] io::expected_void_t pump_t::push_state(state_item_t const &si, bool force) const noexcept
    {
        if (force || si.current != si.desired)
            return args_.modbus->write_register(si.reg, si.desired);
        else
            return io::expected_void_t{};
    }

    [[nodiscard]] io::expected_void_t pump_t::pull_state(state_item_t &si) noexcept
    {
        auto expected = args_.modbus->read_holding_register(si.reg);
        if (!expected)
            return tl::make_unexpected(expected.error());

        si.current = expected.value();
        return io::expected_void_t{};
    }

    [[nodiscard]] bool pump_t::is_running() noexcept
    {
        return state_.run.desired == args_.run_args.run;
    }

    void pump_t::pull_full_state() noexcept
    {
        auto expected = pull_state(state_.run);
        logger_.log_on_error(expected);
        expected = pull_state(state_.frequency);
        logger_.log_on_error(expected);
    }

    void pump_t::push_full_state() noexcept
    {
        auto expected = push_state(state_.run);
        logger_.log_on_error(expected);
        expected = push_state(state_.frequency);
        logger_.log_on_error(expected);
    }

    void pump_t::handle_pressure_update(optional_value_t expected_pressure) noexcept
    {
        if (expected_pressure)
        {
            failed_pressure_ = 0u;
            uint16_t const pressure = *expected_pressure;
            update_run(pressure);
            update_frequency(pressure);
            logger_.log(io::value_msg_t{ "pressure: ", reg_pressure, pressure });
        }
        else
        {
            if (++failed_pressure_ == 3u)
            {
                failed_pressure_ = 0u;
                full_stop();
            }
        }
    }

    void pump_t::handle_flood_condition() noexcept
    {
        if (args_.flood)
        {
            auto expected_flood = read_input(*args_.flood);
            if (expected_flood)
            {
                uint16_t flood = *expected_flood;
                update_flood(flood);
                logger_.log(io::value_msg_t{ "flood: ", reg_flood, flood });
            }
    
            if (flooded_)
                full_stop();
        }
    }

    void pump_t::update_run(uint16_t pressure) noexcept
    {
        stepper_levels_t const &levels = args_.levels;

        // Handle start condition.
        if (pressure <= levels.start.pressure)
            state_.run.desired = args_.run_args.run;

        // Handle stop condition.
        if (pressure >= levels.stop.pressure)
            state_.run.desired = args_.run_args.stop;
    }

    void pump_t::update_frequency(uint16_t pressure) noexcept
    {
        stepper_levels_t const &levels = args_.levels;
        if (state_.run.desired == args_.run_args.run)
        {
            if (pressure < levels.fill.pressure)
                state_.frequency.desired = levels.start.frequency;
            else if (pressure < levels.stop.pressure)
                state_.frequency.desired = levels.fill.frequency;
            else
                state_.frequency.desired = levels.stop.frequency;
        }
        else
        {
            state_.frequency.desired = 0u;
        }
    }

    void pump_t::update_flood(uint16_t flood) noexcept
    {
        if ((flood <= args_.flood_trigger_value) && !flooded_)
        {
            flooded_ = true;
            auto flooded_callback = [](chrono::time_point_t scheduled_time, chrono::time_point_t now)
            {
                ::instance->flooded_ = false;
            };

            auto now = time_.now();
            auto end_timeout = now + args_.flood_timeout;
            events_.schedule(chrono::event_t{ flooded_callback, end_timeout });
        }
    }

    void pump_t::full_stop() noexcept
    {
        state_.frequency.desired = 0u;
        state_.run.desired = args_.run_args.stop;
    }
}