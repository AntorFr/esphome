from esphome import automation
import esphome.codegen as cg
from esphome.components import water_heater
import esphome.config_validation as cv
from esphome.const import CONF_LAMBDA, CONF_OPTIMISTIC, CONF_RESTORE_MODE

from .. import template_ns

TemplateWaterHeater = template_ns.class_(
    "TemplateWaterHeater", water_heater.WaterHeater, cg.Component
)

TemplateWaterHeaterRestoreMode = template_ns.enum("TemplateWaterHeaterRestoreMode")
RESTORE_MODES = {
    "NO_RESTORE": TemplateWaterHeaterRestoreMode.WATER_HEATER_NO_RESTORE,
    "RESTORE": TemplateWaterHeaterRestoreMode.WATER_HEATER_RESTORE,
    "RESTORE_AND_CALL": TemplateWaterHeaterRestoreMode.WATER_HEATER_RESTORE_AND_CALL,
}

CONF_MODE_STATE_TOPIC = "mode_state_topic"
CONF_CURRENT_TEMPERATURE_STATE_TOPIC = "current_temperature_state_topic"
CONF_TARGET_TEMPERATURE_STATE_TOPIC = "target_temperature_state_topic"
CONF_SET_MODE_ACTION = "set_mode_action"
CONF_SET_TARGET_TEMPERATURE_ACTION = "set_target_temperature_action"
CONF_SET_AWAY_ACTION = "set_away_action"

CONFIG_SCHEMA = (
    water_heater.water_heater_schema(TemplateWaterHeater)
    .extend(
        {
            cv.Optional(CONF_LAMBDA): cv.returning_lambda,
            cv.Optional(CONF_OPTIMISTIC, default=False): cv.boolean,
            cv.Optional(CONF_SET_MODE_ACTION): automation.validate_automation(
                single=True
            ),
            cv.Optional(
                CONF_SET_TARGET_TEMPERATURE_ACTION
            ): automation.validate_automation(single=True),
            cv.Optional(CONF_SET_AWAY_ACTION): automation.validate_automation(
                single=True
            ),
            cv.Optional(CONF_RESTORE_MODE, default="NO_RESTORE"): cv.enum(
                RESTORE_MODES, upper=True
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = await water_heater.new_water_heater(config)
    await cg.register_component(var, config)

    if lambda_config := config.get(CONF_LAMBDA):
        template_ = await cg.process_lambda(
            lambda_config,
            [],
            return_type=cg.optional.template(water_heater.WaterHeaterState),
        )
        cg.add(var.set_state_lambda(template_))

    if set_mode_action := config.get(CONF_SET_MODE_ACTION):
        await automation.build_automation(
            var.get_mode_trigger(),
            [(water_heater.WaterHeaterMode, "mode")],
            set_mode_action,
        )
        cg.add(var.set_supports_mode(True))

    if set_target_temperature_action := config.get(CONF_SET_TARGET_TEMPERATURE_ACTION):
        await automation.build_automation(
            var.get_target_temperature_trigger(),
            [(cg.float_, "target_temperature")],
            set_target_temperature_action,
        )
        cg.add(var.set_supports_target_temperature(True))

    if set_away_action := config.get(CONF_SET_AWAY_ACTION):
        await automation.build_automation(
            var.get_away_trigger(),
            [(cg.bool_, "away")],
            set_away_action,
        )
        cg.add(var.set_supports_away(True))

    cg.add(var.set_optimistic(config[CONF_OPTIMISTIC]))
    cg.add(var.set_restore_mode(config[CONF_RESTORE_MODE]))
