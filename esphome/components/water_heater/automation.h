#pragma once

#include "esphome/core/automation.h"
#include "water_heater.h"

namespace esphome::water_heater {

template<typename... Ts> class ControlAction : public Action<Ts...> {
 public:
  explicit ControlAction(WaterHeater *water_heater) : water_heater_(water_heater) {}

  TEMPLATABLE_VALUE(WaterHeaterMode, mode)
  TEMPLATABLE_VALUE(float, target_temperature)
  TEMPLATABLE_VALUE(bool, away)

  void play(Ts... x) override {
    auto call = this->water_heater_->make_call();
    if (this->mode_.has_value()) {
      call.set_mode(this->mode_.value(x...));
    }
    if (this->target_temperature_.has_value()) {
      call.set_target_temperature(this->target_temperature_.value(x...));
    }
    if (this->away_.has_value()) {
      call.set_away(this->away_.value(x...));
    }
    call.perform();
  }

 protected:
  WaterHeater *water_heater_;
};

class StateTrigger : public Trigger<WaterHeater &> {
 public:
  explicit StateTrigger(WaterHeater *water_heater) {
    water_heater->add_on_state_callback([this](WaterHeater &water_heater) { this->trigger(water_heater); });
  }
};

class ControlTrigger : public Trigger<WaterHeaterCall &> {
 public:
  explicit ControlTrigger(WaterHeater *water_heater) {
    water_heater->add_on_control_callback([this](WaterHeaterCall &call) { this->trigger(call); });
  }
};

}  // namespace esphome::water_heater
