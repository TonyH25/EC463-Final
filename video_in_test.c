#include "address_map_arm.h"

void VGA_load_image_sdram(short int image[][320]);
void VGA_load_sdram(int top_x ,int bot_x, int top_y, int bot_y,short);
void VGA_text(int top_x, int top_y, char * txt);
void VGA_box(int x1, int y1, int x2, int y2, short pixel_color);
void VGA_outline_y(int x1, int y1, short pixel_color);
void VGA_outline_x(int x1, int y1, short pixel_color);

int main(void)
{
	volatile int * VIDEO_IN_CONTROL_ptr  = (int *) VIDEO_IN_BASE;
	volatile int * KEY_ptr = (int *)KEY_BASE;
	volatile short * fpga_chip = (short *) FPGA_ONCHIP_BASE;	//  on-chip buffer
	volatile short * sdram = (short *)SDRAM_BASE;				//  SRAM buffer
	volatile int * VGA_DMA_CONTROL_ptr  = (int *) PIXEL_BUF_CTRL_BASE;
	volatile int row, col;
	volatile int show_live_video;
	
	char text[] = "EC463 USNA MICROCONTROLLER INTERFACING FINAL PROJECT\0";
	char text2[] = "EC463 - DIGIT CLASSIFIER\0";
	/* Write a text string to VGA */
	VGA_text(20, 1, text);

	// blue screen
	short blue[240][320];
	for (row = 0; row <= 239; row++)
	{
		for (col = 0; col <= 319; col++)
		{
			blue[row][col] = 0x001F;
		}
	}
	row = 0;
	col = 0;
	
	// Intialize SDRAM buffer to a blue image
	VGA_load_image_sdram(blue);
	VGA_load_sdram(100,140,140,180,0xFFFF);
	
	// Initialize Video in and VGA interfaces	
	*(VIDEO_IN_CONTROL_ptr + 3)  = (1<<2);			// enable live video --> frame = on-chip buffer by default
	*(VGA_DMA_CONTROL_ptr + 1) = FPGA_ONCHIP_BASE; 	// live video input will be shown on VGA
	*(VGA_DMA_CONTROL_ptr + 0) = 1;
	show_live_video = 1;
	
	while(1)
	{	
		// if KEY(3) is detected, enable or disable (toggle) the video in interface
		if (*(KEY_ptr + 3) & 0x08){					// if KEY(3) is detected
		
			*(VIDEO_IN_CONTROL_ptr + 3)  ^= (1<<2);	// toggle (enable/disable) video_in
			if(*(VIDEO_IN_CONTROL_ptr+3)&1<<2 == 1){
				VGA_outline_x(146,106,0xffff);
				VGA_outline_x(146,134,0xffff);
				VGA_outline_y(146,106,0xffff);
				VGA_outline_y(174,106,0xffff);
			}
			
			*(KEY_ptr + 3) = (1 << 3); 				// clear flag for KEY(3)
		}
		
		// if KEY(2) is detected, swap buffers for VGA display
		else if (*(KEY_ptr + 3) & 0x04)	{	// if KEY(2) is detected
			
			show_live_video ^= 1;
			if (show_live_video)
			{
				*(VGA_DMA_CONTROL_ptr + 1) = (int)FPGA_ONCHIP_BASE;
			}
			else
			{
				*(VGA_DMA_CONTROL_ptr + 1) = (int)SDRAM_BASE;
				VGA_text(20,1,text2);
			}
			*(VGA_DMA_CONTROL_ptr + 0) = 1;
			
			*(KEY_ptr + 3) = (1 << 2);  // clear flag for KEY(2)
		}
	}
}

/***************************************************************
	Function to load and dispaly an image on SDRAM
*************************************************************/
void VGA_load_image_sdram(short int image[][320])
{
	int offset, row, col;
	volatile short * pixel_buffer = (short *)SDRAM_BASE;	//  SRAM buffer
	/* assume that the box coordinates are valid */
	for (row = 0; row <= 239; row++)
	{
		for (col = 0; col <= 319; col++)
		{
			offset = (row << 9) + col;						// compute offset 
			*(pixel_buffer + offset) = (short)(image[row][col]);		// set pixel value
		}
	}
}

/***************************************************************
	Function to draw a a string of text (NULL terminated)
	(x, y): coordinate of first character in the character buffer
	resolution of character buffer = 80x60
*************************************************************/
void VGA_text(int x, int y, char * text_ptr)
{
	int offset;
	char * character_buffer = (char *)FPGA_CHAR_BASE;	// VGA character buffer

	/* compute offset and set character */
	offset = (y << 7) + x;
	while (*(text_ptr))	// NULL character = end of array
	{
		*(character_buffer + offset) = *(text_ptr);
		++text_ptr;
		++offset;
	}
}

void VGA_load_sdram(int top_x,int bot_x, int top_y, int bot_y,short color){
	int offset, row, col;
	volatile short * pixel_buffer = (short *)SDRAM_BASE;	//  SRAM buffer
	/* assume that the box coordinates are valid */
	for (row = top_x; row <= bot_x; row++)
	{
		for (col = top_y; col <= bot_y; col++)
		{
			offset = (row << 9) + col;						// compute offset 
			*(pixel_buffer + offset) = color;		// set pixel value
		}
	}
}

void VGA_box(int x1, int y1, int x2, int y2, short pixel_color){
    int offset, row, col;
    short *pixel_buffer = (short *)FPGA_ONCHIP_BASE; // pixel buffer
    /* assume that the box coordinates are valid */
    for (row = y1; row <= y2; row++){
        for (col = x1; col <= x2; col++){
            offset = (row << 9) + col;
            *(pixel_buffer + offset) = (short)pixel_color;
        }
    }
}
void VGA_outline_x(int x1, int y1, short pixel_color){
    int offset, row, col;
    short *pixel_buffer = (short *)FPGA_ONCHIP_BASE; // pixel buffer
    /* assume that the box coordinates are valid */
	col = y1;
	for (row = x1; row <= x1+28; row++){
		offset = (row << 9) + col;
		*(pixel_buffer + offset) = (short)pixel_color;
	}
}
void VGA_outline_y(int x1, int y1, short pixel_color){
    int offset, row, col;
    short *pixel_buffer = (short *)FPGA_ONCHIP_BASE; // pixel buffer
    /* assume that the box coordinates are valid */
	row = x1;
	for (col = y1; col <= y1+28; col++){
		offset = (row << 9) + col;
		*(pixel_buffer + offset) = (short)pixel_color;
	}
}