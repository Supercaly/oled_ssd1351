#include "oled_ssd1351.h"

namespace oled
{
    const Command init_cmds[] = {
        {OLED_CMD_SET_COMMAND_LOCK, CMD_BYTE},
        {0x12, DATA_BYTE},
        {OLED_CMD_SET_COMMAND_LOCK, CMD_BYTE},
        {0xB1, DATA_BYTE},
        {OLED_CMD_SET_SLEEP_MODE_OFF, CMD_BYTE},
        {OLED_CMD_DIV_SET, CMD_BYTE},
        {0xF1, DATA_BYTE},
        {OLED_CMD_SET_MUX_RATIO, CMD_BYTE},
        {0x5F, DATA_BYTE},
        {OLED_CMD_SET_REMAP, CMD_BYTE},
        {0x60, DATA_BYTE},
        {OLED_CMD_SET_COLUMN_ADDR, CMD_BYTE},
        {0x00, DATA_BYTE},
        {0x5F, DATA_BYTE},
        {OLED_CMD_SET_ROW_ADDR, CMD_BYTE},
        {0x00, DATA_BYTE},
        {0x5F, DATA_BYTE},
        {OLED_CMD_SET_DISPLAY_START_LINE, CMD_BYTE},
        {0x80, DATA_BYTE},
        {OLED_CMD_SET_DISPLAY_OFFSET, CMD_BYTE},
        {0x60, DATA_BYTE},
        {OLED_CMD_SET_RESET_PRECHARGE, CMD_BYTE},
        {0x32, CMD_BYTE},
        {OLED_CMD_SET_V_COMH, CMD_BYTE},
        {0x05, CMD_BYTE},
        {OLED_CMD_SET_DISPLAY_MODE_NORMAL, CMD_BYTE},
        {OLED_CMD_SET_CONTRAST, CMD_BYTE},
        {0x8A, DATA_BYTE},
        {0x51, DATA_BYTE},
        {0x8A, DATA_BYTE},
        {OLED_CMD_SET_MASTER_CONTRAST, CMD_BYTE},
        {0xCF, DATA_BYTE},
        {OLED_CMD_SET_VSL, CMD_BYTE},
        {0xA0, DATA_BYTE},
        {0xB5, DATA_BYTE},
        {0x55, DATA_BYTE},
        {OLED_CMD_SET_SECOND_PRECHARGE_PERIOD, CMD_BYTE},
        {0x01, DATA_BYTE},
        {OLED_CMD_SET_SLEEP_MODE_OFF, CMD_BYTE}};

    Oled::Oled(PinName mosiPin, PinName sclkPin,
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

        // reset dynamic area
        _dynamic_area.xCrd = 0;
        _dynamic_area.yCrd = 0;
        _dynamic_area.width = 0;
        _dynamic_area.height = 0;
        _dynamic_area.buffer = NULL;

        // send init commands to OLED
        for (int i = 0; i < 39; ++i)
        {
            send_cmd(init_cmds[i].cmd, init_cmds[i].type);
        }
    }

    Oled::~Oled() {}

    Status Oled::dim_screen_off()
    {
        send_cmd(OLED_CMD_SET_MASTER_CONTRAST, CMD_BYTE);
        send_cmd(0xF, DATA_BYTE);

        return Status::OLED_STATUS_SUCCESS;
    }

    Status Oled::dim_screen_on()
    {
        send_cmd(OLED_CMD_SET_MASTER_CONTRAST, CMD_BYTE);
        send_cmd(0x0, DATA_BYTE);

        return Status::OLED_STATUS_SUCCESS;
    }

    Status Oled::fill_screen(Color color)
    {
        DynamicArea area = {
            .xCrd = 0,
            .yCrd = 0,
            .width = OLED_SCREEN_WIDTH,
            .height = OLED_SCREEN_HEIGHT};

        set_dynamic_area(&area);

        uint8_t *data = (uint8_t *)_dynamic_area.buffer;
        for (uint16_t i = 0; i < (OLED_SCREEN_WIDTH * OLED_SCREEN_HEIGHT); i++)
        {
            data[2 * i] = color >> 8;
            data[2 * i + 1] = color;
        }

        draw_screen_buffer();
        destroy_dynamic_area();

        return Status::OLED_STATUS_SUCCESS;
    }

    Status Oled::draw_screen(const uint8_t *image, Transition transition)
    {
        DynamicArea area = {
            .xCrd = 0,
            .yCrd = 0,
            .width = OLED_SCREEN_WIDTH,
            .height = OLED_SCREEN_HEIGHT};

        set_dynamic_area(&area);
        if (_dynamic_area.buffer == NULL)
        {
            return Status::OLED_STATUS_INIT_ERROR;
        }

        update_screen_buffer(image);

        switch (transition)
        {
        case Transition::OLED_TRANSITION_NONE:
            draw_screen_buffer();
            break;
        case Transition::OLED_TRANSITION_TOP_DOWN:
            draw_screen_top_down();
            break;
        case Transition::OLED_TRANSITION_DOWN_TOP:
            draw_screen_down_top();
            break;
        case Transition::OLED_TRANSITION_LEFT_RIGHT:
            draw_screen_left_right();
            break;
        case Transition::OLED_TRANSITION_RIGHT_LEFT:
            draw_screen_right_left();
            break;
        }

        destroy_dynamic_area();
        return Status::OLED_STATUS_SUCCESS;
    }

