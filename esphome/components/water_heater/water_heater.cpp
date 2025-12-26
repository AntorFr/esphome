#include "water_heater.h"

namespace esphome::water_heater {

static const char *const TAG = "water_heater";

WaterHeaterCall &WaterHeaterCall::set_mode(WaterHeaterMode mode) {
  this->mode_ = mode;
  return *this;
}
WaterHeaterCall &WaterHeaterCall::set_mode(optional<WaterHeaterMode> mode) {
  this->mode_ = mode;
  return *this;
}
WaterHeaterCall &WaterHeaterCall::set_mode(const std::string &mode) {
  if (str_equals_case_insensitive(mode, "OFF")) {
    this->set_mode(WATER_HEATER_MODE_OFF);
  } else if (str_equals_case_insensitive(mode, "ECO")) {
    this->set_mode(WATER_HEATER_MODE_ECO);
  } else if (str_equals_case_insensitive(mode, "ELECTRIC")) {
    this->set_mode(WATER_HEATER_MODE_ELECTRIC);
  } else if (str_equals_case_insensitive(mode, "PERFORMANCE")) {
    this->set_mode(WATER_HEATER_MODE_PERFORMANCE);
  } else if (str_equals_case_insensitive(mode, "HIGH_DEMAND")) {
    this->set_mode(WATER_HEATER_MODE_HIGH_DEMAND);
  } else if (str_equals_case_insensitive(mode, "HEAT_PUMP")) {
    this->set_mode(WATER_HEATER_MODE_HEAT_PUMP);
  } else if (str_equals_case_insensitive(mode, "GAS")) {
    this->set_mode(WATER_HEATER_MODE_GAS);
  } else {
    ESP_LOGW(TAG, "'%s' - Unrecognized mode %s", this->parent_->get_name().c_str(), mode.c_str());
  }
  return *this;
}
WaterHeaterCall &WaterHeaterCall::set_target_temperature(float target_temperature) {
  this->target_temperature_ = target_temperature;
  return *this;
}
WaterHeaterCall &WaterHeaterCall::set_target_temperature(optional<float> target_temperature) {
  this->target_temperature_ = target_temperature;
  return *this;
}
WaterHeaterCall &WaterHeaterCall::set_away(bool away) {
  this->away_ = away;
  return *this;
}
WaterHeaterCall &WaterHeaterCall::set_away(optional<bool> away) {
  this->away_ = away;
  return *this;
}

const optional<WaterHeaterMode> &WaterHeaterCall::get_mode() const { return this->mode_; }
const optional<float> &WaterHeaterCall::get_target_temperature() const { return this->target_temperature_; }
const optional<bool> &WaterHeaterCall::get_away() const { return this->away_; }

void WaterHeaterCall::validate_() {
  auto traits = this->parent_->get_traits();

  if (this->mode_.has_value() && !traits.supports_mode(*this->mode_)) {
    ESP_LOGW(TAG, "  Mode %s is not supported by this device!", water_heater_mode_to_string(*this->mode_));
    this->mode_.reset();
  }

  if (this->target_temperature_.has_value()) {
    float min_temp = traits.get_visual_min_temperature();
    float max_temp = traits.get_visual_max_temperature();
    float target = *this->target_temperature_;
    if (target < min_temp || target > max_temp) {
      ESP_LOGW(TAG, "  Target temperature %.1f is out of range (%.1f - %.1f)!", target, min_temp, max_temp);
      this->target_temperature_ = clamp(target, min_temp, max_temp);
    }
  }

  if (this->away_.has_value() && !traits.get_supports_away()) {
    ESP_LOGW(TAG, "  Away mode is not supported by this device!");
    this->away_.reset();
  }
}

void WaterHeaterCall::perform() {
  ESP_LOGD(TAG, "'%s' - Setting", this->parent_->get_name().c_str());
  this->validate_();

  if (this->mode_.has_value()) {
    ESP_LOGD(TAG, "  Mode: %s", water_heater_mode_to_string(*this->mode_));
  }
  if (this->target_temperature_.has_value()) {
    ESP_LOGD(TAG, "  Target Temperature: %.2f", *this->target_temperature_);
  }
  if (this->away_.has_value()) {
    ESP_LOGD(TAG, "  Away: %s", ONOFF(*this->away_));
  }

  this->parent_->control_callback_.call(*this);
  this->parent_->control(*this);
}

WaterHeaterCall WaterHeater::make_call() { return WaterHeaterCall(this); }

void WaterHeater::add_on_state_callback(std::function<void(WaterHeater &)> &&callback) {
  this->state_callback_.add(std::move(callback));
}

void WaterHeater::add_on_control_callback(std::function<void(WaterHeaterCall &)> &&callback) {
  this->control_callback_.add(std::move(callback));
}

void WaterHeater::publish_state() {
  ESP_LOGD(TAG, "'%s' - Publishing:", this->get_name().c_str());
  ESP_LOGD(TAG, "  Mode: %s", water_heater_mode_to_string(this->mode));
  if (!std::isnan(this->current_temperature)) {
    ESP_LOGD(TAG, "  Current Temperature: %.2f°C", this->current_temperature);
  }
  if (!std::isnan(this->target_temperature)) {
    ESP_LOGD(TAG, "  Target Temperature: %.2f°C", this->target_temperature);
  }
  ESP_LOGD(TAG, "  Away: %s", ONOFF(this->away));

  this->state_callback_.call(*this);
  this->save_state_();
}

WaterHeaterTraits WaterHeater::get_traits() {
  auto traits = this->traits();
  if (this->visual_min_temperature_override_.has_value()) {
    traits.set_visual_min_temperature(*this->visual_min_temperature_override_);
  }
  if (this->visual_max_temperature_override_.has_value()) {
    traits.set_visual_max_temperature(*this->visual_max_temperature_override_);
  }
  if (this->visual_temperature_step_override_.has_value()) {
    traits.set_visual_temperature_step(*this->visual_temperature_step_override_);
  }
  return traits;
}

void WaterHeater::set_visual_min_temperature_override(float visual_min_temperature_override) {
  this->visual_min_temperature_override_ = visual_min_temperature_override;
}
void WaterHeater::set_visual_max_temperature_override(float visual_max_temperature_override) {
  this->visual_max_temperature_override_ = visual_max_temperature_override;
}
void WaterHeater::set_visual_temperature_step_override(float visual_temperature_step_override) {
  this->visual_temperature_step_override_ = visual_temperature_step_override;
}

optional<WaterHeaterDeviceRestoreState> WaterHeater::restore_state_() {
  this->rtc_ = global_preferences->make_preference<WaterHeaterDeviceRestoreState>(this->get_object_id_hash());
  WaterHeaterDeviceRestoreState recovered{};
  if (!this->rtc_.load(&recovered))
    return {};
  return recovered;
}

void WaterHeater::save_state_() {
  WaterHeaterDeviceRestoreState state{};
  state.mode = this->mode;
  state.target_temperature = this->target_temperature;
  state.away = this->away;
  this->rtc_.save(&state);
}

void WaterHeater::dump_traits_(const char *tag) {
  auto traits = this->get_traits();
  ESP_LOGCONFIG(tag, "  Supported modes:");
  for (auto mode : traits.get_supported_modes()) {
    ESP_LOGCONFIG(tag, "    - %s", water_heater_mode_to_string(mode));
  }
  ESP_LOGCONFIG(tag, "  Supports current temperature: %s", YESNO(traits.get_supports_current_temperature()));
  ESP_LOGCONFIG(tag, "  Supports away mode: %s", YESNO(traits.get_supports_away()));
  ESP_LOGCONFIG(tag, "  Visual min temperature: %.1f", traits.get_visual_min_temperature());
  ESP_LOGCONFIG(tag, "  Visual max temperature: %.1f", traits.get_visual_max_temperature());
  ESP_LOGCONFIG(tag, "  Visual temperature step: %.1f", traits.get_visual_temperature_step());
}

WaterHeaterCall WaterHeaterDeviceRestoreState::to_call(WaterHeater *water_heater) {
  auto call = water_heater->make_call();
  call.set_mode(this->mode);
  call.set_target_temperature(this->target_temperature);
  call.set_away(this->away);
  return call;
}

void WaterHeaterDeviceRestoreState::apply(WaterHeater *water_heater) {
  water_heater->mode = this->mode;
  water_heater->target_temperature = this->target_temperature;
  water_heater->away = this->away;
}

}  // namespace esphome::water_heater
