# Loxone Component

ESPHome external component for exchanging newline-delimited string commands with
a Loxone Miniserver over UDP or TCP.

This is a fork of [hzkincony/esphome-loxone](https://github.com/hzkincony/esphome-loxone)
with a compatibility fix for ESPHome 2026.x. Releases are cut with
[release-please](https://github.com/googleapis/release-please); pin a tag from
the [releases page](https://github.com/smarthome-wroclaw/esphome-loxone/releases).

more information, you can check with KinCony's webpage: https://www.kincony.com

# Requirements

This component uses the Arduino `AsyncUDP` / `AsyncTCP` libraries, so it only
builds under the **Arduino** framework:

```yaml
esp32:
  framework:
    type: arduino
```

Under `esp-idf` the config is rejected with a clear message instead of a build
failure.

# Core Yaml
```yaml
external_components:
  - source:
      type: git
      url: https://github.com/smarthome-wroclaw/esphome-loxone
      ref: v1.3.1

esp32:
  framework:
    type: arduino

switch:
  - platform: gpio
    pin: 22
    name: "Loxone Switch 1"
    id: loxone_switch_1
    on_turn_on:
      - lambda: !lambda |-
          id(loxone1).send_string_data("RELAY-SET-255,1,1,OK");
    on_turn_off:
      - lambda: !lambda |-
          id(loxone1).send_string_data("RELAY-SET-255,1,0,OK");

binary_sensor:
  - platform: gpio
    pin: 23
    publish_initial_state: true
    name: "Loxone Binary Sensor 1"
    on_press:
      - lambda: !lambda |-
          id(loxone1).send_string_data("RELAY-GET_INPUT-255,1,1,OK");
    on_release:
      - lambda: !lambda |-
          id(loxone1).send_string_data("RELAY-GET_INPUT-255,1,0,OK");

loxone:
  id: loxone1
  protocol: udp
  loxone_ip: "192.168.50.124" # loxone server ip
  loxone_port: 9999 # loxone server port
  listen_port: 8888 # esp32 will listen on this port
  delimiter: "\n" # delimiter used to identify the end of a command

  # Send buffer length
  # When the network is not ready, it can buffer some commands that are pending to be sent.
  send_buffer_length: 64

  # The on_string_data function can handle received string commands.
  # All received commands will be split using the delimiter,
  # and this will trigger the logic of on_string_data
  on_string_data:
    - lambda: !lambda |-
        if (data == "RELAY-SET-255,1,1") {
          id(loxone_switch_1).turn_on();
        } else if (data == "RELAY-SET-255,1,0") {
          id(loxone_switch_1).turn_off();
        }
```

# Config text sensors (optional)

The `loxone:` block itself creates no entities. To surface the connection
settings in the ESPHome web UI / Home Assistant (they show under **Configuration**),
add the `text_sensor` platform:

```yaml
text_sensor:
  - platform: loxone
    loxone_id: loxone1          # optional, only if you have more than one `loxone:`
    miniserver_ip:
      name: "Miniserver IP"
    miniserver_port:
      name: "Miniserver UDP Port"
    listen_port:
      name: "Listen Port"
```

All three keys are optional. Values are published once at boot.

The component also logs its configuration at boot (`[loxone]` at the default log
level):

```
[C][loxone:0xx]: Loxone:
[C][loxone:0xx]:   Protocol: udp
[C][loxone:0xx]:   Miniserver: 192.168.50.124:9999
[C][loxone:0xx]:   Listen port: 8888
[C][loxone:0xx]:   Send buffer length: 64
```
