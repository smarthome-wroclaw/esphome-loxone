#pragma once

#include <string>
#include <queue>

#include <memory>

#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/socket/socket.h"
#include "loxone_web.h"
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#include "AsyncUDP.h"
#include "AsyncTCP.h"

#define TAG "loxone"

namespace esphome {
  namespace loxone {
    class OnStringDataTrigger;

    class LoxoneComponent : public PollingComponent {
    public:
      LoxoneComponent() : PollingComponent(5000) {};
      void setup() override;
      void dump_config() override;
      void update() override;
      void loop() override;
      void send_string_data(std::string data);

      void set_probe_port(uint16_t port) { this->probe_port_ = port; };
      void set_check_interval(uint32_t ms) { this->check_interval_ms_ = ms; };
      void set_check_timeout(uint32_t ms) { this->check_timeout_ms_ = ms; };

#ifdef USE_BINARY_SENSOR
      void set_connected_binary_sensor(binary_sensor::BinarySensor *s) {
        this->connected_binary_sensor_ = s;
      };
#endif
#ifdef USE_SENSOR
      void set_last_message_age_sensor(sensor::Sensor *s) {
        this->last_message_age_sensor_ = s;
      };
#endif

#ifdef USE_TEXT_SENSOR
      void set_miniserver_ip_text_sensor(text_sensor::TextSensor *s) {
        this->miniserver_ip_text_sensor_ = s;
      };
      void set_miniserver_port_text_sensor(text_sensor::TextSensor *s) {
        this->miniserver_port_text_sensor_ = s;
      };
      void set_listen_port_text_sensor(text_sensor::TextSensor *s) {
        this->listen_port_text_sensor_ = s;
      };
      void set_last_message_text_sensor(text_sensor::TextSensor *s) {
        this->last_message_text_sensor_ = s;
      };
#endif
      void set_protocol(std::string protocol) {
        this->protocol_ = protocol;
      };
      void set_loxone_ip(std::string loxone_ip) {
        this->loxone_ip_ = loxone_ip;
      };
      void set_loxone_port(uint16_t port) {
        this->loxone_port_ = port;
      };
      void set_listen_port(uint16_t port) {
        this->listen_port_ = port;
      };
      void set_delimiter(std::string delimiter) {
        this->delimiter_ = delimiter;
      };
      void set_send_buffer_length(uint8_t buffer_length) {
        this->send_buffer_length_ = buffer_length;
      };

      void add_string_trigger(OnStringDataTrigger *trigger) {
        this->string_triggers_.push_back(trigger);
      };

#if defined(USE_NETWORK) && !defined(USE_ZEPHYR)
      // Enable the /loxone/ config-template endpoint. Bodies are rendered at
      // codegen time from the `template:` block.
      void set_loxone_template(const std::string &index_html, const std::string &inputs_xml,
                               const std::string &outputs_xml) {
        this->template_handler_ = new LoxoneTemplateHandler(index_html, inputs_xml, outputs_xml);
      };
#endif
    protected:
      std::vector<OnStringDataTrigger *> string_triggers_{};
      std::string protocol_;
      std::string loxone_ip_;
      uint16_t loxone_port_;
      uint16_t listen_port_;
      uint8_t send_buffer_length_;
      std::string delimiter_;
      AsyncUDP udp_client_;
      AsyncUDP udp_server_;
      AsyncClient tcp_client_;
      AsyncServer* tcp_server_;
      std::string receive_string_buffer_;
      std::queue<std::string> send_string_buffer_{};
      bool server_ready_ = false;
      bool client_ready_ = false;

      // Inbound-traffic liveness tracking
      uint32_t last_rx_ms_{0};
      bool have_rx_{false};
      void note_rx_();

      // Communication logging (TX/RX at DEBUG, transitions + heartbeat at INFO)
      uint32_t tx_count_{0};
      uint32_t rx_packet_count_{0};
      uint32_t rx_cmd_count_{0};
      int8_t client_ready_state_{-1};  // -1 unknown, 0 down, 1 up (edge-logging)
      uint32_t last_heartbeat_ms_{0};
      uint32_t last_heartbeat_activity_{0};
      void log_tx_(const std::string &data);
      void log_rx_(const char *transport, const void *data, size_t len);
      void set_client_ready_(bool ready);
      void log_stats_();
#ifdef USE_TEXT_SENSOR
      text_sensor::TextSensor *miniserver_ip_text_sensor_{nullptr};
      text_sensor::TextSensor *miniserver_port_text_sensor_{nullptr};
      text_sensor::TextSensor *listen_port_text_sensor_{nullptr};
      text_sensor::TextSensor *last_message_text_sensor_{nullptr};
#endif
#ifdef USE_SENSOR
      sensor::Sensor *last_message_age_sensor_{nullptr};
#endif

      // TCP reachability probe (non-blocking) for the `connected` binary sensor
      uint16_t probe_port_{80};
      uint32_t check_interval_ms_{30000};
      uint32_t check_timeout_ms_{4000};
#ifdef USE_BINARY_SENSOR
      binary_sensor::BinarySensor *connected_binary_sensor_{nullptr};
      enum ProbeState { PROBE_IDLE, PROBE_CONNECTING };
      ProbeState probe_state_{PROBE_IDLE};
      std::unique_ptr<socket::Socket> probe_socket_;
      struct sockaddr_storage probe_addr_;
      socklen_t probe_addrlen_{0};
      uint32_t probe_started_ms_{0};
      uint32_t next_probe_ms_{0};
      void probe_loop_();
      void probe_start_();
      void probe_finish_(bool reachable);
#endif

#if defined(USE_NETWORK) && !defined(USE_ZEPHYR)
      LoxoneTemplateHandler *template_handler_{nullptr};
#endif

      void ensure_listen_udp();
      void ensure_listen_tcp();
      void fire_triggers();
      void ensure_connect_tcp();
      void ensure_connect_udp();
    };

    class OnStringDataTrigger : public Trigger<std::string>, public Component {
      friend class LoxoneComponent;

    public:
      explicit OnStringDataTrigger(LoxoneComponent *parent)
        : parent_(parent){};

      void setup() override { this->parent_->add_string_trigger(this); }

    protected:
      LoxoneComponent *parent_;
    };

  }
}
