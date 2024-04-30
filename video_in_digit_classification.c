#include "address_map_arm.h"
#include "weights_fp.h"
#include "classification.h"
#include "Image.h"
#include "img.h"

void VGA_load_image_sdram(short int image[][320]);
void VGA_text(int top_x, int top_y, char * txt);

void VGA_outline_y(int x1, int y1, short pixel_color);
void VGA_outline_x(int x1, int y1, short pixel_color);

void CopyInput(short int img[][28]);
void VGA_loadInit(int top_x,int top_y, short int img[][28]);
void VGA_loadInit2(int top_x,int top_y, char img[][28]);
int findAverage(short int img[][28]);
void findBinIMG(short src_img[][28], int threshold, char dst_img[][28]);
void Write_small_img(int x1, int y1, char image[][28], int base_address);

int main(void)
{
	volatile int * VIDEO_IN_CONTROL_ptr  = (int *) VIDEO_IN_BASE;
	volatile int * KEY_ptr = (int *)KEY_BASE;
	volatile short * fpga_chip = (short *) FPGA_ONCHIP_BASE;	//  on-chip buffer
	volatile short * sdram = (short *)SDRAM_BASE;				//  SRAM buffer
	volatile int * VGA_DMA_CONTROL_ptr  = (int *) PIXEL_BUF_CTRL_BASE;
	volatile int row, col, avg, threshold, num;
	volatile int show_live_video;
	volatile int * LEDR_ptr 		= (int *) LEDR_BASE;
	volatile int * SW_ptr 				= (int *) SW_BASE ;	
	volatile int *HEX3_HEX0_ptr = (int *)HEX3_HEX0_BASE;
	
	int ssd[] = { 0xbf, 0x86, 0xdb, 0xcf, 0xe6, 0xed, 0xfd, 0x87,
				  0xff, 0xef, 0xf7, 0xfc, 0xb9, 0xde, 0xf9, 0xf1, 0x00 }; // patterns for ssd
	
	char text[] = "USNA EC463 MICROCOMPUTER INTERFACING FINAL PROJECT\0";
	/* Write a text string to VGA */
	VGA_text(20, 0, text);

	// blue screen
	short blue[240][320];
	short int initIMG[28][28]; //Holds the value pulled from the yellow box of the ON_CHIP buffer (Where the number will be)
	short int binIMG[28][28]; //Holds the value for the binarya image based on thresholding value from SW
	char  networkInput[28][28]; //Converted equiv of initIMG to pass to the classifier

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
	VGA_load_image_sdram(Initial_Screen);
	
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
			
			//Create an outline around an area when you disable the camera. Used to convert camera into NN input
			//YELLOW in 24 bit RGB is 225, 231, 16 -> weights of 0.87890625,0.90234375,0.0625 -> 28,58,2 -> 0xE742
			VGA_outline_x(105,145,0xe742); //Left side of the square
			VGA_outline_y(105,146,0xe742); //Top of the square
			VGA_outline_y(135,146,0xe742);
			VGA_outline_x(106,174,0xe742);
			
			
			*(KEY_ptr + 3) = (1 << 3); 				// clear flag for KEY(3)
		}
		
		// if KEY(2) is detected, swap buffers for VGA display
		else if (*(KEY_ptr + 3) & 0x04)	{	// if KEY(2) is detected
			show_live_video ^= 1;
			if (((*SW_ptr) & (1<<7)))
			{
				*(VGA_DMA_CONTROL_ptr + 1) = (int)FPGA_ONCHIP_BASE;
			}
			else if(show_live_video)
			{
				*(VGA_DMA_CONTROL_ptr + 1) = (int)SDRAM_BASE;
			} else {
				*(VGA_DMA_CONTROL_ptr + 1) = (int)FPGA_ONCHIP_BASE;
			}
			*(VGA_DMA_CONTROL_ptr + 0) = 1;
			
			CopyInput(initIMG);				// copy current box to internal image initIMG
			VGA_load_image_sdram(Initial_Screen);
			VGA_loadInit(140,117,initIMG);

			
			avg = findAverage(initIMG);		// find average intensity of the internal image
			*LEDR_ptr   = avg;				// show avg value on red LEDs
			
			*(KEY_ptr + 3) = (1 << 2);  // clear flag for KEY(2)
		}
		
		// if KEY(1) is detected, swap buffers for VGA display
		else if (*(KEY_ptr + 3) & 0x02)	{	// if KEY(1) is detected
			threshold  = (*SW_ptr) & 0x7F;	// 8-bit threshold value
			if((*SW_ptr) & (1<<9)){
				VGA_load_image_sdram(blue);
				Write_small_img(0, 10, Image1, FPGA_ONCHIP_BASE);
				Write_small_img(0, 10, Image1, SDRAM_BASE);
				Write_small_img(30, 10, Image2, SDRAM_BASE);
				Write_small_img(60, 10, Image3, SDRAM_BASE);
				Write_small_img(90, 10, Image4, SDRAM_BASE);
				Write_small_img(120, 10, Image5, SDRAM_BASE);
				Write_small_img(150, 10, Image6, SDRAM_BASE);
				Write_small_img(180, 10, Image7, SDRAM_BASE);
				Write_small_img(210, 10, Image8, SDRAM_BASE);
				Write_small_img(240, 10, Image9, SDRAM_BASE);
				Write_small_img(270, 10, Image10, SDRAM_BASE);

				findBinIMG( initIMG, threshold, networkInput);	// find binary (8-bit and 16-bit) images
				Write_small_img(150, 140, networkInput, SDRAM_BASE);
				VGA_loadInit(150,100,initIMG);
			} else {
				VGA_load_image_sdram(Finished);
				VGA_loadInit(50,137,initIMG);
				VGA_loadInit(207,137,initIMG);
			}
			num = classify(networkInput);
			*HEX3_HEX0_ptr = (ssd[16] << 24) + (ssd[16] << 16)+ (ssd[16] << 8) + (ssd[num]);
			*(KEY_ptr + 3) = (1 << 1);		// clear flag for KEY(1)
		}
	}
}

