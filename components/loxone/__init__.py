import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components.socket import SocketType, consume_sockets
from esphome.core import CORE
from esphome.const import (
    CONF_ID,
    CONF_TRIGGER_ID,
)
from .loxone_template import CONF_TEMPLATE, TEMPLATE_SCHEMA, add_template_code

DEPENDENCIES = ['network']
AUTO_LOAD = ['async_tcp', 'socket', 'web_server_base']

CONF_LOXONE_ID = "loxone_id"
CONF_PROBE_PORT = "probe_port"
CONF_CHECK_INTERVAL = "check_interval"
CONF_CHECK_TIMEOUT = "check_timeout"
CONF_AUTO_CONTROL_LIGHTS = "auto_control_lights"

loxone_ns = cg.esphome_ns.namespace('loxone')
LoxoneComponent = loxone_ns.class_('LoxoneComponent', cg.PollingComponent)
OnStringDataTrigger = loxone_ns.class_("OnStringDataTrigger",
                                 automation.Trigger.template(cg.std_string, cg.Component))

LOXONE_PROTOCOLS = {
    "tcp": "tcp",
    "udp": "udp"
}

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(LoxoneComponent),
    cv.Required("protocol"): cv.enum(LOXONE_PROTOCOLS),
    cv.Required("loxone_ip"): cv.ipv4address,
    cv.Required("loxone_port"): cv.int_range(0, 65535),
    cv.Required("listen_port"): cv.int_range(0, 65535),
    cv.Optional("send_buffer_length", default=20): cv.int_range(0, 1024),
    cv.Optional("auto_send_sensors", default=False): cv.boolean,
    cv.Optional(CONF_AUTO_CONTROL_LIGHTS, default=False): cv.boolean,
    cv.Optional("delimiter", default="\n"): cv.string,
    # Reachability probe for the optional `connected` binary sensor: the ESP
    # opens a short TCP connection to the Miniserver on `probe_port` (its web
    # UI, 80) every `check_interval`, giving up after `check_timeout`.
    cv.Optional(CONF_PROBE_PORT, default=80): cv.port,
    cv.Optional(CONF_CHECK_INTERVAL, default="30s"): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_CHECK_TIMEOUT, default="4s"): cv.positive_time_period_milliseconds,
    cv.Optional("on_string_data"): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(OnStringDataTrigger),
        }
    ),
    # Optional: serve importable Loxone Config templates at GET /loxone/
    # (needs a `web_server:` component). Auto-controlled lights are added to
    # the generated outputs when `auto_control_lights` is enabled.
    cv.Optional(CONF_TEMPLATE): TEMPLATE_SCHEMA,
}).extend(cv.COMPONENT_SCHEMA)

# This component pulls in the Arduino AsyncUDP / AsyncTCP libraries and the
# legacy `esphome.h` header, so it only builds under the Arduino framework.
CONFIG_SCHEMA = cv.All(
    CONFIG_SCHEMA,
    cv.only_with_arduino,
    consume_sockets(1, "loxone", SocketType.TCP),
)

def to_code(config):
    # `AsyncUDP` ships bundled with the Arduino ESP32 core. Enable it (and the
    # `Network` library it depends on in Arduino's Kconfig - it must come first,
    # otherwise CONFIG_ARDUINO_SELECTIVE_AsyncUDP is dropped on regeneration)
    # via ESPHome's selective-compilation names. The old "ESP32 Async UDP"
    # Arduino-IDE name is not a PlatformIO package and makes ESPHome 2026's
    # library resolver abort with UnknownPackageError.
    cg.add_library("Network", None)
    cg.add_library("AsyncUDP", None)
    #cg.add_library("esphome/AsyncTCP-esphome", "2.0.1")
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_protocol(config["protocol"]))
    cg.add(var.set_loxone_ip(str(config["loxone_ip"])))
    cg.add(var.set_loxone_port(config["loxone_port"]))
    cg.add(var.set_listen_port(config["listen_port"]))
    cg.add(var.set_send_buffer_length(config["send_buffer_length"]))
    cg.add(var.set_delimiter(config["delimiter"]))
    cg.add(var.set_probe_port(config[CONF_PROBE_PORT]))
    cg.add(var.set_check_interval(config[CONF_CHECK_INTERVAL].total_milliseconds))
    cg.add(var.set_check_timeout(config[CONF_CHECK_TIMEOUT].total_milliseconds))
    yield cg.register_component(var, config)

    # Entity IDs are assigned during config validation, so subscribe to sensors
    # and lights without per-entity YAML.
    if config["auto_send_sensors"]:
        for domain, method in (
            ("sensor", "add_auto_sensor"),
            ("binary_sensor", "add_auto_binary_sensor"),
            ("light", "add_auto_light"),
        ):
            for entity_config in CORE.config.get(domain, []):
                entity_id = entity_config.get(CONF_ID)
                if entity_id is None:
                    continue
                entity_var = yield cg.get_variable(entity_id)
                command = "".join(ch if ch.isalnum() else "_" for ch in str(entity_id)).upper()
                cg.add(getattr(var, method)(entity_var, command))

    template_config = config.get(CONF_TEMPLATE)
    if config[CONF_AUTO_CONTROL_LIGHTS]:
        light_configs = CORE.config.get("light", [])
        configured_outputs = {} if template_config is None else {
            output["command"]: output for output in template_config.get("outputs", [])
        }
        for light_config in light_configs:
            light_id = light_config.get(CONF_ID)
            if light_id is None:
                continue
            light_var = yield cg.get_variable(light_id)
            command = "".join(ch if ch.isalnum() else "_" for ch in str(light_id)).upper()
            output_config = configured_outputs.get(command)
            min_value = 0 if output_config is None else output_config["min"]
            max_value = 100 if output_config is None else output_config["max"]
            cg.add(var.add_auto_control_light(light_var, command, min_value, max_value))

        if template_config is not None:
            template_config = dict(template_config)
            outputs = list(template_config.get("outputs", []))
            existing_commands = {output["command"] for output in outputs}
            for light_config in light_configs:
                light_id = light_config.get(CONF_ID)
                if light_id is None:
                    continue
                command = "".join(ch if ch.isalnum() else "_" for ch in str(light_id)).upper()
                if command not in existing_commands:
                    outputs.append({"command": command, "analog": True, "min": 0, "max": 100})
                    existing_commands.add(command)
            template_config["outputs"] = outputs

    if template_config is not None:
        template_codegen_config = dict(config)
        template_codegen_config[CONF_TEMPLATE] = template_config
        add_template_code(template_codegen_config, var)

    for conf in config.get("on_string_data", []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        #yield cg.register_component(trigger, conf)
        cg.add(var.add_string_trigger(trigger))
        yield automation.build_automation(trigger, [(cg.std_string, "data")], conf)
