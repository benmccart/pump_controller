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