/***************************************************************************
	Function to draw a filled rectangle on the VGA monitor with an image
	(x1, y1): coordinate of the top left corner on the screen
	image[][28]: pointer to the current image
	base_address: base address of the buffer (SDRAM_BASE or FPGA_ONCHIP_BASE)
****************************************************************************/
void Write_small_img(int x1, int y1, char image[][28], int base_address)
{
	int offset, row, col, x, y;
	volatile short* pixel_buffer = (short*) base_address;
	volatile short pixel;

	/* assume that the box coordinates are valid */
	y = 0;
	for (row = y1; row < (y1+28); row++)
	{
		x = 0;
		for (col = x1; col < (x1+28); col++)
		{
			offset = (row << 9) + col;
			if (image[x][y] != 0)
				pixel = 0xFFFF;
			else
				pixel = 0x0000;  // can display any color for digit background
			*(pixel_buffer + offset) = (short)pixel;
			x++;
		}
		y++;
	}
}
/**
 * Copy a 28x28 box that goes from (106,146) to (133,173) to global array initIMG[28][28] 
 */
void CopyInput(short int img[][28]){
	volatile int * VIDEO_IN_CONTROL_ptr  = (int *) VIDEO_IN_BASE;
	*(VIDEO_IN_CONTROL_ptr + 3)  &= ~(1<<2);	// disable video_in
	int o_set = 0, row, col;
	int i = 0, j = 0;
	volatile short * fpga_chip = (short *) FPGA_ONCHIP_BASE;	//  on-chip buffer
	//106+27, 146+27
	for (row = 106; row <= 133; row++){
		i=0;
		for (col = 146; col <= 173; col++){
			o_set = (row << 9) + col;						// compute offset
			img[i][j] = *(fpga_chip+o_set);
			//j++;
			i++;
		}
		//i++;
		j++;
	}
	*(VIDEO_IN_CONTROL_ptr + 3)  |= (1<<2);	// disable video_in
}
//Draws a 28x28 array based on the coords given (top_x to top_x+27) and (top_y to top_y+27)
void VGA_loadInit(int top_x, int top_y, short int img[][28]){
	int offset, row, col;
	int i = 0, j = 0;
	volatile short * pixel_buffer = (short *)SDRAM_BASE;	//  SRAM buffer
	/* assume that the box coordinates are valid */
	//for (row = top_x; row < (top_x+28); row++)
	for (row = top_y; row < (top_y+28); row++)
	{
		i=0;
		//for (col = top_y; col < (top_y+28); col++)
		for (col = top_x; col < (top_x+28); col++)
		{
			offset = (row << 9) + col;						// compute offset
			*(pixel_buffer + offset) = (short)img[i][j];		// set pixel value
			//j++;
			i++;
		}
		//i++;
		j++;
	}
}
int findAverage(short int img[][28]){
	
	volatile int row, col, R, G, B, gray, sum, avg;
	
	/* assume that the box coordinates are valid */
	sum = 0;
	for (row = 0; row < 28; row++)
	{
		for (col = 0; col < 28; col++)
		{
			R = (img[col][row] >> 11) & 0x1F;
			G = (img[col][row] >> 5) & 0x3F;
			B = (img[col][row]) & 0x1F;
			gray = (R + G + B) / 3;
			sum += gray;
		}
	}
	
	avg = sum/784;
	
	return avg;
	
}


