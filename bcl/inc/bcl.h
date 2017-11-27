#ifndef _BCL_H
#define _BCL_H

// Miscellaneous

#include <bc_ir_rx.h>
#include <bc_irq.h>
#include <bc_led_strip.h>
#include <bc_radio.h>
#include <bc_log.h>

// Peripheral drivers

#include <bc_adc.h>
#include <bc_button.h>
#include <bc_dac.h>
#include <bc_eeprom.h>
#include <bc_gpio.h>
#include <bc_i2c.h>
#include <bc_led.h>
#include <bc_rtc.h>
#include <bc_uart.h>
#include <bc_spi.h>

// Chip drivers

#include <bc_lis2dh12.h>
#include <bc_spirit1.h>
#include <bc_hc_sr04.h>
#include <bc_cp201t.h>

// BigClown tags

#include <bc_tag_barometer.h>
#include <bc_tag_humidity.h>
#include <bc_tag_lux_meter.h>
#include <bc_tag_temperature.h>

// BigClown modules

#include <bc_module_battery.h>
#include <bc_module_climate.h>
#include <bc_module_core.h>
#include <bc_module_encoder.h>
#include <bc_module_pir.h>
#include <bc_module_power.h>
#include <bc_module_relay.h>
#include <bc_module_sigfox.h>
#include <bc_module_lcd.h>
#include <bc_module_co2.h>
#include <bc_module_sensor.h>

// Other

#include <bc_analog_sensor.h>
#include <bc_data_stream.h>
#include <bc_flood_detector.h>
#include <bc_pulse_counter.h>
#include <bc_co2_sensor.h>
#include <bc_font_common.h>
#include <bc_image.h>

//! @mainpage BigClown firmware SDK
//! This is API documentation of BigClown SDK

#endif // _BCL_H
