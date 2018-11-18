#include <application.h>

#define UPDATE_INTERVAL (5 * 1000)

void ds18b20_event_handler(bc_ds18b20_t *self, bc_ds18b20_event_t event, void *event_param)
{
    float value;

    if (event != BC_DS18B20_EVENT_UPDATE)
    {
        return;
    }

    if (bc_ds18b20_get_temperature_celsius(self, &value))
    {
        bc_log_info("Temperature: %f", value);
    }
}

void application_init(void)
{
    bc_log_init(BC_LOG_LEVEL_DUMP, BC_LOG_TIMESTAMP_ABS);

    bc_module_sensor_init();
    bc_module_sensor_set_pull(BC_MODULE_SENSOR_CHANNEL_A, BC_MODULE_SENSOR_PULL_UP_4K7);

    static bc_ds18b20_t ds18b20;
    bc_ds18b20_init(&ds18b20, BC_GPIO_P4, 0x00);
    bc_ds18b20_set_update_interval(&ds18b20, UPDATE_INTERVAL);
    bc_ds18b20_set_event_handler(&ds18b20, ds18b20_event_handler, NULL);
}
