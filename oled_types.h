/** OLED Types
 *  This file contains OLED-related data structures.
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

#ifndef OLED_TYPES_H_
#define OLED_TYPES_H_

#include <stdint.h>

namespace oled
{
  // Represent all possible transitions
  enum class Transition
  {
    NONE,
    TOP_DOWN,
    DOWN_TOP,
    LEFT_RIGHT,
    RIGHT_LEFT
  };

  // Represent all possible status
  enum class Status
  {
    SUCCESS,        // success
    ERROR,          // fail
    PROTOCOL_ERROR, // SPI failure
    INIT_ERROR,     // initialization error
    DEINIT_ERROR,   // deinitialization error
    COORD_ERROR,    // invalid coordinates
    AREA_NOT_SET    // using dynamic area w/out setting it
  };

  // Redefine the type of a single pixel
  typedef uint16_t pixel_t;
  // TODO: remove this
  typedef uint16_t *oled_pixel_t;

  // Represent a dynamic area of the OLED
  // with an associated memory buffer
  struct DynamicArea
  {
    uint8_t xCrd;
    uint8_t yCrd;
    uint8_t width;
    uint8_t height;
    pixel_t *buffer;
  };

  // Represent the most used colors
  enum Color
  {
    BLACK = 0x0000,
    BLUE_1 = 0x06FF,
    BLUE = 0x001F,
    RED = 0xF800,
    GREEN = 0x07E0,
    CYAN = 0x07FF,
    MAGENTA = 0xF81F,
    YELLOW = 0xFFE0,
    GRAY = 0x528A,
    WHITE = 0xFFFF
  };

  // Represent all possible text alignments
  typedef uint8_t TextAlign;

#define TEXT_ALIGN_LEFT 0x1
#define TEXT_ALIGN_CENTER 0x2
#define TEXT_ALIGN_RIGHT 0x3
#define TEXT_ALIGN_TOP 0x10
#define TEXT_ALIGN_VCENTER 0x20
#define TEXT_ALIGN_BOTTOM 0x30

  // Represent all properties assignable to the text
  // displayed by the OLED
  struct TextProperties
  {
    const uint8_t *font;
    Color fontColor;
    TextAlign alignParam;
    pixel_t *bgImage;
  };

  // Represent a command sent to the OLED
  struct Command
  {
    uint32_t cmd;
    uint8_t type;
  };
} // namespace oled

#endif // OLED_TYPES_H_