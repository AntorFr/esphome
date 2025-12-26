from esphome import automation
import esphome.codegen as cg
from esphome.components import web_server
import esphome.config_validation as cv
from esphome.const import (
    CONF_AWAY,
    CONF_ID,
    CONF_MAX_TEMPERATURE,
    CONF_MIN_TEMPERATURE,
    CONF_MODE,
    CONF_ON_CONTROL,
    CONF_ON_STATE,
    CONF_TARGET_TEMPERATURE,
    CONF_TEMPERATURE_STEP,
    CONF_TRIGGER_ID,
    CONF_VISUAL,
    CONF_WEB_SERVER_ID,
)
from esphome.core import CORE, coroutine_with_priority
from esphome.core.entity_helpers import setup_entity
from esphome.cpp_generator import MockObjClass

IS_PLATFORM_COMPONENT = True

CODEOWNERS = ["@esphome/core"]
water_heater_ns = cg.esphome_ns.namespace("water_heater")

WaterHeater = water_heater_ns.class_("WaterHeater", cg.EntityBase)
WaterHeaterCall = water_heater_ns.class_("WaterHeaterCall")
WaterHeaterTraits = water_heater_ns.class_("WaterHeaterTraits")

WaterHeaterMode = water_heater_ns.enum("WaterHeaterMode")
WATER_HEATER_MODES = {
    "OFF": WaterHeaterMode.WATER_HEATER_MODE_OFF,
    "ECO": WaterHeaterMode.WATER_HEATER_MODE_ECO,
    "ELECTRIC": WaterHeaterMode.WATER_HEATER_MODE_ELECTRIC,
    "PERFORMANCE": WaterHeaterMode.WATER_HEATER_MODE_PERFORMANCE,
    "HIGH_DEMAND": WaterHeaterMode.WATER_HEATER_MODE_HIGH_DEMAND,
    "HEAT_PUMP": WaterHeaterMode.WATER_HEATER_MODE_HEAT_PUMP,
    "GAS": WaterHeaterMode.WATER_HEATER_MODE_GAS,
}
validate_water_heater_mode = cv.enum(WATER_HEATER_MODES, upper=True)

# Actions
ControlAction = water_heater_ns.class_("ControlAction", automation.Action)
StateTrigger = water_heater_ns.class_(
    "StateTrigger", automation.Trigger.template(WaterHeater.operator("ref"))
)
ControlTrigger = water_heater_ns.class_(
    "ControlTrigger", automation.Trigger.template(WaterHeaterCall.operator("ref"))
)

visual_temperature = cv.float_with_unit(
    "visual_temperature", "(°C|° C|°|C|° K|° K|K|°F|° F|F)?"
)

WATER_HEATER_SCHEMA = (
    cv.ENTITY_BASE_SCHEMA.extend(web_server.WEBSERVER_SORTING_SCHEMA)
    .extend(cv.MQTT_COMMAND_COMPONENT_SCHEMA)
    .extend(
        {
            cv.GenerateID(): cv.declare_id(WaterHeater),
            cv.Optional(CONF_VISUAL, default={}): cv.Schema(
                {
                    cv.Optional(CONF_MIN_TEMPERATURE): cv.temperature,
                    cv.Optional(CONF_MAX_TEMPERATURE): cv.temperature,
                    cv.Optional(CONF_TEMPERATURE_STEP): visual_temperature,
                }
            ),
            cv.Optional(CONF_ON_CONTROL): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(ControlTrigger),
                }
            ),
            cv.Optional(CONF_ON_STATE): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(StateTrigger),
                }
            ),
        }
    )
)


def water_heater_schema(
    class_: MockObjClass = cv.UNDEFINED,
) -> cv.Schema:
    schema = {}
    if class_ is not cv.UNDEFINED:
        schema[cv.GenerateID()] = cv.declare_id(class_)
    return WATER_HEATER_SCHEMA.extend(schema)


async def setup_water_heater_core_(var, config):
    await setup_entity(var, config, "water_heater")

    visual = config[CONF_VISUAL]
    if (min_temp := visual.get(CONF_MIN_TEMPERATURE)) is not None:
        cg.add(var.set_visual_min_temperature_override(min_temp))
    if (max_temp := visual.get(CONF_MAX_TEMPERATURE)) is not None:
        cg.add(var.set_visual_max_temperature_override(max_temp))
    if (temp_step := visual.get(CONF_TEMPERATURE_STEP)) is not None:
        cg.add(var.set_visual_temperature_step_override(temp_step))

    for conf in config.get(CONF_ON_STATE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(
            trigger, [(WaterHeater.operator("ref"), "x")], conf
        )

    for conf in config.get(CONF_ON_CONTROL, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(
            trigger, [(WaterHeaterCall.operator("ref"), "x")], conf
        )

    if (webserver_id := config.get(CONF_WEB_SERVER_ID)) is not None:
        web_server_ = await cg.get_variable(webserver_id)
        web_server.add_entity_to_sorting_list(web_server_, var, config)


async def register_water_heater(var, config):
    if not CORE.has_id(config[CONF_ID]):
        var = cg.Pvariable(config[CONF_ID], var)
    cg.add(cg.App.register_water_heater(var))
    CORE.register_platform_component("water_heater", var)
    await setup_water_heater_core_(var, config)


async def new_water_heater(config, *args):
    var = cg.new_Pvariable(config[CONF_ID], *args)
    await register_water_heater(var, config)
    return var


WATER_HEATER_CONTROL_ACTION_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ID): cv.use_id(WaterHeater),
        cv.Optional(CONF_MODE): cv.templatable(validate_water_heater_mode),
        cv.Optional(CONF_TARGET_TEMPERATURE): cv.templatable(cv.temperature),
        cv.Optional(CONF_AWAY): cv.templatable(cv.boolean),
    }
)


@automation.register_action(
    "water_heater.control", ControlAction, WATER_HEATER_CONTROL_ACTION_SCHEMA
)
async def water_heater_control_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, paren)
    if (mode := config.get(CONF_MODE)) is not None:
        template_ = await cg.templatable(mode, args, WaterHeaterMode)
        cg.add(var.set_mode(template_))
    if (target_temp := config.get(CONF_TARGET_TEMPERATURE)) is not None:
        template_ = await cg.templatable(target_temp, args, float)
        cg.add(var.set_target_temperature(template_))
    if (away := config.get(CONF_AWAY)) is not None:
        template_ = await cg.templatable(away, args, bool)
        cg.add(var.set_away(template_))
    return var


@coroutine_with_priority(100.0)
async def to_code(config):
    cg.add_define("USE_WATER_HEATER")
    cg.add_global(water_heater_ns.using)
