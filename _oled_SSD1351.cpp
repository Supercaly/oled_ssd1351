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
 */

#include "_oled_SSD1351.h"
#include "OpenSans_Font.h"
#include "mbed.h"

namespace oled
{
  const init_cmd_t seq[] = {
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

    _spi.frequency(8000000);

    _dc = 0;
    power_off();
    wait_ms(1);
    _rst = 0;
    wait_ms(1);
    _rst = 1;
    wait_ms(1);
    power_on();

    // reset text prop
    currentChar_width = 0,
    currentChar_height = 0;
    colorMask = Color::WHITE;
    oled_text_properties.alignParam = OLED_TEXT_ALIGN_CENTER;
    oled_text_properties.background = NULL;
    oled_text_properties.font = OpenSans_10x15_Regular;
    oled_text_properties.fontColor = Color::WHITE;
    SetTextProperties(&oled_text_properties);

    // reset dynamic area
    _dynamic_area.xCrd = 0;
    _dynamic_area.yCrd = 0;
    _dynamic_area.width = 0;
    _dynamic_area.height = 0;
    _dynamic_area.buffer = NULL;

    // send init commands to OLED
    for (int i = 0; i < 39; i++)
    {
      SendCmd(seq[i].cmd, seq[i].type);
    }
  }

  SSD1351::~SSD1351(void)
  {
    // TO_DO
    // Run Free and zero pointers.
  }

  void SSD1351::dim_screen_on()
  {
    for (int i = 0; i < 16; i++)
    {
      SendCmd(OLED_CMD_CONTRASTMASTER, CMD_BYTE);
      SendCmd(0xC0 | (0xF - i), DATA_BYTE);
      wait_ms(20);
    }
  }

  void SSD1351::dim_screen_off()
  {
    SendCmd(OLED_CMD_CONTRASTMASTER, CMD_BYTE);
    SendCmd(0xC0 | 0xF, DATA_BYTE);
  }

  void SSD1351::power_on()
  {
    _power = 1;
  }

  void SSD1351::power_off()
  {
    _power = 0;
  }

  void SSD1351::SendCmd(uint32_t cmd,
                        uint8_t isFirst)
  {

    uint8_t
        txSize = 1,
        txBuf[4];

    memcpy((void *)txBuf, (void *)&cmd, txSize);

    if (isFirst)
    {
      _dc = 0;
    }
    else
    {
      _dc = 1;
    }

    _cs = 0;
    _spi.write(*txBuf);
    _cs = 1;
  }

  void SSD1351::SendData(const uint8_t *dataToSend,
                         uint32_t dataSize)

  {

    uint16_t *arrayPtr = (uint16_t *)dataToSend;

    for (uint32_t i = 0; i < dataSize / 2; i++)
    {
      arrayPtr[i] &= colorMask;
    }

    SendCmd(OLED_CMD_WRITERAM, CMD_BYTE);

    /* sending data -> set DC pin */
    _dc = 1;
    _cs = 0;

    const uint8_t *
        /* traversing pointer */
        bufPtr = dataToSend;

    for (uint32_t i = 0; i < dataSize; i++)
    {
      _spi.write(*bufPtr);
      bufPtr += 1;
    }

    _cs = 1;
  }

  Status SSD1351::set_dynamic_area(DynamicArea *area)
  {
    // check if given area is valid
    if (!check_coord(area->xCrd, area->yCrd, area->width, area->height))
    {
      return Status::COORD_ERROR;
    }

    // destroy dynamic area if the new one has different size
    if (area->width != _dynamic_area.width || area->height != _dynamic_area.height)
    {
      destroy_dynamic_area();
    }
    // allocate memory for the area
    if (_dynamic_area.buffer == NULL)
    {
      _dynamic_area.buffer = (pixel_t *)malloc(area->width * area->height * sizeof(pixel_t));
    }

    // if, after allocating memory the buffer is null something wrong happened... return an error
    if (_dynamic_area.buffer == NULL)
    {
      return Status::INIT_ERROR;
    }

    // set the coordinates and border
    _dynamic_area.xCrd = area->xCrd;
    _dynamic_area.yCrd = area->yCrd;
    _dynamic_area.width = area->width;
    _dynamic_area.height = area->height;
    set_buffer_border(_dynamic_area.xCrd, _dynamic_area.yCrd, _dynamic_area.width, _dynamic_area.height);

    return Status::SUCCESS;
  }

