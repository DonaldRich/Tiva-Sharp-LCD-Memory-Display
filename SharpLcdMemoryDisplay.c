//
// SharpLcdMemoryDisplay.c - Use the Sharp LCD booster pack to display memory
// for debugging (or other purposes).
//
//     Memory is minimized by limiting the characters in the font and not
//     supporting general purpose graphics.
//
// Copyright (c) 2015 Donald Rich.
//
// This software is supplied solely as a programming example.
//
// This file is part of SharpLcdMemoryDisplay.
//
// SharpLcdMemoryDisplay is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SharpLcdMemoryDisplay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SharpLcdMemoryDisplay.  If not, see
// <http://www.gnu.org/licenses/>.
//
//*****************************************************************************

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/systick.h"

#include "SharpLcdMemoryDisplay.h"

// General use counter. Incremented each time the VCOM inversion interrupt is
// called. Not used in the Sharp96x96 code. Can be reset by user code.
unsigned int Sharp96x96_VCOM_Count = 0;

//*****************************************************************************
// The error routine that is called if the driver library encounters an error.
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

// Ports for TM4C1233H6PM connections to LCD
#define LCD_SPI_SI_PORT                     SYSCTL_PERIPH_GPIOB
#define LCD_SPI_CLK_PORT                    SYSCTL_PERIPH_GPIOB
#define LCD_DISP_PORT                       SYSCTL_PERIPH_GPIOE
#define LCD_POWER_PORT                      SYSCTL_PERIPH_GPIOB
#define LCD_SPI_CS_PORT     	            SYSCTL_PERIPH_GPIOE

// Port base address for TM4C1233H6PM connections to LCD
#define LCD_SPI_SI_PORT_BASE                GPIO_PORTB_BASE
#define LCD_SPI_CLK_PORT_BASE               GPIO_PORTB_BASE
#define LCD_DISP_PORT_BASE                  GPIO_PORTE_BASE
#define LCD_POWER_PORT_BASE                 GPIO_PORTB_BASE
#define LCD_SPI_CS_PORT_BASE	            GPIO_PORTE_BASE

// Pins for TM4C1233H6PM connections to LCD
#define LCD_SPI_SI_PIN                      GPIO_PIN_7
#define LCD_SPI_CLK_PIN                     GPIO_PIN_4
#define LCD_DISP_PIN                        GPIO_PIN_4
#define LCD_POWER_PIN                       GPIO_PIN_5
#define LCD_SPI_CS_PIN                      GPIO_PIN_5

// Definition of SSI base address to be used for SPI communication
#define LCD_SSI_BASE						SSI2_BASE

// Delay counts used to enforce SPI timing constraints.
static unsigned long twoUsDelayCount;
static unsigned long sixUsDelayCount;

