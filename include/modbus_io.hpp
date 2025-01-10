#ifndef MODBUS_IO_HPP_
#define MODBUS_IO_HPP_

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

#include <tl/expected.hpp>

#include <ModbusMaster.h>

namespace io
{
    enum class modbus_error_t
    {
        success = ModbusMaster::ku8MBSuccess,
        illegal_function = ModbusMaster::ku8MBIllegalFunction,
        illegal_data_address = ModbusMaster::ku8MBIllegalDataAddress,
        illegal_data_value = ModbusMaster::ku8MBIllegalDataValue,
        slave_device_failure = ModbusMaster::ku8MBSlaveDeviceFailure,
        invalid_slave_id = ModbusMaster::ku8MBInvalidSlaveID,
        invalid_function = ModbusMaster::ku8MBInvalidFunction,
        response_timeout = ModbusMaster::ku8MBResponseTimedOut,
        invalid_crc = ModbusMaster::ku8MBInvalidCRC
    };

    enum class modbus_connection_t
    {
        disconnected,
        connected,
        response_timeout,
        slave_device_failure,
        invalid_slave_id
    };

    using expected_void_t = tl::expected<void,modbus_error_t>;
    using expected_value_t = tl::expected<uint16_t,modbus_error_t>;

    struct pin_t
    {
        uint8_t id = ~0;
    };
    using optional_pint_t = std::optional<pin_t>;

    struct connection_args_t
    {
        uint8_t id = 0u;
        Stream &serial;
        optional_pint_t data_enable;
        optional_pint_t receiver_enable;
    };

    struct register_t
    {
        uint16_t address = 0u;
    };

    class modbus_t
    {
     public:
        modbus_t() noexcept;

        void connect(connection_args_t) noexcept;
        [[nodiscard]] constexpr modbus_connection_t connection_status() const noexcept  { return connection_status_; }
        [[nodiscard]] expected_value_t read_holding_register(register_t);
        [[nodiscard]] expected_void_t write_register(register_t,uint16_t) noexcept;
        void reset() noexcept;

     private:
        void pre_transmission() noexcept;
        void post_transmission() noexcept;

        Stream *stream_;
        ModbusMaster *modbus_;
        alignas(alignof(ModbusMaster)) char buffer_[sizeof(ModbusMaster)];
        modbus_connection_t connection_status_;
        optional_pint_t data_enable;
        optional_pint_t receiver_enable;
    };

    constexpr std::string_view error_message(modbus_error_t error) noexcept
    {
        switch (error)
        {
            case modbus_error_t::success: return "modbus success";
            case modbus_error_t::illegal_function: return "modbus error: illegal function";
            case modbus_error_t::illegal_data_address: return "modbus error: illegal data address";
            case modbus_error_t::illegal_data_value: return "modbus error: illegal data value";
            case modbus_error_t::slave_device_failure: return "modbus error: slave device failure";
            case modbus_error_t::invalid_slave_id: return "modbus error: invalid slave id";
            case modbus_error_t::invalid_function: return "modbus error: invalid function";
            case modbus_error_t::response_timeout: return "modbus error: response timeout";
            case modbus_error_t::invalid_crc: return "modbus error: invalid crc";
            default: return "";
        }
    }
}

#endif // MODBUS_IO_HPP_
