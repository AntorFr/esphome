#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/template_lambda.h"
#include "esphome/components/water_heater/water_heater.h"

namespace esphome::template_ {

enum TemplateWaterHeaterRestoreMode {
  WATER_HEATER_NO_RESTORE,
  WATER_HEATER_RESTORE,
  WATER_HEATER_RESTORE_AND_CALL,
};

struct WaterHeaterState {
  water_heater::WaterHeaterMode mode;
  float current_temperature;
  float target_temperature;
  bool away;
};

class TemplateWaterHeater final : public water_heater::WaterHeater, public Component {
 public:
  TemplateWaterHeater();

  template<typename F> void set_state_lambda(F &&f) { this->state_f_.set(std::forward<F>(f)); }

  Trigger<water_heater::WaterHeaterMode> *get_mode_trigger() const;
  Trigger<float> *get_target_temperature_trigger() const;
  Trigger<bool> *get_away_trigger() const;

  void set_optimistic(bool optimistic);
  void set_supports_mode(bool supports_mode);
  void set_supports_target_temperature(bool supports_target_temperature);
  void set_supports_away(bool supports_away);
  void set_restore_mode(TemplateWaterHeaterRestoreMode restore_mode) { this->restore_mode_ = restore_mode; }

  void setup() override;
  void loop() override;
  void dump_config() override;

  float get_setup_priority() const override;

 protected:
  void control(const water_heater::WaterHeaterCall &call) override;
  water_heater::WaterHeaterTraits traits() override;

  TemplateWaterHeaterRestoreMode restore_mode_{WATER_HEATER_NO_RESTORE};
  TemplateLambda<WaterHeaterState> state_f_;
  bool optimistic_{false};
  bool supports_mode_{false};
  bool supports_target_temperature_{false};
  bool supports_away_{false};

  Trigger<water_heater::WaterHeaterMode> *mode_trigger_;
  Trigger<float> *target_temperature_trigger_;
  Trigger<bool> *away_trigger_;
};

}  // namespace esphome::template_
