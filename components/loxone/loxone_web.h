#pragma once

#include "esphome/core/defines.h"

#if defined(USE_NETWORK) && !defined(USE_ZEPHYR)
#include <string>
#include "esphome/components/web_server_base/web_server_base.h"

namespace esphome {
namespace loxone {

/** Serves ready-to-import Loxone Config templates for this device.
 *
 * Routes (registered on the shared web_server_base):
 *   GET /loxone/            - HTML page: download links, import steps, matching ESPHome lambdas
 *   GET /loxone/inputs.xml  - <VirtualInUdp>  template  (device -> Loxone)
 *   GET /loxone/outputs.xml - <VirtualOut> UDP template (Loxone -> device)
 *
 * The XML/HTML bodies are rendered at compile time from the `template:` block and
 * baked in as strings. The only runtime work is substituting "{IP}" (this device's
 * address, taken from the request Host header) into the outputs template and page.
 */
class LoxoneTemplateHandler : public AsyncWebHandler {
 public:
  LoxoneTemplateHandler(std::string index_html, std::string inputs_xml, std::string outputs_xml)
      : index_html_(std::move(index_html)),
        inputs_xml_(std::move(inputs_xml)),
        outputs_xml_(std::move(outputs_xml)) {}

  bool canHandle(AsyncWebServerRequest *request) const override;
  void handleRequest(AsyncWebServerRequest *request) override;
  bool isRequestHandlerTrivial() const override { return true; }

 protected:
  // This device's address as the browser reached it (Host header, ":port" stripped),
  // falling back to the first configured IPv4 address.
  std::string device_ip_(AsyncWebServerRequest *request) const;

  std::string index_html_;   // contains "{IP}" placeholder
  std::string inputs_xml_;
  std::string outputs_xml_;  // contains "{IP}" placeholder
};

}  // namespace loxone
}  // namespace esphome

#endif  // USE_NETWORK && !USE_ZEPHYR
