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
#include "common.h"

#define SSD_MGR_MAX_MULTIPLEXED_DISPLAYS        4U

static uint8_t displays_counter;
static uint8_t display_no;
static SSD_MGR_config_t const *ssd_config;
static SSD_MGR_displays_t *displays[SSD_MGR_MAX_MULTIPLEXED_DISPLAYS];

/*! \todo make it really in FLASH memory */
static const uint8_t dig_data[SSD_SENTINEL] =
{
    [SSD_DIGIT_0]       = 0x3F,
    [SSD_DIGIT_1]       = 0x06,
    [SSD_DIGIT_2]       = 0x5B,
    [SSD_DIGIT_3]       = 0x4F,
    [SSD_DIGIT_4]       = 0x66,
    [SSD_DIGIT_5]       = 0x6D,
    [SSD_DIGIT_6]       = 0x7D,
    [SSD_DIGIT_7]       = 0x07,
    [SSD_DIGIT_8]       = 0x7F,
    [SSD_DIGIT_9]       = 0x6F,
    [SSD_BLANK]         = 0x00,
    [SSD_CHAR_h]        = 0x74,
    [SSD_CHAR_C]        = 0x39,
    [SSD_CHAR_F]        = 0x71,
    [SSD_CHAR_DEGREE]   = 0x63,
    [SSD_SYMBOL_MINUS]  = 0x40
};

static void clear(void)
{
    for(uint8_t i = 0u; i < displays_counter;  i++)
    {
        GPIO_write_pin(displays[i]->config[0],
                displays[i]->config[1],
                ssd_config->is_displays_inverted);
    }
}

static void set_segments(uint8_t segs)
{
    for(uint8_t i = 0u; i < ARRAY_2D_ROW(ssd_config->segments); i++)
    {
        const bool val = (((segs >> i ) & 0x01u) != 0u);
        GPIO_write_pin(ssd_config->segments[i][0],
                ssd_config->segments[i][1],
                ssd_config->is_segments_inverted ? !val : val);
    }
}

static void set_display(uint8_t const *config)
{
    GPIO_write_pin(config[0], config[1],
            !ssd_config->is_displays_inverted);
}


static void ssd_mgr_main(void)
{
    if(displays_counter != 0U)
    {
        SSD_MGR_displays_t const *display = displays[display_no];
        clear();
        set_segments(dig_data[display->value]);
        set_display(display->config);
        display_no++;
        display_no %= displays_counter;
    }
}

int8_t SSD_MGR_display_set(SSD_MGR_displays_t *display, uint8_t value)
{
    if(display == NULL)
    {
        return -1;
    }

    if(value >= SSD_SENTINEL)
    {
        return -1;
    }

    display->value = value;
    return 0;
}


int8_t SSD_MGR_display_create(SSD_MGR_displays_t *display,
        uint8_t const *config)
{
    if(display != NULL &&
        config != NULL &&
        displays_counter < ARRAY_SIZE(displays))
    {
        display->config = config;
        display->value = 0u;

        GPIO_config_pin(config[0], config[1], GPIO_OUTPUT_PUSH_PULL);
        displays[displays_counter] = display;
        displays_counter++;
        return 0;
    }

    return -1;
}

int8_t SSD_MGR_initialize(const SSD_MGR_config_t *config)
{
    if(config == NULL)
    {
        return -1;
    }

    if(SYSTEM_register_task(ssd_mgr_main, 5u) != 0)
    {
        return -1;
    }

    for(uint8_t i = 0; i < ARRAY_2D_ROW(config->segments); i++)
    {
        GPIO_config_pin(config->segments[i][0],
                config->segments[i][1], GPIO_OUTPUT_PUSH_PULL);
    }

    ssd_config = config;
    return 0;
}
