# Logitech G600 on Linux

Utility program for binding actions to keys on the Logitech G600 gaming mouse. Supports 16 keys and the G-shift button for a total of 32 fast shortcuts.

## Initial setup

Before running this program open the Logitech Gaming Software on a Windows or Mac OS machine. Assign the three basic mouse buttons to their standard functions. The G-shift button should be assigned to the G-shift function. All the remaining buttons (scroll left, scroll right, G7, ... G20) should be set to emulate (unique) keyboard keys (but not modifier keys).

## Usage

1. Clone this repository.
2. Open `g600.cpp` and fill in the commands for the keys.
3. Compile with `g++ g600.cpp -o g600` (or `g++ -std=c++17 g600.cpp -o g600` if you get errors).
4. Run with `sudo ./g600`.
5. If some key mappings are wrong, tweak the `g600.cpp` as needed and repeat from step 2.

For command ideas you can look at programs like `xdotool`, `xdo`, `pulseaudio-ctl`, `xclip`, `i3-msg`. You can also run your own scripts.

The program needs privileges to communicate with G600 so typically it'll be started with `sudo`. Alternatively (works on Ubuntu 18.04) you can force the program to run as the `input` group with:

```bash
sudo chown .input g600
sudo chmod g+s g600
```
