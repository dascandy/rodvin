#include "ps2.h"
#include "Scancodes.h"
#include <stdio.h>

static ScanCode ScanCodesTable[] = {
  ScanCodes::Invalid, // 0x00
  ScanCodes::F9, // 0x01
  ScanCodes::Invalid, // 0x02
  ScanCodes::F5, // 0x03
  ScanCodes::F3, // 0x04
  ScanCodes::F1, // 0x05
  ScanCodes::F2, // 0x06
  ScanCodes::F12, // 0x07
  ScanCodes::Invalid, // 0x08
  ScanCodes::F10, // 0x09
  ScanCodes::F8, // 0x0A
  ScanCodes::F6, // 0x0B
  ScanCodes::F4, // 0x0C
  ScanCodes::Tab, // 0x0D
  ScanCodes::BackTick, // 0x0E
  ScanCodes::Invalid, // 0x0F
  ScanCodes::Invalid, // 0x10
  ScanCodes::AltLeft, // 0x11
  ScanCodes::ShiftLeft, // 0x12
  ScanCodes::Invalid, // 0x13
  ScanCodes::CtrlLeft, // 0x14
  ScanCodes::Q, // 0x15
  ScanCodes::N1, // 0x16
  ScanCodes::Invalid, // 0x17
  ScanCodes::Invalid, // 0x18
  ScanCodes::Invalid, // 0x19
  ScanCodes::Z, // 0x1A
  ScanCodes::S, // 0x1B
  ScanCodes::A, // 0x1C
  ScanCodes::W, // 0x1D
  ScanCodes::N2, // 0x1E
  ScanCodes::Invalid, // 0x1F
  ScanCodes::Invalid, // 0x20
  ScanCodes::C, // 0x21
  ScanCodes::X, // 0x22
  ScanCodes::D, // 0x23
  ScanCodes::E, // 0x24
  ScanCodes::N4, // 0x25
  ScanCodes::N3, // 0x26
  ScanCodes::Invalid, // 0x27
  ScanCodes::Invalid, // 0x28
  ScanCodes::Space, // 0x29
  ScanCodes::V, // 0x2A
  ScanCodes::F, // 0x2B
  ScanCodes::T, // 0x2C
  ScanCodes::R, // 0x2D
  ScanCodes::N5, // 0x2E
  ScanCodes::Invalid, // 0x2F
  ScanCodes::Invalid, // 0x30
  ScanCodes::N, // 0x31
  ScanCodes::B, // 0x32
  ScanCodes::H, // 0x33
  ScanCodes::G, // 0x34
  ScanCodes::Y, // 0x35
  ScanCodes::N6, // 0x36
  ScanCodes::Invalid, // 0x37
  ScanCodes::Invalid, // 0x38
  ScanCodes::Invalid, // 0x39
  ScanCodes::M, // 0x3A
  ScanCodes::J, // 0x3B
  ScanCodes::U, // 0x3C
  ScanCodes::N7, // 0x3D
  ScanCodes::N8, // 0x3E
  ScanCodes::Invalid, // 0x3F
  ScanCodes::Invalid, // 0x40
  ScanCodes::Comma, // 0x41
  ScanCodes::K, // 0x42
  ScanCodes::I, // 0x43
  ScanCodes::O, // 0x44
  ScanCodes::N0, // 0x45
  ScanCodes::N9, // 0x46
  ScanCodes::Invalid, // 0x47
  ScanCodes::Invalid, // 0x48
  ScanCodes::Dot, // 0x49
  ScanCodes::Slash, // 0x4A
  ScanCodes::L, // 0x4B
  ScanCodes::Semicolon, // 0x4C
  ScanCodes::P, // 0x4D
  ScanCodes::Dash, // 0x4E
  ScanCodes::Invalid, // 0x4F
  ScanCodes::Invalid, // 0x50
  ScanCodes::Invalid, // 0x51
  ScanCodes::Apostrophe, // 0x52
  ScanCodes::Invalid, // 0x53
  ScanCodes::BracketOpen, // 0x54
  ScanCodes::Equals, // 0x55
  ScanCodes::Invalid, // 0x56
  ScanCodes::Invalid, // 0x57
  ScanCodes::CapsLock, // 0x58
  ScanCodes::ShiftRight, // 0x59
  ScanCodes::Enter, // 0x5A
  ScanCodes::BracketClose, // 0x5B
  ScanCodes::Invalid, // 0x5C
  ScanCodes::Backslash, // 0x5D
  ScanCodes::Invalid, // 0x5E
  ScanCodes::Invalid, // 0x5F
  ScanCodes::Invalid, // 0x60
  ScanCodes::Invalid, // 0x61
  ScanCodes::Invalid, // 0x62
  ScanCodes::Invalid, // 0x63
  ScanCodes::Invalid, // 0x64
  ScanCodes::Invalid, // 0x65
  ScanCodes::Backspace, // 0x66
  ScanCodes::Invalid, // 0x67
  ScanCodes::Invalid, // 0x68
  ScanCodes::N1K, // 0x69
  ScanCodes::Invalid, // 0x6A
  ScanCodes::N4K, // 0x6B
  ScanCodes::N7K, // 0x6C
  ScanCodes::Invalid, // 0x6D
  ScanCodes::Invalid, // 0x6E
  ScanCodes::Invalid, // 0x6F
  ScanCodes::N0K, // 0x70
  ScanCodes::DotK, // 0x71
  ScanCodes::N2K, // 0x72
  ScanCodes::N5K, // 0x73
  ScanCodes::N6K, // 0x74
  ScanCodes::N8K, // 0x75
  ScanCodes::Escape, // 0x76
  ScanCodes::NumLock, // 0x77
  ScanCodes::F11, // 0x78
  ScanCodes::PlusK, // 0x79
  ScanCodes::N3K, // 0x7A
  ScanCodes::DashK, // 0x7B
  ScanCodes::MultiplyK, // 0x7C
  ScanCodes::N9K, // 0x7D
  ScanCodes::ScrollLock, // 0x7E
  ScanCodes::Invalid, // 0x7F
  ScanCodes::Invalid, // 0x80
  ScanCodes::Invalid, // 0x81
  ScanCodes::Invalid, // 0x82
  ScanCodes::F7, // 0x83
};