    Status Oled::draw_image(const uint8_t *image,
                            int8_t x, int8_t y,
                            uint8_t width, uint8_t height)
    {

        if (!check_coordinates(x, y, width, height))
        {
            return Status::OLED_STATUS_INIT_ERROR;
        }

        DynamicArea area = {
            .xCrd = x,
            .yCrd = y,
            .width = width,
            .height = height};

        set_dynamic_area(&area);
        if (_dynamic_area.buffer == NULL)
        {
            return Status::OLED_STATUS_INIT_ERROR;
        }

        update_screen_buffer(image);
        draw_screen_buffer();
        destroy_dynamic_area();
        return Status::OLED_STATUS_SUCCESS;
    }

    Status Oled::label(const char *text, uint8_t x, uint8_t y)
    {
        if (text == NULL)
        {
            return Status::OLED_STATUS_ERROR;
        }

        // DynamicArea textArea;
        // textArea.xCrd = x;
        // textArea.yCrd = y;
        // textArea.width = 0;
        // textArea.height = 0;
        // set_dynamic_area(&textArea);

        return Status::OLED_STATUS_SUCCESS;
    }

    /////////////////////
    // private methods //
    /////////////////////

    void Oled::send_cmd(uint32_t cmd, bool first)
    {
        uint8_t txBuffer[4];
        memcpy((void *)txBuffer, (void *)&cmd, 1);

        _dc = first ? 0 : 1;
        _cs = 0;
        _spi.write(*txBuffer);
        _cs = 1;
    }

    void Oled::send_data(const uint8_t *data, size_t size)
    {
        send_cmd(OLED_CMD_WRITE_RAM, true);
        _dc = 1;
        _cs = 0;

        const uint8_t *bufferPtr = data;
        for (size_t i = 0; i < size; ++i)
        {
            _spi.write(*bufferPtr);
            bufferPtr++;
        }
        _cs = 1;
    }

    void Oled::set_dynamic_area(DynamicArea *area)
    {
        // create memory for dynamic area's buffer
        if (area->height != _dynamic_area.height || area->width != _dynamic_area.width)
        {
            destroy_dynamic_area();
        }
        if (_dynamic_area.buffer == NULL)
        {
            _dynamic_area.buffer = (pixel_t *)malloc(area->width * area->height);
        }

        _dynamic_area.xCrd = area->xCrd;
        _dynamic_area.yCrd = area->yCrd;
        _dynamic_area.width = area->width;
        _dynamic_area.height = area->height;

        // set display borders
        set_buffer_border(_dynamic_area.xCrd, _dynamic_area.yCrd, _dynamic_area.width, _dynamic_area.height);
    }

    void Oled::destroy_dynamic_area()
    {
        if (_dynamic_area.buffer != NULL)
        {
            free(_dynamic_area.buffer);
            _dynamic_area.buffer = NULL;
        }
    }