// generated from Lucida Console using The Dot Factory:s
// http://www.eran.io/the-dot-factory-an-lcd-font-and-image-generator/
static const unsigned char bitmaps[] =
{
	// @9 '0' (6 pixels wide)
	0x30, //   ##
	0x48, //  #  #
	0x84, // #    #
	0x84, // #    #
	0x84, // #    #
	0x84, // #    #
	0x84, // #    #
	0x48, //  #  #
	0x78, //  ####

	// @18 '1' (7 pixels wide)
	0x30, //   ##
	0xD0, // ## #
	0x10, //    #
	0x10, //    #
	0x10, //    #
	0x10, //    #
	0x10, //    #
	0x10, //    #
	0xFE, // #######5

	// @27 '2' (5 pixels wide)
	0xF0, // ####
	0x08, //     #
	0x08, //     #
	0x08, //     #
	0x10, //    #
	0x20, //   #
	0x40, //  #
	0x80, // #
	0xF8, // #####

	// @36 '3' (5 pixels wide)
	0xF0, // ####
	0x08, //     #
	0x08, //     #
	0x10, //    #
	0x60, //  ##
	0x18, //    ##
	0x08, //     #
	0x08, //     #
	0xF0, // ####

	// @45 '4' (6 pixels wide)
	0x08, //     #
	0x18, //    ##
	0x28, //   # #
	0x28, //   # #
	0x48, //  #  #
	0x88, // #   #
	0xFC, // ######
	0x08, //     #
	0x08, //     #

	// @54 '5' (5 pixels wide)
	0xFC, // ######
	0x80, // #
	0x80, // #
	0xF8, // #####
	0x04, //      #
	0x04, //      #
	0x04, //      #
	0x04, //      #
	0xF8, // #####

	// @63 '6' (6 pixels wide)
	0x38, //   ###
	0x40, //  #
	0x80, // #
	0xB8, // # ###
	0xC4, // ##   #
	0x84, // #    #
	0x84, // #    #
	0x44, //  #   #
	0x38, //   ###

	// @72 '7' (6 pixels wide)
	0xFC, // ######
	0x04, //      #
	0x08, //     #
	0x10, //    #
	0x10, //    #
	0x20, //   #
	0x20, //   #
	0x40, //  #
	0x40, //  #

	// @81 '8' (6 pixels wide)
	0x78, //  ####
	0x84, // #    #
	0x84, // #    #
	0x48, //  #  #
	0x78, //  ####
	0x8C, // #   ##
	0x84, // #    #
	0x84, // #    #
	0x78, //  ####

	// @90 '9' (6 pixels wide)
	0x78, //  ####
	0x84, // #    #
	0x84, // #    #
	0x84, // #    #
	0x7C, //  #####
	0x04, //      #
	0x04, //      #
	0x08, //     #
	0x70, //  ###

	// @108 'A' (7 pixels wide)
	0x00, //
	0x30, //   ##
	0x30, //   ##
	0x48, //  #  #
	0x48, //  #  #
	0x48, //  #  #
	0xFC, // ######
	0x84, // #    #
	0x84, // #    #

	// @117 'B' (7 pixels wide)
	0x00, //
	0xFC, // ######
	0x82, // #     #
	0x82, // #     #
	0xFC, // ######
	0x82, // #     #
	0x82, // #     #
	0x82, // #     #
	0xFC, // ######

	// @126 'C' (7 pixels wide)
	0x00, //
	0x3E, //   #####
	0x40, //  #
	0x80, // #
	0x80, // #
	0x80, // #
	0x80, // #
	0x40, //  #
	0x3E, //   #####

	// @135 'D' (7 pixels wide)
	0x00, //
	0xF8, // #####
	0x84, // #    #
	0x82, // #     #
	0x82, // #     #
	0x82, // #     #
	0x82, // #     #
	0x84, // #    #
	0xF8, // #####

	// @144 'E' (6 pixels wide)
	0x00, //
	0xFC, // ######
	0x80, // #
	0x80, // #
	0x80, // #
	0xF8, // #####
	0x80, // #
	0x80, // #
	0xFC, // ######

	// @153 'F' (6 pixels wide)
	0x00, //
	0xFC, // ######
	0x80, // #
	0x80, // #
	0x80, // #
	0xF8, // #####
	0x80, // #
	0x80, // #
	0x80, // #

	// @99 ':' (2 pixels wide)
	0x00, //
	0x00, //
	0xC0, // ##
	0xC0, // ##
	0x00, //
	0x00, //
	0x00, //
	0xC0, // ##
	0xC0, // ##

	// @0 '-' (6 pixels wide)
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0x00, //
	0xFC, // ######
	0x00, //
	0x00, //
	0x00, //
};

static void
ClearCS(void)
{
	GPIOPinWrite(LCD_SPI_CS_PORT_BASE, LCD_SPI_CS_PIN, 0);
}

static void
SetCS(void)
{
	GPIOPinWrite(LCD_SPI_CS_PORT_BASE, LCD_SPI_CS_PIN, LCD_SPI_CS_PIN);
}

static long
GetCS(void)
{
	return GPIOPinRead(LCD_SPI_CS_PORT_BASE, LCD_SPI_CS_PIN);
}

