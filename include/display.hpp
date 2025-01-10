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

#ifndef DISPLAY_HPP_
#define DISPLAY_HPP_

#include <chrono>
#include <cstdint>
#include <optional>
#include <string_view>

#include <etl/circular_buffer.h>

#include "modbus_io.hpp"
#include "monotonic_clock.hpp"

// NOTE: PlatformIO's copy of standard Arduino libraries for this platform were stale:
// "%UserProfile%\.platformio\packages\framework-arduinorenesas-uno\libraries\Arduino_LED_Matrix"
// so I overwrote it with the one copied from where Arduino IDE installed it at:
// "%UserProfile%\AppData\Local\Arduino15\packages\arduino\hardware\renesas_uno\1.3.1\libraries\Arduino_LED_Matrix"
// You may also need to do the same if there are missing method calls of your Arduino_LED_Matrix object.

#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#include "TextAnimation.h"

namespace io
{

    struct value_msg_t
    {
        std::string_view msg;
        register_t id;
        uint64_t value;
    };

    class display_t
    {
    public:
        display_t() noexcept;

        bool begin() noexcept;
        void update(chrono::time_point_t) noexcept;

        void set(modbus_error_t) noexcept;
        void set(value_msg_t) noexcept;
        void set(std::string_view) noexcept;

    private:
        void next(chrono::time_point_t) noexcept;
        bool has_msg() noexcept;
        void print_next_msg(ArduinoLEDMatrix &matrix) noexcept;

        ArduinoLEDMatrix matrix_;
        bool next_;
        uint8_t toggle_msg_;
        chrono::time_point_t next_timeout_;
        
        etl::circular_buffer<modbus_error_t,8> errors_;
        etl::circular_buffer<value_msg_t,8> values_;
        etl::circular_buffer<std::string_view,8> failures_;
    };

}
#endif // DISPLAY_HPP_
