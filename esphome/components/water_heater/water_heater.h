#pragma once

#include "esphome/core/component.h"
#include "esphome/core/entity_base.h"
#include "esphome/core/helpers.h"
#include "esphome/core/preferences.h"
#include "esphome/core/log.h"
#include "water_heater_mode.h"
#include "water_heater_traits.h"

namespace esphome::water_heater {

#define LOG_WATER_HEATER(prefix, type, obj) \
  if ((obj) != nullptr) { \
    ESP_LOGCONFIG(TAG, "%s%s '%s'", prefix, LOG_STR_LITERAL(type), (obj)->get_name().c_str()); \
  }

class WaterHeater;

/** This class is used to encode all control actions on a water heater device.
 *
 * It is supposed to be used by all code that wishes to control a water heater device (mqtt, api, lambda etc).
 * Create an instance of this class by calling `id(water_heater_device).make_call();`. Then set all attributes
 * with the `set_x` methods. Finally, to apply the changes call `.perform();`.
 */
class WaterHeaterCall {
 public:
  explicit WaterHeaterCall(WaterHeater *parent) : parent_(parent) {}

  /// Set the mode of the water heater device.
  WaterHeaterCall &set_mode(WaterHeaterMode mode);
  /// Set the mode of the water heater device.
  WaterHeaterCall &set_mode(optional<WaterHeaterMode> mode);
  /// Set the mode of the water heater device based on a string.
  WaterHeaterCall &set_mode(const std::string &mode);
  /// Set the target temperature of the water heater device.
  WaterHeaterCall &set_target_temperature(float target_temperature);
  /// Set the target temperature of the water heater device.
  WaterHeaterCall &set_target_temperature(optional<float> target_temperature);
  /// Set the away mode of the water heater device.
  WaterHeaterCall &set_away(bool away);
  /// Set the away mode of the water heater device.
  WaterHeaterCall &set_away(optional<bool> away);

  void perform();

  const optional<WaterHeaterMode> &get_mode() const;
  const optional<float> &get_target_temperature() const;
  const optional<bool> &get_away() const;

 protected:
  void validate_();

  WaterHeater *const parent_;
  optional<WaterHeaterMode> mode_;
  optional<float> target_temperature_;
  optional<bool> away_;
};

/// Struct used to save the state of the water heater device in restore memory.
struct WaterHeaterDeviceRestoreState {
  WaterHeaterMode mode;
  float target_temperature;
  bool away;

  /// Convert this struct to a water heater call that can be performed.
  WaterHeaterCall to_call(WaterHeater *water_heater);
  /// Apply these settings to the water heater device.
  void apply(WaterHeater *water_heater);
} __attribute__((packed));

/**
 * WaterHeater - This is the base class for all water heater integrations. Each integration
 * needs to extend this class and implement two functions:
 *
 *  - get_traits() - return the static traits of the water heater device
 *  - control(WaterHeaterCall call) - Apply the given changes from call.
 */
class WaterHeater : public EntityBase {
 public:
  WaterHeater() = default;

  /// The active mode of the water heater device.
  WaterHeaterMode mode{WATER_HEATER_MODE_OFF};

  /// The current temperature of the water heater device, as reported from the integration.
  float current_temperature{NAN};

  /// The target temperature of the water heater device.
  float target_temperature{NAN};

  /// Whether the water heater is in away mode.
  bool away{false};

  /** Add a callback for the water heater device state, each time the state of the water heater device is updated
   * (using publish_state), this callback will be called.
   */
  void add_on_state_callback(std::function<void(WaterHeater &)> &&callback);

  /**
   * Add a callback for the water heater device configuration; each time the configuration parameters of a water heater
   * device is updated (using perform() of a WaterHeaterCall), this callback will be called, before any on_state
   * callback.
   */
  void add_on_control_callback(std::function<void(WaterHeaterCall &)> &&callback);

  /** Make a water heater device control call, this is used to control the water heater device, see the WaterHeaterCall
   * description for more info.
   * @return A new WaterHeaterCall instance targeting this water heater device.
   */
  WaterHeaterCall make_call();

  /** Publish the state of the water heater device, to be called from integrations.
   *
   * This will schedule the water heater device to publish its state to all listeners and save the current state
   * to recover memory.
   */
  void publish_state();

  /** Get the traits of this water heater device with all overrides applied.
   *
   * Traits are static data that encode the capabilities and static data for a water heater device such as supported
   * modes, temperature range etc.
   */
  WaterHeaterTraits get_traits();

  void set_visual_min_temperature_override(float visual_min_temperature_override);
  void set_visual_max_temperature_override(float visual_max_temperature_override);
  void set_visual_temperature_step_override(float visual_temperature_step_override);

 protected:
  friend WaterHeaterCall;

  /** Get the default traits of this water heater device.
   *
   * Traits are static data that encode the capabilities and static data for a water heater device such as supported
   * modes, temperature range etc. Each integration must implement this method and the return value must
   * be constant during all of execution time.
   */
  virtual WaterHeaterTraits traits() = 0;

  /** Control the water heater device, this is a virtual method that each water heater integration must implement.
   *
   * See more info in WaterHeaterCall. The integration should check all of its values in this method and
   * set them accordingly. At the end of the call, the integration must call `publish_state()` to
   * notify the frontend of a changed state.
   */
  virtual void control(const WaterHeaterCall &call) = 0;

  /// Restore the state of the water heater device, call this from your setup() method.
  optional<WaterHeaterDeviceRestoreState> restore_state_();

  /** Internal method to save the state of the water heater device to recover memory. This is automatically
   * called from publish_state()
   */
  void save_state_();

  void dump_traits_(const char *tag);

  CallbackManager<void(WaterHeater &)> state_callback_{};
  CallbackManager<void(WaterHeaterCall &)> control_callback_{};
  ESPPreferenceObject rtc_;
  optional<float> visual_min_temperature_override_{};
  optional<float> visual_max_temperature_override_{};
  optional<float> visual_temperature_step_override_{};
};

}  // namespace esphome::water_heater
