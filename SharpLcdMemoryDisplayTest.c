//*****************************************************************************
//
// SharpLcdMemoryDisplayTest.c - Use the Sharp LCD booster pack to display
// memory for debugging (or other purposes).
//
//     Memory use is minimized by limiting the characters in the font and not
//     supporting general purpose graphics.
//
// Copyright (c) 2015 Donald Bryant-Rich.
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

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"

// Includes needed to only get the data to write.
#include "inc/hw_memmap.h"		// Only required by use of GPIO_PORTE_BASE;
#include "driverlib/gpio.h" 	// Only required by use of GPIOPinRead.
#include "driverlib/systick.h"	// Only required by use of SysTickValueGet.

#include "SharpLcdMemoryDisplay.h"

void clockInit(void)
{
    // Set the clocking to 20Mhz (400Mhz PLL / 2 default divider / 10).
	// Clock must be less than 32Mhz for use with SharpLcdMemoryDisplay.
	// The limitation is imposed by the use of SysTick for VCOM inversion
	// timing.
	SysCtlClockSet(SYSCTL_SYSDIV_10 |SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
		SYSCTL_XTAL_16MHZ);
}

int
main(void)
{
    // Set the clock to 20Mhz.
	clockInit();

	Sharp96x96_InitDisplay();

	Sharp96x96_ClearScreen();

	// Sample values
	Sharp96x96_WriteWord(0, 0x2345ABCD);
	Sharp96x96_WriteWord(1, 0x00000000);
	Sharp96x96_WriteWord(2, 0xDEADBEEF);
	Sharp96x96_WriteWord(3, 0xA5503255);
	Sharp96x96_WriteWord(4, 0xABECADEF);
	Sharp96x96_WriteWord(5, 0x01234567);
	Sharp96x96_WriteWord(6, 0x89ABCDEF);
	Sharp96x96_WriteWord(7, 0x00000000);

	static unsigned long counter = 0;

    while(1) // Loop forever.
    {
    	if(Sharp96x96_VCOM_Count >= 2) // Update each second.
    	{
    		Sharp96x96_VCOM_Count = 0;
    		Sharp96x96_WriteWord(0, SysTickValueGet());
    		 // Port with LCD CS
    		Sharp96x96_WriteWord(4, GPIOPinRead(GPIO_PORTE_BASE, 0xFF));
    		Sharp96x96_WriteWord(7, ++counter);
    	}
    }
}
