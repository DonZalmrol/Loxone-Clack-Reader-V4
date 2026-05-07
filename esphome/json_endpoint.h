#pragma once
#include "esphome.h"
#include <ctime>
#include "dashboard.h"
#include "config_page.h"

#ifdef USE_WEBSERVER
#include "esphome/components/web_server_base/web_server_base.h"
#include "esphome/components/wifi/wifi_component.h"
#include "esphome/components/i2c/i2c_bus.h"
#include <esp_wifi.h>
#include <esp_system.h>

// --- API authentication ---
// Token is injected through a PlatformIO build flag in .clack-base.yaml.
// When empty, authentication is disabled (all endpoints are open).
#ifndef CLACK_API_TOKEN
#define CLACK_API_TOKEN ""
#endif
static const char *API_TOKEN = CLACK_API_TOKEN;

static bool api_auth_enabled() {
  return API_TOKEN[0] != '\0' && std::string(API_TOKEN) != "${api_token}";
}

// Check token from query param ?token=X or header "Authorization: Bearer X"
// Returns true if auth is disabled (empty token) or if the token matches.
static bool check_api_auth(AsyncWebServerRequest *request) {
  if (!api_auth_enabled()) return true;

  // Check query parameter
  if (request->hasParam("token")) {
    if (request->getParam("token")->value() == API_TOKEN) return true;
  }

  // Check Authorization header
  if (request->hasHeader("Authorization")) {
    auto auth = request->get_header("Authorization");
    if (auth.has_value() && auth->size() > 7 && auth->substr(0, 7) == "Bearer ") {
      if (auth->substr(7) == API_TOKEN) return true;
    }
  }

  return false;
}

static void send_unauthorized(AsyncWebServerRequest *request) {
  request->send(401, "text/plain", "Unauthorized: invalid or missing token");
}

static std::string json_escape(const std::string &s) {
  std::string out;
  out.reserve(s.length() + 4);
  for (unsigned char c : s) {
    switch (c) {
      case '"':  out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      case '\b': out += "\\b"; break;
      case '\f': out += "\\f"; break;
      default:
        if (c < 0x20) {
          char buf[8];
          snprintf(buf, sizeof(buf), "\\u%04X", c);
          out += buf;
        } else {
          out += (char)c;
        }
    }
  }
  return out;
}

// Redirect / to /dashboard
class RootRedirectHandler : public AsyncWebHandler {
 public:
  bool canHandle(AsyncWebServerRequest *request) const override {
    return request->url() == "/" && request->method() == HTTP_GET;
  }
  void handleRequest(AsyncWebServerRequest *request) override {
    request->redirect("/dashboard");
  }
};