    void Oled::set_buffer_border(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
    {
        // set display borders
        send_cmd(OLED_CMD_SET_COLUMN_ADDR, CMD_BYTE);
        send_cmd(x + OLED_COLUMN_OFFSET, DATA_BYTE);
        send_cmd(x + OLED_COLUMN_OFFSET + w - 1, DATA_BYTE);
        send_cmd(OLED_CMD_SET_ROW_ADDR, CMD_BYTE);
        send_cmd(y + OLED_ROW_OFFSET, DATA_BYTE);
        send_cmd(y + OLED_ROW_OFFSET + h - 1, DATA_BYTE);
    }

    void Oled::draw_screen_buffer()
    {
        send_data((uint8_t *)_dynamic_area.buffer, _dynamic_area.width * _dynamic_area.height * OLED_SCREEN_BPP);
    }

    void Oled::update_screen_buffer(const uint8_t *image)
    {
        memcpy(_dynamic_area.buffer, image, _dynamic_area.width * _dynamic_area.height * OLED_SCREEN_BPP);
    }

    void Oled::traspose_screen_buffer()
    {
        pixel_t tmpBuff[_dynamic_area.width * _dynamic_area.height];
        memcpy(tmpBuff, _dynamic_area.buffer, _dynamic_area.width * _dynamic_area.height * sizeof(pixel_t));
        for (uint8_t i = 0; i < _dynamic_area.height; ++i)
        {
            for (uint8_t j = 0; j < _dynamic_area.width; ++j)
            {
                _dynamic_area.buffer[j * _dynamic_area.height + i] = tmpBuff[i * _dynamic_area.width + j];
            }
        }
    }

    void Oled::draw_screen_top_down()
    {
        int transStep = 1;
        int partImgSize = _dynamic_area.width * transStep;
        uint8_t *partImgPtr = (uint8_t *)_dynamic_area.buffer +
                              (_dynamic_area.height - transStep) *
                                  (_dynamic_area.width * OLED_SCREEN_BPP);

        while (partImgSize <= _dynamic_area.width * _dynamic_area.height)
        {
            set_buffer_border(_dynamic_area.xCrd, _dynamic_area.yCrd,
                              _dynamic_area.width, _dynamic_area.height);
            send_data((const uint8_t *)partImgPtr, partImgSize * OLED_SCREEN_BPP);
            partImgPtr -= (_dynamic_area.width * transStep) * OLED_SCREEN_BPP;
            partImgSize += (_dynamic_area.width * transStep);
            transStep++;
        }
        set_buffer_border(_dynamic_area.xCrd, _dynamic_area.yCrd,
                          _dynamic_area.width, _dynamic_area.height);
        send_data((const uint8_t *)_dynamic_area.buffer, _dynamic_area.width * _dynamic_area.height * OLED_SCREEN_BPP);
    }

    void Oled::draw_screen_down_top()
    {
        int transStep = 1;
        int partImgSize = _dynamic_area.width * transStep;
        uint8_t *partImgPtr = (uint8_t *)_dynamic_area.buffer;
        uint8_t yCrd_moving = _dynamic_area.yCrd + _dynamic_area.height - 1;

        while (partImgSize <= _dynamic_area.width * _dynamic_area.height && yCrd_moving >= _dynamic_area.yCrd)
        {
            set_buffer_border(
                _dynamic_area.xCrd,
                yCrd_moving,
                _dynamic_area.width,
                _dynamic_area.yCrd + _dynamic_area.height - yCrd_moving);
            send_data((const uint8_t *)partImgPtr, partImgSize * OLED_SCREEN_BPP);
            yCrd_moving -= transStep;
            partImgSize += _dynamic_area.width * transStep;
            transStep++;
        }
        set_buffer_border(
            _dynamic_area.xCrd, _dynamic_area.xCrd,
            _dynamic_area.width, _dynamic_area.height);
        send_data((const uint8_t *)_dynamic_area.buffer, _dynamic_area.width * _dynamic_area.height * OLED_SCREEN_BPP);
    }

    void Oled::draw_screen_left_right()
    {
        // set vertical address increment
        send_cmd(OLED_CMD_SET_REMAP, CMD_BYTE);
        send_cmd(0x61, DATA_BYTE);

        uint16_t transStep = 1;
        uint16_t partImgSize = _dynamic_area.height * transStep;

        uint8_t *partImgPtr = (uint8_t *)_dynamic_area.buffer +
                              (_dynamic_area.width - transStep) *
                                  (_dynamic_area.height * OLED_SCREEN_BPP);

        traspose_screen_buffer();

        while (partImgSize <= _dynamic_area.width * _dynamic_area.height)
        {
            set_buffer_border(_dynamic_area.xCrd, _dynamic_area.yCrd,
                              _dynamic_area.width, _dynamic_area.height);
            send_data((const uint8_t *)partImgPtr, partImgSize * OLED_SCREEN_BPP);
            partImgPtr -= transStep * _dynamic_area.height * OLED_SCREEN_BPP;
            partImgSize += transStep * _dynamic_area.height;
            transStep++;
        }
        set_buffer_border(_dynamic_area.xCrd, _dynamic_area.yCrd,
                          _dynamic_area.width, _dynamic_area.height);
        send_data((const uint8_t *)_dynamic_area.buffer, _dynamic_area.width * _dynamic_area.height * OLED_SCREEN_BPP);

        // reset horizontal address increment
        send_cmd(OLED_CMD_SET_REMAP, CMD_BYTE);
        send_cmd(0x60, DATA_BYTE);
    }

    void Oled::draw_screen_right_left()
    {
        int transStep = 1;
        int partImgSize = _dynamic_area.height * transStep;
        uint8_t *partImgPtr = (uint8_t *)_dynamic_area.buffer;
        uint8_t xCrd_moving = _dynamic_area.xCrd + _dynamic_area.width - 1;

        // set vertical address increment
        send_cmd(OLED_CMD_SET_REMAP, CMD_BYTE);
        send_cmd(0x61, DATA_BYTE);

        traspose_screen_buffer();

        while (partImgSize <= _dynamic_area.width * _dynamic_area.height && xCrd_moving >= _dynamic_area.xCrd)
        {
            set_buffer_border(
                xCrd_moving,
                _dynamic_area.yCrd,
                _dynamic_area.xCrd + _dynamic_area.width - xCrd_moving,
                _dynamic_area.height);
            send_data((const uint8_t *)partImgPtr, partImgSize * OLED_SCREEN_BPP);

            transStep++;
            xCrd_moving -= transStep;
            partImgSize += _dynamic_area.height * transStep;
        }
        set_buffer_border(
            _dynamic_area.xCrd,
            _dynamic_area.yCrd,
            _dynamic_area.width,
            _dynamic_area.height);
        send_data((const uint8_t *)_dynamic_area.buffer, _dynamic_area.width * _dynamic_area.height * OLED_SCREEN_BPP);

        // reset horizontal address increment
        send_cmd(OLED_CMD_SET_REMAP, CMD_BYTE);
        send_cmd(0x60, DATA_BYTE);
    }

} // namespace oled