static ScanCode ScanCodesTableE0[128] = {
  ScanCodes::Invalid, // 0x00
  ScanCodes::Invalid, // 0x01
  ScanCodes::Invalid, // 0x02
  ScanCodes::Invalid, // 0x03
  ScanCodes::Invalid, // 0x04
  ScanCodes::Invalid, // 0x05
  ScanCodes::Invalid, // 0x06
  ScanCodes::Invalid, // 0x07
  ScanCodes::Invalid, // 0x08
  ScanCodes::Invalid, // 0x09
  ScanCodes::Invalid, // 0x0A
  ScanCodes::Invalid, // 0x0B
  ScanCodes::Invalid, // 0x0C
  ScanCodes::Invalid, // 0x0D
  ScanCodes::Invalid, // 0x0E
  ScanCodes::Invalid, // 0x0F
  ScanCodes::MultimediaInternetSearch, // 0x10
  ScanCodes::AltRight         , // 0x11
  ScanCodes::PrintScreen, // 0x12
  ScanCodes::Invalid, // 0x13
  ScanCodes::CtrlRight, // 0x14
  ScanCodes::PreviousTrack, // 0x15
  ScanCodes::Invalid, // 0x16
  ScanCodes::Invalid, // 0x17
  ScanCodes::MultimediaInternetFavorites, // 0x18
  ScanCodes::Invalid, // 0x19
  ScanCodes::Invalid, // 0x1A
  ScanCodes::Invalid, // 0x1B
  ScanCodes::Invalid, // 0x1C
  ScanCodes::Invalid, // 0x1D
  ScanCodes::Invalid, // 0x1E
  ScanCodes::PlatformLeft, // 0x1F
  ScanCodes::MultimediaInternetRefresh, // 0x20
  ScanCodes::VolumeDown, // 0x21
  ScanCodes::Invalid, // 0x22
  ScanCodes::Mute, // 0x23
  ScanCodes::Invalid, // 0x24
  ScanCodes::Invalid, // 0x25
  ScanCodes::Invalid, // 0x26
  ScanCodes::PlatformRight, // 0x27
  ScanCodes::MultimediaInternetStop, // 0x28
  ScanCodes::Invalid, // 0x29
  ScanCodes::Invalid, // 0x2A
  ScanCodes::ProgramsCalculator, // 0x2B
  ScanCodes::Invalid, // 0x2C
  ScanCodes::Invalid, // 0x2D
  ScanCodes::Invalid, // 0x2E
  ScanCodes::Menu, // 0x2F
  ScanCodes::MultimediaInternetForward, // 0x30
  ScanCodes::Invalid, // 0x31
  ScanCodes::VolumeUp, // 0x32
  ScanCodes::Invalid, // 0x33
  ScanCodes::PlayPause, // 0x34
  ScanCodes::Invalid, // 0x35
  ScanCodes::Invalid, // 0x36
  ScanCodes::AcpiPower, // 0x37
  ScanCodes::MultimediaInternetBack, // 0x38
  ScanCodes::Invalid, // 0x39
  ScanCodes::MultimediaInternetHomepage, // 0x3A
  ScanCodes::MultimediaStop, // 0x3B
  ScanCodes::Invalid, // 0x3C
  ScanCodes::Invalid, // 0x3D
  ScanCodes::Invalid, // 0x3E
  ScanCodes::AcpiSleep, // 0x3F
  ScanCodes::ProgramsExplorer, // 0x40
  ScanCodes::Invalid, // 0x41
  ScanCodes::Invalid, // 0x42
  ScanCodes::Invalid, // 0x43
  ScanCodes::Invalid, // 0x44
  ScanCodes::Invalid, // 0x45
  ScanCodes::Invalid, // 0x46
  ScanCodes::Invalid, // 0x47
  ScanCodes::ProgramsEmail, // 0x48
  ScanCodes::Invalid, // 0x49
  ScanCodes::SlashK, // 0x4A
  ScanCodes::Invalid, // 0x4B
  ScanCodes::Invalid, // 0x4C
  ScanCodes::NextTrack, // 0x4D
  ScanCodes::Invalid, // 0x4E
  ScanCodes::Invalid, // 0x4F
  ScanCodes::MultimediaMediaSelect, // 0x50
  ScanCodes::Invalid, // 0X51
  ScanCodes::Invalid, // 0X52
  ScanCodes::Invalid, // 0x53
  ScanCodes::Invalid, // 0x54
  ScanCodes::Invalid, // 0x55
  ScanCodes::Invalid, // 0x56
  ScanCodes::Invalid, // 0x57
  ScanCodes::Invalid, // 0x58
  ScanCodes::Invalid, // 0x59
  ScanCodes::EnterK, // 0x5A
  ScanCodes::Invalid, // 0x5B
  ScanCodes::Invalid, // 0x5C
  ScanCodes::Invalid, // 0x5D
  ScanCodes::AcpiWake, // 0x5E
  ScanCodes::Invalid, // 0x5F
  ScanCodes::Invalid, // 0x60
  ScanCodes::Invalid, // 0x61
  ScanCodes::Invalid, // 0x62
  ScanCodes::Invalid, // 0x63
  ScanCodes::Invalid, // 0x64
  ScanCodes::Invalid, // 0x65
  ScanCodes::Invalid, // 0x66
  ScanCodes::Invalid, // 0x67
  ScanCodes::Invalid, // 0x68
  ScanCodes::End       , // 0x69
  ScanCodes::Invalid, // 0x6A
  ScanCodes::ArrowLeft, // 0x6B
  ScanCodes::Home            , // 0x6C
  ScanCodes::Invalid, // 0x6D
  ScanCodes::Invalid, // 0x6E
  ScanCodes::Invalid, // 0x6F
  ScanCodes::Insert  , // 0x70
  ScanCodes::Delete  , // 0x71
  ScanCodes::ArrowDown, // 0x72
  ScanCodes::Invalid, // 0x73
  ScanCodes::ArrowRight, // 0x74
  ScanCodes::ArrowUp, // 0x75
  ScanCodes::Invalid, // 0x76
  ScanCodes::Invalid, // 0x77
  ScanCodes::Invalid, // 0x78
  ScanCodes::Invalid, // 0x79
  ScanCodes::PageDown, // 0x7A
  ScanCodes::Invalid, // 0x7B
  ScanCodes::Invalid, // 0x7C
  ScanCodes::PageUp, // 0x7D
};

