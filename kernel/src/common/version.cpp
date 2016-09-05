#include <version.h>

const char* getVersion() {
#if RODVIN_VERSION == 1
  // Splash screen with UTF-8 font support
  return "Rødvin Abrusco";
#elif RODVIN_VERSION == 2
STILL TO DO:
- USB xHCI
- HID
- USB keyboard
- USB Rpi

NICE TO HAVE:
- tab completion
- Ext2 support
- VFS with multiple FS support
- IME for non-ascii inputs (compose key?)
  // Text input and shell with cd/ls/cat/echo commands. Only read-only FS support.
  return "Rødvin Bordeaux";
#elif RODVIN_VERSION == 3
  // Basic UI with touchscreen (multitouch) and PS2+USB mouse support
  return "Rødvin Cabernet";
#elif RODVIN_VERSION == 4
  // Write support for filesystems and cp/mv/del/vim commands (including external application running).
  return "Rødvin Dornfelder";
#elif RODVIN_VERSION == 5
  // Can compile apps on OS itself, and can self-compile itself on itself.
  return "Rødvin Espadeiro";
#elif RODVIN_VERSION == 6
  // Have a nice IDE to create apps for Rødvin itself.
  return "Rødvin Fortana";
#elif RODVIN_VERSION == 7
  // Have a TCP/IP stack and a basic browser that works for clean pages.
  return "Rødvin Gamaret";
#elif RODVIN_VERSION == 8
  // Undefined.
  return "Rødvin Helfensteiner";
#else
  // Not yet doing the next target :-)
  return "Rødvin Pre-Alpha";
#endif
}


