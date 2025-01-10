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