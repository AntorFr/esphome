#include "template_water_heater.h"
#include "esphome/core/log.h"

namespace esphome::template_ {

using namespace esphome::water_heater;

static const char *const TAG = "template.water_heater";

TemplateWaterHeater::TemplateWaterHeater()
    : mode_trigger_(new Trigger<WaterHeaterMode>()),
      target_temperature_trigger_(new Trigger<float>()),
      away_trigger_(new Trigger<bool>()) {}

void TemplateWaterHeater::setup() {
  switch (this->restore_mode_) {
    case WATER_HEATER_NO_RESTORE:
      break;
    case WATER_HEATER_RESTORE: {
      auto restore = this->restore_state_();
      if (restore.has_value()) {
        restore->apply(this);
      }
      break;
    }
    case WATER_HEATER_RESTORE_AND_CALL: {
      auto restore = this->restore_state_();
      if (restore.has_value()) {
        restore->to_call(this).perform();
      }
      break;
    }
  }
  if (!this->state_f_.has_value())
    this->disable_loop();
}

void TemplateWaterHeater::loop() {
  bool changed = false;

  auto s = this->state_f_();
  if (s.has_value()) {
    if (s->mode != this->mode) {
      this->mode = s->mode;
      changed = true;
    }
    if (s->current_temperature != this->current_temperature) {
      this->current_temperature = s->current_temperature;
      changed = true;
    }
    if (s->target_temperature != this->target_temperature) {
      this->target_temperature = s->target_temperature;
      changed = true;
    }
    if (s->away != this->away) {
      this->away = s->away;
      changed = true;
    }
  }

  if (changed)
    this->publish_state();
}

void TemplateWaterHeater::set_optimistic(bool optimistic) { this->optimistic_ = optimistic; }
void TemplateWaterHeater::set_supports_mode(bool supports_mode) { this->supports_mode_ = supports_mode; }
void TemplateWaterHeater::set_supports_target_temperature(bool supports_target_temperature) {
  this->supports_target_temperature_ = supports_target_temperature;
}
void TemplateWaterHeater::set_supports_away(bool supports_away) { this->supports_away_ = supports_away; }

float TemplateWaterHeater::get_setup_priority() const { return setup_priority::HARDWARE; }

Trigger<WaterHeaterMode> *TemplateWaterHeater::get_mode_trigger() const { return this->mode_trigger_; }
Trigger<float> *TemplateWaterHeater::get_target_temperature_trigger() const { return this->target_temperature_trigger_; }
Trigger<bool> *TemplateWaterHeater::get_away_trigger() const { return this->away_trigger_; }

void TemplateWaterHeater::dump_config() {
  LOG_WATER_HEATER("", "Template Water Heater", this);
  ESP_LOGCONFIG(TAG, "  Optimistic: %s", YESNO(this->optimistic_));
}

void TemplateWaterHeater::control(const WaterHeaterCall &call) {
  if (call.get_mode().has_value()) {
    auto mode = *call.get_mode();
    this->mode_trigger_->trigger(mode);
    if (this->optimistic_) {
      this->mode = mode;
    }
  }
  if (call.get_target_temperature().has_value()) {
    auto target_temperature = *call.get_target_temperature();
    this->target_temperature_trigger_->trigger(target_temperature);
    if (this->optimistic_) {
      this->target_temperature = target_temperature;
    }
  }
  if (call.get_away().has_value()) {
    auto away = *call.get_away();
    this->away_trigger_->trigger(away);
    if (this->optimistic_) {
      this->away = away;
    }
  }
  this->publish_state();
}

WaterHeaterTraits TemplateWaterHeater::traits() {
  auto traits = WaterHeaterTraits();
  traits.set_supports_current_temperature(true);
  traits.set_supports_away(this->supports_away_);
  return traits;
}

}  // namespace esphome::template_
