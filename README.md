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
      ref: v1.6.0

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

# Status & config entities (optional)

The `loxone:` block itself creates no entities. Add any of the `text_sensor`,
`binary_sensor` and `sensor` platforms to see what's going on from the ESPHome
web UI / Home Assistant:

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
    last_message:                # last complete command received from Loxone
      name: "Loxone Last Message"

binary_sensor:
  - platform: loxone
    connected:                   # device_class: connectivity (green/red dot)
      name: "Loxone Connected"

sensor:
  - platform: loxone
    last_message_age:            # seconds since the last inbound packet
      name: "Loxone Last Message Age"
```

All keys are optional. `miniserver_ip` / `miniserver_port` / `listen_port` are
static config values (published once at boot, `entity_category: config`).

**`connected`** — because Loxone UDP is connectionless, "is the Miniserver
there?" cannot be answered from the UDP socket. Instead the ESP opens a short
TCP connection to the Miniserver's web UI (`probe_port`, default `80`) every
`check_interval` (default `30s`, giving up after `check_timeout`, default `4s`).
Success → on. It confirms the Miniserver is powered on and on the network; it
does **not** prove the UDP path — pair it with `last_message_age` for that.

Probe tuning goes on the `loxone:` block:

```yaml
loxone:
  # ...
  probe_port: 80
  check_interval: 30s
  check_timeout: 4s
```

The component also logs its configuration at boot (`[loxone]` at the default log
level):

```
[C][loxone:0xx]: Loxone:
[C][loxone:0xx]:   Protocol: udp
[C][loxone:0xx]:   Miniserver: 192.168.50.124:9999
[C][loxone:0xx]:   Listen port: 8888
[C][loxone:0xx]:   Send buffer length: 64
[C][loxone:0xx]:   For TX/RX traffic: logger -> logs: {loxone: DEBUG}
[C][loxone:0xx]:   Reachability probe: 192.168.50.124:80 every 30000ms (timeout 4000ms)
[C][loxone:0xx]:   Config template: http://<device-ip>/loxone/  (ready)
```

# Seeing the traffic in the logs

At the **default** log level the component prints, under the `loxone` tag:

```
[I][loxone:0xx]: client connected (udp 192.168.50.124:9999)
[I][loxone:0xx]: stats: tx=12 rx=40 cmd=40 last_rx=3s connected=yes
```

`client connected` / `client disconnected` are logged only on change; the
`stats:` line appears at most once a minute and only when traffic moved since
the last one. Together with the `connected` binary sensor this is enough to
confirm the link without touching the Loxone side.

For the individual messages, raise the component to `DEBUG`:

```yaml
logger:
  logs:
    loxone: DEBUG
```

```
[D][loxone:0xx]: TX -> 192.168.50.124:9999  "RELAY-SET-255,1,1,OK"
[D][loxone:0xx]: RX <- udp 18 B  "RELAY-SET-255,1,0"
[D][loxone:0xx]: RX cmd "RELAY-SET-255,1,0"
```

`RX <-` is each raw packet; `RX cmd` is each complete command after the
`delimiter` split (what `on_string_data` receives).

# Loxone Config templates (optional)

Add a `template:` block and the device serves ready-to-import Loxone Config
templates from its own web server (needs a `web_server:` component):

| URL | Content |
| --- | --- |
| `http://<device-ip>/loxone/` | HTML page: download links, import steps, the `<VirtualOut>` target address filled in with this device's IP, and copy-paste ESPHome lambdas for each command |
| `http://<device-ip>/loxone/inputs.xml` | `<VirtualInUdp>` template — import under Loxone Config → *Virtual Inputs* → *Import Template* |
| `http://<device-ip>/loxone/outputs.xml` | `<VirtualOut>` UDP template — import under *Virtual Outputs* → *Import Template* |

```yaml
loxone:
  id: loxone1
  protocol: udp
  loxone_ip: "192.168.50.124"
  loxone_port: 9999
  listen_port: 8888

  template:
    title: "BoneIO Dimmer"        # optional, defaults to the device name
    inputs:                       # device -> Loxone  (<VirtualInUdp>)
      - { command: "IN_01" }                                  # digital, 0/1
      - { command: "BONEIO_TEMP", analog: true, min: -20, max: 80 }
    outputs:                      # Loxone -> device  (<VirtualOut>, UDP)
      - { command: "BUZZER" }                                 # digital, 0/1
      - { command: "RGBW_L_BRI", analog: true, min: 0, max: 255 }
```

**The `template:` block is descriptive only — it generates no send/receive
code.** It turns a list of command names into the XML templates and a matching
lambda skeleton. You still wire the actual behaviour yourself:

- **device → Loxone**: call `send_string_data("IN_01 " + ...)` from the source
  entity's `on_state` / `on_value`. The Loxone side recognises `IN_01 <value>`.
- **Loxone → device**: handle the command in `on_string_data` (Loxone sends
  `BUZZER 1` / `RGBW_L_BRI <v>`, terminated with the configured `delimiter`).

The `/loxone/` page prints both skeletons pre-filled with your command names.

Per command: `command` (required), `analog` (default `false` → digital 0/1),
`min` / `max` (analog range, default `0`/`1`). Set the Miniserver's *UDP receive
port* to match `loxone_port`.
