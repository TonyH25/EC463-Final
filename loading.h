// Tony Harrington
// This file has the function to animate the loading screen for the digit
//   classifier

typedef short int (*vga_img)[320];

//void VGA_box(int x1, int y1, int x2, int y2, short int p[][320])
//{
//  int offset, row, col;
//  short * pixel_buffer = (short *) FPGA_ONCHIP_BASE; // pixel buffer
//  /* assume that the box coordinates are valid */
//  for (row = y1; row <= y2; row++)
//  {
//    for (col = x1; col <= x2; col++ )
//    {
//      offset = (row << 9) + col;
//      *(pixel_buffer + offset) = (short)p[row][col];
//    }
//  }
//}

void loadscreen(vga_img images[3])
{
  VGA_box(0,0,319,239,images[0]);
  int i;
  unsigned long j;
  for(i = 0; i < 10; i++)
  {
    VGA_box(210,95,240,105,images[i%3]);
    for(j = 0; j < 500000000; j++)
      {}
    if(i == 1)
      VGA_box(50,120 ,85, 160, Load_Bar);
    if(i == 3)
      VGA_box(85, 120, 118 , 160, Load_Bar);
    if(i == 5)
      VGA_box(118, 120, 151 , 160, Load_Bar);
    if(i == 7)
      VGA_box(151, 120, 184 , 160, Load_Bar);
    if(i == 9)
      VGA_box(184, 120, 216 , 160, Load_Bar);
  }
}
