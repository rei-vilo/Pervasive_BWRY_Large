//
// Pervasive_BWRY_Large.cpp
// Class library C++ code
// ----------------------------------
//
// Project Pervasive Displays Library Suite
//
// Copyright (c) Pervasive Displays Inc., 2021-2026
// Licence All rights reserved
//
// See Pervasive_BWRY_Large.h for references
//
// Release 904: Added new driver library
// Release 1003: Added support for BWRY large screens
//

// Header
#include "Pervasive_BWRY_Large.h"

//
// === COG section
//
//
// --- Large screens with Q film
//
void Pervasive_BWRY_Large::COG_reset()
{
    // Application note § 2. Power on COG driver
    b_reset(20, 10, 20, 10, 10); // BWRY specific
    b_waitBusy(); // BWRY specific
}

void Pervasive_BWRY_Large::COG_getDataOTP()
{
    hV_HAL_SPI_end(); // With unicity check
    hV_HAL_SPI3_begin(); // Define 3-wire SPI pins

    // 1.6 Read OTP memory mapping data
    uint16_t _chipId;
    uint16_t _readBytes = 0;
    u_flagOTP = false;

    _chipId = 0x0d04;
    _readBytes = 112;

    // GPIO
    hV_HAL_GPIO_set(b_pin.panelReset);

    // Check
    hV_HAL_GPIO_clear(b_pin.panelDC); // Command
    hV_HAL_GPIO_clear(b_pin.panelCS); // Select
    hV_HAL_SPI3_write(0x70);
    hV_HAL_GPIO_set(b_pin.panelCS); // Unselect
    hV_HAL_delayMilliseconds(8); // Added

    uint16_t ui16 = 0;
    hV_HAL_GPIO_set(b_pin.panelDC); // Data
    hV_HAL_GPIO_clear(b_pin.panelCS); // Select
    ui16 = hV_HAL_SPI3_read();
    hV_HAL_GPIO_set(b_pin.panelCS); // Unselect
    ui16 <<= 8;
    hV_HAL_GPIO_set(b_pin.panelDC); // Data
    hV_HAL_GPIO_clear(b_pin.panelCS); // Select
    ui16 |= hV_HAL_SPI3_read();
    hV_HAL_GPIO_set(b_pin.panelCS); // Unselect
    Serial.println(ui16, HEX);
    hV_HAL_Serial_crlf();
    if (ui16 == _chipId)
    {
        hV_HAL_log(LEVEL_INFO, "OTP check 1 passed - Chip ID %04x as expected", ui16);
    }
    else
    {
        hV_HAL_Serial_crlf();
        hV_HAL_log(LEVEL_CRITICAL, "OTP check 1 failed - Chip ID 0x%04x, expected 0x%04x", ui16, _chipId);
        hV_HAL_exit(0x01);
    }

    // Read OTP
    uint8_t ui8 = 0;
    uint16_t offset = 0x0000;

    hV_HAL_GPIO_clear(b_pin.panelDC); // Command
    hV_HAL_GPIO_clear(b_pin.panelCS); // Select
    hV_HAL_SPI3_write(0x90);
    hV_HAL_GPIO_set(b_pin.panelCS); // Unselect
    hV_HAL_delayMilliseconds(8);

    hV_HAL_GPIO_clear(b_pin.panelDC); // Command
    hV_HAL_GPIO_clear(b_pin.panelCS); // Select
    hV_HAL_SPI3_write(0xa2);
    hV_HAL_GPIO_set(b_pin.panelCS); // Unselect

    hV_HAL_GPIO_set(b_pin.panelDC); // Data
    hV_HAL_GPIO_clear(b_pin.panelCS); // Select
    hV_HAL_SPI3_write(0x00);
    hV_HAL_GPIO_set(b_pin.panelCS); // Unselect
    hV_HAL_GPIO_clear(b_pin.panelCS); // Select
    hV_HAL_SPI3_write(0x15);
    hV_HAL_GPIO_set(b_pin.panelCS); // Unselect
    hV_HAL_GPIO_clear(b_pin.panelCS); // Select
    hV_HAL_SPI3_write(0x00);
    hV_HAL_GPIO_set(b_pin.panelCS); // Unselect
    hV_HAL_GPIO_clear(b_pin.panelCS); // Select
    hV_HAL_SPI3_write(0x00);
    hV_HAL_GPIO_set(b_pin.panelCS); // Unselect
    hV_HAL_GPIO_clear(b_pin.panelCS); // Select
    hV_HAL_SPI3_write(0xe0);
    hV_HAL_GPIO_set(b_pin.panelCS); // Unselect

    hV_HAL_GPIO_clear(b_pin.panelDC); // Command
    hV_HAL_GPIO_clear(b_pin.panelCS); // Select
    hV_HAL_SPI3_write(0x92);
    hV_HAL_GPIO_set(b_pin.panelCS); // Unselect

    hV_HAL_GPIO_set(b_pin.panelDC); // Data
    hV_HAL_GPIO_clear(b_pin.panelCS); // Select
    ui8 = hV_HAL_SPI3_read();
    hV_HAL_GPIO_set(b_pin.panelCS); // Unselect
    // hV_HAL_log(LEVEL_DEBUG, "Dummy read 0x%02x", ui8);

    hV_HAL_GPIO_clear(b_pin.panelCS); // Select
    COG_data[0] = hV_HAL_SPI3_read(); // First byte for check
    hV_HAL_GPIO_set(b_pin.panelCS); // Unselect
    Serial.println(COG_data[0], HEX);
    // Check table start and set bank offset
    if (COG_data[0] == 0xa5)
    {
        offset = 0x00;
    }
    else
    {
        offset = 0x70;
        for (uint16_t index = 1; index < offset; index += 1)
        {
            hV_HAL_GPIO_clear(b_pin.panelCS); // Select
            hV_HAL_SPI3_read(); // Ignore bytes 1..offset
            hV_HAL_GPIO_set(b_pin.panelCS); // Unselect
        }

        hV_HAL_GPIO_clear(b_pin.panelCS); // Select
        COG_data[0] = hV_HAL_SPI3_read(); // First byte for check
        hV_HAL_GPIO_set(b_pin.panelCS); // Unselect

        if (COG_data[0] == 0xa5) // First byte check = 0xa5
        {
            offset = 0x00;
        }
        else
        {
            hV_HAL_Serial_crlf();
            hV_HAL_log(LEVEL_CRITICAL, "OTP check 2 failed - Bank %i, first 0x%02x, expected 0x%02x", 0, COG_data[0], 0xa5);
            hV_HAL_exit(0x01);
        }
    }
    hV_HAL_log(LEVEL_INFO, "OTP check 2 passed - Bank %i, first 0x%02x as expected", (offset > 0x00), COG_data[0]);

    // Populate COG_data
    for (uint16_t index = 1; index < _readBytes; index += 1)
    {
        digitalWrite(b_pin.panelCS, LOW); // Select
        COG_data[index] = hV_HAL_SPI3_read(); // Read OTP
        Serial.println(COG_data[index], HEX);
        digitalWrite(b_pin.panelCS, HIGH); // Unselect
    }

    hV_HAL_SPI3_end();
    u_flagOTP = true;

#if (DEBUG_OTP == 1) // Debug COG_data
    debugOTP(COG_data, _readBytes, COG_BWRY_LARGE, SCREEN_DRIVER(u_eScreen_EPD));
#endif // DEBUG_OTP
}