class JsonApiHandler : public AsyncWebHandler {
 public:
  bool canHandle(AsyncWebServerRequest *request) const override {
    return request->url() == "/json" && request->method() == HTTP_GET;
  }
  void handleRequest(AsyncWebServerRequest *request) override {
    std::string json = "{";
    bool first = true;

    auto comma = [&]() {
      if (!first) json += ",";
      first = false;
    };

    // Timestamp (ISO 8601) and uptime in seconds
    {
      auto now = id(sntp_time).now();
      if (now.is_valid()) {
        char ts[32];
        snprintf(ts, sizeof(ts), "%04d-%02d-%02dT%02d:%02d:%02d",
                 now.year, now.month, now.day_of_month,
                 now.hour, now.minute, now.second);
        json += "\"timestamp\":\"";
        json += ts;
        json += "\"";
        first = false;
      }
      comma();
      char up[32];
      snprintf(up, sizeof(up), "%lu", (unsigned long)(millis() / 1000));
      json += "\"uptime_seconds\":";
      json += up;
    }

#ifdef USE_SENSOR
    for (auto *obj : App.get_sensors()) {
      if (obj->is_internal()) continue;
      comma();
      json += "\"";
      json += obj->get_object_id();
      json += "\":";
      if (std::isnan(obj->state)) {
        json += "null";
      } else {
        char buf[32];
        snprintf(buf, sizeof(buf), "%g", obj->state);
        json += buf;
      }
      auto uom = obj->get_unit_of_measurement();
      if (!uom.empty()) {
        comma();
        json += "\"";
        json += obj->get_object_id();
        json += "_unit\":\"";
        json += json_escape(uom);
        json += "\"";
      }
    }
#endif

#ifdef USE_TEXT_SENSOR
    for (auto *obj : App.get_text_sensors()) {
      if (obj->is_internal()) continue;
      comma();
      json += "\"";
      json += obj->get_object_id();
      json += "\":\"";
      json += json_escape(obj->state);
      json += "\"";
    }
#endif

#ifdef USE_BINARY_SENSOR
    for (auto *obj : App.get_binary_sensors()) {
      if (obj->is_internal()) continue;
      comma();
      json += "\"";
      json += obj->get_object_id();
      json += "\":";
      json += obj->state ? "true" : "false";
    }
#endif

#ifdef USE_SWITCH
    for (auto *obj : App.get_switches()) {
      if (obj->is_internal()) continue;
      comma();
      json += "\"";
      json += obj->get_object_id();
      json += "\":";
      json += obj->state ? "true" : "false";
    }
#endif

#ifdef USE_NUMBER
    for (auto *obj : App.get_numbers()) {
      if (obj->is_internal()) continue;
      comma();
      json += "\"";
      json += obj->get_object_id();
      json += "\":";
      if (std::isnan(obj->state)) {
        json += "null";
      } else {
        char buf[32];
        snprintf(buf, sizeof(buf), "%g", obj->state);
        json += buf;
      }
      auto uom = obj->get_unit_of_measurement();
      if (!uom.empty()) {
        comma();
        json += "\"";
        json += obj->get_object_id();
        json += "_unit\":\"";
        json += json_escape(uom);
        json += "\"";
      }
    }
#endif

#ifdef USE_SELECT
    for (auto *obj : App.get_selects()) {
      if (obj->is_internal()) continue;
      comma();
      json += "\"";
      json += obj->get_object_id();
      json += "\":\"";
      json += json_escape(obj->state);
      json += "\"";
    }
#endif

    json += "}";

    request->send(200, "application/json", json.c_str());
  }
};

class DashboardHandler : public AsyncWebHandler {
 public:
  bool canHandle(AsyncWebServerRequest *request) const override {
    return request->url() == "/dashboard" && request->method() == HTTP_GET;
  }
  void handleRequest(AsyncWebServerRequest *request) override {
    auto *response = request->beginResponse(200, "text/html", (const char *)DASHBOARD_HTML);
    response->addHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
    response->addHeader("Pragma", "no-cache");
    request->send(response);
  }
};

class ConfigPageHandler : public AsyncWebHandler {
 public:
  bool canHandle(AsyncWebServerRequest *request) const override {
    return request->url() == "/config" && request->method() == HTTP_GET;
  }
  void handleRequest(AsyncWebServerRequest *request) override {
    auto *response = request->beginResponse(200, "text/html", (const char *)CONFIG_HTML);
    response->addHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
    response->addHeader("Pragma", "no-cache");
    request->send(response);
  }
};

