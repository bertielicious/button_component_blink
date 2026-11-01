/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Pseudo code

   START PROGRAM

DEFINE button GPIO as GPIO16
DEFINE button active level as LOW

DEFINE LED GPIO as GPIO18

CREATE global variable led_state = OFF

--------------------------------------------------
BUTTON CALLBACK FUNCTIONS
--------------------------------------------------

WHEN button is single-clicked
    Display "Button single click!"

WHEN button is double-clicked
    Display "Button double click!"

WHEN button is long-pressed
    Display "Button long press!"

WHEN button is repeatedly pressed
    Display "Button press repeat!"

--------------------------------------------------
LED FUNCTIONS
--------------------------------------------------

FUNCTION blink_led
    Set LED GPIO output equal to led_state
END FUNCTION

FUNCTION configure_led
    Display "Configured LED GPIO"
    Reset LED GPIO pin
    Configure LED GPIO as OUTPUT
END FUNCTION

--------------------------------------------------
MAIN PROGRAM
--------------------------------------------------

FUNCTION app_main

    Configure the LED

    Create button configuration:
        Long press time = 5000 ms
        Short press time = 200 ms

    Create GPIO button configuration:
        GPIO number = 16
        Active level = LOW
        Pull resistor enabled

    Create button device

    Register callback for:
        Single click
        Double click
        Long press release
        Repeated press

    REPEAT FOREVER

        IF led_state is ON
            Display "Turning LED ON"
        ELSE
            Display "Turning LED OFF"
        END IF

        Call blink_led

        Toggle led_state
            ON becomes OFF
            OFF becomes ON

        Wait for CONFIG_BLINK_PERIOD milliseconds

    END REPEAT

END FUNCTION

END PROGRAM


Start
  |
  v
Configure LED GPIO
  |
  v
Create button configuration
  |
  v
Create button device
  |
  v
Register button event callbacks
  |
  v
+-----------------------+
| Infinite Loop         |
+-----------------------+
          |
          v
    Display LED state
          |
          v
      Update LED
          |
          v
    Toggle LED state
          |
          v
       Delay
          |
          +------> Back to start of loop

Button Events (occur asynchronously):
    Single Click  --> Print message
    Double Click  --> Print message
    Long Press    --> Print message
    Repeat Press  --> Print message
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "iot_button.h"
#include "button_gpio.h"

#include "sdkconfig.h"
// Define button GPIO and active level
#define BUTTON_IO_NUM           16 // GPIO number for the button input
#define BUTTON_ACTIVE_LEVEL     0 // Active level for the button (0 for active low, 1 for active high)

static const char *TAG = "BUTTON";

//static const char *TAG = "example";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO 18

static uint8_t s_led_state = 0;

#ifdef CONFIG_BLINK_LED_STRIP//??

#elif CONFIG_BLINK_LED_GPIO//??

// Callback functions for button events
static void button_single_click_event_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Button single click!");
}

static void button_double_click_event_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Button double click!");
}

static void button_long_press_event_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Button long press!");
}

static void button_repeat_event_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Button press repeat!");
}

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

#else
#error "unsupported LED type"
#endif

void app_main(void)
{

    /* Configure the peripheral according to the LED type */
    configure_led();

    // Create button configurations
    const button_config_t btn_cfg = {
        .long_press_time = 5000,      // Long press time in milliseconds
        .short_press_time = 200,      // Short press time in milliseconds
    };

        const button_gpio_config_t btn_gpio_cfg = {
        .gpio_num = BUTTON_IO_NUM,
        .active_level = BUTTON_ACTIVE_LEVEL,
        .disable_pull = false,
    };

        // Button handle
    button_handle_t btn;
    // Create a new button device
    esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &btn);

    // Register callback for button press
    ret = iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, NULL, button_single_click_event_cb, NULL);
    ESP_ERROR_CHECK(ret);
    ret = iot_button_register_cb(btn, BUTTON_DOUBLE_CLICK, NULL, button_double_click_event_cb, NULL);
    ESP_ERROR_CHECK(ret);
    ret = iot_button_register_cb(btn, BUTTON_LONG_PRESS_UP, NULL, button_long_press_event_cb, NULL);
    ESP_ERROR_CHECK(ret);
    ret = iot_button_register_cb(btn, BUTTON_PRESS_REPEAT, NULL, button_repeat_event_cb, NULL);
    ESP_ERROR_CHECK(ret);   

    while (1) {
        ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        blink_led();
        /* Toggle the LED state */
        s_led_state = !s_led_state;
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}
