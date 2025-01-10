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

#include <algorithm>

#include "display.hpp"

namespace
{
    io::display_t *instance = nullptr;

    // 40 characters.
    TEXT_ANIMATION_DEFINE(anim, 200)
}

    
namespace io
{
    display_t::display_t() noexcept
    : next_(true), toggle_msg_(0u)
    {
        if (::instance == nullptr)
            ::instance = this;
    }

    bool display_t::begin() noexcept
    {
        return matrix_.begin();
    }

    void display_t::update(chrono::time_point_t now) noexcept
    {
        if ((now > next_timeout_) && !next_)
            next_ = true;

        if (!next_)
            return;

        if (!has_msg())
            return;

        next(now);
    }
   
    void display_t::set(modbus_error_t err) noexcept
    {
        auto itr = std::find(errors_.begin(), errors_.end(), err);
        if (itr == errors_.end() && errors_.size() < errors_.capacity())
        {
            errors_.push(err);
        }
    }

    void display_t::set(value_msg_t msg) noexcept
    {
        auto itr = std::find_if(values_.begin(), values_.end(), [&](value_msg_t const &m)
        {
            return m.id.address == msg.id.address;
        });
        if (itr != values_.end())
        {
            *itr = msg;
        }
        else if (values_.size() < values_.capacity())
        {
            values_.push(msg);
        }
    }

    void display_t::set(std::string_view msg) noexcept
    {
        auto itr = std::find(failures_.begin(), failures_.end(), msg);
        if (itr == failures_.end() && failures_.size() < failures_.capacity())
        {
            failures_.push(msg);
        }
    }

    void display_t::next(chrono::time_point_t now) noexcept
    {
        next_ = false;

        constexpr uint32_t milliseconds_per_frame = 100u;

        matrix_.beginDraw();
        matrix_.stroke(0xFFFFFFFF);
        matrix_.textFont(Font_5x7);
        matrix_.textScrollSpeed(milliseconds_per_frame);
        matrix_.setCallback([]()
        {
            ::instance->next_ = true;
        });
  
        matrix_.beginText(0, 1, 0xFFFFFF);
        print_next_msg(matrix_);
        matrix_.endTextAnimation(SCROLL_LEFT, anim);
  
        matrix_.loadTextAnimationSequence(anim);
        matrix_.play();

        uint32_t const frame_count = (anim_buf_used / 4) / sizeof(uint32_t);
        uint32_t const milliseconds_trip = (frame_count + 2u) * milliseconds_per_frame;
        next_timeout_ = now + std::chrono::milliseconds{ milliseconds_trip };
    }

    bool display_t::has_msg() noexcept
    {
        if (!errors_.empty())
            return true;

        if (!values_.empty())
            return true;

        if (!failures_.empty())
            return true;
        
        return false;
    }

    constexpr uint16_t num_msg_categories = 3u;
    void display_t::print_next_msg(ArduinoLEDMatrix &matrix) noexcept
    {
        ++toggle_msg_;
        for (uint16_t i = 0; i != num_msg_categories; ++i, ++toggle_msg_)
        {
            uint16_t const option = toggle_msg_ % num_msg_categories;
            if (option == 0u && !errors_.empty())
            {
                modbus_error_t err = errors_.front();
                errors_.pop();
                auto msg = error_message(err);
                matrix.print("  ");
                matrix.println(msg.data());
                return;
            }
            if (option == 1u && !values_.empty())
            {
                value_msg_t vm = values_.front();
                values_.pop();
                matrix.print("  ");
                matrix.print(vm.msg.data());
                matrix.println(vm.value);
                return;
            }
            if (option == 2u && !failures_.empty())
            {
                std::string_view msg = failures_.front();
                failures_.pop();
                matrix.print("  ");
                matrix.println(msg.data());
                return;
            }
        }
    }
}