void Pervasive_BWRY_Large::COG_initial()
{
    b_sendCommandDataSelect8(0xe6, u_temperature, PANEL_CS_BOTH); //	25 C temp
    b_sendCommandDataSelect8(0xe0, 0x02, PANEL_CS_BOTH); //

    switch (u_eScreen_EPD)
    {
        case eScreen_EPD_969_QS_0B:

            b_sendCommandSelect8(0xa5, PANEL_CS_BOTH); //
            b_waitBusy();
            break;

        default:

            break;
    }

    // Application note § 3. COG initial
    // Work settings
    b_sendCommandDataSelect8(0x01, COG_data[16], PANEL_CS_BOTH); // PWR
    b_sendIndexDataSelect(0x00, &COG_data[26], 3, PANEL_CS_BOTH); // PSR
    b_sendIndexDataSelect(0x61, &COG_data[19], 4, PANEL_CS_BOTH); // TRES
    b_sendIndexDataSelect(0x61, &COG_data[19], 4, PANEL_CS_BOTH); // TRES
    uint8_t temphold[3] = {COG_data[17], COG_data[18], COG_data[29]};
    b_sendIndexDataSelect(0x00, &temphold[0], 3, PANEL_CS_BOTH); // PSR
    b_sendIndexDataSelect(0x06, &COG_data[23], 3, PANEL_CS_BOTH); // BTST_P
    b_sendIndexDataSelect(0x03, &COG_data[30], 3, PANEL_CS_BOTH); // PFS
    b_sendCommandDataSelect8(0xe7, COG_data[33], PANEL_CS_BOTH); // PFS
    b_sendIndexDataSelect(0x65, &COG_data[34], 4, PANEL_CS_BOTH); // TRES
    b_sendCommandDataSelect8(0x30, COG_data[38], PANEL_CS_BOTH); // PLL
    b_sendCommandDataSelect8(0x50, COG_data[39], PANEL_CS_BOTH); // CDI
    b_sendIndexDataSelect(0x60, &COG_data[40], 2, PANEL_CS_BOTH); // TCON
    b_sendCommandDataSelect8(0xe3, COG_data[42], PANEL_CS_BOTH); // PWS
    b_sendCommandDataSelect8(0xff, 0xa5, PANEL_CS_BOTH); //
    b_sendIndexDataSelect(0xef, &COG_data[43], 8, PANEL_CS_BOTH); // TRES

    b_sendCommandDataSelect8(0xdc, COG_data[59], PANEL_CS_BOTH); // CDI
    b_sendCommandDataSelect8(0xdd, COG_data[60], PANEL_CS_BOTH); // CDI
    b_sendCommandDataSelect8(0xde, COG_data[61], PANEL_CS_BOTH); // CDI
    b_sendCommandDataSelect8(0xe8, COG_data[62], PANEL_CS_BOTH); // CDI
    b_sendCommandDataSelect8(0xda, COG_data[63], PANEL_CS_BOTH); // CDI
    b_sendCommandDataSelect8(0xff, 0xe3, PANEL_CS_BOTH); //
    b_sendCommandDataSelect8(0xe9, 0x01, PANEL_CS_BOTH); //
}