PS2Keyboard::PS2Keyboard(PS2* bus) 
: bus(bus)
, offset(0)
{
  register_interrupt(this);
}

void PS2Keyboard::onInterrupt() {
  // TODO: this fails when you have more than one device
  uint8_t status = bus->ReadStatus();
  if ((status & 1) == 0)
    return;

  buffer[offset++] = bus->ReadData();
  if (offset == 0)
    return;

  ScanCode sc = ScanCodes::Invalid;
  switch(buffer[0]) {
    case 0xE0:
      if (offset == 2 && buffer[1] != 0xF0) {
        offset = 0;
        sc = ScanCodesTableE0[buffer[1] & 0x7F];
      } else if (offset == 3 && buffer[1] == 0xF0) {
        offset = 0;
        sc = ScanCodesTableE0[buffer[2] & 0x7F] | ScanCodes::KeyReleased;
      }
      break;
    case 0xE1:
      if (offset == 3 && buffer[1] != 0xF0) {
        offset = 0;
        sc = ScanCodes::Pause;
      } else if (offset == 5 && buffer[1] == 0xF0) {
        offset = 0;
        sc = ScanCodes::Pause | ScanCodes::KeyReleased;
      }
      break;
    case 0xF0:
      if (offset == 2) {
        offset = 0;
        sc = ScanCodesTable[buffer[1]] | ScanCodes::KeyReleased;
      }
      break;
    default:
      {
        offset = 0;
        sc = ScanCodesTable[buffer[0]];
      }
  }
  if (sc != ScanCodes::Invalid)
    keyboard->onInput(sc);
}


