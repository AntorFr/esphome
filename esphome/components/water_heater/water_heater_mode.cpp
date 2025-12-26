#include "water_heater_mode.h"

namespace esphome::water_heater {

const char *water_heater_mode_to_string(WaterHeaterMode mode) {
  switch (mode) {
    case WATER_HEATER_MODE_OFF:
      return "OFF";
    case WATER_HEATER_MODE_ECO:
      return "ECO";
    case WATER_HEATER_MODE_ELECTRIC:
      return "ELECTRIC";
    case WATER_HEATER_MODE_PERFORMANCE:
      return "PERFORMANCE";
    case WATER_HEATER_MODE_HIGH_DEMAND:
      return "HIGH_DEMAND";
    case WATER_HEATER_MODE_HEAT_PUMP:
      return "HEAT_PUMP";
    case WATER_HEATER_MODE_GAS:
      return "GAS";
    default:
      return "UNKNOWN";
  }
}

}  // namespace esphome::water_heater
