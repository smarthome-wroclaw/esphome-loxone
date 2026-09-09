#include "loxone_component.h"

#include <cerrno>
#include <cmath>

namespace esphome {
  namespace loxone {
    void LoxoneComponent::setup() {
#ifdef USE_BINARY_SENSOR
      // let the network come up before the first reachability probe
      this->next_probe_ms_ = millis() + 3000;
#endif
#ifdef USE_TEXT_SENSOR
      if (this->miniserver_ip_text_sensor_ != nullptr) {
        this->miniserver_ip_text_sensor_->publish_state(this->loxone_ip_);
      }
      if (this->miniserver_port_text_sensor_ != nullptr) {
        this->miniserver_port_text_sensor_->publish_state(std::to_string(this->loxone_port_));
      }
      if (this->listen_port_text_sensor_ != nullptr) {
        this->listen_port_text_sensor_->publish_state(std::to_string(this->listen_port_));
      }
#endif

#if defined(USE_NETWORK) && !defined(USE_ZEPHYR)
      if (this->template_handler_ != nullptr) {
        if (web_server_base::global_web_server_base != nullptr) {
          web_server_base::global_web_server_base->add_handler(this->template_handler_);
        } else {
          ESP_LOGW(TAG, "Loxone template endpoint needs a `web_server:` component");
        }
      }
#endif
    }

    void LoxoneComponent::dump_config() {
      ESP_LOGCONFIG(TAG, "Loxone:");
      ESP_LOGCONFIG(TAG, "  Protocol: %s", this->protocol_.c_str());
      ESP_LOGCONFIG(TAG, "  Miniserver: %s:%u", this->loxone_ip_.c_str(), this->loxone_port_);
      ESP_LOGCONFIG(TAG, "  Listen port: %u", this->listen_port_);
      ESP_LOGCONFIG(TAG, "  Send buffer length: %u", this->send_buffer_length_);
      ESP_LOGCONFIG(TAG, "  For TX/RX traffic: logger -> logs: {loxone: DEBUG}");
#ifdef USE_BINARY_SENSOR
      if (this->connected_binary_sensor_ != nullptr) {
        ESP_LOGCONFIG(TAG, "  Reachability probe: %s:%u every %ums (timeout %ums)",
                      this->loxone_ip_.c_str(), this->probe_port_,
                      (unsigned) this->check_interval_ms_, (unsigned) this->check_timeout_ms_);
      }
#endif
#if defined(USE_NETWORK) && !defined(USE_ZEPHYR)
      if (this->template_handler_ != nullptr) {
        bool served = web_server_base::global_web_server_base != nullptr &&
                      web_server_base::global_web_server_base->get_server() != nullptr;
        ESP_LOGCONFIG(TAG, "  Config template: http://<device-ip>/loxone/  (%s)",
                      served ? "ready" : "needs web_server:");
      }
#endif
    }

    void LoxoneComponent::note_rx_() {
      this->last_rx_ms_ = millis();
      this->have_rx_ = true;
    }

    void LoxoneComponent::log_tx_(const std::string &data) {
      this->tx_count_++;
      ESP_LOGD(TAG, "TX -> %s:%u  \"%s\"", this->loxone_ip_.c_str(), this->loxone_port_, data.c_str());
    }

    void LoxoneComponent::log_rx_(const char *transport, const void *data, size_t len) {
      this->note_rx_();
      this->rx_packet_count_++;
      ESP_LOGD(TAG, "RX <- %s %u B  \"%.*s\"", transport, (unsigned) len, (int) len, (const char *) data);
    }

    void LoxoneComponent::set_client_ready_(bool ready) {
      this->client_ready_ = ready;
      int8_t want = ready ? 1 : 0;
      if (this->client_ready_state_ == want) {
        return;
      }
      if (ready) {
        ESP_LOGI(TAG, "client connected (%s %s:%u)", this->protocol_.c_str(), this->loxone_ip_.c_str(),
                 this->loxone_port_);
      } else if (this->client_ready_state_ == 1) {
        ESP_LOGI(TAG, "client disconnected");
      }
      this->client_ready_state_ = want;
    }

    void LoxoneComponent::loop() {
#ifdef USE_BINARY_SENSOR
      if (this->connected_binary_sensor_ != nullptr) {
        this->probe_loop_();
      }
#endif
    }

#ifdef USE_BINARY_SENSOR
    void LoxoneComponent::probe_loop_() {
      const uint32_t now = millis();
      switch (this->probe_state_) {
        case PROBE_IDLE:
          if ((int32_t) (now - this->next_probe_ms_) >= 0 && network::is_connected()) {
            this->probe_start_();
          }
          break;
        case PROBE_CONNECTING: {
          // Poll the non-blocking connect by re-calling connect(): lwIP returns
          // EISCONN once the handshake is done, EALREADY while still pending,
          // or the real error (ECONNREFUSED / EHOSTUNREACH / ...).
          int err = this->probe_socket_->connect((struct sockaddr *) &this->probe_addr_,
                                                 this->probe_addrlen_);
          if (err == 0 || errno == EISCONN) {
            this->probe_finish_(true);
          } else if (errno == EALREADY || errno == EINPROGRESS ||
                     errno == EWOULDBLOCK || errno == EAGAIN) {
            if ((int32_t) (now - this->probe_started_ms_) >= (int32_t) this->check_timeout_ms_) {
              this->probe_finish_(false);
            }
          } else {
            this->probe_finish_(false);
          }
          break;
        }
      }
    }

