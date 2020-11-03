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

typedef struct
{
    SSD_MGR_config_t config;
    uint16_t value;
} SSD_MGR_module_t;

static SSD_MGR_module_t ssd_mgr;

static uint8_t segments[8][2];
static uint8_t displays[SSD_MGR_MAX_MULTIPLEXED_DISPLAYS][2];
static uint8_t displays_size;
static uint8_t display_no;
static bool is_segment_mode;
static bool is_disp_inverted_logic;
static bool is_segment_inverted_logic;
static bool is_turned_on;

/*! \todo make it really in FLASH memory */
static const uint8_t dig_data[10] =
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
};

static inline void clear(const uint8_t (*gpio)[2], uint8_t size,
        bool is_inverted_logic)
{
    bool val = is_inverted_logic ? true : false;

    for(uint8_t i = 0u; i < size;  i++)
    {
        GPIO_write_pin(gpio[i][0], gpio[i][1], val);
    }
}

static void light_segments(const uint8_t (*gpio)[2], uint8_t segs,
        bool is_inverted_logic)
{
    for(uint8_t i = 0u; i < 7u; i++)
    {
        const bool val = (((segs >> i ) & 0x01u) != 0u);
        GPIO_write_pin(gpio[i][0], gpio[i][1],
                is_inverted_logic ? !val : val);
    }
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

    clear(displays, displays_size, is_disp_inverted_logic);

    uint8_t digit = get_digit(value, display_no);

    light_segments(segments, dig_data[digit], is_segment_inverted_logic);

    bool val = is_disp_inverted_logic ? false : true;
    GPIO_write_pin(displays[display_no][0], displays[display_no][1], val);

    display_no++;
    display_no %= displays_size;
}

static void ssd_mgr_main(void)
{
    if(is_turned_on)
    {
        uint16_t value = ssd_mgr.value;

        if(ssd_mgr.config.is_segment_mode)
        {
            //multiplex_in_segment_mode(value);
        }
        else
        {
            multiplex_in_digit_mode(value);
        }
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
        clear(displays, displays_size, is_disp_inverted_logic);
        display_no = 0u;
    }
    else
    {
        is_turned_on = true;
        ssd_mgr.value = value;
    }

    return 0;
}

int8_t SSD_MGR_initialize(const SSD_MGR_config_t *config)
{
    if(config == NULL)
    {
        return -1;
    }

    uint8_t interval = config->is_segment_mode ? 0u : 5u;

    if(SYSTEM_register_task(ssd_mgr_main, interval) != 0)
    {
        return -1;
    }

    if(config->disp_config_size == 0)
    {
        return -1;
    }

    if(config->disp_config_size > SSD_MGR_MAX_MULTIPLEXED_DISPLAYS)
    {
        return -1;
    }

    memcpy(segments, config->seg_config, sizeof(config->seg_config));
    memcpy(displays, config->disp_config, 2*config->disp_config_size);
    displays_size = config->disp_config_size;
    is_segment_mode = config->is_segment_mode;
    is_disp_inverted_logic = config->is_disp_inverted_logic;
    is_segment_inverted_logic = config->is_segment_inverted_logic;
    is_turned_on = true;

    for(uint8_t i = 0; i < 8u; i++)
    {
        GPIO_config_pin(segments[i][0], segments[i][1], GPIO_OUTPUT_PUSH_PULL);
    }

    for(uint8_t i = 0; i < 4u; i++)
    {
        GPIO_config_pin(displays[i][0], displays[i][1], GPIO_OUTPUT_PUSH_PULL);
    }

    clear(displays, displays_size, is_disp_inverted_logic);

    return 0;
}
