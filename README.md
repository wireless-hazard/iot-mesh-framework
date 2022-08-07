# IoT Mesh Framework
A WiFi based Mesh Network for the ESP32 that allows its nodes to use IoT related protocols

This framework is build upon the [ESP-WIFI-MESH](https://www.espressif.com/en/products/sdks/esp-wifi-mesh/overview) using the [Espressif IoT Development Framework](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/), its main goal being to ease the usage of ESP-WIFI-MESH, and add the possibility to use IoT related protocols in each node that the network is composed of.

## Installing and Getting Started

First step is to clone the repository

```shell
git clone https://github.com/wireless-hazard/iot-mesh-framework
```

This framework behaves like a normal esp-idf component so, after you've downloaded it, you just need to add it into your project's extra components folder.
Make sure you have the following line on your top-level CMakeLists.txt file (your some equivalent command)

```cmake
set(EXTRA_COMPONENT_DIRS ${project_dir}/components)
```

## Starting the Wireless Mesh Network

Some configurations related to the Gateway and ip addresses need to be set beforehand using:

```shell
idf.py menuconfig
```

They'll be under the **Mesh Configuration** menu.



The following snippet shows the minimum amount of code necessary to build a functional network. If _meshf_start()_ returns **ESP_OK**, that node can already use the network infrastructure.

```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mesh_framework.h"

static uint8_t rx_mensagem[180] = {0};

void app_main(void) {
  meshf_init(); //Init Mesh Mandatory Configurations
  //Waits up to 45 seconds until the mesh is built
  esp_err_t mesh_on = meshf_start(45000/portTICK_PERIOD_MS); //Init the Mesh Network itself 
  if (mesh_on != ESP_OK){
    //Error handling in case the Network is not able to the built
  }
  meshf_rx(rx_mensagem);
}
```

## Current Features

Althought the title promisses IoT related protocols in general, at the current state, two protocols are supported:

- :heavy_check_mark: [SNTP](https://en.wikipedia.org/wiki/Network_Time_Protocol) 
- :heavy_check_mark: [MQTT](https://mqtt.org/)

The usage of the protocols is still straightforward. Basically, you need to initialize it once, and then call the protocol's functionality that you want.

```c
//An example on how to use publish a message using MQTT
const char *payload = "I am a pretty important message";
const char *mqtt_topic = "/iot/esp32";
meshf_start_mqtt(); //Connects to MQTT's Broker
//Publishes the payload message in the mqtt_topic
esp_err_t pub_err = meshf_mqtt_publish(mqtt_topic,strlen(mqtt_topic),payload,strlen(payload));
if (pub_err != ESP_OK){
  //Message couldn't be published!
}
```
