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
#define DEBUG_APP_ID    "SSDM"
#define DEBUG_ENABLED   DEBUG_SSDM_ENABLED
#define DEBUG_LEVEL     DEBUG_SSDM_LEVEL

#include "ssd_mgr.h"
#include "hardware.h"
#include <stddef.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <assert.h>
#include "system.h"
#include "debug.h"
#include "common.h"

#define SSD_MGR_MAX_MULTIPLEXED_DISPLAYS        4U
#define SEGMENTS_AMOUNT                         (7u)

STATIC_ASSERT((F_CPU == 16000000UL), F_CPU_not_supported);

static uint8_t displays_counter;
static uint8_t display_no;
static SSD_MGR_displays_t *displays[SSD_MGR_MAX_MULTIPLEXED_DISPLAYS];

static const uint8_t dig_data[SSD_SENTINEL] PROGMEM =
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
    [SSD_CHAR_E]        = 0x79,
    [SSD_CHAR_F]        = 0x71,
    [SSD_CHAR_r]        = 0x50,
    [SSD_CHAR_DEGREE]   = 0x63,
    [SSD_SYMBOL_MINUS]  = 0x40
};

static void clear(void)
{
    const bool is_displays_inverted = pgm_read_byte(&ssd_config.is_displays_inverted);

    for(uint8_t i = 0u; i < displays_counter;  i++)
    {
        GPIO_write_pin(displays[i]->config, is_displays_inverted);
    }
}

static void set_segments(uint8_t segs)
{
    const bool is_segments_inverted  = pgm_read_byte(&ssd_config.is_segments_inverted);

    for(uint8_t i = 0u; i < SEGMENTS_AMOUNT; i++)
    {
        const bool val = ((((uint8_t)(segs >> i )) & 0x01u) != 0u);

        uint8_t id = GPIO_CHANNEL_SEGMENTA;
        switch(i)
        {
            case 0: id = GPIO_CHANNEL_SEGMENTA ; break;
            case 1: id = GPIO_CHANNEL_SEGMENTB ; break;
            case 2: id = GPIO_CHANNEL_SEGMENTC ; break;
            case 3: id = GPIO_CHANNEL_SEGMENTD ; break;
            case 4: id = GPIO_CHANNEL_SEGMENTE ; break;
            case 5: id = GPIO_CHANNEL_SEGMENTF ; break;
            case 6: id = GPIO_CHANNEL_SEGMENTG ; break;
            default:
                ASSERT(false);
                break;
        }

        GPIO_write_pin(id, is_segments_inverted ? !val : val);
    }
}

static void set_display(uint8_t config)
{
    const bool is_displays_inverted = pgm_read_byte(&ssd_config.is_displays_inverted);

    GPIO_write_pin(config, !is_displays_inverted);
}

void SSD_MGR_display_set(SSD_MGR_displays_t *display, uint8_t value)
{
    ASSERT(display != NULL);

    if(value < SSD_SENTINEL)
    {
        display->value = value;
    }
}


void SSD_MGR_display_create(SSD_MGR_displays_t *display,
        uint8_t config)
{
    ASSERT(display != NULL);
    ASSERT(displays_counter < ARRAY_SIZE(displays));

    display->config = config;
    display->value = 0u;

    displays[displays_counter] = display;
    displays_counter++;
}

ISR(TIMER2_OVF_vect)
{
    if(displays_counter != 0U)
    {
        SSD_MGR_displays_t const *display = displays[display_no];
        clear();
        set_segments(pgm_read_byte(&dig_data[display->value]));
        set_display(display->config);
        display_no++;
        display_no %= displays_counter;
    }
}

ISR(TIMER2_COMP_vect)
{
    clear();
}

void SSD_MGR_initialize(void)
{
    TCCR2 = (3u << 1u);
    TIMSK |= (3u << 6u);
    OCR2 = UINT8_MAX;
}
