// Tony Harrington
// This file has the function to animate the loading screen for the digit
//   classifier

typedef short int (*vga_img)[320];
typedef short int (*out_img)[36];
short int zero[36][36],one[36][36],two[36][36],three[36][36],four[36][36],
          five[36][36],six[36][36],seven[36][36],eight[36][36],nine[36][36];


void VGA_box(int x1, int y1, int x2, int y2, short int p[][320])
{
  int offset, row, col;
  //short * pixel_buffer = (short *) FPGA_ONCHIP_BASE; // pixel buffer
  volatile short * pixel_buffer = (short *)SDRAM_BASE;	//  SRAM buffer
  /* assume that the box coordinates are valid */
  for (row = y1; row <= y2; row++)
  {
    for (col = x1; col <= x2; col++ )
    {
      offset = (row << 9) + col;
      *(pixel_buffer + offset) = (short)p[row][col];
    }
  }
}

void loadscreen(int time)
{
  vga_img imag[3] = {One_Dot,Two_Dot,Three_Dot};
  VGA_box(0,0,319,239,imag[0]);
  int i;
  unsigned long j;
  for(i = 0; i < 12; i++)
  {
    VGA_box(210,95,240,105,imag[i%3]);
    for(j = 0; j < time; j++)
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
/*
//Input defined X and Y values, and a short int *[36][36]
void capture36x36(int x, int y, short int in[][36])
{
  int i,j;
  for(i = 0; i < 36; i++)
  {
    for(j = 0; j < 36; j++)
    {
      in[i][j] = Numbers[x+j][y+i];
    }
  }
}
*/
//Input 36x36; prints out final screen with classified number
void show_end(short int dig[][36])
{
  int offset, row, col;
  VGA_box(0,0,320,240,Finished);
  //short * pixel_buffer = (short *) FPGA_ONCHIP_BASE; // pixel buffer
  volatile short * pixel_buffer = (short *)SDRAM_BASE;	//  SRAM buffer
  /* assume that the box coordinates are valid */
  for (row = 132; row <= 167; row++)
  {
    for (col = 201; col <= 236; col++ )
    {
      offset = (row << 9) + col;
      *(pixel_buffer + offset) = (short)dig[col-200][row-132];
    }
  }
}
