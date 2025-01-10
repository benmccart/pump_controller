/**
* Copyright (c) 2024 Ben McCart
* The following references were used/consulted for the development of this project.
*
*
* https://docs.arduino.cc/hardware/uno-r4-wifi/


* https://docs.platformio.org/en/latest/boards/renesas-ra/uno_r4_wifi.html
* https://docs.platformio.org/en/latest/platforms/renesas-ra.html#packages
* https://docs.platformio.org/en/latest/projectconf/sections/env/options/platform/index.html#platform-packages
* https://github.com/renesas/fsp/releases
*
*
*
*/

#include <array>
#include <chrono>
#include <cstdint>
#include <string_view>

#include "config.hpp"
#include "display.hpp"
#include "event_queue.hpp"
#include "logging.hpp"
#include "modbus_io.hpp"
#include "pump_state.hpp"
#include "monotonic_clock.hpp"


io::modbus_t modbus;
io::display_t display;
io::logger_t logger{ display };
chrono::monotonic_clock_t rtc_time;
chrono::event_queue_t events;
control::pump_t pump{ logger, rtc_time, events };

constexpr chrono::duration_t pump_update_interval = std::chrono::milliseconds(250u);
void handle_pump_update(chrono::time_point_t scheduled_time, chrono::time_point_t now)
{
  pump.update();
  events.schedule(chrono::event_t{ handle_pump_update, now + pump_update_interval });
}

void setup() 
{
  display.begin();
  delay(100);

  // Disable other SPI devices.
  pinMode(io::cs_pin, OUTPUT);
  digitalWrite(io::cs_pin, HIGH);

  logger.log_on_failure(SD.begin(io::cs_pin), "SD init failed");
  logger.begin_log(io::log_file_path);
  logger.log("-- pump controller startup --");

  io::configuration_t config = read_config("CONFIG.JSN", modbus, logger);

  Serial1.begin(config.modbus_buad);
  modbus.connect(io::connection_args_t{config.modbus_id, Serial1 });
  
  delay(100); // Allow some start-up time after modbus connection before pump start-up logic.
  pump.begin(config.args);
  
  // Start events processing!
  chrono::time_point_t now = rtc_time.now();
  events.schedule(chrono::event_t{ handle_pump_update, now + pump_update_interval });
}


constexpr std::size_t flush_interval = 100u; // A 4 second interval.
std::size_t loop_iterator = 0;
void loop() 
{
  auto t0 = millis();
  auto now = rtc_time.now();
  events.process_events(now);
  now = rtc_time.now();
  display.update(now);

  // Flush file buffer on interval.
  if (++loop_iterator % flush_interval == 0u)
    logger.flush();

  // Calc the duration to delay, if any.
  auto t1 = millis();
  auto elapsed = t1 - t0;
  if (elapsed <= 10ul)
    delay(10ul - elapsed);
}