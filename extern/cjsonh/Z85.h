#ifndef Z85_H
#define Z85_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

static char encoder[85 + 1] = {"0123456789"
                               "abcdefghij"
                               "klmnopqrst"
                               "uvwxyzABCD"
                               "EFGHIJKLMN"
                               "OPQRSTUVWX"
                               "YZ.-:+=^!/"
                               "*?&<>()[]{"
                               "}@%$#"};

//  Maps base 85 to base 256
//  We chop off lower 32 and higher 128 ranges
static byte decoder[96] = {0x00, 0x44, 0x00, 0x54, 0x53, 0x52, 0x48, 0x00, 0x4B, 0x4C, 0x46, 0x41, 0x00, 0x3F,
                           0x3E, 0x45, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x40, 0x00,
                           0x49, 0x42, 0x4A, 0x47, 0x51, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,
                           0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A,
                           0x3B, 0x3C, 0x3D, 0x4D, 0x00, 0x4E, 0x43, 0x00, 0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
                           0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
                           0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x4F, 0x00, 0x50, 0x00, 0x00};

//  --------------------------------------------------------------------------
//  Encode a byte array as a string

char *Z85_encode(byte *data, size_t size) {
  //  Accepts only byte arrays bounded to 4 bytes
  if (size % 4)
    return NULL;

  size_t encoded_size = size * 5 / 4;
  char *encoded = malloc(encoded_size + 1);
  unsigned char_nbr = 0;
  unsigned byte_nbr = 0;
  uint32_t value = 0;
  while (byte_nbr < size) {
    //  Accumulate value in base 256 (binary)
    value = value * 256 + data[byte_nbr++];
    if (byte_nbr % 4 == 0) {
      //  Output value in base 85
      unsigned divisor = 85 * 85 * 85 * 85;
      while (divisor) {
        encoded[char_nbr++] = encoder[value / divisor % 85];
        divisor /= 85;
      }
      value = 0;
    }
  }
  assert(char_nbr == encoded_size);
  encoded[char_nbr] = 0;
  return encoded;
}

//  --------------------------------------------------------------------------
//  Decode an encoded string into a byte array; size of array will be
//  strlen (string) * 4 / 5.

byte *Z85_decode(char *string) {
  //  Accepts only strings bounded to 5 bytes
  if (strlen(string) % 5)
    return NULL;

  size_t decoded_size = strlen(string) * 4 / 5;
  byte *decoded = malloc(decoded_size);

  unsigned byte_nbr = 0;
  unsigned char_nbr = 0;
  uint32_t value = 0;
  while (char_nbr < strlen(string)) {
    //  Accumulate value in base 85
    value = value * 85 + decoder[(byte)string[char_nbr++] - 32];
    if (char_nbr % 5 == 0) {
      //  Output value in base 256
      unsigned divisor = 256 * 256 * 256;
      while (divisor) {
        decoded[byte_nbr++] = value / divisor % 256;
        divisor /= 256;
      }
      value = 0;
    }
  }
  assert(byte_nbr == decoded_size);
  return decoded;
}

#endif