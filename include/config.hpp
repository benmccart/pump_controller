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

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include <chrono>
#include <string_view>

#include <etl/vector.h>

#include "logging.hpp"
#include "modbus_io.hpp"
#include "pump_state.hpp"

namespace io
{

    struct init_register_t
    {
        register_t reg;
        uint16_t value = 0u;
    };
    using init_registers_t = etl::vector<init_register_t, 8u>;



    // Constant expressions for default values.
    constexpr unsigned long modbus_serial_speed = 19200;
    constexpr uint8_t modbus_id = 1u;
    constexpr io::register_t reg_main_run_sel{ 0x0002 };
    constexpr io::register_t reg_main_freq_sel{ 0x0005 };
    constexpr io::register_t reg_flood{ 0x0C19 };
    constexpr io::register_t reg_pressure{ 0x0C1A };
    constexpr io::register_t reg_run{ 0x2501 };
    constexpr io::register_t reg_frequency{ 0x2502 };
    constexpr uint16_t flood_trigger = 500u;
    constexpr std::chrono::system_clock::duration flood_timeout = std::chrono::minutes(60u);

    constexpr control::stepper_levels_t stepper_levels
    { 
        .stop = control::stepper_point_t
        {
            .pressure = 840u,
            .frequency = 0u
        },
        .fill = control::stepper_point_t
        {
            .pressure = 790,
            .frequency = 5400u
        },
        .start = control::stepper_point_t
        {
            .pressure = 765,
            .frequency = 6000u
        }
    };

    constexpr control::run_args_t run_args
    {
        .run = 1u,
        .stop = 0u,
    };

    constexpr pin_size_t cs_pin = 10u;

    struct configuration_t
    {
        uint8_t modbus_id = 1u;
        uint32_t modbus_buad = 0u;
        init_registers_t init_registers;
        control::args_t args;
    };

    configuration_t read_config(std::string_view, modbus_t &, logger_t&) noexcept;
}

#endif // CONFIG_HPP_