/***************************************************************
	Calculate binary image based on threshold value
*************************************************************/
void findBinIMG(short src_img[][28], int threshold, char dst_img[][28]) {
	volatile  int row, col, R, G, B, RGB, gray, invertValues;
	volatile int * SW_ptr 				= (int *) SW_BASE ;	
	int i = 0, j = 0;
	invertValues = (*SW_ptr) & (1<<8);	// 8-bit threshold value
	
	j = 0;
	for (row = 0; row < 28; row++)
	{
		i=0;
		for (col = 0; col < 28; col++)
		{
			R = (src_img[col][row] >> 11) & 0x1F;
			G = (src_img[col][row] >> 5) & 0x3F;
			B = (src_img[col][row]) & 0x1F;
			gray = (R + G + B) / 3;
			
			if (gray > threshold)
			{
				if(invertValues){
					dst_img [i][j]		= (char)0x00;
				} else {
					dst_img [i][j]		= (char)0xFF;
				}
			}
			else
			{
				if(invertValues){
					dst_img [i][j]		= (char)0xFF;
				} else {
					dst_img [i][j]		= (char)0x00;
				}
			}
			
			i++;
		}
		j++;
	}
	
}

/*
 * (row, col, color) for a 29 pixel long vertical line
 */
void VGA_outline_x(int x1, int y1, short pixel_color){
    int offset, row, col;
    short *pixel_buffer = (short *)FPGA_ONCHIP_BASE; // pixel buffer
    /* assume that the box coordinates are valid */
	col = y1;
	for (row = x1; row <= x1+29; row++){
		offset = (row << 9) + col;
		*(pixel_buffer + offset) = (short)pixel_color;
	}
}
/*
 * (row, col, color) for a 29 pixel long horizontal line
 */
void VGA_outline_y(int x1, int y1, short pixel_color){
    int offset, row, col;
    short *pixel_buffer = (short *)FPGA_ONCHIP_BASE; // pixel buffer
    /* assume that the box coordinates are valid */
	row = x1;
	for (col = y1; col <= y1+29; col++){
		offset = (row << 9) + col;
		*(pixel_buffer + offset) = (short)pixel_color;
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




