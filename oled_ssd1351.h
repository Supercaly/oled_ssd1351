#ifndef OLED_SSD1351_H_
#define OLED_SSD1351_H_

#include "mbed.h"

// OLED commands
#define OLED_CMD_SET_COLUMN_ADDR (0x15)
#define OLED_CMD_SET_ROW_ADDR (0x75)
#define OLED_CMD_WRITE_RAM (0x5C)
#define OLED_CMD_READ_RAM (0x5D)
#define OLED_CMD_SET_REMAP (0xA0)
#define OLED_CMD_SET_DISPLAY_START_LINE (0xA1)
#define OLED_CMD_SET_DISPLAY_OFFSET (0xA2)
#define OLED_CMD_SET_DISPLAY_MODE_ALL_OFF (0xA4)
#define OLED_CMD_SET_DISPLAY_MODE_ALL_ON (0xA5)
#define OLED_CMD_SET_DISPLAY_MODE_NORMAL (0xA6)
#define OLED_CMD_SET_DISPLAY_MODE_INVERSE (0xA7)
#define OLED_CMD_FUNCTION_SELECTION (0xAB)
#define OLED_CMD_NOP (0xAD)
#define OLED_CMD_SET_SLEEP_MODE_ON (0xAE)
#define OLED_CMD_SET_SLEEP_MODE_OFF (0xAF)
#define OLED_CMD_SET_RESET_PRECHARGE (0xB1)
#define OLED_CMD_DISPLAY_ENHANCEMENT (0xB2)
#define OLED_CMD_DIV_SET (0xB3)
#define OLED_CMD_SET_VSL (0xB4)
#define OLED_CMD_SET_GPIO (0xB5)
#define OLED_CMD_SET_SECOND_PRECHARGE_PERIOD (0xB6)
#define OLED_CMD_LOOKUP_TABLE_GRAY_SCALE (0xB8)
#define OLED_CMD_USE_BUILTIN_LUT (0xB9)
#define OLED_CMD_SET_PRECHARGE_VOLTAGE (0xBB)
#define OLED_CMD_SET_V_COMH (0xBE)
#define OLED_CMD_SET_CONTRAST (0xC1)
#define OLED_CMD_SET_MASTER_CONTRAST (0xC7)
#define OLED_CMD_SET_MUX_RATIO (0xCA)
#define OLED_CMD_SET_COMMAND_LOCK (0xFD)
#define OLED_CMD_HORIZONTAL_SCROLL (0x96)
#define OLED_CMD_STOP_MOVING (0x9E)
#define OLED_CMD_START_MOVING (0x9F)

// OLED size
#define OLED_SCREEN_WIDTH (96)
#define OLED_SCREEN_HEIGHT (96)
#define OLED_SCREEN_SIZE (OLED_SCREEN_WIDTH * OLED_SCREEN_HEIGHT)
#define OLED_SCREEN_BPP (2)

#define CMD_BYTE (true)
#define DATA_BYTE (false)

#define OLED_COLUMN_OFFSET (16)
#define OLED_ROW_OFFSET (0)

#define check_coordinates(x, y, w, h) (x >= 0) && ((x + w - 1) <= OLED_SCREEN_WIDTH) && (y >= 0) && ((y + h - 1) <= OLED_SCREEN_HEIGHT)

namespace oled
{
    struct Command
    {
        uint8_t cmd;
        bool type;
    };

    enum class Transition
    {
        OLED_TRANSITION_NONE,
        OLED_TRANSITION_TOP_DOWN,
        OLED_TRANSITION_DOWN_TOP,
        OLED_TRANSITION_LEFT_RIGHT,
        OLED_TRANSITION_RIGHT_LEFT
    };

    enum Color
    {
        COLOR_BLACK = 0x0000,
        COLOR_BLUE_1 = 0x06FF,
        COLOR_BLUE = 0x001F,
        COLOR_RED = 0xF800,
        COLOR_GREEN = 0x07E0,
        COLOR_CYAN = 0x07FF,
        COLOR_MAGENTA = 0xF81F,
        COLOR_YELLOW = 0xFFE0,
        COLOR_GRAY = 0x528A,
        COLOR_WHITE = 0xFFFF
    };

    typedef uint16_t pixel_t;

    enum class Status
    {
        OLED_STATUS_SUCCESS,        // success
        OLED_STATUS_ERROR,          // fail
        OLED_STATUS_PROTOCOL_ERROR, // SPI failure
        OLED_STATUS_INIT_ERROR,     // initialization error
        OLED_STATUS_DEINIT_ERROR    // deinitialization error
    };

    struct DynamicArea
    {
        int8_t xCrd;
        int8_t yCrd;
        uint8_t width;
        uint8_t height;
        pixel_t *buffer;
    };

    class Oled
    {
    public:
        Oled(PinName mosiPin, PinName sclkPin,
             PinName pwrPin, PinName csPin,
             PinName rstPin, PinName dcPin);
        ~Oled();

        Status dim_screen_off();
        Status dim_screen_on();

        // Fill the entire screen with given Color
        Status fill_screen(Color color);

        Status draw_screen(const uint8_t *image, Transition transition);

        // Draw an image to the screen at given position
        Status draw_image(const uint8_t *image,
                          int8_t x, int8_t y,
                          uint8_t width, uint8_t height);

        // Draw a text label to the screen at given position
        Status label(const char *text, uint8_t x, uint8_t y);

    private:
        SPI _spi;
        DigitalOut _power;
        DigitalOut _cs;
        DigitalOut _rst;
        DigitalOut _dc;

        DynamicArea _dynamic_area;

        // Send a command to the OLED
        void send_cmd(uint32_t cmd, bool first);

        // Send raw data to OLED's RAM
        void send_data(const uint8_t *data, size_t size);

        // Power off the OLED
        void power_off() { _power = 0; }

        // Power on the OLED
        void power_on() { _power = 1; }

        // Set the new dynamic area
        void set_dynamic_area(DynamicArea *area);

        // Destroy the dynamic area
        void destroy_dynamic_area();

        // Set the limits of the screen buffer
        void set_buffer_border(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

        // Draw the dynamic area's buffer to the screen
        void draw_screen_buffer();

        // Update the values of dynamic area's buffer
        void update_screen_buffer(const uint8_t *image);

        // Transpose the dynamic area's buffer
        void traspose_screen_buffer();

        // Functions to draw screen with each transition 
        void draw_screen_top_down();
        void draw_screen_down_top();
        void draw_screen_left_right();
        void draw_screen_right_left();

    };
} // namespace oled

#endif // OLED_SSD1351_H_