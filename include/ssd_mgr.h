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

#define SSD_MGR_MAX_MULTIPLEXED_DISPLAYS        4U

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

typedef struct
{
    uint8_t seg_config[8][2];
    uint8_t (*disp_config)[2];
    uint8_t disp_config_size;
    bool is_segment_mode;
    bool is_disp_inverted_logic;
    bool is_segment_inverted_logic;
} SSD_MGR_config_t;

int8_t SSD_MGR_set(uint16_t value);
int8_t SSD_MGR_initialize(const SSD_MGR_config_t *config);
#endif
