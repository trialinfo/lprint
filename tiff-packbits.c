//
// TIFF PackBits algorithm
//
// Copyright © 2024 by Andreas Grünbacher.
//
// Licensed under Apache License v2.0.  See the file "LICENSE" for more
// information.

#include <string.h>
#include "tiff-packbits.h"

// tiff_packbits - pack a sequence of bytes
//
// Encodes sequences of repeating and non-repeating bytes as the length of each
// sequence, followed by the contents of the sequence: a length byte between 0
// and 127 indicates a sequence of N + 1 non-repeating bytes; a length byte
// between -127 and -1 indicates a byte that is repeated 1 - N times.  For
// example,
//
//   abcdef  =>  <5>abcdef  (7 bytes)
//   abbbbc  =>  <0>a <-3>b <0>c  (6 bytes)
//   aabbcc  =>  <-1>a <-1>b <-1>c (6 bytes)
//   abbccd  =>  <5>abbccd (7 bytes)
//
// This function never uses -128 as the value of the length byte.
//
// The output will be at most tiff_packbits_bufsize(len) bytes in size.
//
// Returns the size of the encoded byte sequence.
//
unsigned tiff_packbits(unsigned char *out, const unsigned char *in, unsigned len)
{
  unsigned char *orig_out = out;
  const unsigned char *start = in;  // start of the literal to emit
  const unsigned char *pos = in;  // start of the run to emit
  const unsigned char *next = in;  // start of the next pattern
  const unsigned char *end = in + len;  // end of @in

  while (start != end) {
    while (pos != end) {
      // extend the literal as long as nothing repeats
      while (pos + 1 != end && pos[0] != pos[1])
	pos++;

      // find the next run
      next = pos;
      if (next != end) {
	while (next + 1 != end && pos[0] == next[1])
	  next++;
      }

      // Require at least three repetitions to start a run if we already have a
      // literal (start != pos), and two otherwise: if we already have a
      // literal, extending it by two bytes is as cheap as starting a run, but
      // we can still extend that literal later at no extra cost.  If we start
      // a run, starting another literal will require an extra byte.

      if (next - pos >= 2 + (start != pos))
	break;

      // append to the literal instead
      pos = next;
    }

    // emit the literal
    while (start != pos) {
      unsigned chunk = pos - start;
      if (chunk > 128)
	chunk = 128;
      *out++ = chunk - 1;
      memcpy(out, start, chunk);
      out += chunk;
      start += chunk;
    }

    // emit the run
    while (next - pos > 1) {
      unsigned chunk = next - pos;
      if (chunk > 128)
	chunk = 128;
      *out++ = 1 - chunk;
      *out++ = pos[0];
      pos += chunk;
    }

    // convert a potential remaining run of 1 into a literal
    start = pos;
    pos = next;
  }

  return out - orig_out;
}
