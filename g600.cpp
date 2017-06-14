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

struct Command {
  const int scancode;
  const char* command;
};

// ADD KEY->COMMAND MAPPINGS HERE:
const Command kCommands[] = {
  { 4, "xdotool key Page_Up" }, // scroll left
  { 5, "xdotool key Page_Down" }, // scroll right
  { 6, "xdotool key ctrl+c" }, // G8
  { 7, "xdotool key ctrl+shift+c" }, // G7
  { 8, "i3-msg workspace next_on_output" }, // G9
  { 9, "i3-msg move workspace next_on_output" }, // G10
  { 10, "xdotool key ctrl+w" }, // G11
  { 11, "pulseaudio-ctl down" }, // G12
  { 12, "pulseaudio-ctl mute" }, // G13
  { 13, "xdotool key ctrl+z" }, // G14
  { 14, "xdotool key End" }, // G15
  { 15, "xdotool key ctrl+End" }, // G16
  { 16, "xdotool key Return" }, // G17
  { 17, "i3-msg fullscreen" }, // G18
  { 18, "xdotool key ctrl+slash t" }, // G19
  { 19, "" }, // G20
  { 20, "xdotool key alt+Left" }, // G-shift + scroll left
  { 21, "xdotool key alt+Right" }, // G-shift + scroll right
  { 22, "xdotool key ctrl+v" }, // G-shift + G8
  { 23, "xdotool key ctrl+shift+v" }, // G-shift + G7
  { 24, "i3-msg workspace prev_on_output" }, // G-shift + G9
  { 25, "i3-msg move workspace prev_on_output" }, // G-shift + G10
  { 26, "i3-msg kill" }, // G-shift + G11
  { 27, "pulseaudio-ctl up" }, // G-shift + G12
  { 28, "pulseaudio-ctl mute" }, // G-shift + G13
  { 29, "xdotool key ctrl+shift+z ctrl+y" }, // G-shift + G14
  { 30, "xdotool key Home" }, // G-shift + G15
  { 31, "xdotool key ctrl+Home" }, // G-shift + G16
  { 32, "xdotool key Escape" }, // G-shift + G17
  { 33, "i3-msg fullscreen" }, // G-shift + G18
  { 34, "" }, // G-shift + G19
  { 35, "" }, // G-shift + G20
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
  printf("Starting G600 Linux controller.\n\n");
  printf("It's a good idea to configure G600 with Logitech Gaming Software before running this program:\n");
  printf(" - assign left, right, middle mouse button and vertical mouse wheel to their normal functions\n");
  printf(" - assign the G-Shift button to \"G-Shift\"\n");
  printf(" - assign all other keys (including horizontal mouse wheel) to arbitrary (unique) keyboard keys\n");
  printf("\n");    
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
  printf("G600 controller started successfully.\n\n");
  while (1) {
    size_t n = read(fd, events, sizeof(events));
    if (n <= 0) return 2;
    if (n < sizeof(input_event) * 2) continue;
    if (events[0].type != 4) continue;
    if (events[0].code != 4) continue;
    if (events[1].type != 1) continue;
    bool pressed = events[1].value;
    if (!pressed) continue;
    int scancode = events[0].value & ~0x70000;

    const Command* cmd = nullptr;
    for (size_t i = 0; i < sizeof(kCommands) / sizeof(Command); ++i) {
      if (kCommands[i].scancode == scancode) {
	cmd = &kCommands[i];
	break;
      }
    }

    if (cmd == nullptr) {
      printf("Warning: Pressed a key (%d) without a mapping.\n", scancode);
      printf("Suggestion: Add a mapping by editing \"g600.cpp\".\n");
      printf("\n");
      continue;
    }
    printf("Pressed scancode %d. Mapped command: \"%s\"\n", scancode, cmd->command);
    if (strlen(cmd->command) == 0) continue;
    system(cmd->command);
    printf("Subprocess finished.\n\n");
  }
  
  close(fd);
}
