#include <stdlib.h>
#include <string.h>
#include "devices.h"

device_t *device_search(device_list_t *devices, uint8_t *mac) {
    for (int i = 0; i < devices->size; i++) {
        if (memcmp(mac, (uint8_t *)devices->list[i].mac, sizeof(uint8_t) * 6) == 0) {
            return &devices->list[i];
        }
    }

    return NULL;
}

uint8_t device_index_of(device_list_t *devices, uint8_t *mac) {
    for (int i = 0; i < devices->size; i++) {
        if (memcmp(mac, (uint8_t *)devices->list[i].mac, sizeof(uint8_t) * 6) == 0) {
            return i;
        }
    }

    return -1;
}

void device_insert_if_not_exists(device_list_t *devices, uint8_t *mac) {
    uint8_t idx = device_index_of(devices, mac);
    if (idx > -1)
        return;

    if (devices->size == 256)
        return; // Nem a pau um ESP32 vai suportar mais de 256 rádio clients...

    devices->list[devices->size] = {
        .mac = {mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]}};
    devices->size++;
}

void device_remove_if_exists(device_list_t *devices, uint8_t *mac) {
    uint8_t idx = device_index_of(devices, mac);

    if (idx == -1)
        return; // Not found

    for (int current = idx, next = idx + 1; current < devices->size; current++, next++) {
        if (next < devices->size) {
            devices->list[current] = devices->list[next];
        } else {
            devices->list[current] = { };
        }
    }
}
