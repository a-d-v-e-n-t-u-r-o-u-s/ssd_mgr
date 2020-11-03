/*!
 * \file
 * \brief Seven segment display driver manager implementation file
 * \author Dawid Babula
 * \email dbabula@adventurous.pl

 *
 * \par Copyright (C) Dawid Babula, 2019
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "ssd_mgr.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <assert.h>
#include "system.h"
#include "debug.h"

uint16_t ssd_value;
static SSD_MGR_config_t const *ssd_config;
static uint8_t display_no;
static bool is_turned_on;

/*! \todo make it really in FLASH memory */
static const uint8_t dig_data[SSD_SENTINEL] =
{
    [SSD_DIGIT_0] = 0x3F,
    [SSD_DIGIT_1] = 0x06,
    [SSD_DIGIT_2] = 0x5B,
    [SSD_DIGIT_3] = 0x4F,
    [SSD_DIGIT_4] = 0x66,
    [SSD_DIGIT_5] = 0x6D,
    [SSD_DIGIT_6] = 0x7D,
    [SSD_DIGIT_7] = 0x07,
    [SSD_DIGIT_8] = 0x7F,
    [SSD_DIGIT_9] = 0x6F,
    [SSD_BLANK]   = 0x00,
};

static void clear(void)
{
    for(uint8_t i = 0u; i < ssd_config->displays_amount;  i++)
    {
        GPIO_write_pin(ssd_config->displays[i][0],
                ssd_config->displays[i][1],
                ssd_config->is_displays_inverted);
    }
}

static void set_segments(uint8_t segs)
{
    for(uint8_t i = 0u; i < 7u; i++)
    {
        const bool val = (((segs >> i ) & 0x01u) != 0u);
        GPIO_write_pin(ssd_config->segments[i][0],
                ssd_config->segments[i][1],
                ssd_config->is_segments_inverted ? !val : val);
    }
}

static void set_display(uint8_t no)
{
    GPIO_write_pin(ssd_config->displays[no][0],
            ssd_config->displays[no][1],
            !ssd_config->is_displays_inverted);
}

static inline uint8_t get_digit(uint16_t value, uint8_t position)
{
     switch(position)
     {
         case 0:
             return value%10u;
         case 1:
             return (value/10u)%10u;
         case 2:
             return (value/100u)%10u;
         case 3:
             return (value/1000u)%10u;
     }

     return 0U;
}

static void multiplex_in_digit_mode(uint16_t value)
{
    uint8_t digit = get_digit(value, display_no);

    clear();
    set_segments(dig_data[digit]);
    set_display(display_no);

    display_no++;
    display_no %= ssd_config->displays_amount;
}

static void ssd_mgr_main(void)
{
    if(is_turned_on)
    {
        multiplex_in_digit_mode(ssd_value);
    }
}

int8_t SSD_MGR_set(uint16_t value)
{
    if((value != UINT16_MAX) && (value > 9999U))
    {
        return -1;
    }

    if(value == UINT16_MAX)
    {
        is_turned_on = false;
        clear();
        display_no = 0u;
    }
    else
    {
        is_turned_on = true;
        ssd_value = value;
    }

    return 0;
}

int8_t SSD_MGR_initialize(const SSD_MGR_config_t *config)
{
    if(config == NULL)
    {
        return -1;
    }

    if(config->displays_amount == 0)
    {
        return -1;
    }

    if(config->displays_amount > SSD_MGR_MAX_MULTIPLEXED_DISPLAYS)
    {
        return -1;
    }

    if(SYSTEM_register_task(ssd_mgr_main, 5u) != 0)
    {
        return -1;
    }

    is_turned_on = true;
    ssd_config = config;

    for(uint8_t i = 0; i < 8u; i++)
    {
        GPIO_config_pin(config->segments[i][0],
                config->segments[i][1], GPIO_OUTPUT_PUSH_PULL);
    }

    for(uint8_t i = 0; i < 4u; i++)
    {
        GPIO_config_pin(config->displays[i][0],
                config->displays[i][1], GPIO_OUTPUT_PUSH_PULL);
    }

    clear();

    return 0;
}
