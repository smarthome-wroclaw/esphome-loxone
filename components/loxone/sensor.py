import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_SECOND,
)

from . import CONF_LOXONE_ID, LoxoneComponent

DEPENDENCIES = ["loxone"]

CONF_LAST_MESSAGE_AGE = "last_message_age"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_LOXONE_ID): cv.use_id(LoxoneComponent),
        cv.Optional(CONF_LAST_MESSAGE_AGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_SECOND,
            icon="mdi:timer-sand",
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LOXONE_ID])

    if conf := config.get(CONF_LAST_MESSAGE_AGE):
        sens = await sensor.new_sensor(conf)
        cg.add(parent.set_last_message_age_sensor(sens))
