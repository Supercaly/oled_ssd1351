/** OLED Display Driver for Hexiwear
 *  This file contains OLED driver functionality for drawing images and text
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

#include "oled_SSD1351.h"
#include "font/opensans_font.h"

namespace oled
{
  const Command seq[] = {
      OLED_CMD_SET_CMD_LOCK, CMD_BYTE,
      OLED_UNLOCK, DATA_BYTE,
      OLED_CMD_SET_CMD_LOCK, CMD_BYTE,
      OLED_ACC_TO_CMD_YES, DATA_BYTE,
      OLED_CMD_SET_SLEEP_MODE_ON, CMD_BYTE,
      OLED_CMD_SET_OSC_FREQ_AND_CLOCKDIV, CMD_BYTE,
      0xF1, DATA_BYTE,
      OLED_CMD_SET_MUX_RATIO, CMD_BYTE,
      0x5F, DATA_BYTE,
      OLED_CMD_SET_REMAP, CMD_BYTE,
      OLED_REMAP_SETTINGS, DATA_BYTE,
      OLED_CMD_SET_COLUMN, CMD_BYTE,
      0x00, DATA_BYTE,
      0x5F, DATA_BYTE,
      OLED_CMD_SET_ROW, CMD_BYTE,
      0x00, DATA_BYTE,
      0x5F, DATA_BYTE,
      OLED_CMD_STARTLINE, CMD_BYTE,
      0x80, DATA_BYTE,
      OLED_CMD_DISPLAYOFFSET, CMD_BYTE,
      0x60, DATA_BYTE,
      OLED_CMD_SET_RESET_PRECHARGE, CMD_BYTE,
      0x32, CMD_BYTE,
      OLED_CMD_VCOMH, CMD_BYTE,
      0x05, CMD_BYTE,
      OLED_CMD_SET_DISPLAY_MODE_NORMAL, CMD_BYTE,
      OLED_CMD_CONTRASTABC, CMD_BYTE,
      0x8A, DATA_BYTE,
      0x51, DATA_BYTE,
      0x8A, DATA_BYTE,
      OLED_CMD_CONTRASTMASTER, CMD_BYTE,
      0xCF, DATA_BYTE,
      OLED_CMD_SETVSL, CMD_BYTE,
      0xA0, DATA_BYTE,
      0xB5, DATA_BYTE,
      0x55, DATA_BYTE,
      OLED_CMD_PRECHARGE2, CMD_BYTE,
      0x01, DATA_BYTE,
      OLED_CMD_SET_SLEEP_MODE_OFF, CMD_BYTE};

  SSD1351::SSD1351(PinName mosiPin, PinName sclkPin,
                   PinName pwrPin, PinName csPin,
                   PinName rstPin, PinName dcPin) : _spi(mosiPin, NC, sclkPin),
                                                    _power(pwrPin),
                                                    _cs(csPin),
                                                    _rst(rstPin),
                                                    _dc(dcPin)
  {
    _init = false;

    _spi.frequency(8000000);

    _dc = 0;
    power_off();
    ThisThread::sleep_for(1ms);
    _rst = 0;
    ThisThread::sleep_for(1ms);
    _rst = 1;
    ThisThread::sleep_for(1ms);
    power_on();

    // reset text prop
    _text_properties.alignParam = TEXT_ALIGN_LEFT | TEXT_ALIGN_TOP;
    _text_properties.bgImage = NULL;
    _text_properties.font = OpenSans_15_Regular;
    _text_properties.fontColor = Color::WHITE;
    set_text_properties(&_text_properties);

    // reset dynamic area
    _dynamic_area.xCrd = 0;
    _dynamic_area.yCrd = 0;
    _dynamic_area.width = OLED_SCREEN_WIDTH;
    _dynamic_area.height = OLED_SCREEN_HEIGHT;
    _area_buffer = (pixel_t *)malloc(OLED_SCREEN_WIDTH * OLED_SCREEN_HEIGHT * sizeof(pixel_t));

    // send init commands to OLED
    for (int i = 0; i < 39; i++)
    {
      send_cmd(seq[i]);
    }
  }

  SSD1351::~SSD1351(void)
  {
    free(_area_buffer);
  }

  void SSD1351::dim_screen_on()
  {
    for (int i = 0; i < 16; i++)
    {
      send_cmd({OLED_CMD_CONTRASTMASTER, CMD_BYTE});
      send_cmd({(uint32_t)(0xC0 | (0xF - i)), DATA_BYTE});
      ThisThread::sleep_for(20ms);
    }
  }

  void SSD1351::dim_screen_off()
  {
    send_cmd({OLED_CMD_CONTRASTMASTER, CMD_BYTE});
    send_cmd({0xC0 | 0xF, DATA_BYTE});
  }

  void SSD1351::power_on()
  {
    _power = 1;
  }

  void SSD1351::power_off()
  {
    _power = 0;
  }

  Status SSD1351::set_dynamic_area(DynamicArea area)
  {
    // check if given area is valid
    if (!check_coord(area.xCrd, area.yCrd, area.width, area.height))
    {
      return Status::COORD_ERROR;
    }

    if (!_init)
    {
      _init = true;
      set_buffer_border(0, 0, OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT);
    }

    // set the coordinates and border
    _dynamic_area = area;

    return Status::SUCCESS;
  }

  Status SSD1351::fill_screen(Color color)
  {
    DynamicArea area = {
        .xCrd = 0,
        .yCrd = 0,
        .width = OLED_SCREEN_WIDTH,
        .height = OLED_SCREEN_HEIGHT};
    Status status = set_dynamic_area(area);
    if (status != Status::SUCCESS)
    {
      return status;
    }

    uint16_t swappedColor = swap_color(color);
    for (size_t i = 0; i < (_dynamic_area.width * _dynamic_area.height); i++)
    {
      _area_buffer[i] = swappedColor;
    }

    draw_screen_buffer();

    return Status::SUCCESS;
  }

  Status SSD1351::draw_image(const uint8_t *image)
  {
    if (_area_buffer == NULL)
    {
      return Status::AREA_NOT_SET;
    }

    update_screen_buffer((pixel_t *)image);
    draw_screen_buffer();

    return Status::SUCCESS;
  }

  Status SSD1351::draw_screen(const uint8_t *image, Transition transition)
  {
    DynamicArea area = {
        .xCrd = 0,
        .yCrd = 0,
        .width = OLED_SCREEN_WIDTH,
        .height = OLED_SCREEN_HEIGHT};
    Status status = set_dynamic_area(area);
    if (status != Status::SUCCESS)
    {
      return status;
    }

    memcpy(_area_buffer, (pixel_t *)image, OLED_SCREEN_WIDTH * OLED_SCREEN_HEIGHT * sizeof(pixel_t));

    switch (transition)
    {
    case Transition::NONE:
    {
      draw_screen_buffer();
      break;
    }
    case Transition::TOP_DOWN:
    {
      draw_screen_top_down();
      break;
    }
    case Transition::DOWN_TOP:
    {
      draw_screen_down_top();
      break;
    }
    case Transition::LEFT_RIGHT:
    {
      draw_screen_left_right();
      break;
    }
    case Transition::RIGHT_LEFT:
    {
      draw_screen_right_left();
      break;
    }
    }

    return Status::SUCCESS;
  }

  Status SSD1351::draw_box(Color color)
  {
    if (_area_buffer == NULL)
    {
      return Status::AREA_NOT_SET;
    }

    pixel_t *buff = (pixel_t *)malloc(sizeof(pixel_t) * _dynamic_area.width * _dynamic_area.height);
    uint16_t swappedColor = swap_color(color);
    for (size_t y = 0; y < _dynamic_area.width * _dynamic_area.height; ++y)
    {
      buff[y] = swappedColor;
    }

    update_screen_buffer(buff);
    free(buff);
    draw_screen_buffer();

    return Status::SUCCESS;
  }

  Status SSD1351::draw_pixel(uint8_t x, uint8_t y, Color color)
  {
    DynamicArea area = {
        .xCrd = x,
        .yCrd = y,
        .width = 1,
        .height = 1};
    Status status = set_dynamic_area(area);
    if (status != Status::SUCCESS)
    {
      return status;
    }

    pixel_t *pixelBuff = (pixel_t *)malloc(sizeof(pixel_t));
    pixelBuff[0] = swap_color(color);
    update_screen_buffer(pixelBuff);
    free(pixelBuff);
    draw_screen_buffer();

    return Status::SUCCESS;
  }

  Status SSD1351::text_box(const char *text)
  {
    if (text == NULL)
    {
      return Status::ERROR;
    }

    return draw_text(text);
  }

  Status SSD1351::label(const char *text, uint8_t x, uint8_t y)
  {
    if (text == NULL)
    {
      return Status::ERROR;
    }

    DynamicArea txtArea = {
        .xCrd = x,
        .yCrd = y,
        .width = get_text_width(text),
        .height = (uint8_t)selectedFont_height};
    Status status = set_dynamic_area(txtArea);
    if (status != Status::SUCCESS)
    {
      return status;
    }

    status = draw_text(text);

    return status;
  }

  uint8_t SSD1351::get_text_width(const char *text)
  {
    uint8_t chrCnt = 0;
    uint8_t text_width = 0;

    while (text[chrCnt] != 0)
    {
      text_width += *(_text_properties.font + 8 + (uint16_t)((text[chrCnt++] - selectedFont_firstChar) << 2));
      //  make 1px space between chars
      text_width++;
    }
    // remove the final space
    text_width--;
    return text_width;
  }

  void SSD1351::get_text_properties(TextProperties *prop)
  {
    prop->font = _text_properties.font;
    prop->fontColor = _text_properties.fontColor;
    prop->alignParam = _text_properties.alignParam;
    prop->bgImage = _text_properties.bgImage;
  }

  void SSD1351::set_text_properties(TextProperties *prop)
  {
    _text_properties.font = prop->font;
    _text_properties.fontColor = prop->fontColor;
    _text_properties.alignParam = prop->alignParam;
    _text_properties.bgImage = prop->bgImage;

    selectedFont_firstChar = prop->font[2] | ((uint16_t)prop->font[3] << 8);
    selectedFont_lastChar = prop->font[4] | ((uint16_t)prop->font[5] << 8);
    selectedFont_height = prop->font[6];
  }

  /////////////////////
  // private methods //
  /////////////////////

  void SSD1351::send_cmd(Command command)
  {
    uint8_t txBuf[4];

    memcpy(txBuf, (void *)&command.cmd, 1);
    _dc = command.type ? 0 : 1;
    _cs = 0;
    _spi.write(*txBuf);
    _cs = 1;
  }

  void SSD1351::send_data(const uint8_t *dataToSend, uint32_t dataSize)
  {
    send_cmd({OLED_CMD_WRITERAM, CMD_BYTE});

    /* sending data -> set DC pin */
    _dc = 1;
    _cs = 0;

    const uint8_t *bufPtr = dataToSend;
    for (uint32_t i = 0; i < dataSize; i++)
    {
      _spi.write(*bufPtr);
      bufPtr += 1;
    }

    _cs = 1;
  }

  void SSD1351::set_buffer_border(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
  {
    send_cmd({OLED_CMD_SET_COLUMN, CMD_BYTE});
    send_cmd({(uint32_t)x + OLED_COLUMN_OFFSET, DATA_BYTE});
    send_cmd({(uint32_t)x + OLED_COLUMN_OFFSET + w - 1, DATA_BYTE});
    send_cmd({OLED_CMD_SET_ROW, CMD_BYTE});
    send_cmd({(uint32_t)y + OLED_ROW_OFFSET, DATA_BYTE});
    send_cmd({(uint32_t)y + OLED_ROW_OFFSET + h - 1, DATA_BYTE});
  }

  void SSD1351::update_screen_buffer(pixel_t *image)
  {
    for (size_t y = 0; y < _dynamic_area.height; y++)
    {
      for (size_t x = 0; x < _dynamic_area.width; x++)
      {
        _area_buffer[(y + _dynamic_area.yCrd) * OLED_SCREEN_WIDTH + (x + _dynamic_area.xCrd)] = image[y * _dynamic_area.width + x];
      }
    }
  }

  void SSD1351::transpose_screen_buffer()
  {
    pixel_t *tmpBuff = (pixel_t *)malloc(_dynamic_area.width * _dynamic_area.height * sizeof(pixel_t));
    memcpy(tmpBuff, _area_buffer, _dynamic_area.width * _dynamic_area.height * sizeof(pixel_t));
    for (uint8_t i = 0; i < _dynamic_area.height; i++)
    {
      for (uint8_t j = 0; j < _dynamic_area.width; j++)
      {
        _area_buffer[j * _dynamic_area.height + i] = tmpBuff[i * _dynamic_area.width + j];
      }
    }
    free(tmpBuff);
  }

  void SSD1351::draw_screen_buffer()
  {
    send_data((const uint8_t *)_area_buffer, OLED_SCREEN_WIDTH * OLED_SCREEN_HEIGHT * OLED_BYTES_PER_PIXEL);
  }

  void SSD1351::draw_screen_top_down()
  {
    uint16_t transStep = OLED_TRANSITION_STEP;

    uint16_t partImgSize = _dynamic_area.width * transStep;

    uint8_t *partImgPtr = (uint8_t *)_area_buffer +
                          (_dynamic_area.height - transStep) *
                              (_dynamic_area.width * OLED_BYTES_PER_PIXEL);

    while (1)
    {
      set_buffer_border(_dynamic_area.xCrd, _dynamic_area.yCrd, _dynamic_area.width, _dynamic_area.height);

      if (partImgSize > _dynamic_area.width * _dynamic_area.height)
      {
        send_data((const uint8_t *)_area_buffer,
                  _dynamic_area.width * _dynamic_area.height * OLED_BYTES_PER_PIXEL);
        break;
      }
      else
      {
        send_data((const uint8_t *)partImgPtr, partImgSize * OLED_BYTES_PER_PIXEL);
      }

      partImgPtr -= _dynamic_area.width * transStep * OLED_BYTES_PER_PIXEL;
      partImgSize += _dynamic_area.width * transStep;
      transStep++;
    }
  }

  void SSD1351::draw_screen_down_top()
  {
    uint16_t transStep = OLED_TRANSITION_STEP;

    uint16_t partImgSize = _dynamic_area.width * transStep;

    uint8_t *partImgPtr = (uint8_t *)_area_buffer;

    uint8_t yCrd_moving = _dynamic_area.yCrd + _dynamic_area.height - 1;

    while (1)
    {
      if (partImgSize > OLED_SCREEN_WIDTH * OLED_SCREEN_HEIGHT || yCrd_moving < _dynamic_area.yCrd)
      {
        set_buffer_border(_dynamic_area.xCrd, _dynamic_area.yCrd,
                          _dynamic_area.width, _dynamic_area.height);
        send_data((const uint8_t *)_area_buffer,
                  _dynamic_area.width * _dynamic_area.height * OLED_BYTES_PER_PIXEL);
        break;
      }
      else
      {
        set_buffer_border(_dynamic_area.xCrd, yCrd_moving,
                          _dynamic_area.width, _dynamic_area.yCrd + _dynamic_area.height - yCrd_moving);
        send_data((const uint8_t *)partImgPtr, partImgSize * OLED_BYTES_PER_PIXEL);
      }

      yCrd_moving -= transStep;
      partImgSize += _dynamic_area.width * transStep;
      transStep++;
    }
  }

  void SSD1351::draw_screen_left_right()
  {
    transpose_screen_buffer();

    send_cmd({OLED_CMD_SET_REMAP, CMD_BYTE});
    send_cmd({OLED_REMAP_SETTINGS | REMAP_VERTICAL_INCREMENT, DATA_BYTE});

    uint16_t transStep = OLED_TRANSITION_STEP;
    uint16_t partImgSize = _dynamic_area.height * transStep;
    uint8_t *partImgPtr = (uint8_t *)_area_buffer +
                          (_dynamic_area.width - transStep) *
                              (_dynamic_area.height * OLED_BYTES_PER_PIXEL);

    while (1)
    {
      set_buffer_border(_dynamic_area.xCrd, _dynamic_area.yCrd, _dynamic_area.width, _dynamic_area.height);
      if (partImgSize > _dynamic_area.width * _dynamic_area.height)
      {
        send_data((const uint8_t *)_area_buffer, _dynamic_area.width * _dynamic_area.height * OLED_BYTES_PER_PIXEL);
        break;
      }
      else
      {
        send_data((const uint8_t *)partImgPtr, partImgSize * OLED_BYTES_PER_PIXEL);
      }

      partImgPtr -= transStep * _dynamic_area.height * OLED_BYTES_PER_PIXEL;
      partImgSize += transStep * _dynamic_area.height;
      transStep++;
    }

    send_cmd({OLED_CMD_SET_REMAP, CMD_BYTE});
    send_cmd({OLED_REMAP_SETTINGS, DATA_BYTE});

    transpose_screen_buffer();
  }

  void SSD1351::draw_screen_right_left()
  {
    transpose_screen_buffer();

    send_cmd({OLED_CMD_SET_REMAP, CMD_BYTE});
    send_cmd({OLED_REMAP_SETTINGS | REMAP_VERTICAL_INCREMENT, DATA_BYTE});

    uint16_t transStep = OLED_TRANSITION_STEP;
    uint16_t partImgSize = _dynamic_area.height * transStep;
    uint8_t *partImgPtr = (uint8_t *)_area_buffer;
    uint8_t xCrd_moving = _dynamic_area.xCrd + _dynamic_area.width - 1;

    while (1)
    {
      if ((partImgSize > _dynamic_area.width * _dynamic_area.height) || (xCrd_moving < _dynamic_area.xCrd))
      {
        set_buffer_border(_dynamic_area.xCrd, _dynamic_area.yCrd, _dynamic_area.width, _dynamic_area.height);
        send_data((const uint8_t *)_area_buffer, _dynamic_area.height * _dynamic_area.width * OLED_BYTES_PER_PIXEL);
        break;
      }
      else
      {
        set_buffer_border(xCrd_moving, _dynamic_area.yCrd,
                          _dynamic_area.xCrd + _dynamic_area.width - xCrd_moving, _dynamic_area.height);
        send_data((const uint8_t *)partImgPtr, partImgSize * OLED_BYTES_PER_PIXEL);
      }
      xCrd_moving -= transStep;
      partImgSize += _dynamic_area.height * transStep;
      transStep++;
    }

    send_cmd({OLED_CMD_SET_REMAP, CMD_BYTE});
    send_cmd({OLED_REMAP_SETTINGS, DATA_BYTE});

    transpose_screen_buffer();
  }

  Status SSD1351::draw_text(const char *text)
  {
    if (_area_buffer == NULL)
    {
      return Status::AREA_NOT_SET;
    }

    int textHeight = selectedFont_height;
    int textWidth = get_text_width(text);

    if (textWidth > _dynamic_area.width || textHeight > _dynamic_area.height)
    {
      // TODO: Resize the dynamic area and return error only if the text is bigger that the screen
      // TODO: Rename this error type
      return Status::INIT_ERROR;
    }

    uint8_t char_x_offset = 0,
            char_y_offset = 0;

    // compute text alignment
    compute_alignment(textWidth, &char_x_offset, &char_y_offset);

    // create text background
    if (_text_properties.bgImage != NULL)
    {
      update_screen_buffer(_text_properties.bgImage);
    }

    // write characters in their space one by one
    pixel_t *textBuff = (pixel_t *)malloc(_dynamic_area.width * _dynamic_area.height * sizeof(pixel_t));
    memset(textBuff, 0, _dynamic_area.width * _dynamic_area.height * sizeof(pixel_t));

    int charCount = 0;
    while (text[charCount] != 0)
    {
      write_char_to_buffer(textBuff, text[charCount], &char_x_offset, &char_y_offset);
      charCount++;
    }

    for (size_t y = 0; y < _dynamic_area.height; y++)
    {
      for (size_t x = 0; x < _dynamic_area.width; x++)
      {
        if (textBuff[y * _dynamic_area.width + x] != 0)
        {
          _area_buffer[(y + _dynamic_area.yCrd) * OLED_SCREEN_WIDTH + (x + _dynamic_area.xCrd)] =
              textBuff[y * _dynamic_area.width + x];
        }
      }
    }
    // update_screen_buffer(textBuff);
    free(textBuff);

    // display the text on the OLED
    draw_screen_buffer();
    return Status::SUCCESS;
  }

  void SSD1351::compute_alignment(uint8_t textWidth, uint8_t *xOff, uint8_t *yOff)
  {
    int xAlign = _text_properties.alignParam & 0x0F;
    int yAlign = _text_properties.alignParam & 0xF0;

    switch (xAlign)
    {
    case TEXT_ALIGN_LEFT:
      *xOff = 0;
      break;
    case TEXT_ALIGN_RIGHT:
      *xOff = _dynamic_area.width - textWidth;
      break;
    case TEXT_ALIGN_CENTER:
      *xOff = (_dynamic_area.width - textWidth) >> 1;
      break;
    default:
      *xOff = 0;
      break;
    }

    switch (yAlign)
    {
    case TEXT_ALIGN_TOP:
      *yOff = 0;
      break;
    case TEXT_ALIGN_BOTTOM:
      *yOff = _dynamic_area.height - selectedFont_height;
      break;
    case TEXT_ALIGN_VCENTER:
      *yOff = (_dynamic_area.height - selectedFont_height) >> 1;
      break;
    default:
      *yOff = 0;
      break;
    }
  }

  void SSD1351::write_char_to_buffer(pixel_t *buff, char charToWrite, uint8_t *xOffset, uint8_t *yOffset)
  {
    if (charToWrite < selectedFont_firstChar || charToWrite > selectedFont_lastChar)
    {
      // If we're tying to write a character not present in the font we write '?' instead
      charToWrite = '?';
    }

    const uint8_t *charOffetTable = _text_properties.font + 8 +
                                    (uint16_t)((charToWrite - selectedFont_firstChar) << 2);
    uint32_t offset = (uint32_t)charOffetTable[1] |
                      ((uint32_t)charOffetTable[2] << 8) |
                      ((uint32_t)charOffetTable[3] << 16);
    uint8_t charWidth = *charOffetTable;
    const uint8_t *charBitMap = _text_properties.font + offset;

    uint8_t foo = 0, mask;
    for (uint8_t yCnt = 0; yCnt < selectedFont_height; ++yCnt)
    {
      mask = 0;
      for (uint8_t xCnt = 0; xCnt < charWidth; ++xCnt)
      {
        if (mask == 0)
        {
          mask = 1;
          foo = *charBitMap++;
        }

        *(buff +
          (yCnt + (*yOffset)) * _dynamic_area.width +
          (xCnt + (*xOffset))) = ((foo & mask) != 0) ? (uint16_t)swap_color(_text_properties.fontColor) : 0;
        mask <<= 1;
      }
    }

    *xOffset += charWidth;
  }
} // namespace oled