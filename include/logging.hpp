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

#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include <array>
#include <string_view>

#include <tl/expected.hpp>

#include "display.hpp"
#include "modbus_io.hpp"

#include <SPI.h>
#include <SD.h>


namespace io
{
    constexpr char *log_file_path = "PMPCTRL.LOG";
    class logger_t
    {
    public:
        logger_t(display_t&) noexcept;

        void begin_log(std::string_view) noexcept;

        template <class T>
        void log_on_error(tl::expected<T,modbus_error_t> const&) noexcept;

        void log_on_failure(bool, std::string_view) noexcept;

        void log(modbus_error_t) noexcept;
        void log(value_msg_t) noexcept;
        void log(std::string_view) noexcept;
        void log(std::string_view, std::uint32_t) noexcept;

        void flush() noexcept;

    private:
        display_t &display_;
        File file_;
    };

    template <class T>
    void logger_t::log_on_error(tl::expected<T,modbus_error_t> const &exp) noexcept
    {
        if (!exp)
            log(exp.error());
    }


}




#endif // LOGGING_HPP_