  void SSD1351::destroy_dynamic_area()
  {
    if (_dynamic_area.buffer != NULL)
    {
      free(_dynamic_area.buffer);
      _dynamic_area.buffer = NULL;
    }
  }

  Status SSD1351::fill_screen(Color color)
  {
    DynamicArea area = {
        .xCrd = 0,
        .yCrd = 0,
        .width = OLED_SCREEN_WIDTH,
        .height = OLED_SCREEN_HEIGHT};
    Status status = set_dynamic_area(&area);
    if (status != Status::SUCCESS)
    {
      return status;
    }

    uint8_t *data = (uint8_t *)_dynamic_area.buffer;
    for (size_t i = 0; i < (OLED_SCREEN_WIDTH * OLED_SCREEN_HEIGHT); i++)
    {
      data[2 * i] = color >> 8;
      data[2 * i + 1] = color;
    }

    draw_screen_buffer();
    destroy_dynamic_area();

    return Status::SUCCESS;
  }

  Status SSD1351::draw_image(const uint8_t *image)
  {
    update_screen_buffer((pixel_t *)image);
    draw_screen_buffer();
    return Status::SUCCESS;
  }

  Status SSD1351::draw_image(const uint8_t *image, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
  {
    DynamicArea area = {
        .xCrd = x,
        .yCrd = y,
        .width = w,
        .height = h};
    Status status = set_dynamic_area(&area);
    if (status != Status::SUCCESS)
    {
      return status;
    }

    update_screen_buffer((pixel_t *)image);
    draw_screen_buffer();

    destroy_dynamic_area();
    return Status::SUCCESS;
  }

  Status SSD1351::draw_screen(const uint8_t *image, Transition transition)
  {
    DynamicArea area = {
        .xCrd = 0,
        .yCrd = 0,
        .width = OLED_SCREEN_WIDTH,
        .height = OLED_SCREEN_HEIGHT};
    Status status = set_dynamic_area(&area);
    if (status != Status::SUCCESS)
    {
      return status;
    }

    update_screen_buffer((pixel_t *)image);
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

    destroy_dynamic_area();
    return Status::SUCCESS;
  }

  Status SSD1351::draw_box(uint8_t x, uint8_t y, uint8_t w, uint8_t h, Color color)
  {
    uint32_t boxSize = w * h;
    DynamicArea boxArea = {
        .xCrd = x,
        .yCrd = y,
        .width = w,
        .height = h};
    Status status = set_dynamic_area(&boxArea);
    if (status != Status::SUCCESS)
    {
      return status;
    }

    for (uint16_t i = 0; i < boxSize; i++)
    {
      _dynamic_area.buffer[2 * i] = color >> 8;
      _dynamic_area.buffer[2 * i + 1] = color;
    }

    draw_screen_buffer();
    destroy_dynamic_area();

    return Status::SUCCESS;
  }

  Status SSD1351::draw_pixel(uint8_t x, uint8_t y, Color color)
  {
    DynamicArea area = {
        .xCrd = x,
        .yCrd = y,
        .width = 1,
        .height = 1};
    Status status = set_dynamic_area(&area);
    if (status != Status::SUCCESS)
    {
      return status;
    }

    pixel_t dot = swap_pixel(color);
    _dynamic_area.buffer[0] = dot;
    draw_screen_buffer();
    
    destroy_dynamic_area();
    
    return Status::SUCCESS;
  }

  Status SSD1351::SetFont(
      const uint8_t *newFont,
      uint16_t newColor)
  {
    /* save the new values in intern variables */

    selectedFont = newFont;
    selectedFont_firstChar = newFont[2] | ((uint16_t)newFont[3] << 8);
    selectedFont_lastChar = newFont[4] | ((uint16_t)newFont[5] << 8);
    selectedFont_height = newFont[6];
    selectedFont_color = newColor;

    OLED_SwapMe(selectedFont_color);

    return Status::SUCCESS;
  }

  void SSD1351::SetTextProperties(oled_text_properties_t *textProperties)
  {
    oled_text_properties.font = textProperties->font;
    oled_text_properties.fontColor = textProperties->fontColor;
    oled_text_properties.alignParam = textProperties->alignParam;
    oled_text_properties.background = textProperties->background;

    SetFont(oled_text_properties.font, oled_text_properties.fontColor);
  }

  void SSD1351::GetTextProperties(oled_text_properties_t *textProperties)
  {
    textProperties->font = oled_text_properties.font;
    textProperties->fontColor = oled_text_properties.fontColor;
    textProperties->alignParam = oled_text_properties.alignParam;
    textProperties->background = oled_text_properties.background;
  }

  uint8_t SSD1351::GetTextWidth(const uint8_t *text)
  {
    uint8_t chrCnt = 0;
    uint8_t text_width = 0;

    while (0 != text[chrCnt])
    {
      text_width += *(selectedFont + 8 + (uint16_t)((text[chrCnt++] - selectedFont_firstChar) << 2));
      /* make 1px space between chars */
      text_width++;
    }
    /* remove the final space */
    text_width--;

    return text_width;
  }

  uint8_t SSD1351::CharCount(uint8_t width, const uint8_t *font, const uint8_t *text, uint8_t length)
  {
    uint8_t chrCnt = 0;
    uint8_t text_width = 0;
    uint16_t firstChar;

    firstChar = font[2] | ((uint16_t)font[3] << 8);

    while (chrCnt < length)
    {
      text_width += *(font + 8 + (uint16_t)((text[chrCnt++] - firstChar) << 2));
      if (text_width > width)
      {
        chrCnt--;
        break;
      }
      /* make 1px space between chars */
      text_width++;
    }

    return chrCnt;
  }

  Status SSD1351::AddText(const uint8_t *text, int8_t xCrd, int8_t yCrd)
  {
    uint16_t
        chrCnt = 0;
    oled_pixel_t
        chrBuf = NULL;

    uint8_t
        currentChar_x = 0,
        currentChar_y = 0;

    uint8_t
        text_height = 0,
        text_width = 0;

    text_width = GetTextWidth(text);

    /*
     * set default values, if necessary
     */

    text_height = selectedFont_height;
    DynamicArea textArea;

    textArea.width = text_width;
    textArea.height = text_height;
    textArea.xCrd = xCrd;
    textArea.yCrd = yCrd;
    set_dynamic_area(&textArea);

    currentChar_y = (_dynamic_area.height - text_height) >> 1;

    switch (oled_text_properties.alignParam & OLED_TEXT_HALIGN_MASK)
    {
    case OLED_TEXT_ALIGN_LEFT:
    {
      currentChar_x = 0;
      break;
    }

    case OLED_TEXT_ALIGN_RIGHT:
    {
      currentChar_x = (_dynamic_area.width - text_width);
      break;
    }

    case OLED_TEXT_ALIGN_CENTER:
    {
      currentChar_x += (_dynamic_area.width - text_width) >> 1;
      break;
    }

    case OLED_TEXT_ALIGN_NONE:
    {
      break;
    }

    default:
    {
    }
    }

    if (CreateTextBackground() != Status::SUCCESS)
    {
      return Status::ERROR;
    }

    /*
     * write the characters into designated space, one by one
     */

    chrCnt = 0;
    while (0 != text[chrCnt])
    {
      WriteCharToBuf(text[chrCnt++], &chrBuf);

      if (NULL == chrBuf)
      {
        return Status::INIT_ERROR;
      }

      else
      {
        if (
            ((currentChar_x + currentChar_width) > _dynamic_area.width) || ((currentChar_y + currentChar_height) > _dynamic_area.height))
        {
          // destroy_dynamic_area(chrBuf);
          chrBuf = NULL;
          return Status::ERROR;
        }

        /* copy data */
        oled_pixel_t
            copyAddr = _dynamic_area.buffer + (currentChar_y * _dynamic_area.width + currentChar_x);

        AddCharToTextArea(chrBuf, currentChar_width, currentChar_height, copyAddr, _dynamic_area.width);

        currentChar_x += (currentChar_width + 1);
        currentChar_y += 0;

        // destroy_dynamic_area(chrBuf);
        chrBuf = NULL;
      }
    }

    UpdateBuffer(
        _dynamic_area.xCrd,
        _dynamic_area.yCrd,
        _dynamic_area.width,
        _dynamic_area.height,
        (const uint8_t *)_dynamic_area.buffer);

    return Status::SUCCESS;
  }

  Status SSD1351::AddText(const uint8_t *text)
  {
    uint16_t
        chrCnt = 0;
    oled_pixel_t
        chrBuf = NULL;

    uint8_t
        currentChar_x = 0,
        currentChar_y = 0;

    uint8_t
        text_height = 0,
        text_width = 0;

    text_width = GetTextWidth(text);

    /**
     * set default values, if necessary
     */

    text_height = selectedFont_height;

    if ((_dynamic_area.width < text_width) || (_dynamic_area.height < text_height))
    {
      DynamicArea textArea;
      textArea.width = text_width;
      textArea.height = text_height;
      set_dynamic_area(&textArea);
    }

    currentChar_y = (_dynamic_area.height - text_height) >> 1;

    switch (oled_text_properties.alignParam & OLED_TEXT_HALIGN_MASK)
    {
    case OLED_TEXT_ALIGN_LEFT:
    {
      currentChar_x = 0;
      break;
    }

    case OLED_TEXT_ALIGN_RIGHT:
    {
      currentChar_x = (_dynamic_area.width - text_width);
      break;
    }

    case OLED_TEXT_ALIGN_CENTER:
    {
      currentChar_x += (_dynamic_area.width - text_width) >> 1;
      break;
    }

    case OLED_TEXT_ALIGN_NONE:
    {
      break;
    }

    default:
    {
    }
    }

    if (CreateTextBackground() != Status::SUCCESS)
    {
      return Status::ERROR;
    }

    /**
     * write the characters into designated space, one by one
     */

    chrCnt = 0;
    while (0 != text[chrCnt])
    {
      WriteCharToBuf(text[chrCnt++], &chrBuf);

      if (NULL == chrBuf)
      {
        return Status::INIT_ERROR;
      }

      else
      {
        if (
            ((currentChar_x + currentChar_width) > _dynamic_area.width) || ((currentChar_y + currentChar_height) > _dynamic_area.height))
        {
          // destroy_dynamic_area(chrBuf);
          chrBuf = NULL;
          return Status::ERROR;
        }

        /* copy data */
        oled_pixel_t
            copyAddr = _dynamic_area.buffer + (currentChar_y * _dynamic_area.width + currentChar_x);

        AddCharToTextArea(chrBuf, currentChar_width, currentChar_height, copyAddr, _dynamic_area.width);

        currentChar_x += (currentChar_width + 1);
        currentChar_y += 0;

        // destroy_dynamic_area(chrBuf);
        chrBuf = NULL;
      }
    }

    UpdateBuffer(
        _dynamic_area.xCrd,
        _dynamic_area.yCrd,
        _dynamic_area.width,
        _dynamic_area.height,
        (const uint8_t *)_dynamic_area.buffer);

    return Status::SUCCESS;
  }

  Status SSD1351::DrawText(const uint8_t *text)
  {

    if (NULL == text)
    {
      return Status::ERROR;
    }

    AddText(text);
    /* set the locations */
    set_buffer_border(_dynamic_area.xCrd, _dynamic_area.yCrd, _dynamic_area.width, _dynamic_area.height);
    /* fill the GRAM */
    SendData((const uint8_t *)_dynamic_area.buffer, _dynamic_area.width * _dynamic_area.height * OLED_BYTES_PER_PIXEL);

    // free( currentTextAreaImage );
    return Status::SUCCESS;
  }

  void SSD1351::Swap(
      oled_pixel_t imgDst,
      const uint8_t *imgSrc,
      uint16_t imgSize)
  {
    for (int var = 0; var < imgSize; var++)
    {
      *imgDst = *imgSrc << 8;
      imgSrc++;
      *imgDst |= *imgSrc;
      imgDst++;
      imgSrc++;
    }
  }

  void SSD1351::UpdateBuffer(
      int8_t xCrd,
      int8_t yCrd,
      uint8_t width,
      uint8_t height,
      const uint8_t *image)
  {
    /* copy data */
    oled_pixel_t
        copyAddr = (oled_pixel_t)screenBuf + (yCrd * OLED_SCREEN_WIDTH + xCrd);

    for (uint8_t i = 0; i < height; i++)
    {
      memcpy((void *)copyAddr, (void *)image, width * OLED_BYTES_PER_PIXEL);
      copyAddr += OLED_SCREEN_WIDTH;
      image += width * OLED_BYTES_PER_PIXEL;
    }
  }

  Status SSD1351::Label(const uint8_t *text, int8_t xCrd, int8_t yCrd)
  {

    if (NULL == text)
    {
      return Status::ERROR;
    }

    AddText(text, xCrd, yCrd);

    /* set the locations */
    set_buffer_border(_dynamic_area.xCrd, _dynamic_area.yCrd, _dynamic_area.width, _dynamic_area.height);

    /* fill the GRAM */
    SendData((const uint8_t *)_dynamic_area.buffer, _dynamic_area.width * _dynamic_area.height * OLED_BYTES_PER_PIXEL);

    // free( currentTextAreaImage );
    return Status::SUCCESS;
  }

  Status SSD1351::TextBox(const uint8_t *text, int8_t xCrd, int8_t yCrd, uint8_t width, uint8_t height)
  {

    if (NULL == text)
    {
      return Status::ERROR;
    }

    DynamicArea textArea;
    textArea.width = width;
    textArea.height = height;
    textArea.xCrd = xCrd;
    textArea.yCrd = yCrd;

    set_dynamic_area(&textArea);
    DrawText(text);

    return Status::SUCCESS;
  }

  /////////////////////
  // private methods //
  /////////////////////

  void SSD1351::set_buffer_border(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
  {
    SendCmd(OLED_CMD_SET_COLUMN, CMD_BYTE);
    SendCmd(x + OLED_COLUMN_OFFSET, DATA_BYTE);
    SendCmd(x + OLED_COLUMN_OFFSET + w - 1, DATA_BYTE);
    SendCmd(OLED_CMD_SET_ROW, CMD_BYTE);
    SendCmd(y + OLED_ROW_OFFSET, DATA_BYTE);
    SendCmd(y + OLED_ROW_OFFSET + h - 1, DATA_BYTE);
  }

  void SSD1351::update_screen_buffer(pixel_t *image)
  {
    memcpy(_dynamic_area.buffer, image, _dynamic_area.width * _dynamic_area.height * sizeof(pixel_t));
  }

  void SSD1351::transpose_screen_buffer()
  {
    pixel_t *tmpBuff = (pixel_t *)malloc(_dynamic_area.width * _dynamic_area.height * sizeof(pixel_t));
    memcpy(tmpBuff, _dynamic_area.buffer, _dynamic_area.width * _dynamic_area.height * sizeof(pixel_t));
    for (uint8_t i = 0; i < _dynamic_area.height; i++)
    {
      for (uint8_t j = 0; j < _dynamic_area.width; j++)
      {
        _dynamic_area.buffer[j * _dynamic_area.height + i] = tmpBuff[i * _dynamic_area.width + j];
      }
    }
    free(tmpBuff);
  }

  void SSD1351::draw_screen_buffer()
  {
    SendData((const uint8_t *)_dynamic_area.buffer, _dynamic_area.width * _dynamic_area.height * OLED_BYTES_PER_PIXEL);
  }

  void SSD1351::draw_screen_top_down()
  {
    uint16_t transStep = OLED_TRANSITION_STEP;

    uint16_t partImgSize = _dynamic_area.width * transStep;

    uint8_t *partImgPtr = (uint8_t *)_dynamic_area.buffer +
                          (_dynamic_area.height - transStep) *
                              (_dynamic_area.width * OLED_BYTES_PER_PIXEL);

    while (1)
    {
      set_buffer_border(_dynamic_area.xCrd, _dynamic_area.yCrd, _dynamic_area.width, _dynamic_area.height);

      if (partImgSize > _dynamic_area.width * _dynamic_area.height)
      {
        SendData((const uint8_t *)_dynamic_area.buffer,
                 _dynamic_area.width * _dynamic_area.height * OLED_BYTES_PER_PIXEL);
        break;
      }
      else
      {
        SendData((const uint8_t *)partImgPtr, partImgSize * OLED_BYTES_PER_PIXEL);
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

    uint8_t *partImgPtr = (uint8_t *)_dynamic_area.buffer;

    uint8_t yCrd_moving = _dynamic_area.yCrd + _dynamic_area.height - 1;

    while (1)
    {
      if (partImgSize > OLED_SCREEN_SIZE || yCrd_moving < _dynamic_area.yCrd)
      {
        set_buffer_border(_dynamic_area.xCrd, _dynamic_area.yCrd,
                          _dynamic_area.width, _dynamic_area.height);
        SendData((const uint8_t *)_dynamic_area.buffer,
                 _dynamic_area.width * _dynamic_area.height * OLED_BYTES_PER_PIXEL);
        break;
      }
      else
      {
        set_buffer_border(_dynamic_area.xCrd, yCrd_moving,
                          _dynamic_area.width, _dynamic_area.yCrd + _dynamic_area.height - yCrd_moving);
        SendData((const uint8_t *)partImgPtr, partImgSize * OLED_BYTES_PER_PIXEL);
      }

      yCrd_moving -= transStep;
      partImgSize += _dynamic_area.width * transStep;
      transStep++;
    }
  }

  void SSD1351::draw_screen_left_right()
  {
    transpose_screen_buffer();

    SendCmd(OLED_CMD_SET_REMAP, CMD_BYTE);
    SendCmd(OLED_REMAP_SETTINGS | REMAP_VERTICAL_INCREMENT, DATA_BYTE);

    uint16_t transStep = OLED_TRANSITION_STEP;
    uint16_t partImgSize = _dynamic_area.height * transStep;
    uint8_t *partImgPtr = (uint8_t *)_dynamic_area.buffer +
                          (_dynamic_area.width - transStep) *
                              (_dynamic_area.height * OLED_BYTES_PER_PIXEL);

    while (1)
    {
      set_buffer_border(_dynamic_area.xCrd, _dynamic_area.yCrd, _dynamic_area.width, _dynamic_area.height);
      if (partImgSize > _dynamic_area.width * _dynamic_area.height)
      {
        SendData((const uint8_t *)_dynamic_area.buffer, _dynamic_area.width * _dynamic_area.height * OLED_BYTES_PER_PIXEL);
        break;
      }
      else
      {
        SendData((const uint8_t *)partImgPtr, partImgSize * OLED_BYTES_PER_PIXEL);
      }

      partImgPtr -= transStep * _dynamic_area.height * OLED_BYTES_PER_PIXEL;
      partImgSize += transStep * _dynamic_area.height;
      transStep++;
    }

    SendCmd(OLED_CMD_SET_REMAP, CMD_BYTE);
    SendCmd(OLED_REMAP_SETTINGS, DATA_BYTE);
  }

  void SSD1351::draw_screen_right_left()
  {
    transpose_screen_buffer();

    SendCmd(OLED_CMD_SET_REMAP, CMD_BYTE);
    SendCmd(OLED_REMAP_SETTINGS | REMAP_VERTICAL_INCREMENT, DATA_BYTE);

    uint16_t transStep = OLED_TRANSITION_STEP;
    uint16_t partImgSize = _dynamic_area.height * transStep;
    uint8_t *partImgPtr = (uint8_t *)_dynamic_area.buffer;
    uint8_t xCrd_moving = _dynamic_area.xCrd + _dynamic_area.width - 1;

    while (1)
    {
      if ((partImgSize > _dynamic_area.width * _dynamic_area.height) || (xCrd_moving < _dynamic_area.xCrd))
      {
        set_buffer_border(_dynamic_area.xCrd, _dynamic_area.yCrd, _dynamic_area.width, _dynamic_area.height);
        SendData((const uint8_t *)_dynamic_area.buffer, _dynamic_area.height * _dynamic_area.width * OLED_BYTES_PER_PIXEL);
        break;
      }
      else
      {
        set_buffer_border(xCrd_moving, _dynamic_area.yCrd,
                          _dynamic_area.xCrd + _dynamic_area.width - xCrd_moving, _dynamic_area.height);
        SendData((const uint8_t *)partImgPtr, partImgSize * OLED_BYTES_PER_PIXEL);
      }
      xCrd_moving -= transStep;
      partImgSize += _dynamic_area.height * transStep;
      transStep++;
    }

    SendCmd(OLED_CMD_SET_REMAP, CMD_BYTE);
    SendCmd(OLED_REMAP_SETTINGS, DATA_BYTE);
  }

  Status SSD1351::CreateTextBackground()
  {
    uint8_t
        xCrd = _dynamic_area.xCrd,
        yCrd = _dynamic_area.yCrd,
        width = _dynamic_area.width,
        height = _dynamic_area.height;

    oled_pixel_t
        imgBuf = _dynamic_area.buffer,
        copyAddr;

    const uint8_t *
        background = oled_text_properties.background;

    /* copy data */

    if (
        (NULL == imgBuf) || ((xCrd + width) > OLED_SCREEN_WIDTH) || ((yCrd + height) > OLED_SCREEN_HEIGHT))
    {
      return Status::INIT_ERROR;
    }

    if (NULL == background)
    {
      for (uint8_t i = 0; i < height; i++)
      {
        memset((void *)imgBuf, 0, width * OLED_BYTES_PER_PIXEL);
        imgBuf += width;
      }
    }

    else
    {
      copyAddr = (oled_pixel_t)(BMP_SkipHeader(background)) + (yCrd * OLED_SCREEN_WIDTH + xCrd);
      for (uint8_t i = 0; i < height; i++)
      {
        Swap((oled_pixel_t)imgBuf, (const uint8_t *)copyAddr, width);
        imgBuf += width;
        copyAddr += OLED_SCREEN_WIDTH;
      }
    }

    return Status::SUCCESS;
  }

  void SSD1351::WriteCharToBuf(
      uint16_t charToWrite,
      oled_pixel_t *chrBuf)
  {
    uint8_t
        foo = 0,
        mask;

    const uint8_t *
        pChTable = selectedFont + 8 + (uint16_t)((charToWrite - selectedFont_firstChar) << 2);

    currentChar_width = *pChTable,
    currentChar_height = selectedFont_height;

    uint32_t
        offset = (uint32_t)pChTable[1] | ((uint32_t)pChTable[2] << 8) | ((uint32_t)pChTable[3] << 16);

    const uint8_t *
        pChBitMap = selectedFont + offset;

    /* allocate space for char image */
    *chrBuf = (oled_pixel_t)malloc(currentChar_height * currentChar_width);

    if (NULL == *chrBuf)
    {
      return;
    }

    for (uint8_t yCnt = 0; yCnt < currentChar_height; yCnt++)
    {
      mask = 0;

      for (uint8_t xCnt = 0; xCnt < currentChar_width; xCnt++)
      {
        if (0 == mask)
        {
          mask = 1;
          foo = *pChBitMap++;
        }

        if (0 != (foo & mask))
        {
          *(*chrBuf + yCnt * currentChar_width + xCnt) = selectedFont_color;
        }

        else
        {
          *(*chrBuf + yCnt * currentChar_width + xCnt) = 0;
        }

        mask <<= 1;
      }
    }
  }

  Status SSD1351::AddCharToTextArea(
      oled_pixel_t chrPtr,
      uint8_t chrWidth,
      uint8_t chrHeight,
      oled_pixel_t copyAddr,
      uint8_t imgWidth)
  {
    if (NULL == copyAddr)
    {
      return Status::INIT_ERROR;
    }

    for (uint8_t i = 0; i < chrHeight; i++)
    {
      for (uint8_t j = 0; j < chrWidth; j++)
      {
        if (0 != chrPtr[j])
        {
          copyAddr[j] = chrPtr[j];
        }
      }
      copyAddr += imgWidth;
      chrPtr += chrWidth;
    }
    return Status::SUCCESS;
  }

} // namespace oled