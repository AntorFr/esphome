#pragma once

#include <cstdint>

namespace esphome::water_heater {

/// Enum for all modes a water heater can be in.
enum WaterHeaterMode : uint8_t {
  /// The water heater is off.
  WATER_HEATER_MODE_OFF = 0,
  /// The water heater is in eco/energy saving mode.
  WATER_HEATER_MODE_ECO = 1,
  /// The water heater is using electric heating.
  WATER_HEATER_MODE_ELECTRIC = 2,
  /// The water heater is in performance/boost mode.
  WATER_HEATER_MODE_PERFORMANCE = 3,
  /// The water heater is in high demand mode.
  WATER_HEATER_MODE_HIGH_DEMAND = 4,
  /// The water heater is using heat pump.
  WATER_HEATER_MODE_HEAT_PUMP = 5,
  /// The water heater is using gas heating.
  WATER_HEATER_MODE_GAS = 6,
};

/// Convert the given WaterHeaterMode to a human-readable string.
const char *water_heater_mode_to_string(WaterHeaterMode mode);

}  // namespace esphome::water_heater
