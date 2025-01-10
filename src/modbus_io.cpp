#include "modbus_io.hpp"

namespace // Anonymouse namespace to prevent export via 'extern' keyword.
{
    io::modbus_t *instance = nullptr;
}

namespace io
{
    constexpr std::size_t buffer_size = sizeof(ModbusMaster);

    constexpr modbus_connection_t error_to_connection(modbus_error_t error) noexcept
    {
        switch (error)
        {
            case modbus_error_t::slave_device_failure: return modbus_connection_t::slave_device_failure;
            case modbus_error_t::invalid_slave_id: return modbus_connection_t::invalid_slave_id;
            case modbus_error_t::response_timeout: return modbus_connection_t::response_timeout;
            default: return modbus_connection_t::connected;
        }
    }

    modbus_t::modbus_t() noexcept
    : stream_(nullptr), modbus_(nullptr), connection_status_(modbus_connection_t::disconnected)
    {
        if (::instance == nullptr)
            ::instance = this;

        reset();
    }

    void modbus_t::connect(connection_args_t args) noexcept
    {
        stream_ = &args.serial;
        modbus_->begin(args.id, args.serial);

        auto setup_pin = [](optional_pint_t op)
        {
            if (op)
            {
                pinMode(op->id, OUTPUT);
                digitalWrite(op->id, LOW);
            }
        };

        setup_pin(args.data_enable);
        setup_pin(args.receiver_enable);

        modbus_->preTransmission([]()
        {
            ::instance->pre_transmission();
        });
        modbus_->postTransmission([]()
        {
            ::instance->post_transmission();
        });

        connection_status_ = modbus_connection_t::connected;
    }

    [[nodiscard]] expected_value_t modbus_t::read_holding_register(register_t reg) noexcept
    {
        delay(20); // NOTE: Manual large minimum silent interval since TECO A510 has modbus implementation issues.
        auto err = modbus_->readHoldingRegisters(reg.address, 1u);
        if (err != ModbusMaster::ModbusMaster::ku8MBSuccess)
        {
            modbus_error_t const me = static_cast<modbus_error_t>(err);
            connection_status_ = error_to_connection(me);
            return tl::make_unexpected(me);
        }

        connection_status_ = modbus_connection_t::connected;
        uint16_t const value = modbus_->getResponseBuffer(0u);
        modbus_->clearResponseBuffer();
        return value;
    }

    [[nodiscard]] expected_void_t modbus_t::write_register(register_t reg, uint16_t value) noexcept
    {
        delay(20); // NOTE: Manual large minimum silent interval since TECO A510 has modbus implementation issues.
        auto err = modbus_->writeSingleRegister(reg.address, value);
        if (err != ModbusMaster::ModbusMaster::ku8MBSuccess)
        {
            modbus_error_t const me = static_cast<modbus_error_t>(err);
            connection_status_ = error_to_connection(me);
            return tl::make_unexpected(me);
        }

        return expected_void_t{};
    }

    void modbus_t::reset() noexcept
    {
        static_assert(sizeof(buffer_) == sizeof(ModbusMaster));
        std::fill_n(buffer_, sizeof(ModbusMaster), '\0');
        modbus_ = new (buffer_) ModbusMaster{};
    }

    void write_pin(optional_pint_t op, int v)
    {
        if (op)
            digitalWrite(op->id, v);
    }

    void modbus_t::pre_transmission() noexcept
    {
        write_pin(data_enable, HIGH);
        write_pin(receiver_enable, HIGH);
    }

    void modbus_t::post_transmission() noexcept
    {
        write_pin(data_enable, LOW);
        write_pin(receiver_enable, LOW);
    }
}