    void LoxoneComponent::probe_start_() {
      this->probe_socket_ = socket::socket_ip(SOCK_STREAM, IPPROTO_TCP);
      if (this->probe_socket_ == nullptr) {
        ESP_LOGW(TAG, "probe: could not allocate socket");
        this->probe_finish_(false);
        return;
      }
      this->probe_socket_->setblocking(false);
      this->probe_addrlen_ = socket::set_sockaddr((struct sockaddr *) &this->probe_addr_,
                                                  sizeof(this->probe_addr_),
                                                  this->loxone_ip_, this->probe_port_);
      if (this->probe_addrlen_ == 0) {
        ESP_LOGW(TAG, "probe: invalid miniserver ip '%s'", this->loxone_ip_.c_str());
        this->probe_finish_(false);
        return;
      }
      int err = this->probe_socket_->connect((struct sockaddr *) &this->probe_addr_,
                                             this->probe_addrlen_);
      if (err == 0) {
        this->probe_finish_(true);
        return;
      }
      if (errno != EINPROGRESS && errno != EALREADY && errno != EWOULDBLOCK && errno != EAGAIN) {
        this->probe_finish_(false);
        return;
      }
      this->probe_started_ms_ = millis();
      this->probe_state_ = PROBE_CONNECTING;
    }

    void LoxoneComponent::probe_finish_(bool reachable) {
      if (this->probe_socket_ != nullptr) {
        this->probe_socket_->close();
        this->probe_socket_ = nullptr;
      }
      this->probe_state_ = PROBE_IDLE;
      this->next_probe_ms_ = millis() + this->check_interval_ms_;
      if (this->connected_binary_sensor_ != nullptr) {
        this->connected_binary_sensor_->publish_state(reachable);
      }
    }
#endif

    void LoxoneComponent::ensure_listen_udp() {
      if (protocol_ != "udp") {
        return;
      }

      if (server_ready_) {
        return;
      }

      if (udp_server_.listen(listen_port_)) {
        server_ready_ = true;
        ESP_LOGD(TAG, "listening for Loxone on udp/%u", listen_port_);
        udp_server_.onPacket([this](AsyncUDPPacket packet) {
          this->log_rx_("udp", packet.data(), packet.length());
          receive_string_buffer_.append((char*)packet.data(), packet.length());
          ESP_LOGV(TAG, "rx buffer: \"%s\"", receive_string_buffer_.c_str());
          fire_triggers();
        });
      }
    }

    void LoxoneComponent::ensure_listen_tcp() {
      if (protocol_ != "tcp") {
        return;
      }

      if (server_ready_) {
        return;
      }

      tcp_server_ = new AsyncServer(listen_port_);
      tcp_server_->onClient([this](void* arg, AsyncClient *client) {
        // Note: AsyncClient::remoteIP() is compiled only when the AsyncTCP
        // library sees `ARDUINO` defined, which is not the case for the
        // managed component ESPHome pulls in - calling it fails to link.
        ESP_LOGD(TAG, "Loxone opened a tcp connection");
        client->onData([this](void* arg, AsyncClient *client, void *data, size_t len) {
          this->log_rx_("tcp", data, len);
          receive_string_buffer_.append((char*)data, len);
          ESP_LOGV(TAG, "rx buffer: \"%s\"", receive_string_buffer_.c_str());
          fire_triggers();
        }, nullptr);
      }, nullptr);
      tcp_server_->begin();
      server_ready_ = true;
      ESP_LOGD(TAG, "listening for Loxone on tcp/%u", listen_port_);
    }

    void LoxoneComponent::fire_triggers() {
      // 检查缓冲区中是否含有 '\n'，即是否有完整的指令
      if (delimiter_ == "") {
        return;
      }

      size_t pos;
      while ((pos = receive_string_buffer_.find(delimiter_)) != std::string::npos) {
        // 提取完整的指令
        std::string command = receive_string_buffer_.substr(0, pos);
        receive_string_buffer_.erase(0, pos + 1); // 从缓冲区中移除这个指令

        // 对每一个完整的指令调用 triggers_ 的 trigger 方法
        if (!command.empty()) {
          this->rx_cmd_count_++;
          ESP_LOGD(TAG, "RX cmd \"%s\"", command.c_str());
#ifdef USE_TEXT_SENSOR
          if (this->last_message_text_sensor_ != nullptr) {
            this->last_message_text_sensor_->publish_state(command);
          }
#endif
          // 假设 triggers_ 是一个能够响应字符串指令的对象
          for (auto& trigger : string_triggers_) {
            trigger->trigger(command);
          }
        }
      }
    }

