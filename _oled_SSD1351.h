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

#ifndef OLED_SSD1351_H_
#define OLED_SSD1351_H_

#include "mbed.h"
#include "oled_info.h"
#include "oled_types.h"

namespace oled
{
    class SSD1351
    {
    public:
        SSD1351(PinName mosiPin, PinName sclkPin,
                PinName pwrPin, PinName csPin,
                PinName rstPin, PinName dcPin);
        ~SSD1351();

        // Dim OLED screen on
        void dim_screen_on();

        // Return OLED back to full contrast
        void dim_screen_off();

        // Turn on Power for OLED Display
        void power_on();

        // Turn off Power for OLED Display
        void power_off();

        // Set OLED dynamic area
        void set_dynamic_area(DynamicArea *dynamic_area);

        // Destroy current OLED dynamic area
        void destroy_dynamic_area();

        // Fill the entire screen with specified color
        Status fill_screen(Color color);

        // Send a command to the OLED
        void SendCmd(uint32_t cmd, uint8_t isFirst);

        // Send raw data to the OLED
        void SendData(const uint8_t *dataToSend, uint32_t dataSize);

        // Draw a box on the OLED
        Status DrawBox(int8_t xCrd, int8_t yCrd, uint8_t width, uint8_t height, uint16_t color);

        // Draw a single pixel
        Status DrawPixel(int8_t xCrd, int8_t yCrd, uint16_t color);

        // Draw an image in the entire screen with a transition
        Status DrawScreen(const uint8_t *image, int8_t xCrd, int8_t yCrd,
                          uint8_t width, uint8_t height, Transition transition);

        // Set the font to use
        Status SetFont(const uint8_t *newFont, uint16_t newColor);

        // Set the OLED text properties
        void SetTextProperties(oled_text_properties_t *textProperties);

        // Get OLED text properties
        void GetTextProperties(oled_text_properties_t *textProperties);

        // Return the width in px required for the given string to be displayed
        uint8_t GetTextWidth(const uint8_t *text);

        // Count the characters of a text
        uint8_t CharCount(uint8_t width, const uint8_t *font, const uint8_t *text, uint8_t length);

        // Add text to the main screen buffer at position x,y.
        Status AddText(const uint8_t *text, int8_t xCrd, int8_t yCrd);

        // Add text to the main screen buffer. Used with SetDynamicArea() Function.
        Status AddText(const uint8_t *text);

        // Write text on OLED at position set in Dynamic Area Field. Used with SetDynamicArea() Function.
        Status DrawText(const uint8_t *text);

        // Return the dimensions of image
        void GetImageDimensions(uint8_t *width, uint8_t *height, const uint8_t *image);

        // Add image to the main screen buffer.Used with SetDynamicArea() Function.
        Status AddImage(const uint8_t *image);

        // Add image to the main screen buffer at position x,y
        Status AddImage(const uint8_t *image, int8_t xCrd, int8_t yCrd);

        // Send image to OLED GRAM.Used with SetDynamicArea() Function for positioning image.
        Status DrawImage(const uint8_t *image);

        // Send image to OLED GRAM at position x,y.
        Status DrawImage(const uint8_t *image, int8_t xCrd, int8_t yCrd);

        // Send image to OLED GRAM at position x,y and width,height.
        Status DrawImage(const uint8_t *image, int8_t xCrd, int8_t yCrd, uint8_t width, uint8_t height);

        void Swap(oled_pixel_t imgDst, const uint8_t *imgSrc, uint16_t imgSize);

        void UpdateBuffer(int8_t xCrd, int8_t yCrd, uint8_t width, uint8_t height, const uint8_t *image);

        // Write text on OLED at position x,y. Recommended for Static Text.
        Status Label(const uint8_t *text, int8_t xCrd, int8_t yCrd);

        // Create a text box of width,height at position x,y. Recommended for Dynamic Text.
        Status TextBox(const uint8_t *text, int8_t xCrd, int8_t yCrd, uint8_t width, uint8_t height);

    private:
        SPI _spi;
        DigitalOut _power;
        DigitalOut _cs;
        DigitalOut _rst;
        DigitalOut _dc;

        const uint8_t *selectedFont;

        uint8_t
            currentChar_width,
            currentChar_height,
            screenBuf[OLED_GRAM_SIZE];

        uint16_t
            selectedFont_color,
            selectedFont_firstChar, /* first character in the font table */
            selectedFont_lastChar,  /* last character in the font table */
            selectedFont_height,
            colorMask;

        DynamicArea _dynamic_area;
        oled_text_properties_t oled_text_properties;

        /* Internal Functions */
        void set_buffer_border(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
        void draw_screen_buffer();

        void Transpose(oled_pixel_t transImage, const oled_pixel_t image, uint8_t width, uint8_t height);
        Status None(const uint8_t *image, int8_t xCrd, int8_t yCrd, uint8_t width, uint8_t height);
        Status TopDown(const uint8_t *image, int8_t xCrd, int8_t yCrd, uint8_t width, uint8_t height);
        Status DownTop(const uint8_t *image, int8_t xCrd, int8_t yCrd, uint8_t width, uint8_t height);
        Status LeftRight(const uint8_t *image, int8_t xCrd, int8_t yCrd, uint8_t width, uint8_t height);
        Status RightLeft(const uint8_t *image, int8_t xCrd, int8_t yCrd, uint8_t width, uint8_t height);
        Status CreateTextBackground();
        void WriteCharToBuf(uint16_t charToWrite, oled_pixel_t *chrBuf);
        Status AddCharToTextArea(oled_pixel_t chrPtr, uint8_t chrWidth, uint8_t chrHeight, oled_pixel_t copyAddr, uint8_t imgWidth);
        void *AllocateDynamicArea(uint32_t area);
        Status DestroyDynamicArea(void *ptr);
    };
} // namespace oled

#endif // OLED_SSD1351_H_