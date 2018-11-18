#include <bc_ds18b20.h>
#include <bc_timer.h>

#define SCRATCHPAD_SIZE 9
#define CONFIGURATION 4

#define RESOLUTION_9_BIT 0x1F
#define RESOLUTION_10_BIT 0x3F
#define RESOLUTION_11_BIT 0x5F
#define RESOLUTION_12_BIT 0x7F

static void _bc_ds18b20_task_interval(void *param);
static void _bc_ds18b20_task_measure(void *param);
static void _bc_ds18b20_task_result(void *param);
bool bc_ds18b20_measure(bc_ds18b20_t *self);
int8_t _bc_ds18b20_is_parasite(bc_ds18b20_t *self);
int8_t _bc_ds18b20_get_resolution(bc_ds18b20_t *self);
bool _bc_ds18b20_read_scratchpad(bc_ds18b20_t *self, uint8_t *scratchpad);
bool _bc_ds18b20_write_scratchpad(bc_ds18b20_t *self, uint8_t *scratchpad);

bool bc_ds18b20_init(bc_ds18b20_t *self, bc_gpio_channel_t channel, uint64_t device_number)
{
    memset(self, 0, sizeof(*self));
    self->_device_number = device_number;
    self->_channel = channel;

    bc_onewire_init(self->_channel);

    self->_task_id_interval = bc_scheduler_register(_bc_ds18b20_task_interval, self, BC_TICK_INFINITY);
    self->_task_id_measure = bc_scheduler_register(_bc_ds18b20_task_measure, self, BC_TICK_INFINITY);
    self->_task_id_result = bc_scheduler_register(_bc_ds18b20_task_result, self, BC_TICK_INFINITY);

    self->_temperature_valid = false;

    self->_parasite = _bc_ds18b20_is_parasite(self);
    if (self->_parasite < 0)
    {
        return false;
    }

    self->_resolution = _bc_ds18b20_get_resolution(self);
    if (self->_resolution < 0)
    {
        return false;
    }

    return true;
}

