/** OLED Screen Info
 *  This file contains OLED screen SSD1351-related defines and macros.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of NXP, nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * visit: http://www.mikroe.com and http://www.nxp.com
 *
 * get support at: http://www.mikroe.com/forum and https://community.nxp.com
 *
 * Project HEXIWEAR, 2015
 * Rewrite by Lorenzo Calisti, 2022
 */

#ifndef OLED_INFO_H_
#define OLED_INFO_H_

// command byte number
#define CMD_BYTE (1)
#define DATA_BYTE (0)

// OLED size
#define OLED_SCREEN_WIDTH (96)
#define OLED_SCREEN_HEIGHT (96)
#define OLED_BYTES_PER_PIXEL (2)

// OLED offset
#define OLED_COLUMN_OFFSET (16)
#define OLED_ROW_OFFSET (0)

#define OLED_TRANSITION_STEP (1)

// text stuff
#define OLED_CHAR_WIDTH_AUTO (0xFF)
#define OLED_CHAR_WIDTH_MAX (0xFE)
#define OLED_CHAR_WIDTH_NUM (0xFD)
#define OLED_CHAR_WIDTH_KEEP (0xFC)
#define OLED_CHAR_HEIGHT_AUTO (0xFB)
#define OLED_CHAR_HEIGHT_KEEP (0xFA)
#define OLED_TEXT_HALIGN_SHIFT (0)
#define OLED_TEXT_HALIGN_MASK (0x03 << OLED_TEXT_HALIGN_SHIFT)
#define OLED_TEXT_VALIGN_SHIFT (4)
#define OLED_TEXT_VALIGN_MASK (0x03 << OLED_TEXT_VALIGN_SHIFT)

// OLED commands
#define OLED_CMD_SET_COLUMN (0x15)
#define OLED_CMD_SET_ROW (0x75)
#define OLED_CMD_WRITERAM (0x5C)
#define OLED_CMD_READRAM (0x5D)
#define OLED_CMD_SET_REMAP (0xA0)
#define OLED_CMD_DISPLAYOFFSET (0xA2)
#define OLED_CMD_STARTLINE (0xA1)
#define OLED_CMD_SET_DISPLAY_MODE_ALL_OFF (0xA4)
#define OLED_CMD_SET_DISPLAY_MODE_ALL_ON (0xA5)
#define OLED_CMD_SET_DISPLAY_MODE_NORMAL (0xA6)
#define OLED_CMD_SET_DISPLAY_MODE_INVERSE (0xA7)
#define OLED_CMD_FUNCTIONSELECT (0xAB)
#define OLED_CMD_SET_SLEEP_MODE_ON (0xAE)
#define OLED_CMD_SET_SLEEP_MODE_OFF (0xAF)
#define OLED_CMD_SET_RESET_PRECHARGE (0xB1)
#define OLED_CMD_DISPLAYENHANCE (0xB2)
#define OLED_CMD_SET_OSC_FREQ_AND_CLOCKDIV (0xB3)
#define OLED_CMD_SETVSL (0xB4)
#define OLED_CMD_SETGPIO (0xB5)
#define OLED_CMD_PRECHARGE2 (0xB6)
#define OLED_CMD_SETGRAY (0xB8)
#define OLED_CMD_USELUT (0xB9)
#define OLED_CMD_PRECHARGELEVEL (0xBB)
#define OLED_CMD_VCOMH (0xBE)
#define OLED_CMD_CONTRASTABC (0xC1)
#define OLED_CMD_CONTRASTMASTER (0xC7)
#define OLED_CMD_SET_MUX_RATIO (0xCA)
#define OLED_CMD_NOP (0xD1)
#define OLED_CMD_SET_CMD_LOCK (0xFD)
#define OLED_CMD_HORIZSCROLL (0x96)
#define OLED_CMD_STOPSCROLL (0x9E)
#define OLED_CMD_STARTSCROLL (0x9F)

// remap settings
#define REMAP_HORIZONTAL_INCREMENT (0)
#define REMAP_VERTICAL_INCREMENT (1)
#define REMAP_COLUMNS_LEFT_TO_RIGHT (0)
#define REMAP_COLUMNS_RIGHT_TO_LEFT (2)
#define REMAP_ORDER_ABC (0)
#define REMAP_ORDER_CBA (4)
#define REMAP_SCAN_UP_TO_DOWN (0)
#define REMAP_SCAN_DOWN_TO_UP (0x10)
#define REMAP_COM_SPLIT_ODD_EVEN_DIS (0)
#define REMAP_COM_SPLIT_ODD_EVEN_EN (0x20)
#define REMAP_COLOR_RGB565 (0x40)
#define OLED_COLOR_DEPTH_262K (0x80)
#define OLED_COLOR_DEPTH_262K_16BIT (0xC0)
#define OLED_REMAP_SETTINGS (REMAP_ORDER_ABC | REMAP_COM_SPLIT_ODD_EVEN_EN | REMAP_COLOR_RGB565 | REMAP_COLUMNS_LEFT_TO_RIGHT | REMAP_SCAN_UP_TO_DOWN | REMAP_HORIZONTAL_INCREMENT)

// lock settings
#define OLED_UNLOCK (0x12)
#define OLED_LOCK (0x16)
#define OLED_ACC_TO_CMD_NO (0xB0)
#define OLED_ACC_TO_CMD_YES (0xB1)

// macros

// check if given coordinates are valid
#define check_coord(x, y, w, h)                     \
    (x >= 0) && (x + w - 1 <= OLED_SCREEN_WIDTH) && \
        (y >= 0) && (y + h - 1 <= OLED_SCREEN_HEIGHT)

// swap a color in little endian form
#define swap_color(p) ((p & 0xFF00) >> 8) | ((p & 0x00FF) << 8)

#endif // OLED_INFO_H_
