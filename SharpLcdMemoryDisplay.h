//*****************************************************************************
//
// SharpLcdMemoryDisplay.h - Use the Sharp LCD booster pack to display memory
// for debugging (or other purposes).
//
//     Memory use is minimized by limiting the characters in the font and not
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

#ifndef SHARPLCDMEMORYDISPLAYX_SHARPLCDMEMORYDISPLAY_H_
#define SHARPLCDMEMORYDISPLAYX_SHARPLCDMEMORYDISPLAY_H_

// A counter driven at 2Hz by the VCOM inversion interrupt. Not otherwise
// used in the LCD code but provided for use in timing output writes.
extern unsigned int Sharp96x96_VCOM_Count;

// Initialize the display interface. Must be done before using the LCD.
void Sharp96x96_InitDisplay(void);

// Clear the screen. Should be done to clear random bits before writing.
void Sharp96x96_ClearScreen (void);

// Display the value for word in hex at one of 8 lines on the LCD.
// The line parameter is masked to a value between 0 and 7 inclusive.
void Sharp96x96_WriteWord(unsigned char line, unsigned long word);

#endif /* SHARPLCDMEMORYDISPLAYX_SHARPLCDMEMORYDISPLAY_H_ */
