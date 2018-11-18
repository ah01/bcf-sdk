#ifndef _BC_DS18B20_H
#define _BC_DS18B20_H

#include <bc_onewire.h>
#include <bc_scheduler.h>

typedef enum
{
    //! @brief Error event
    BC_DS18B20_EVENT_ERROR = 0,

    //! @brief Update event
    BC_DS18B20_EVENT_UPDATE = 1

} bc_ds18b20_event_t;

typedef struct bc_ds18b20_t bc_ds18b20_t;

struct bc_ds18b20_t
{
    uint64_t _device_number;
    bc_gpio_channel_t _channel;
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    bc_scheduler_task_id_t _task_id_result;
    void (*_event_handler)(bc_ds18b20_t *, bc_ds18b20_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    bc_tick_t _update_interval;
    int8_t _parasite;
    int8_t _resolution;
    bool _temperature_valid;
    int16_t _temperature;

};

//! @brief Initialize DS18B20 thermometer
//! @param[in] self Instance
//! @param[in] channel GPIO channel
//! @param[in] device_number OneWire device number
//! @return true When successfully initialized
//! @return false When not successfully initialized

bool bc_ds18b20_init(bc_ds18b20_t *self, bc_gpio_channel_t channel, uint64_t device_number);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_ds18b20_set_event_handler(bc_ds18b20_t *self, void (*event_handler)(bc_ds18b20_t *, bc_ds18b20_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_ds18b20_set_update_interval(bc_ds18b20_t *self, bc_tick_t interval);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_ds18b20_get_temperature_celsius(bc_ds18b20_t *self, float *celsius);

//! @brief Get current resolution of the device, 9-12
//! @param[in] self Instance
//! @param[in] resolution Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid
bool bc_ds18b20_get_resolution(bc_ds18b20_t *self, uint8_t *resolution);

//! @brief Set resolution of the device, 9-12
//! @param[in] self Instance
//! @param[in] resolution Resolution of the device, 9-12
//! @return true On success
//! @return false On failure
bool bc_ds18b20_set_resolution(bc_ds18b20_t *self, uint8_t resolution);

#endif // _BC_DS18B20_H
