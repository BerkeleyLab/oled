// adapted from M. Abrash, Zen of Graphics Programming
// Function to draw an antialiased line from (X0,Y0) to (X1,Y1), using an
// antialiasing approach published by Xiaolin Wu in the July 1991 issue of
// Computer Graphics.

#include "frame_buffer.h"

// hardcoded parameters for 4 bit greyscale mode
// changed arithmetic to picorv32 native 32 bit
#define INTENSITY_BITS 4
#define BASE_COLOR 0x0F
#define N_BITS 32  // number of bits in an unsigned int

// number of bits by which to shift ErrorAcc to get intensity level
#define INTENSITY_SHIFT (N_BITS - INTENSITY_BITS)
#define INTENSITY_MASK ((1 << INTENSITY_BITS) - 1)


// draw white anti-aliased line from (x0, y0) to (x1, y1)
void drawLine(int X0, int Y0, int X1, int Y1)
{
   uint32_t ErrorAdj, ErrorAcc;
   uint32_t ErrorAccTemp, Weighting;
   int32_t DeltaX, DeltaY, Temp, XDir;

   // Make sure the line runs top to bottom
   if (Y0 > Y1) {
      Temp = Y0; Y0 = Y1; Y1 = Temp;
      Temp = X0; X0 = X1; X1 = Temp;
   }
   // Draw the initial pixel, which is always exactly intersected by
   // the line and so needs no weighting
   setPixel(X0, Y0, BASE_COLOR);

   if ((DeltaX = X1 - X0) >= 0) {
      XDir = 1;
   } else {
      XDir = -1;
      DeltaX = -DeltaX; // make DeltaX positive
   }
   // Special-case horizontal, vertical, and diagonal lines, which
   // require no weighting because they go right through the center of
   // every pixel
   if ((DeltaY = Y1 - Y0) == 0) {
      // Horizontal line
      while (DeltaX-- != 0) {
         X0 += XDir;
         setPixel(X0, Y0, BASE_COLOR);
      }
      return;
   }
   if (DeltaX == 0) {
      // Vertical line
      do {
         Y0++;
         setPixel(X0, Y0, BASE_COLOR);
      } while (--DeltaY != 0);
      return;
   }
   if (DeltaX == DeltaY) {
      // Diagonal line
      do {
         X0 += XDir;
         Y0++;
         setPixel(X0, Y0, BASE_COLOR);
      } while (--DeltaY != 0);
      return;
   }
   // line is not horizontal, diagonal, or vertical
   ErrorAcc = 0;
   // Is this an X-major or Y-major line?
   if (DeltaY > DeltaX) {
      // Y-major line; calculate N_BITS-bit fixed-point fractional part of a
      // pixel that X advances each time Y advances 1 pixel, truncating the
      // result so that we won't overrun the endpoint along the X axis
      ErrorAdj = ((uint64_t) DeltaX << N_BITS) / (uint64_t) DeltaY;
      // Draw all pixels other than the first and last
      while (--DeltaY) {
         ErrorAccTemp = ErrorAcc;   // remember current accumulated error
         ErrorAcc += ErrorAdj;      // calculate error for next pixel
         if (ErrorAcc <= ErrorAccTemp)
            X0 += XDir;
         Y0++; // Y-major, so always advance Y
         Weighting = ErrorAcc >> INTENSITY_SHIFT;
         setPixel(X0, Y0, Weighting  ^ INTENSITY_MASK);
         setPixel(X0 + XDir, Y0, Weighting);
      }
      // the final pixel, which is always exactly intersected by the line
      setPixel(X1, Y1, BASE_COLOR);
      return;
   }
   // It's an X-major line;
   ErrorAdj = ((uint64_t) DeltaY << N_BITS) / (uint64_t) DeltaX;
   // Draw all pixels other than the first and last
   while (--DeltaX) {
      ErrorAccTemp = ErrorAcc;   // remember current accumulated error
      ErrorAcc += ErrorAdj;      // calculate error for next pixel
      if (ErrorAcc <= ErrorAccTemp)
         Y0++;
      X0 += XDir; // X-major, so always advance X
      Weighting = ErrorAcc >> INTENSITY_SHIFT;
      setPixel(X0, Y0, Weighting ^ INTENSITY_MASK);
      setPixel(X0, Y0 + 1, Weighting);
   }
   setPixel(X1, Y1, BASE_COLOR);
}