void bc_ds18b20_set_event_handler(bc_ds18b20_t *self, void (*event_handler)(bc_ds18b20_t *, bc_ds18b20_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_ds18b20_set_update_interval(bc_ds18b20_t *self, bc_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(self->_task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        bc_ds18b20_measure(self);
    }
}

bool bc_ds18b20_measure(bc_ds18b20_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    bc_scheduler_plan_now(self->_task_id_measure);

    return true;
}

static void _bc_ds18b20_task_interval(void *param)
{
    bc_ds18b20_t *self = param;

    bc_ds18b20_measure(self);

    bc_scheduler_plan_current_relative(self->_update_interval);
}

static void _bc_ds18b20_task_measure(void *param)
{
    bc_ds18b20_t *self = param;

    if (!bc_onewire_reset(self->_channel))
    {
        self->_temperature_valid = false;
        self->_measurement_active = false;
        if (self->_event_handler != NULL)
        {
            self->_event_handler(self, BC_DS18B20_EVENT_ERROR, self->_event_param);
        }
        return;
    }

    bc_onewire_select(self->_channel, &self->_device_number);

    bc_onewire_write_8b(self->_channel, 0x44);

    if (self->_parasite)
    {
        // TODO:  If the device is being used in parasite power mode,
        // within 10s (max) after this command is issued the master
        // must enable a strong pullup on the 1-Wire bus for the
        // duration of the conversion
    }

    uint16_t wait;
    switch (self->_resolution)
    {
        case 9:
            wait = 94;
            break;
        case 10:
            wait = 188;
            break;
        case 11:
            wait = 375;
            break;
        default:
            wait = 750;
    }

    bc_scheduler_plan_from_now(self->_task_id_result, wait);
}

static void _bc_ds18b20_task_result(void *param)
{
    bc_ds18b20_t *self = param;

    uint8_t scratchpad[SCRATCHPAD_SIZE];
    if (!_bc_ds18b20_read_scratchpad(self, scratchpad))
    {
        self->_temperature_valid = false;
        self->_measurement_active = false;
        if (self->_event_handler != NULL)
        {
            self->_event_handler(self, BC_DS18B20_EVENT_ERROR, self->_event_param);
        }
        return;
    }

    self->_temperature = (((int16_t)scratchpad[1]) << 11) |
        (((int16_t)scratchpad[0]) << 3);

    if (self->_temperature <= -7040)
    {
        self->_temperature_valid = false;
        self->_measurement_active = false;
        if (self->_event_handler != NULL)
        {
            self->_event_handler(self, BC_DS18B20_EVENT_ERROR, self->_event_param);
        }
        return;
    }

    self->_temperature_valid = true;
    self->_measurement_active = false;

    if (self->_event_handler != NULL)
    {
        self->_event_handler(self, BC_DS18B20_EVENT_UPDATE, self->_event_param);
    }
}

int8_t _bc_ds18b20_is_parasite(bc_ds18b20_t *self)
{
    if (!bc_onewire_reset(self->_channel))
    {
        return -1;
    }

    bc_onewire_select(self->_channel, &self->_device_number);

    bc_onewire_write_8b(self->_channel, 0xB4);

    if (bc_onewire_read_8b(self->_channel) == 0)
    {
        return 1;
    }

    return 0;
}

bool _bc_ds18b20_read_scratchpad(bc_ds18b20_t *self, uint8_t *scratchpad)
{
    if (!bc_onewire_reset(self->_channel))
    {
        return false;
    }

    bc_onewire_select(self->_channel, &self->_device_number);

    bc_onewire_write_8b(self->_channel, 0xBE);

    bc_onewire_read(self->_channel, scratchpad, SCRATCHPAD_SIZE);

    return true;
}

bool _bc_ds18b20_write_scratchpad(bc_ds18b20_t *self, uint8_t *scratchpad)
{
    if (!bc_onewire_reset(self->_channel))
    {
        return false;
    }

    bc_onewire_select(self->_channel, &self->_device_number);

    bc_onewire_write_8b(self->_channel, 0x4E);

    bc_onewire_write_8b(self->_channel, scratchpad[2]);
    bc_onewire_write_8b(self->_channel, scratchpad[3]);
    bc_onewire_write_8b(self->_channel, scratchpad[4]);

    // save the newly written values to eeprom

    if (!bc_onewire_reset(self->_channel))
    {
        return false;
    }

    bc_onewire_select(self->_channel, &self->_device_number);

    bc_onewire_write_8b(self->_channel, 0x48);

    bc_timer_delay(20000); // 20 ms

    if (self->_parasite)
    {
        bc_timer_delay(10000); // 10 ms
    }

    return true;
}

int8_t _bc_ds18b20_get_resolution(bc_ds18b20_t *self)
{
    uint8_t scratchpad[SCRATCHPAD_SIZE];
    if (!_bc_ds18b20_read_scratchpad(self, scratchpad))
    {
        return -1;
    }
    switch (scratchpad[CONFIGURATION])
    {
        case RESOLUTION_12_BIT:
            return 12;

        case RESOLUTION_11_BIT:
            return 11;

        case RESOLUTION_10_BIT:
            return 10;

        case RESOLUTION_9_BIT:
            return 9;

        default:
            return -1;
    }
    return -1;
}

bool bc_ds18b20_get_resolution(bc_ds18b20_t *self, uint8_t *resolution)
{
    if (self->_resolution < 0) {
        return false;
    }
    *resolution = self->_resolution;
    return true;
}

bool bc_ds18b20_set_resolution(bc_ds18b20_t *self, uint8_t resolution)
{
    if (resolution == self->_resolution) {
        return true;
    }
    uint8_t scratchpad[SCRATCHPAD_SIZE];
    if (!_bc_ds18b20_read_scratchpad(self, scratchpad))
    {
        return false;
    }
    switch (resolution)
    {
        case 12:
            scratchpad[CONFIGURATION] = RESOLUTION_12_BIT;
            break;
        case 11:
            scratchpad[CONFIGURATION] = RESOLUTION_11_BIT;
            break;
        case 10:
            scratchpad[CONFIGURATION] = RESOLUTION_10_BIT;
            break;
        case 9:
        default:
            scratchpad[CONFIGURATION] = RESOLUTION_9_BIT;
            break;
    }

    return _bc_ds18b20_write_scratchpad(self, scratchpad);
}

bool bc_ds18b20_get_temperature_celsius(bc_ds18b20_t *self, float *celsius)
{
    if (!self->_temperature_valid)
    {
        return false;
    }

    *celsius = (float)self->_temperature * 0.0078125;

    return true;
}
