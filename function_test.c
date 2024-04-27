// Tony Harrington, Jadiel Arana
// This program applies a Linear Classification to hand-drawn numbers

#include <stdio.h>
#include "weights_fp.h"
#include "img.h"
#include "address_map_arm.h"
#include "Image.h"
#include "classify.h"
#include "loading.h"
#include "define_digits.h"
//typedef short int (*out_img)[36]

int main()
{
  //Create output source
  volatile int* KEY_ptr = KEY_BASE;
  volatile int* LED_ptr = LEDR_BASE;
  //Create array of 10 potential digits
  img_type img[10] = {Image1, Image2, Image3, Image4, Image5, Image6, Image7, Image8, Image9, Image10};
  vga_img imag[3] = {One_Dot,Two_Dot,Three_Dot};

  int count = 0;

  while(1)
  {
    VGA_box(0,0,319,239,Initial_Screen);
    if((*(KEY_ptr + 0x3) & 0x8))
    {
      loadscreen(imag);
      //Perform linear classification and populate array
      *(LED_ptr) = classify(img[count]);
      VGA_box(216,120,257,160, Load_Bar);
      count++;
      *(KEY_ptr+0x3) = 0x8;  //Clear edge detect register
    }
  }
}
