/*
 * SPDX-FileCopyrightText: 2021-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* ULP-RISC-V example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   This code runs on ULP-RISC-V  coprocessor
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "ulp_riscv.h"
#include "ulp_riscv_utils.h"
#include "ulp_riscv_gpio.h"

#define EXAMPLE_WS2812B_GPIO GPIO_NUM_4 // demo values
#define LED_NUM 6 // demo values

typedef enum {
    GPIO_NOT_INITIATED,
    GPIO_INITIATED,
} gpio_state_t;

gpio_state_t state = GPIO_NOT_INITIATED;
uint8_t led_buffer[LED_NUM * 3]; // GBR


// void ws2812b_write_bit_test_on(bool bit){
//     REG_SET_FIELD(RTC_GPIO_ENABLE_W1TS_REG, RTC_GPIO_ENABLE_W1TS, BIT(EXAMPLE_WS2812B_GPIO));
// }

// void ws2812b_write_bit_test_off(bool bit){
//     REG_SET_FIELD(RTC_GPIO_ENABLE_W1TC_REG, RTC_GPIO_ENABLE_W1TC, BIT(EXAMPLE_WS2812B_GPIO));
// }


void ws2812b_write_bit(bool bit)
{

    asm volatile inline("lui	a3,0x4\n\t"); // GPIO 4
    asm volatile inline("lui	a1,0xa\n\t"   // prep high
                        "add	a1,a1,1040\n\t"
                        "lw     a2,0(a1)\n\t"
                        "and	a2,a2,1023\n\t"
                        "or	    a2,a2,a3\n\t");
    asm volatile inline("lui	a4,0xa\n\t"  // // prep low
                        "add	a4,a4,1044\n\t"
                        "lw     a5,0(a4)\n\t"
                        "and	a5,a5,1023\n\t"
                        "or	    a5,a5,a3\n\t");

    if (bit) { //High: long high, short low
        // time critical section
        asm volatile inline("sw	    a2,0(a1)\n\t"  // GPIO high
                            "sw	    a5,0(a4)\n\t"); // GPIO low
        // end of time critical section

                            //         "nop            \n\t"  // long 
                            // "nop            \n\t"
                            // "nop            \n\t"
                            // "nop            \n\t"
                            // "nop            \n\t"
                            // "nop            \n\t"
                            // "nop            \n\t"
                            // "nop            \n\t"
                            // "nop            \n\t"
                            // "nop            \n\t"
    }
    else{ //Low: short high, long low
        // time critical section
        asm volatile inline("sw	    a2,0(a1)\n\t"   // GPIO high
                            "sw	    a5,0(a4)\n\t"); // GPIO low
        // end of time critical section
    }
    // the remaining "low" period must be shorter than 5 microseconds, and must be at least 900 nanoseconds
}

void ws2812b_write_byte(uint8_t data)
{
    for (int i = 0; i < 8; i++) {
        ws2812b_write_bit((data >> i) & 0x1);
        // ws2812b_write_bit_test_on((data >> i) & 0x1);
        // ws2812b_write_bit_test_off((data >> i) & 0x1);
    }
}

void ws2812b_write_buffer()
{
    for (int i = 0; i < (LED_NUM * 3); i++) {
        ws2812b_write_byte(led_buffer[i]);
    }
}

int main (void)
{

    switch (state) {
    case GPIO_NOT_INITIATED:
        /* Setup GPIO used for WS2812 */
        ulp_riscv_gpio_init(EXAMPLE_WS2812B_GPIO);
        ulp_riscv_gpio_output_enable(EXAMPLE_WS2812B_GPIO);
        ulp_riscv_gpio_set_output_mode(EXAMPLE_WS2812B_GPIO, RTCIO_MODE_OUTPUT_OD);
        ulp_riscv_gpio_pullup(EXAMPLE_WS2812B_GPIO);
        ulp_riscv_gpio_pulldown_disable(EXAMPLE_WS2812B_GPIO);

        ws2812b_write_buffer();
        state = GPIO_INITIATED;
        break;

    case GPIO_INITIATED:
        ws2812b_write_buffer();

        break;
    }

    /* ulp_riscv_halt() is called automatically when main exits,
       main will be executed again at the next timeout period,
       according to ulp_set_wakeup_period()
     */
    return 0;
}

