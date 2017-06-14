# Logitech G600 on Linux

Utility program for binding actions to keys on the Logitech G600 gaming mouse. Supports 16 keys and the G-shift button for a total of 32 fast shortcuts.

Before running this program open the Logitech Gaming Software on a Windows or Mac OS machine. Assign the three basic mouse buttons to their standard functions. The G-shift button should be assigned to the G-shift function. All the remaining buttons (scroll left, scroll right, G7, ... G20) should be set to emulate (unique) keyboard keys (but not modifier keys).

## Usage

1. Clone this repository.
2. Open `g600.cpp` and fill in the commands for the keys.
3. Compile with `g++ g600.cpp -o g600`.
4. Run with `./g600`

For command ideas you can look at programs like `xdotool`, `xdo`, `pulseaudio-ctl`, `xclip`, `i3-msg`. You can also run your own scripts.
