#pragma once

#include "esphome/core/finite_set_mask.h"
#include "esphome/core/helpers.h"
#include "water_heater_mode.h"

namespace esphome::water_heater {

/// Bitmask for water heater modes.
using WaterHeaterModeMask =
    FiniteSetMask<WaterHeaterMode, DefaultBitPolicy<WaterHeaterMode, WATER_HEATER_MODE_GAS + 1>>;

/// This class contains all static data for water heater devices.
class WaterHeaterTraits {
 public:
  WaterHeaterTraits() = default;

  /// Return whether this water heater device supports reporting current temperature.
  bool get_supports_current_temperature() const { return this->supports_current_temperature_; }
  void set_supports_current_temperature(bool supports_current_temperature) {
    this->supports_current_temperature_ = supports_current_temperature;
  }

  /// Return whether this water heater device supports away mode.
  bool get_supports_away() const { return this->supports_away_; }
  void set_supports_away(bool supports_away) { this->supports_away_ = supports_away; }

  /// Set the modes this device supports.
  void set_supported_modes(WaterHeaterModeMask modes) { this->supported_modes_ = modes; }
  const WaterHeaterModeMask &get_supported_modes() const { return this->supported_modes_; }
  bool supports_mode(WaterHeaterMode mode) const { return this->supported_modes_.count(mode) > 0; }

  float get_visual_min_temperature() const { return this->visual_min_temperature_; }
  void set_visual_min_temperature(float visual_min_temperature) {
    this->visual_min_temperature_ = visual_min_temperature;
  }
  float get_visual_max_temperature() const { return this->visual_max_temperature_; }
  void set_visual_max_temperature(float visual_max_temperature) {
    this->visual_max_temperature_ = visual_max_temperature;
  }
  float get_visual_temperature_step() const { return this->visual_temperature_step_; }
  void set_visual_temperature_step(float visual_temperature_step) {
    this->visual_temperature_step_ = visual_temperature_step;
  }

 protected:
  bool supports_current_temperature_{false};
  bool supports_away_{false};
  WaterHeaterModeMask supported_modes_{WATER_HEATER_MODE_OFF};
  float visual_min_temperature_{30.0f};
  float visual_max_temperature_{70.0f};
  float visual_temperature_step_{1.0f};
};

}  // namespace esphome::water_heater
