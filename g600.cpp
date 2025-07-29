#include <array>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <map>
#include <string>
#include <string_view>
#include <unistd.h>

using namespace std;

array<input_event, 64> events;
constexpr auto kDir = "/dev/input/by-id/";
constexpr auto kPrefix = "usb-Logitech_Gaming_Mouse_G600_";
constexpr auto kSuffix = "-if01-event-kbd";

// ADD KEY->COMMAND MAPPINGS HERE:
map<int, string> down_commands = {
    //[scancode] = "command to run",
    {4, "xdotool key Page_Up"},                   // scroll left
    {5, "xdotool key Page_Down"},                 // scroll right
    {6, "xdotool key ctrl+c"},                    // G8
    {7, "xdotool key ctrl+shift+c"},              // G7
    {8, "i3-msg workspace next_on_output"},       // G9
    {9, "i3-msg move workspace next_on_output"},  // G10
    {10, "xdotool key ctrl+w"},                   // G11
    {11, "pulseaudio-ctl down"},                  // G12
    {12, "pulseaudio-ctl mute"},                  // G13
    {13, "xdotool key ctrl+z"},                   // G14
    {14, "xdotool key End"},                      // G15
    {15, "xdotool key ctrl+End"},                 // G16
    {16, "xdotool key Return"},                   // G17
    {17, "i3-msg fullscreen"},                    // G18
    {18, "xdotool key ctrl+slash t"},             // G19
    {19, ""},                                     // G20
    {20, "xdotool key alt+Left"},                 // G-shift + scroll left
    {21, "xdotool key alt+Right"},                // G-shift + scroll right
    {22, "xdotool key ctrl+v"},                   // G-shift + G8
    {23, "xdotool key ctrl+shift+v"},             // G-shift + G7
    {24, "i3-msg workspace prev_on_output"},      // G-shift + G9
    {25, "i3-msg move workspace prev_on_output"}, // G-shift + G10
    {26, "i3-msg kill"},                          // G-shift + G11
    {27, "pulseaudio-ctl up"},                    // G-shift + G12
    {28, "pulseaudio-ctl mute"},                  // G-shift + G13
    {29, "xdotool key ctrl+shift+z ctrl+y"},      // G-shift + G14
    {30, "xdotool key Home"},                     // G-shift + G15
    {31, "xdotool key ctrl+Home"},                // G-shift + G16
    {32, "xdotool key Escape"},                   // G-shift + G17
    {33, "i3-msg fullscreen"},                    // G-shift + G18
    {34, ""},                                     // G-shift + G19
    {35, ""},                                     // G-shift + G20
    {37, "echo button down"}};
map<int, string> up_commands = {
    //[scancode] = "command to run",
    {37, "echo button up"}};

static bool StartsWith(string_view haystack, string_view prefix) {
  if (haystack.size() < prefix.size())
    return false;
  return haystack.substr(0, prefix.size()) == prefix;
}

static bool EndsWith(string_view haystack, string_view suffix) {
  if (haystack.size() < suffix.size())
    return false;
  return haystack.substr(haystack.size() - suffix.size()) == suffix;
}

enum class FindG600Result {
  Success,
  DirectoryNotFound,
  DeviceNotFound,
  PermissionDenied,
};

FindG600Result FindG600(string &out_path) {
  //*path = kDir;
  DIR *dir;
  struct dirent *ent;
  if (!(dir = opendir(kDir))) {
    return FindG600Result::DirectoryNotFound;
  }
  while ((ent = readdir(dir))) {
    string_view d_name(ent->d_name);
    if (StartsWith(ent->d_name, kPrefix) && EndsWith(ent->d_name, kSuffix)) {
      out_path = kDir;
      out_path += d_name;

      printf("full path is %s\n", out_path.c_str());

      closedir(dir);
      return FindG600Result::Success;
    }
  }
  closedir(dir);
  return FindG600Result::DeviceNotFound;
}

int main() {
  printf("Starting G600 Linux controller.\n\n");
  printf("It's a good idea to configure G600 with Logitech Gaming Software "
         "before running this program:\n");
  printf(" - assign left, right, middle mouse button and vertical mouse wheel "
         "to their normal functions\n");
  printf(" - assign the G-Shift button to \"G-Shift\"\n");
  printf(" - assign all other keys (including horizontal mouse wheel) to "
         "arbitrary (unique) keyboard keys\n");
  printf("\n");
  string path;
  auto find_error = FindG600(path);
  if (find_error != FindG600Result::Success) {
    printf("Error: Couldn't find G600 input device.\n");
    switch (find_error) {
    case FindG600Result::DirectoryNotFound:
      printf(
          "Suggestion: Maybe the expected directory (%s) is wrong. Check "
          "whether this directory exists and fix it by editing \"g600.c\".\n",
          kDir);
      break;
    case FindG600Result::DeviceNotFound:
      printf("Suggestion: Maybe the expected device prefix (%s) is wrong. "
             "Check whether a device with this prefix exists in %s and fix it "
             "by editing \"g600.cpp\".\n",
             kPrefix, kDir);
      break;
    default:
      break;
    }
    printf("Suggestion: Maybe a permission is missing. Try running this "
           "program with with sudo.\n");
    return 1;
  }
  int fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) {
    printf("Error: Couldn't open \"%s\" for reading.\n", path.c_str());
    printf("Reason: %s.\n", strerror(errno));
    printf("Suggestion: Maybe a permission is missing. Try running this "
           "program with with sudo.\n");
    return 1;
  }

  ioctl(fd, EVIOCGRAB, 1);
  printf("G600 controller started successfully.\n\n");
  while (1) {
    size_t n = read(fd, events.data(), sizeof(events));
    if (n <= 0)
      return 2;
    if (n < sizeof(struct input_event) * 2)
      continue;
    if (events[0].type != 4)
      continue;
    if (events[0].code != 4)
      continue;
    if (events[1].type != 1)
      continue;
    int pressed = events[1].value;
    int scancode = events[0].value & ~0x70000;

    const char *actionStr = (pressed) ? "Pressed" : "Released";
    printf("%s scancode %d.", actionStr, scancode);

    string downCommand = down_commands[scancode],
           upCommand = up_commands[scancode];
    string_view commandToRun = (pressed) ? downCommand : upCommand;
    if (commandToRun.empty()) {
      printf(" No command configured for this scancode.\n");
      continue;
    }

    printf("Executing: \"%s\"\n", commandToRun.data());
    system(commandToRun.data());
    printf("\n");
  }

  close(fd);
}
