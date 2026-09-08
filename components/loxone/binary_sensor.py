import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import DEVICE_CLASS_CONNECTIVITY, ENTITY_CATEGORY_DIAGNOSTIC

from . import CONF_LOXONE_ID, LoxoneComponent

DEPENDENCIES = ["loxone"]

CONF_CONNECTED = "connected"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_LOXONE_ID): cv.use_id(LoxoneComponent),
        cv.Optional(CONF_CONNECTED): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_CONNECTIVITY,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LOXONE_ID])

    if conf := config.get(CONF_CONNECTED):
        sens = await binary_sensor.new_binary_sensor(conf)
        cg.add(parent.set_connected_binary_sensor(sens))
