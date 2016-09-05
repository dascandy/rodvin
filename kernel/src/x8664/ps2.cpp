#include "ps2.h"
#include "device.h"
#include "portio.h"
#include <stdio.h>

enum {
  Data = 0x60,
  Status = 0x64,
  Command = 0x64
};

enum {
  DisableFirstPort = 0xAD,
  DisableSecondPort = 0xA7,
  ReadConfig = 0x20,
  WriteConfig = 0x60,
  SelfTest = 0xAA,
  TestFirstPort = 0xAB,
  TestSecondPort = 0xA9,
  EnableFirstPort = 0xAE,
  EnableSecondPort = 0xA8,
};

uint8_t PS2::ReadStatus() {
  return inb(Status);
}

void PS2::SendCommand(uint8_t command) {
  while (ReadStatus() & 2) {}
  outb(Command, command);
}

uint8_t PS2::ReadData() {
  while ((ReadStatus() & 1) == 0) {}
  return inb(Data);
}

void PS2::WriteData(uint8_t value) {
  while (ReadStatus() & 2) {}
  outb(Data, value);
}

PS2::PS2() {
  SendCommand(DisableFirstPort);
  SendCommand(DisableSecondPort);
  inb(Data); // To flush the output port

  SendCommand(ReadConfig);
  uint8_t config = ReadData() & 0xBC;
  SendCommand(WriteConfig);
  WriteData(config);
  // TODO: set scancode set 2
/*
  SendCommand(0xF0);
  SendCommand(2);
  if (ReadData() != 0xFA) {
    printf("didn't accept key scancode set\n");
  }
*/
  // if (config & 0x20) then it's a single channel. Not handled.

  SendCommand(SelfTest);
  if (ReadData() != 0x55) {
    // Not good. But then what?
    return;
  }

  SendCommand(TestFirstPort);
  /*uint8_t firstPort =*/ ReadData();
//  SendCommand(TestSecondPort);
//  uint8_t secondPort = ReadData();

//  SendCommand(EnableSecondPort);

   SendCommand(WriteConfig);
   WriteData(config | 0x01);

  // TODO: reset devices, if we want to


  SendCommand(EnableFirstPort);

  register_keyboard(new PS2Keyboard(this));
}

PS2::~PS2() {

}