// GET /api/entities — returns all configurable entities with metadata
class EntitiesApiHandler : public AsyncWebHandler {
 public:
  bool canHandle(AsyncWebServerRequest *request) const override {
    return request->url() == "/api/entities" && request->method() == HTTP_GET;
  }
  void handleRequest(AsyncWebServerRequest *request) override {
    if (!check_api_auth(request)) { send_unauthorized(request); return; }

    std::string json = "{";

    // Numbers
    json += "\"numbers\":[";
    bool first = true;
#ifdef USE_NUMBER
    for (auto *obj : App.get_numbers()) {
      if (obj->is_internal()) continue;
      if (!first) json += ",";
      first = false;
      json += "{\"id\":\"";
      json += json_escape(obj->get_object_id());
      json += "\",\"name\":\"";
      json += json_escape(obj->get_name());
      json += "\",\"state\":";
      if (std::isnan(obj->state)) json += "null";
      else { char buf[32]; snprintf(buf, sizeof(buf), "%g", obj->state); json += buf; }
      json += ",\"min\":";
      { char buf[32]; snprintf(buf, sizeof(buf), "%g", obj->traits.get_min_value()); json += buf; }
      json += ",\"max\":";
      { char buf[32]; snprintf(buf, sizeof(buf), "%g", obj->traits.get_max_value()); json += buf; }
      json += ",\"step\":";
      { char buf[32]; snprintf(buf, sizeof(buf), "%g", obj->traits.get_step()); json += buf; }
      auto uom = obj->get_unit_of_measurement();
      if (!uom.empty()) {
        json += ",\"unit\":\"";
        json += json_escape(uom);
        json += "\"";
      }
      json += "}";
    }
#endif
    json += "],";

    // Selects
    json += "\"selects\":[";
    first = true;
#ifdef USE_SELECT
    for (auto *obj : App.get_selects()) {
      if (obj->is_internal()) continue;
      if (!first) json += ",";
      first = false;
      json += "{\"id\":\"";
      json += json_escape(obj->get_object_id());
      json += "\",\"name\":\"";
      json += json_escape(obj->get_name());
      json += "\",\"state\":\"";
      json += json_escape(std::string(obj->current_option()));
      json += "\",\"options\":[";
      bool ofirst = true;
      for (auto &opt : obj->traits.get_options()) {
        if (!ofirst) json += ",";
        ofirst = false;
        json += "\"";
        json += json_escape(opt);
        json += "\"";
      }
      json += "]}";
    }
#endif
    json += "],";

    // Switches
    json += "\"switches\":[";
    first = true;
#ifdef USE_SWITCH
    for (auto *obj : App.get_switches()) {
      if (obj->is_internal()) continue;
      if (!first) json += ",";
      first = false;
      json += "{\"id\":\"";
      json += json_escape(obj->get_object_id());
      json += "\",\"name\":\"";
      json += json_escape(obj->get_name());
      json += "\",\"state\":";
      json += obj->state ? "true" : "false";
      json += "}";
    }
#endif
    json += "]}";

    request->send(200, "application/json", json.c_str());
  }
};

// POST /api/set?domain=number|select|switch&id=X&value=X|option=X|state=on|off
class EntitySetHandler : public AsyncWebHandler {
 public:
  bool canHandle(AsyncWebServerRequest *request) const override {
    return request->url() == "/api/set" && request->method() == HTTP_POST;
  }
  void handleRequest(AsyncWebServerRequest *request) override {
    if (!check_api_auth(request)) { send_unauthorized(request); return; }

    if (!request->hasParam("domain") || !request->hasParam("id")) {
      request->send(400, "text/plain", "Missing domain or id");
      return;
    }
    std::string domain = request->getParam("domain")->value();
    std::string eid = request->getParam("id")->value();

#ifdef USE_NUMBER
    if (domain == "number") {
      if (!request->hasParam("value")) { request->send(400, "text/plain", "Missing value"); return; }
      float val = atof(request->getParam("value")->value().c_str());
      for (auto *obj : App.get_numbers()) {
        if (obj->get_object_id() == eid) {
          obj->make_call().set_value(val).perform();
          request->send(200, "text/plain", "OK");
          return;
        }
      }
      request->send(404, "text/plain", "Number not found");
      return;
    }
#endif

#ifdef USE_SELECT
    if (domain == "select") {
      if (!request->hasParam("option")) { request->send(400, "text/plain", "Missing option"); return; }
      std::string opt = request->getParam("option")->value();
      for (auto *obj : App.get_selects()) {
        if (obj->get_object_id() == eid) {
          obj->make_call().set_option(opt).perform();
          request->send(200, "text/plain", "OK");
          return;
        }
      }
      request->send(404, "text/plain", "Select not found");
      return;
    }
#endif

#ifdef USE_SWITCH
    if (domain == "switch") {
      if (!request->hasParam("state")) { request->send(400, "text/plain", "Missing state"); return; }
      std::string st = request->getParam("state")->value();
      for (auto *obj : App.get_switches()) {
        if (obj->get_object_id() == eid) {
          if (st == "on") obj->turn_on();
          else obj->turn_off();
          request->send(200, "text/plain", "OK");
          return;
        }
      }
      request->send(404, "text/plain", "Switch not found");
      return;
    }
#endif

    request->send(400, "text/plain", "Unknown domain");
  }
};

