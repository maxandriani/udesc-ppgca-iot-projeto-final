#ifndef DEVICES_H
#define DEVICES_H

typedef struct device {
    uint8_t mac[6];
} device_t;

typedef struct device_list {
    device_t list[256];
    uint8_t size;
} device_list_t;

device_t *device_search(device_list_t *devices, uint8_t *mac);
uint8_t device_index_of(device_list_t *devices, uint8_t *mac);
void device_insert_if_not_exists(device_list_t *devices, uint8_t *mac);
void device_remove_if_exists(device_list_t *devices, uint8_t *mac);

#endif