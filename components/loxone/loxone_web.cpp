#include "loxone_web.h"

#if defined(USE_NETWORK) && !defined(USE_ZEPHYR)
#include <span>
#include "esphome/core/log.h"
#include "esphome/components/network/util.h"
#include "esphome/components/network/ip_address.h"

namespace esphome {
namespace loxone {

static const char *const TAG = "loxone.web";

namespace {

void replace_all(std::string &haystack, const std::string &from, const std::string &to) {
  if (from.empty())
    return;
  size_t pos = 0;
  while ((pos = haystack.find(from, pos)) != std::string::npos) {
    haystack.replace(pos, from.size(), to);
    pos += to.size();
  }
}

std::string request_path(AsyncWebServerRequest *request) {
  char buf[AsyncWebServerRequest::URL_BUF_SIZE];
  return request->url_to(buf).str();
}

}  // namespace

bool LoxoneTemplateHandler::canHandle(AsyncWebServerRequest *request) const {
  if (request->method() != HTTP_GET)
    return false;
  const std::string path = request_path(request);
  return path == "/loxone" || path == "/loxone/" || path == "/loxone/inputs.xml" ||
         path == "/loxone/outputs.xml";
}

std::string LoxoneTemplateHandler::device_ip_(AsyncWebServerRequest *request) const {
  auto host = request->get_header("Host");
  if (host.has_value() && !host.value().empty()) {
    std::string h = host.value();
    const size_t colon = h.find(':');
    if (colon != std::string::npos)
      h.erase(colon);
    if (!h.empty())
      return h;
  }
  for (const auto &ip : network::get_ip_addresses()) {
    if (ip.is_set() && ip.is_ip4()) {
      char buf[network::IP_ADDRESS_BUFFER_SIZE];
      ip.str_to(buf);
      return std::string(buf);
    }
  }
  return "DEVICE_IP";
}

void LoxoneTemplateHandler::handleRequest(AsyncWebServerRequest *request) {
  const std::string path = request_path(request);

  if (path == "/loxone/inputs.xml") {
    auto *r = request->beginResponse(200, "application/xml; charset=utf-8", this->inputs_xml_);
    r->addHeader("Content-Disposition", "attachment; filename=\"loxone-inputs.xml\"");
    request->send(r);
    return;
  }

  if (path == "/loxone/outputs.xml") {
    std::string body = this->outputs_xml_;
    replace_all(body, "{IP}", this->device_ip_(request));
    auto *r = request->beginResponse(200, "application/xml; charset=utf-8", body);
    r->addHeader("Content-Disposition", "attachment; filename=\"loxone-outputs.xml\"");
    request->send(r);
    return;
  }

  // "/loxone" or "/loxone/"
  std::string body = this->index_html_;
  replace_all(body, "{IP}", this->device_ip_(request));
  request->send(request->beginResponse(200, "text/html; charset=utf-8", body));
}

}  // namespace loxone
}  // namespace esphome

#endif  // USE_NETWORK && !USE_ZEPHYR
