#include <linux/input.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

input_event events[64];
const char path[] = "/dev/input/by-id/usb-Logitech_Gaming_Mouse_G600_DF71646D48BA0017-if01-event-kbd";
const char* commands[32] = {
  "",  // scroll left
  "",  // scroll right
  "",  // G8
  "",  // G7
  "i3-msg workspace 1",  // G9
  "i3-msg workspace 2",  // G10
  "i3-msg workspace 3",  // G11
  "",  // G12
  "",  // G13
  "i3-msg fullscreen toggle",  // G14
  "",  // G15
  "",  // G16
  "i3-msg layout toggle",  // G17
  "pulseaudio-ctl down",  // G18
  "pulseaudio-ctl mute",  // G19
  "pulseaudio-ctl up",  // G20
  "",  // G-shift + scroll left
  "",  // G-shift + scroll right
  "",  // G-shift + G8
  "",  // G-shift + G7
  "",  // G-shift + G9
  "",  // G-shift + G10
  "",  // G-shift + G11
  "",  // G-shift + G12
  "",  // G-shift + G13
  "",  // G-shift + G14
  "",  // G-shift + G15
  "",  // G-shift + G16
  "",  // G-shift + G17
  "",  // G-shift + G18
  "",  // G-shift + G19
  "",  // G-shift + G20
};

int main() {
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    printf("Error: Couldn't open %s for reading.\n", path);
    printf("Suggestion: Maybe a permission is missing. Try running this program with with sudo.\n");
    printf("Suggestion: Maybe the path is wrong. Fix it by editing \"g600.cpp\".\n");
    return 1;
  }

  ioctl(fd, EVIOCGRAB, 1);
  while (1) {
    size_t n = read(fd, events, sizeof(events));
    if (n <= 0) return 2;
    if (n < sizeof(input_event) * 2) continue;
    if (events[0].type != 4) continue;
    if (events[0].code != 4) continue;
    if (events[1].type != 1) continue;
    int button = events[0].value - 0x70000 - 4;
    bool pressed = events[1].value;
    printf("%s button %d.\n", pressed ? "Pressed" : "Released", button);
    if (!pressed) continue;
    const char* command = commands[button];
    if (strlen(command) == 0) continue;
    printf("Executing \"%s\"...\n", command);
    system(command);
    printf("Done!\n");
  }
  
  close(fd);
}