// GET /api/wifi — returns current WiFi info as JSON
// POST /api/wifi?ssid=X&password=X — save new WiFi credentials and restart
// POST /api/wifi?reset=1 — clear saved credentials and restart
class WifiApiHandler : public AsyncWebHandler {
 public:
  bool canHandle(AsyncWebServerRequest *request) const override {
    return request->url() == "/api/wifi";
  }
  void handleRequest(AsyncWebServerRequest *request) override {
    if (request->method() == HTTP_GET) {
      if (!check_api_auth(request)) { send_unauthorized(request); return; }
      handleGet(request);
    } else if (request->method() == HTTP_POST) {
      if (!check_api_auth(request)) { send_unauthorized(request); return; }
      handlePost(request);
    } else {
      request->send(405, "text/plain", "Method not allowed");
    }
  }

 private:
  void handleGet(AsyncWebServerRequest *request) {
    auto *wifi = esphome::wifi::global_wifi_component;
    std::string json = "{";

    // SSID
    json += "\"ssid\":\"";
    json += json_escape(wifi->wifi_ssid());
    json += "\"";

    // IP addresses
    auto ips = wifi->get_ip_addresses();
    if (!ips.empty()) {
      json += ",\"ip\":\"";
      json += ips[0].str();
      json += "\"";
    }

    // Gateway and DNS via ESP-IDF
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
      char gw[16], dns_buf[16];
      snprintf(gw, sizeof(gw), IPSTR, IP2STR(&ip_info.gw));
      json += ",\"gateway\":\"";
      json += gw;
      json += "\"";

      esp_netif_dns_info_t dns_info;
      if (esp_netif_get_dns_info(netif, ESP_NETIF_DNS_MAIN, &dns_info) == ESP_OK) {
        snprintf(dns_buf, sizeof(dns_buf), IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
        json += ",\"dns\":\"";
        json += dns_buf;
        json += "\"";
      }
    }

    // RSSI
    json += ",\"rssi\":";
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", wifi->wifi_rssi());
    json += buf;

    // MAC
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    json += ",\"mac\":\"";
    json += mac_str;
    json += "\"";

    // Report whether auth is enabled (helps config page)
    json += ",\"auth_enabled\":";
    json += api_auth_enabled() ? "true" : "false";

    json += "}";
    request->send(200, "application/json", json.c_str());
  }

  void handlePost(AsyncWebServerRequest *request) {
    auto *wifi = esphome::wifi::global_wifi_component;

    // Support JSON body (preferred) and legacy query params for backward compat

    // Check for reset
    if (request->hasParam("reset")) {
      wifi->save_wifi_sta("", "");
      request->send(200, "text/plain", "WiFi reset to defaults. Restarting...");
      delay(500);
      App.safe_reboot();
      return;
    }

    // Get SSID and password from request params (body or query)
    std::string ssid;
    std::string password;

    if (request->hasParam("ssid")) {
      ssid = request->getParam("ssid")->value();
    }

    if (request->hasParam("password")) {
      password = request->getParam("password")->value();
    }

    if (ssid.empty()) {
      request->send(400, "text/plain", "Missing ssid parameter");
      return;
    }
    if (!password.empty() && password.length() < 8) {
      request->send(400, "text/plain", "Password must be at least 8 characters");
      return;
    }

    ESP_LOGI("config", "Saving new WiFi credentials: SSID=%s", ssid.c_str());
    wifi->save_wifi_sta(ssid, password);

    request->send(200, "text/plain", "WiFi credentials saved. Restarting...");
    delay(500);
    App.safe_reboot();
  }
};

