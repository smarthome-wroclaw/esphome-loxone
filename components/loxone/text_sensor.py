import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import ENTITY_CATEGORY_CONFIG, ENTITY_CATEGORY_DIAGNOSTIC

from . import CONF_LOXONE_ID, LoxoneComponent

DEPENDENCIES = ["loxone"]

CONF_MINISERVER_IP = "miniserver_ip"
CONF_MINISERVER_PORT = "miniserver_port"
CONF_LISTEN_PORT = "listen_port"
CONF_LAST_MESSAGE = "last_message"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_LOXONE_ID): cv.use_id(LoxoneComponent),
        cv.Optional(CONF_MINISERVER_IP): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:server-network",
        ),
        cv.Optional(CONF_MINISERVER_PORT): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:ethernet",
        ),
        cv.Optional(CONF_LISTEN_PORT): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:ethernet",
        ),
        cv.Optional(CONF_LAST_MESSAGE): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            icon="mdi:message-text",
        ),
    }
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LOXONE_ID])

    if conf := config.get(CONF_MINISERVER_IP):
        sens = await text_sensor.new_text_sensor(conf)
        cg.add(parent.set_miniserver_ip_text_sensor(sens))
    if conf := config.get(CONF_MINISERVER_PORT):
        sens = await text_sensor.new_text_sensor(conf)
        cg.add(parent.set_miniserver_port_text_sensor(sens))
    if conf := config.get(CONF_LISTEN_PORT):
        sens = await text_sensor.new_text_sensor(conf)
        cg.add(parent.set_listen_port_text_sensor(sens))
    if conf := config.get(CONF_LAST_MESSAGE):
        sens = await text_sensor.new_text_sensor(conf)
        cg.add(parent.set_last_message_text_sensor(sens))
