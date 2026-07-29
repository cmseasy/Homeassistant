// #pragma once
// #include "esphome/core/component.h"
// #include "esphome/core/log.h"
// #include "esp_websocket_client.h"

// class MyWebsocketComponent : public esphome::Component {
//  public:
//   esp_websocket_client_handle_t client = nullptr;
//   std::string latest_payload = ""; 
//   bool has_new_data = false;

//   void setup() override {
//     esp_websocket_client_config_t ws_cfg = {};
//     ws_cfg.uri = "ws://192.168.1.25:5580/ws"; 

//     client = esp_websocket_client_init(&ws_cfg);
    
//     esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, [](void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
//         auto *instance = (MyWebsocketComponent *)handler_args;
//         auto *data = (esp_websocket_event_data_t *)event_data;
        
//         if (event_id == WEBSOCKET_EVENT_CONNECTED) {
//             ESP_LOGI("websocket", "Verbonden! Starten van Matter-sessie...");
//             std::string listen_cmd = R"({"message_id":"start","command":"start_listening"})";
//             instance->send_message(listen_cmd);
            
//             std::string get_node_cmd = R"({"message_id":"1","command":"get_node","args":{"node_id":1}})";
//             instance->send_message(get_node_cmd);
            
//         } else if (event_id == WEBSOCKET_EVENT_DATA) {
//             if (data->data_ptr != nullptr && data->data_len > 0) {
//                 std::string inkomend(data->data_ptr, data->data_len);
                
//                 // DE CORRECTIE: 'inkomend' overal exact correct gespeld
//                 if (inkomend.find("get_node") != std::string::npos || inkomend.find("1/513/0") != std::string::npos) {
//                     instance->latest_payload = inkomend;
//                     instance->has_new_data = true;
//                 }
//             }
//         }
//     }, this);

//     esp_websocket_client_start(client);
//   }

//   void send_message(const std::string &message) {
//     if (client != nullptr && esp_websocket_client_is_connected(client)) {
//         esp_websocket_client_send_text(client, message.c_str(), message.length(), portMAX_DELAY);
//     }
//   }

//   std::string get_payload() {
//     if (has_new_data) {
//         has_new_data = false; 
//         return latest_payload;
//     }
//     return "";
//   }
// };


#pragma once
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include "esp_websocket_client.h"

class MyWebsocketComponent : public esphome::Component {
 public:
  esp_websocket_client_handle_t client = nullptr;
  
  // Twee aparte postbussen voor de YAML-sensoren
  std::string payload_temp = "";
  std::string payload_rh = "";
  bool data_temp_ready = false;
  bool data_rh_ready = false;

  void setup() override {
    esp_websocket_client_config_t ws_cfg = {};
    ws_cfg.uri = "ws://192.168.1.25:5580/ws"; 

    client = esp_websocket_client_init(&ws_cfg);
    
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, [](void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
        auto *instance = (MyWebsocketComponent *)handler_args;
        auto *data = (esp_websocket_event_data_t *)event_data;
        
        // if (event_id == WEBSOCKET_EVENT_CONNECTED) {
        //     ESP_LOGI("websocket", "Verbonden! Starten van Matter-sessie...");
        //     std::string listen_cmd = R"({"message_id":"start","command":"start_listening"})";
        //     instance->send_message(listen_cmd);
            
        //     std::string get_node_cmd = R"({"message_id":"1","command":"get_node","args":{"node_id":1}})";
        //     instance->send_message(get_node_cmd);

        if (event_id == WEBSOCKET_EVENT_CONNECTED) {
          // Log eerst puur de status en zet er een harde newline achter
          //ESP_LOGI("websocket", "Verbonden! Starten van Matter-sessie...\n");
          //printf("I (websocket) Verbonden! Starten van Matter-sessie...\n");
          ESP_LOGI("websocket", "Verbonden! Starten van Matter-sessie...");

    
          // Voeg een piepkleine vertraging toe om de UART-buffer leeg te laten lopen
          vTaskDelay(pdMS_TO_TICKS(10)); 

          std::string listen_cmd = R"({"message_id":"start","command":"start_listening"})";
          instance->send_message(listen_cmd);
    
          vTaskDelay(pdMS_TO_TICKS(10));

          std::string get_node_cmd = R"({"message_id":"1","command":"get_node","args":{"node_id":1}})";
          instance->send_message(get_node_cmd);
        

            
        } else if (event_id == WEBSOCKET_EVENT_DATA) {
            if (data->data_ptr != nullptr && data->data_len > 0) {
                std::string inkomend(data->data_ptr, data->data_len);
                
                // Sorteer de data direct naar de juiste postbus
                if (inkomend.find("get_node") != std::string::npos) {
                    instance->payload_temp = inkomend;
                    instance->payload_rh = inkomend;
                    instance->data_temp_ready = true;
                    instance->data_rh_ready = true;
                }
                if (inkomend.find("1/513/0") != std::string::npos) {
                    instance->payload_temp = inkomend;
                    instance->data_temp_ready = true;
                }
                if (inkomend.find("2/1029/0") != std::string::npos) {
                    instance->payload_rh = inkomend;
                    instance->data_rh_ready = true;
                }
            }
        }
    }, this);

    esp_websocket_client_start(client);
  }

  void send_message(const std::string &message) {
    if (client != nullptr && esp_websocket_client_is_connected(client)) {
        esp_websocket_client_send_text(client, message.c_str(), message.length(), portMAX_DELAY);
    }
  }

  // Ophaal-functies voor de YAML-sensoren
  std::string get_temp_payload() {
    if (data_temp_ready) { data_temp_ready = false; return payload_temp; }
    return "";
  }

  std::string get_rh_payload() {
    if (data_rh_ready) { data_rh_ready = false; return payload_rh; }
    return "";
  }
};
