#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tiff-packbits.h"

void print_char(unsigned char c)
{
  if (isprint(c))
    printf("%c", c);
  else
    printf("<%02x>", c);
}

void print_packed(unsigned char *packed, unsigned packed_size)
{
  while (packed_size) {
    signed char len = packed[0];
    unsigned n;

    printf(" <%d>", len);
    if (len < 0) {
      print_char(packed[1]);
      packed += 2;
      packed_size -= 2;
    } else {
      for (n = 0; n < len + 1; n++)
	print_char(packed[n + 1]);
      packed += len + 2;
      packed_size -= len + 2;
    }
  }
}

unsigned unpack(unsigned char *unpacked, unsigned char *packed, unsigned packed_size)
{
  unsigned unpacked_size = 0;

  while (packed_size) {
    signed char len = packed[0];

    assert(len != -128);
    if (len < 0) {
      memset(unpacked, packed[1], 1 - len);
      unpacked += 1 - len;
      unpacked_size += 1 - len;
      packed += 2;
      packed_size -= 2;
    } else {
      memcpy(unpacked, packed + 1, len + 1);
      unpacked += len + 1;
      unpacked_size += len + 1;
      packed += len + 2;
      packed_size -= len + 2;
    }
  }
  return unpacked_size;
}

void test(const char *str)
{
  unsigned bufsize;
  unsigned char *packed, *unpacked;
  unsigned packed_size, unpacked_size;

  bufsize = tiff_packbits_bufsize(strlen(str));
  packed = malloc(bufsize);
  packed_size = tiff_packbits(packed, (unsigned char *)str, strlen(str));
  printf("\"%s\" =>", str);
  print_packed(packed, packed_size);
  assert(packed_size <= bufsize);
  unpacked = malloc(strlen(str));
  unpacked_size = unpack(unpacked, packed, packed_size);
  assert(unpacked_size == strlen(str));
  assert(!memcmp(str, unpacked, strlen(str)));
  printf("\n");
  free(packed);
  free(unpacked);
}

int main(int argc, char *argv[])
{
  test("");
  test("a");
  test("aa");
  test("abb");
  test("abcdef");
  test("abbbbc");
  test("aabbcc");
  test("abbccd");
  test("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
       "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
       "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
       "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
       "ab");
}
