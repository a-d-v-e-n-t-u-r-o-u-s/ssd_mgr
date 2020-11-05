/*!
 * \file
 * \brief Seven segment display driver manager header file
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
#ifndef SSD_MGR_H
#define SSD_MGR_H

#include <stdint.h>
#include "gpio.h"

#define SSD_DIGIT_0         (0u)
#define SSD_DIGIT_1         (1u)
#define SSD_DIGIT_2         (2u)
#define SSD_DIGIT_3         (3u)
#define SSD_DIGIT_4         (4u)
#define SSD_DIGIT_5         (5u)
#define SSD_DIGIT_6         (6u)
#define SSD_DIGIT_7         (7u)
#define SSD_DIGIT_8         (8u)
#define SSD_DIGIT_9         (9u)
#define SSD_BLANK           (10u)
#define SSD_CHAR_h          (11u)
#define SSD_CHAR_C          (12u)
#define SSD_CHAR_F          (13u)
#define SSD_CHAR_DEGREE     (14u)
#define SSD_SYMBOL_MINUS    (15u)
#define SSD_SENTINEL        (16u)

typedef struct
{
    uint8_t const *config;
    uint8_t value;
} SSD_MGR_displays_t;

typedef struct
{
    uint8_t segments[7][2];
    bool is_segments_inverted;
    bool is_displays_inverted;
} SSD_MGR_config_t;

int8_t SSD_MGR_display_set(SSD_MGR_displays_t *display, uint8_t value);
int8_t SSD_MGR_display_create(SSD_MGR_displays_t *display,
        uint8_t const *config);
int8_t SSD_MGR_initialize(const SSD_MGR_config_t *config);
#endif
