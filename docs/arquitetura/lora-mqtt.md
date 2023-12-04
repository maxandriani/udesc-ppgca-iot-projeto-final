# LoRa Radio

## LoRa message types

* JOIN `0x01`: register the client device to network `join mac_addr token`;
* JOINT `0x02`: registration ack and device configuration `joint mac_addr config_data`;
* SEND `0x03`: Send data to gateway uplink `send mac_addr data`;
* RECV `0x04`: Receive data from gateway downlink `recv mac_addr data`;

## MQTT Topics

* `/register`: Register topic. Used only by gateway to register a new device to the network.
* `/devices/{mac_addr}/config`: Read only topic to receive devices configuration updates.
* `/devices/{mac_addr}/up`: Topic to listen device's data.
* `/devices/{mac_addr}/down`: Topic to send commands to devices.

## LoRa message cycle

``` mermaid

activate LoraGateway
    LoraGateway ->> AppServer: /register { mac: mac_addr, token: token }
    activate AppServer
    AppServer ->> LoraGateway: /devices/{mac_addr}/config { ... }
    deactivate AppServer
    LoraGateway ->> LoraClient: joint mac_addr config_data
deactivate LoraGateway

LoraClient ->> LoraGateway: send mac_addr data

activate LoraGateway
    LoraGateway ->> AppServer: /devices/{mac_addr}/up { ...data }
deactivate LoraGateway

AppServer ->> LoraGateway: /devices/{mac_addr}/down { ...data }
activate LoraGateway
    LoraGateway ->> LoraClient: recv mac_addr data
deactivate LoraGateway

```