//*******************************************************************************
//
//! Reverses the bit order.- Since the bit reversal function is called
//! frequently by several driver functions this function is implemented
//! to maximize code execution
// Taken from TI Sharp96x96.c in EXP430FR5969 Sharp LCD sample code.
//
//*******************************************************************************
static const unsigned char reverse_data[] = {
		0x0, 0x8, 0x4, 0xC,
		0x2, 0xA, 0x6, 0xE,
		0x1, 0x9, 0x5, 0xD,
		0x3, 0xB, 0x7, 0xF
};

static unsigned char
Reverse(unsigned char x)
{
  unsigned char b = 0;

  b  = reverse_data[x & 0xF]<<4;
  b |= reverse_data[(x & 0xF0)>>4];
  return b;
}

static void
WaitForWriteFinish(void)
{
	while (SSIBusy(LCD_SSI_BASE))
	{
		// Just wait
	}
}

static void
WriteByte(unsigned char byte)
{
	WaitForWriteFinish();
	SSIDataPut(LCD_SSI_BASE, byte);
}

#define BLACK							0x00
#define WHITE							0xFF
#define SHARP_LCD_TRAILER_BYTE				0x00
#define SHARP_LCD_CMD_CHANGE_VCOM			0x00
#define SHARP_LCD_CMD_CLEAR_SCREEN			0x20
#define SHARP_LCD_CMD_WRITE_LINE			0x80

static unsigned char
WriteLineCommand[] = {
	SHARP_LCD_CMD_WRITE_LINE,
	0x0, // address (must be in reversed bit order)
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // pixels
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, // pixels
	SHARP_LCD_TRAILER_BYTE, SHARP_LCD_TRAILER_BYTE
};

static unsigned char
ToggleVComCommand[] = {
	SHARP_LCD_CMD_CHANGE_VCOM,
	SHARP_LCD_TRAILER_BYTE
};

static unsigned char
ClearCommand[] = {
	SHARP_LCD_CMD_CLEAR_SCREEN,
	SHARP_LCD_TRAILER_BYTE
};

static unsigned char VCOMbit = 0x40;
static unsigned char currentVCOMbit= 0x40;

static void
WriteCommand(unsigned char *command, unsigned char length)
{
	int i;
	unsigned char * bytePointer = command;

	// Set the VCOM bit in the command byte
	command[0] &= ~VCOMbit;
	command[0] |= currentVCOMbit;

	SetCS();

	// Ensure a 6us min delay to meet the LCD's tsSCS
	SysCtlDelay(sixUsDelayCount);

	for(i = 0; i < length; i++)
	{
		WriteByte(*bytePointer++);
	}

	// Wait for last byte to be sent
	WaitForWriteFinish();

	// Ensure a 2us min delay to meet the LCD's thSCS
	SysCtlDelay(twoUsDelayCount);

	ClearCS();

	// Ensure a 2us delay to meet the LCD's twSCSL
	SysCtlDelay(twoUsDelayCount);
}

static void
SysTickHandler(void)
{
	Sharp96x96_VCOM_Count++; // Used for secondary timing functions unrelated to the Sharp LCD.

	// Toggle the VCOM bit.
	currentVCOMbit ^= VCOMbit;

	if(!GetCS()) // CS is low, not already sending command
	{
		WriteCommand(ToggleVComCommand, 2);
	}
}

static void
VCOMtimerInit(void)
{
    // Enable the timer for periodic interrupts to drive the VCOM inversion
	// and other general timed functions.
	// The clock is configured to timeout every half second to give a 1 hz
	// alternation of VCOM.
	SysTickPeriodSet(SysCtlClockGet() / 2); // must be less than 16,777,216
	SysTickIntRegister(SysTickHandler);
	SysTickIntEnable();
	SysTickEnable();
}

static void
SetLcdLineCommand(unsigned char value, unsigned char charIndex, unsigned char row)
{
	WriteLineCommand[charIndex + 2] = ~(bitmaps[value * 9 + row]);
}

static void
WriteLine (unsigned char line)
{
	unsigned char oldVCOM = currentVCOMbit;

	WriteLineCommand[1] = Reverse(line); // Address in reverse bit order.
	WriteCommand(WriteLineCommand, 16);

	// Sharp96x96_SendToggleVCOMCommand called while sending the command.
	if(oldVCOM != currentVCOMbit)
	{
		WriteCommand(ToggleVComCommand, 2);
	}
}

