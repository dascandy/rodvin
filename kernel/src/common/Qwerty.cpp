#include "Qwerty.h"
#include <stdio.h>

//  uint32_t regular, shifted, alt, altshift;
uint32_t mapping[] = {
  0, 0, 0, 0, // Invalid
  '`', '~', 0, 0, // BackTick,
  '0', ')', 0, 0, // N0,
  '1', '!', 0, 0, // N1,
  '2', '@', 0, 0, // N2,
  '3', '#', 0, 0, // N3,
  '4', '$', 0, 0, // N4,
  '5', '%', U'€', 0, // N5,
  '6', '^', 0, 0, // N6,
  '7', '&', 0, 0, // N7,
  '8', '*', 0, 0, // N8,
  '9', '(', 0, 0, // N9,
  '-', '_', 0, 0, // Dash,
  '=', '+', 0, 0, // Plus,
  '[', '{', 0, 0, // BracketOpen,
  ']', '}', 0, 0, // BracketClose,
  '\\', '|', 0, 0, // Backslash,
  ';', ':', 0, 0, // Semicolon,
  '\'', '"', 0, 0, // Apostrophe,
  ',', '<', 0, 0, // Comma,
  '.', '>', 0, 0, // Dot,
  '/', '?', 0, 0, // Slash,
  ' ', ' ', 0, 0, // Space,
  'a', 'a', 0, 0, // A,
  'b', 'b', 0, 0, // B,
  'c', 'c', 0, 0, // C,
  'd', 'd', 0, 0, // D,
  'e', 'e', 0, 0, // E,
  'f', 'f', 0, 0, // F,
  'g', 'g', 0, 0, // G,
  'h', 'h', 0, 0, // H,
  'i', 'i', 0, 0, // I,
  'j', 'j', 0, 0, // J,
  'k', 'k', 0, 0, // K,
  'l', 'l', 0, 0, // L,
  'm', 'm', 0, 0, // M,
  'n', 'n', 0, 0, // N,
  'o', 'o', 0, 0, // O,
  'p', 'p', 0, 0, // P,
  'q', 'q', 0, 0, // Q,
  'r', 'r', 0, 0, // R,
  's', 's', 0, 0, // S,
  't', 't', 0, 0, // T,
  'u', 'u', 0, 0, // U,
  'v', 'v', 0, 0, // V,
  'w', 'w', 0, 0, // W,
  'x', 'x', 0, 0, // X,
  'y', 'y', 0, 0, // Y,
  'z', 'z', 0, 0, // Z,
  '\n', '\n', 0, 0, // Enter,
  '\b', '\b', 0, 0, // Backspace,
  '\t', '\t', 0, 0, // Tab,
  '.', '.', 0, 0, // DotK,
  '\n', '\n', 0, 0, // EnterK,
  '-', '-', 0, 0, // DashK,
  '*', '*', 0, 0, // MultiplyK,
  '+', '+', 0, 0, // PlusK,
  '/', '/', 0, 0, // SlashK,
  '0', '0', 0, 0, // N0K,
  '1', '1', 0, 0, // N1K,
  '2', '2', 0, 0, // N2K,
  '3', '3', 0, 0, // N3K,
  '4', '4', 0, 0, // N4K,
  '5', '5', 0, 0, // N5K,
  '6', '6', 0, 0, // N6K,
  '7', '7', 0, 0, // N7K,
  '8', '8', 0, 0, // N8K,
  '9', '9', 0, 0, // N9K,
};

uint32_t Qwerty::lookup(ScanCode sc, bool shifted, bool alt) {
  uint32_t index = sc * 4 + alt * 2 + shifted;
  if (index > sizeof(mapping) / sizeof(mapping[0]))
    return 0;
  printf("lookup %d %d %d / %d => %d\n", sc, alt, shifted, index, mapping[index]);

  return mapping[index];
}


