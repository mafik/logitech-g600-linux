#include <linux/input.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>

input_event events[64];
const char kDir[] = "/dev/input/by-id/";
const char kPrefix[] = "usb-Logitech_Gaming_Mouse_G600_";
const char kSuffix[] = "-if01-event-kbd";
const char* kCommands[32] = {
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

bool starts_with(const char* haystack, const char* prefix) {
  auto prefix_length = strlen(prefix), haystack_length = strlen(haystack);
  if (haystack_length < prefix_length) return false;
  return strncmp(prefix, haystack, prefix_length) == 0;
}

bool ends_with(const char* haystack, const char* suffix) {
  auto suffix_length = strlen(suffix), haystack_length = strlen(haystack);
  if (haystack_length < suffix_length) return false;
  auto haystack_end = haystack + haystack_length - suffix_length;
  return strncmp(suffix, haystack_end, suffix_length) == 0;
}

// Returns non-0 on error.
int find_g600(std::string* path) {
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(kDir)) == nullptr) {
    return 1;
  }
  while ((ent = readdir(dir))) {
    if (starts_with(ent->d_name, kPrefix) && ends_with(ent->d_name, kSuffix)) {
      *path = kDir;
      *path += ent->d_name;
      closedir(dir);
      return 0;
    }
  }
  closedir(dir);
  return 2;
}

int main() {
  std::string path;
  int find_error = find_g600(&path);
  if (find_error) {
    printf("Error: Couldn't find G600 input device.\n");
    switch(find_error) {
    case 1: 
      printf("Suggestion: Maybe the expected directory (%s) is wrong. Check whether this directory exists and fix it by editing \"g600.cpp\".\n", kDir);
      break;
    case 2:
      printf("Suggestion: Maybe the expected device prefix (%s) is wrong. Check whether a device with this prefix exists in %s and fix it by editing \"g600.cpp\".\n", kPrefix, kDir);
      break;
    }
    printf("Suggestion: Maybe a permission is missing. Try running this program with with sudo.\n");
    return 1;
  }
  int fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) {
    printf("Error: Couldn't open \"%s%s\" for reading.\n", kDir, path.c_str());
    printf("Reason: %s.\n", strerror(errno));
    printf("Suggestion: Maybe a permission is missing. Try running this program with with sudo.\n");
    return 1;
  }

  ioctl(fd, EVIOCGRAB, 1);
  printf("G600 controller started successfully.\n");
  while (1) {
    size_t n = read(fd, events, sizeof(events));
    if (n <= 0) return 2;
    if (n < sizeof(input_event) * 2) continue;
    if (events[0].type != 4) continue;
    if (events[0].code != 4) continue;
    if (events[1].type != 1) continue;
    int button = events[0].value - 0x70000 - 4;

    // REMAP KEYCODES HERE

    if (button >= 32) {
      printf("Error: Pressed a keycode (%d) outside of supported range (0-31).\n", button);
      printf("Suggestion: Configure G600 with Logitech Gaming Software before running this program:\n");
      printf(" - assign left, right, middle mouse button and vertical mouse wheel to their normal functions\n");
      printf(" - assign the G-Shift button to \"G-Shift\"\n");
      printf(" - assign all other keys (including horizontal mouse wheel) to arbitrary keyboard keys\n");
      printf("Note: It's possible to modify this program to work in the current configuration. Look into \"g600.cpp\" to remap this keycode.\n");
      return 3;
    }
    bool pressed = events[1].value;
    printf("%s button %d.\n", pressed ? "Pressed" : "Released", button);
    if (!pressed) continue;
    const char* command = kCommands[button];
    if (strlen(command) == 0) continue;
    printf("Executing \"%s\"...\n", command);
    system(command);
    printf("Done!\n");
  }
  
  close(fd);
}