    void LoxoneComponent::ensure_connect_tcp() {
      if (protocol_ != "tcp") {
        return;
      }

      this->set_client_ready_(tcp_client_.connected());

      if (tcp_client_.connecting()) {
        ESP_LOGV(TAG, "client still connecting");
        return;
      }

      if (client_ready_) {
        return;
      }

      if (tcp_client_.connect(loxone_ip_.c_str(), loxone_port_)) {
        ESP_LOGV(TAG, "client connecting...");
      } else {
        ESP_LOGD(TAG, "client connect failed");
      }
    }

    void LoxoneComponent::ensure_connect_udp() {
      if (protocol_ != "udp") {
        return;
      }

      this->set_client_ready_(udp_client_.connected());

      if (!client_ready_) {
        ip_addr_t addr;
        ipaddr_aton(loxone_ip_.c_str(), &addr);
        if (udp_client_.connect(&addr, loxone_port_)) {
          ESP_LOGV(TAG, "client connecting...");
        } else {
          ESP_LOGD(TAG, "client connect failed");
        }
      }
    }

    void LoxoneComponent::update() {
#ifdef USE_SENSOR
      if (this->last_message_age_sensor_ != nullptr) {
        float age = this->have_rx_ ? (millis() - this->last_rx_ms_) / 1000.0f : NAN;
        this->last_message_age_sensor_->publish_state(age);
      }
#endif

      this->log_stats_();

      if (!network::is_connected()) {
        ESP_LOGV(TAG, "network not ready");
        return;
      }

      ensure_listen_udp();
      ensure_listen_tcp();
      ensure_connect_tcp();
      ensure_connect_udp();

      if (client_ready_) {
        while (!send_string_buffer_.empty()) {
          std::string d = send_string_buffer_.front();
          if (protocol_ == "udp") {
            udp_client_.print(d.c_str());
            udp_client_.print(delimiter_.c_str());
            this->log_tx_(d);
          } else if (protocol_ == "tcp") {
            tcp_client_.add(d.c_str(), strlen(d.c_str()));
            tcp_client_.add(delimiter_.c_str(), strlen(delimiter_.c_str()));
            tcp_client_.send();
            this->log_tx_(d);
          }

          send_string_buffer_.pop();
        }
      }
    }

    void LoxoneComponent::log_stats_() {
      const uint32_t now = millis();
      if (now - this->last_heartbeat_ms_ < 60000) {
        return;
      }
      this->last_heartbeat_ms_ = now;
      const uint32_t activity = this->tx_count_ + this->rx_packet_count_;
      if (activity == this->last_heartbeat_activity_) {
        return;  // nothing happened since the last heartbeat - stay quiet
      }
      this->last_heartbeat_activity_ = activity;
      if (this->have_rx_) {
        ESP_LOGI(TAG, "stats: tx=%u rx=%u cmd=%u last_rx=%us connected=%s", (unsigned) this->tx_count_,
                 (unsigned) this->rx_packet_count_, (unsigned) this->rx_cmd_count_,
                 (unsigned) ((now - this->last_rx_ms_) / 1000), this->client_ready_ ? "yes" : "no");
      } else {
        ESP_LOGI(TAG, "stats: tx=%u rx=0 cmd=0 last_rx=never connected=%s", (unsigned) this->tx_count_,
                 this->client_ready_ ? "yes" : "no");
      }
    }

    void LoxoneComponent::send_string_data(std::string data) {
      if (!client_ready_) {
        if (send_string_buffer_.size() >= send_buffer_length_) {
          ESP_LOGW(TAG, "send buffer full (%u), dropping oldest", (unsigned) this->send_buffer_length_);
          send_string_buffer_.pop();
        }

        send_string_buffer_.push(data);
        ESP_LOGD(TAG, "queued \"%s\" (%u/%u), client not ready", data.c_str(),
                 (unsigned) this->send_string_buffer_.size(), (unsigned) this->send_buffer_length_);
        return;
      }

      if (protocol_ == "udp") {
        udp_client_.print(data.c_str());
        udp_client_.print(delimiter_.c_str());
      } else if (protocol_ == "tcp") {
        tcp_client_.add(data.c_str(), strlen(data.c_str()));
        tcp_client_.add(delimiter_.c_str(), strlen(delimiter_.c_str()));
        tcp_client_.send();
      } else {
        return;
      }

      this->log_tx_(data);
    }
  }
}
