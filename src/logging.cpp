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

#include "logging.hpp"

namespace io
{

    logger_t::logger_t(display_t &dsply) noexcept
    : display_(dsply)
    {}

    void logger_t::begin_log(std::string_view file) noexcept
    {
        file_ = SD.open(file.data(), FILE_WRITE);
    }

    void logger_t::log(modbus_error_t err) noexcept
    {
        display_.set(err);
        switch (err)
        {
            case modbus_error_t::slave_device_failure:
            case modbus_error_t::invalid_slave_id:
            file_.println(error_message(err).data());
            break;
        }
    }

    void logger_t::log(value_msg_t vm) noexcept
    {
        display_.set(vm);
    }

    void logger_t::log(std::string_view msg) noexcept
    {
        file_.println(msg.data());
    }

    void logger_t::log(std::string_view msg, std::uint32_t value) noexcept
    {
        file_.print(msg.data());
        file_.println(value);
    }

    void logger_t::log_on_failure(bool passed, std::string_view msg) noexcept
    {
        if (!passed)
        {
            display_.set(msg);
            file_.println(msg.data());
        }
    }

    void logger_t::flush() noexcept
    {
        file_.flush();
    }
}