void
Sharp96x96_InitDisplay(void)
{
	// Clock frequency / 3 (cycles per Delay tick) / 500000 (2 uS periods in a second)
	twoUsDelayCount = SysCtlClockGet() / 1500000 + 1;
	sixUsDelayCount = SysCtlClockGet() / 500000 + 1;

	// Configure the GPIO Port B pin 5 for output to power the LCD Display.
    SysCtlPeripheralEnable(LCD_POWER_PORT); // Output by default.
    GPIOPinTypeGPIOOutput(LCD_POWER_PORT_BASE, LCD_POWER_PIN);
    GPIOPinWrite(LCD_POWER_PORT_BASE, LCD_POWER_PIN, LCD_POWER_PIN);

    // Configure the GPIO Port E pin 4 for output to enable the LCD Display.
    SysCtlPeripheralEnable(LCD_DISP_PORT); // Output by default.
    GPIOPinTypeGPIOOutput(LCD_DISP_PORT_BASE, LCD_DISP_PIN);
    GPIOPinWrite(LCD_DISP_PORT_BASE, LCD_DISP_PIN, LCD_DISP_PIN);

    // Configure the GPIO Port E pin 5 for SPI Chip Select for the LCD Display.
    GPIOPinTypeGPIOOutput(LCD_SPI_CS_PORT_BASE, LCD_SPI_CS_PIN);
    GPIOPinWrite(LCD_SPI_CS_PORT_BASE, LCD_SPI_CS_PIN, 0); // Output by default.

	ClearCS();

	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
	SSIClockSourceSet(LCD_SSI_BASE, SSI_CLOCK_SYSTEM);
	SSIConfigSetExpClk(LCD_SSI_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
			SSI_MODE_MASTER, 1000000, 8);
    GPIOPinConfigure(GPIO_PB7_SSI2TX);
    GPIOPinConfigure(GPIO_PB4_SSI2CLK);
	// Split if SPI_SI and SPI_CLK are on different GPIO ports.
    GPIOPinTypeGPIOOutput(LCD_SPI_SI_PORT_BASE, LCD_SPI_SI_PIN | LCD_SPI_CLK_PIN);
	GPIOPinTypeSSI(LCD_SPI_SI_PORT_BASE, LCD_SPI_SI_PIN | LCD_SPI_CLK_PIN);
    SSIEnable(LCD_SSI_BASE);

	VCOMtimerInit();
}

void
Sharp96x96_ClearScreen (void)
{
	unsigned char oldVCOM = currentVCOMbit;

	WriteCommand(ClearCommand, 2);

	// SendToggleVCOMCommand called while sending the command.
	if(oldVCOM != currentVCOMbit)
	{
		WriteCommand(ToggleVComCommand, 2);
	}
}

// Line must be between 0 and 7 inclusive.
void
Sharp96x96_WriteWord(unsigned char line, unsigned long word)
{
	line = line & 0x7;
	unsigned char nibble;
	unsigned char offset = line * 11 + 4;
	unsigned char row;
	for(row = 0; row < 9; row++)
	{
		// Line index
		WriteLineCommand[2] = ~(bitmaps[line * 9 + row]);
		// Colon
		WriteLineCommand[3] = ~(bitmaps[16 * 9 + row]);
		unsigned int top = word >> 16;
		unsigned int bottom = word & 0xFFFF;
		for(nibble = 0; nibble < 4; nibble++)
		{
			SetLcdLineCommand(top & 0xF, 5 - nibble, row);
			SetLcdLineCommand(bottom & 0xF, 10 - nibble, row);
			top >>= 4;
			bottom >>= 4;
		}
		SetLcdLineCommand(17, 6, row); // Dash
		WriteLineCommand[13] = WHITE;
		WriteLine(row + offset);
	}
	unsigned char charIndex;
	for(charIndex = 0; charIndex < 12; charIndex++)
	{
		WriteLineCommand[charIndex + 2] = WHITE;
	}
	WriteLine(9 + offset);
	WriteLine(10 + offset);
}