void Pervasive_BWRY_Large::COG_sendImageDataNormal(FRAMEBUFFER_CONST_TYPE masterFrame, FRAMEBUFFER_CONST_TYPE slaveFrame, uint32_t sizeFrame) // First frame, blackBuffer
{
    // Application note § 4. Input image to the EPD
    b_sendIndexDataSelect(0x10, masterFrame, sizeFrame, PANEL_CS_MASTER); // First frame, blackBuffer

    b_sendIndexDataSelect(0x10, slaveFrame, sizeFrame, PANEL_CS_SLAVE); // First frame, blackBuffer
}

void Pervasive_BWRY_Large::COG_update()
{

    b_sendCommandSelect8(0x04, PANEL_CS_BOTH); // Power on
    b_waitBusy();

    b_sendCommandDataSelect8(0x12, 0x00, PANEL_CS_BOTH); // Display Refresh
    b_waitBusy();
}

void Pervasive_BWRY_Large::COG_stopDCDC()
{
    // Application note § 5. Turn-off DC/DC
    b_sendCommandDataSelect8(0x02, 0x00, PANEL_CS_BOTH); // Turn off DC/DC
    b_waitBusy();

    uint8_t COG_temp[3] = {0x07, 0x2b, 0x01};
    b_sendIndexDataSelect(0x00, &COG_temp[0], 3, PANEL_CS_BOTH); // PSR

    hV_HAL_delayMilliseconds(400);

    b_sendCommandDataSelect8(0xff, 0xa5, PANEL_CS_BOTH); // Turn off DC/DC
    uint8_t COG_temp2[2] = {0xa0, 0x1e};
    b_sendIndexDataSelect(0xee, &COG_temp2[0], 2, PANEL_CS_BOTH); // PSR

    hV_HAL_delayMilliseconds(4);
    uint8_t COG_temp3[2] = {0x00, 0x00};
    b_sendIndexDataSelect(0xee, &COG_temp3[0], 2, PANEL_CS_BOTH); // PSR

    hV_HAL_delayMilliseconds(3);
    b_sendCommandDataSelect8(0xff, 0xe3, PANEL_CS_BOTH); // Turn off DC/DC
    hV_HAL_delayMilliseconds(6);
}
//
// --- End of Large screens with Q film
//
/// @endcond
//
// === End of COG section
//

Pervasive_BWRY_Large::Pervasive_BWRY_Large(eScreen_EPD_t eScreen_EPD, pins_t board)
{
    d_COG = COG_BWRY_LARGE;
    u_eScreen_EPD = eScreen_EPD;
    b_pin = board;
}

void Pervasive_BWRY_Large::begin()
{
    b_begin(b_pin, FAMILY_LARGE, b_delayCS);
    b_resume(); // GPIO

    COG_reset(); // Reset
    COG_getDataOTP(); // 3-wire SPI read OTP memory
}

STRING_CONST_TYPE Pervasive_BWRY_Large::reference()
{
    return formatString("%s v%i.%i.%i", DRIVER_EPD_VARIANT, DRIVER_EPD_RELEASE / 100, (DRIVER_EPD_RELEASE / 10) % 10, DRIVER_EPD_RELEASE % 10);
}

void Pervasive_BWRY_Large::updateNormal(FRAMEBUFFER_CONST_TYPE masterframe, FRAMEBUFFER_CONST_TYPE slaveframe, uint32_t sizeFrame)
{
    b_resume(); // GPIO
    COG_reset(); // Reset

    if (u_flagOTP == false)
    {
        COG_getDataOTP(); // 3-wire SPI read OTP memory
        COG_reset(); // Reset
    }

    // Start SPI
    hV_HAL_SPI_begin(16000000); // Fast 16 MHz, with unicity check

    COG_initial(); // Initialise
    COG_sendImageDataNormal(masterframe, slaveframe, sizeFrame);

    COG_update(); // Update
    COG_stopDCDC(); // Power DC/DC off
}

