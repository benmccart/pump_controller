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

#include <algorithm>

#include "config.hpp"
#include "pump_state.hpp"

#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>

namespace io
{

    init_register_t read_init_register(JsonObjectConst const &obj) noexcept
    {
        uint16_t const reg_addr = obj["reg"];
        uint16_t const value = obj["value"];
        return init_register_t{ register_t{ reg_addr }, value};
    }

    analog_input_t analog_id_to_pin(std::string_view id) noexcept
    {
        if (id == "A0")
            return analog_input_t{ A0 };
        if (id == "A1")
            return analog_input_t{ A1 };
        if (id == "A2")
            return analog_input_t{ A2 };
        if (id == "A3")
            return analog_input_t{ A3 };
        if (id == "A4")
            return analog_input_t{ A4 };
        if (id == "A5")
            return analog_input_t{ A5 };

        return analog_input_t{ 255u };
    }

    input_source_t read_input(JsonVariantConst const &val) noexcept
    {
        
        JsonObjectConst const &obj = val;
        JsonVariantConst const &reg = obj["register"];
        if (!reg.isNull())
            return register_t{ static_cast<uint16_t>(reg) };

        JsonVariantConst const &ai = obj["analog_input"];
        if (!ai.isNull())
            return analog_id_to_pin(static_cast<char const*>(ai));

        return register_t{ 9999u };
    }

    input_source_t read_input(std::string_view key, JsonDocument &doc) noexcept
    {
        JsonVariantConst const &val = doc[key.data()];
        return read_input(val);
    }

    optional_input_t read_optional_input(std::string_view key, JsonDocument &doc) noexcept
    {
        JsonVariantConst const &val = doc[key.data()];
        if (val.isNull())
            return optional_input_t{};

        return read_input(val);
    }

    control::run_args_t read_run_args(JsonDocument &doc) noexcept
    {
        uint16_t const run = doc["run_args"]["run"];
        uint16_t const stop = doc["run_args"]["stop"];
        return control::run_args_t{ .run = run, .stop = stop };
    }

    control::stepper_point_t read_stepper_point(JsonObjectConst const &levels, std::string_view key) noexcept
    {
        JsonObjectConst const &stepper = levels[key.data()];
        uint16_t const pressure = stepper["pressure"];
        uint16_t const frequency = stepper["frequency"];
        return control::stepper_point_t{ .pressure = pressure, .frequency = frequency };
    }

    control::stepper_levels_t read_stepper_levels(JsonDocument &doc) noexcept
    {
        JsonObjectConst const &lvls = doc["stepper_levels"];
        control::stepper_point_t const stop = read_stepper_point(lvls, "stop");
        control::stepper_point_t const fill = read_stepper_point(lvls, "fill");
        control::stepper_point_t const start = read_stepper_point(lvls, "start");

        return control::stepper_levels_t
        {
            .stop = stop,
            .fill = fill,
            .start = start
        };
    }
   
    configuration_t read_config(std::string_view filename, modbus_t &modbus, logger_t &logger) noexcept
    {
        configuration_t default_config
        {
            io::modbus_id,
            io::modbus_serial_speed,
            init_registers_t {},
            control::args_t
            {
                &modbus,
                reg_run,
                reg_frequency,
                reg_pressure,
                reg_flood,
                stepper_levels,
                run_args,
                flood_trigger,
                flood_timeout
            }
        };

        File file = SD.open(filename.data());
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, file);
        if (error)
        {
            logger.log_on_failure(!static_cast<bool>(error), error.c_str());
            return default_config;
        }

        uint32_t const modbus_buad = doc["modbus_baud"];
        uint8_t const modbus_id = static_cast<uint32_t>(doc["modbus_id"]);
        
        init_registers_t init_regs;
        JsonArray jinitregs = doc["init_registers"];
        std::size_t const num_regs = std::min(init_regs.capacity(), jinitregs.size());
        auto itr_end = jinitregs.begin();
        for (std::size_t i = 0; i != num_regs; ++i, ++itr_end) {};
        std::transform(jinitregs.begin(), itr_end, std::back_inserter(init_regs), [&](JsonObjectConst const &obj)
        {
            return read_init_register(obj);
        });

        register_t const run_reg{ static_cast<uint16_t>(doc["run_register"]) };
        register_t const frequency_reg{ static_cast<uint16_t>(doc["frequency_register"]) };
        optional_input_t const flood_input = read_optional_input("flood_input", doc);
        input_source_t const pressure_input = read_input("pressure_input", doc);

        control::stepper_levels_t const stepper_lvls = read_stepper_levels(doc);
        control::run_args_t const run_args = read_run_args(doc);
        uint16_t const flood_trigger_value = doc["flood_trigger_value"];
        chrono::duration_t const flood_timeout = std::chrono::minutes{ static_cast<unsigned long>(doc["flood_timeout"]) };

        return configuration_t
        {
            modbus_id,
            modbus_buad,
            init_regs,
            control::args_t
            {
                &modbus,
                run_reg,
                frequency_reg,
                pressure_input,
                flood_input,
                stepper_lvls,
                run_args,
                flood_trigger_value,
                flood_timeout
            }
        };
    }
}