// POST /api/restart — restart device
class RestartHandler : public AsyncWebHandler {
 public:
  bool canHandle(AsyncWebServerRequest *request) const override {
    return request->url() == "/api/restart" && request->method() == HTTP_POST;
  }
  void handleRequest(AsyncWebServerRequest *request) override {
    if (!check_api_auth(request)) { send_unauthorized(request); return; }

    request->send(200, "text/plain", "Restarting...");
    delay(500);
    App.safe_reboot();
  }
};

// GET /api/diag — I2C bus scan and sensor diagnostics
class DiagHandler : public AsyncWebHandler {
 public:
  bool canHandle(AsyncWebServerRequest *request) const override {
    return request->url() == "/api/diag" && request->method() == HTTP_GET;
  }
  void handleRequest(AsyncWebServerRequest *request) override {
    if (!check_api_auth(request)) { send_unauthorized(request); return; }

    std::string json = "{";

    // I2C bus scan
    json += "\"i2c_scan\":[";
    bool first = true;
    // Get bus_a by ID
    auto *bus = reinterpret_cast<esphome::i2c::I2CBus *>(bus_a);
    if (bus) {
      for (uint8_t addr = 0x08; addr < 0x78; addr++) {
        uint8_t dummy;
        auto err = bus->read(addr, &dummy, 0);
        if (err == esphome::i2c::ERROR_OK) {
          if (!first) json += ",";
          first = false;
          char buf[16];
          snprintf(buf, sizeof(buf), "\"0x%02X\"", addr);
          json += buf;
        }
      }
    }
    json += "],";

    // Report sensor states
    json += "\"sensors\":{";
    first = true;
#ifdef USE_SENSOR
    for (auto *obj : App.get_sensors()) {
      if (obj->is_internal()) continue;
      if (!first) json += ",";
      first = false;
      json += "\"";
      json += obj->get_object_id();
      json += "\":{\"state\":";
      if (std::isnan(obj->state)) json += "null";
      else { char buf[32]; snprintf(buf, sizeof(buf), "%g", obj->state); json += buf; }
      json += ",\"has_state\":";
      json += obj->has_state() ? "true" : "false";
      json += "}";
    }
#endif
    json += "},";

    // I2C bus info
    json += "\"i2c_bus\":{\"sda\":6,\"scl\":5},";

    // Free heap
    json += "\"free_heap\":";
    char buf[16];
    snprintf(buf, sizeof(buf), "%u", (unsigned)esp_get_free_heap_size());
    json += buf;

    json += "}";
    request->send(200, "application/json", json.c_str());
  }
};

static void register_json_endpoint() {
  auto *base = esphome::web_server_base::global_web_server_base;
  if (!base) {
    ESP_LOGW("json", "Web server base not available");
    return;
  }

  if (api_auth_enabled()) {
    ESP_LOGI("json", "API authentication enabled (token set)");
  } else {
    ESP_LOGI("json", "API authentication disabled (no token)");
  }

  base->add_handler(new RootRedirectHandler());
  ESP_LOGI("json", "Registered / -> /dashboard redirect");

  base->add_handler(new JsonApiHandler());
  ESP_LOGI("json", "Registered /json endpoint");

  base->add_handler(new DashboardHandler());
  ESP_LOGI("json", "Registered /dashboard endpoint");

  base->add_handler(new ConfigPageHandler());
  ESP_LOGI("json", "Registered /config endpoint");

  base->add_handler(new EntitiesApiHandler());
  ESP_LOGI("json", "Registered /api/entities endpoint");

  base->add_handler(new EntitySetHandler());
  ESP_LOGI("json", "Registered /api/set endpoint");

  base->add_handler(new WifiApiHandler());
  ESP_LOGI("json", "Registered /api/wifi endpoint");

  base->add_handler(new RestartHandler());
  ESP_LOGI("json", "Registered /api/restart endpoint");

  base->add_handler(new DiagHandler());
  ESP_LOGI("json", "Registered /api/diag endpoint");
}

#else

static void register_json_endpoint() {
  ESP_LOGW("json", "Web server not enabled, /json endpoint not available");
}

#endif
