

#include "address_map_arm.h"
#include "weights_fp.h"
#include "img.h"
#include "classify.h"
#include "address_map_arm.h"
#include "Image.h"
#include <stdio.h>

void VGA_load_image_sdram(short int image[][320]);
void VGA_load_sdram(int top_x ,int bot_x, int top_y, int bot_y,short);
void VGA_load_number_sdram(short int image[][28]);
void VGA_text(int top_x, int top_y, char * txt);
void VGA_box(int x1, int y1, int x2, int y2, short pixel_color);
void VGA_outline_y(int x1, int y1, short pixel_color);
void VGA_outline_x(int x1, int y1, short pixel_color);
void VGA_loadInit(int top_x,int top_y, short int img[][28]);
void VGA_loadImage(int top_x,int top_y, short int img[][28]);
void updateInput();
short findAverage(short img);
void RAMtoClassifier(short int (*)[28]);
void displayConversion();

#define BLACK 0x0000         // Black
#define WHITE 0xFFFF         // White
#define RED 0xF800           // Red
#define BLUE 0x001F          // Blue
#define GREEN 0x07E0         // Green
#define YELLOW 0xE742		     // Yellow
short initIMG[28][28]; //Holds the value pulled from the yellow box of the ON_CHIP buffer (Where the number will be)
char  networkInput[28][28]; //Converted equiv of initIMG to pass to the classifier
short conversion[28][28]; //Contains the green equivalent of networkInput
short testThresh[28][28]; //Contains a threshold/clamped equivalent of initIMG

int main(void){
	volatile int * VIDEO_IN_CONTROL_ptr  = (int *) VIDEO_IN_BASE;
	volatile int * KEY_ptr = (int *)KEY_BASE;
	volatile short * fpga_chip = (short *) FPGA_ONCHIP_BASE;	//  on-chip buffer
	volatile short * sdram = (short *)SDRAM_BASE;			       	//  SRAM buffer
	volatile int * VGA_DMA_CONTROL_ptr  = (int *) PIXEL_BUF_CTRL_BASE;
	volatile int row, col;
	volatile int show_live_video;
	volatile int * LED_ptr = LEDR_BASE;

	char text[] = "EC463 USNA MICROCOMPUTER INTERFACING FINAL PROJECT\0";
	char text2[] = "EC463 - DIGIT CLASSIFIER\0";
	char text3[] = "                        \0";
	/* Write a text string to VGA */
	VGA_text(20, 1, text);

	//generate a blue screen for SDRAM buffer
	short blue[240][320];
	for (row = 0; row <= 239; row++){
		for (col = 0; col <= 319; col++){
			blue[row][col] = 0x001F;
		}
	}

	// Intialize SDRAM buffer to a blue image
	VGA_load_image_sdram(Initial_Screen); //Image offsets are 117,140 to center the image

	// Initialize Video in and VGA interfaces
	*(VIDEO_IN_CONTROL_ptr + 3)  = (1<<2);			// enable live video --> frame = on-chip buffer by default
	*(VGA_DMA_CONTROL_ptr + 1) = FPGA_ONCHIP_BASE; 	// live video input will be shown on VGA
	*(VGA_DMA_CONTROL_ptr + 0) = 1;
	show_live_video = 1;

	while(1){
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
			if (show_live_video){
				*(VGA_DMA_CONTROL_ptr + 1) = (int)FPGA_ONCHIP_BASE;
			}
			else{
				(*LED_ptr) = 0xff;
				*(VIDEO_IN_CONTROL_ptr + 3)  &= ~(1<<2);	// disable video_in
				*(VGA_DMA_CONTROL_ptr + 1) = (int)SDRAM_BASE;
				updateInput();
				//VGA_load_number_sdram(initIMG);
				VGA_loadInit(117,140,initIMG);
				RAMtoClassifier(initIMG);
				VGA_loadInit(0,0,testThresh);
				displayConversion();
				VGA_loadInit(200,200,conversion);
				while((*(KEY_ptr + 3) & 0x02)==0){} //KEY(1) not detected
				(*LED_ptr) = (1 << classify(networkInput));
				*(KEY_ptr + 3) = (1 << 1);  // clear flag for KEY(2)
			}
			*(VGA_DMA_CONTROL_ptr + 0) = 1;

			*(VIDEO_IN_CONTROL_ptr + 3) |= (1<<2);	// enable video_in
			*(KEY_ptr + 3) = (1 << 2);  // clear flag for KEY(2)
		}
	}
}

/***************************************************************
	Function to load and dispaly an image (240x320) on SDRAM
*************************************************************/
void VGA_load_image_sdram(short int image[][320]){
	int offset, row, col;
	volatile short * pixel_buffer = (short *)SDRAM_BASE;	//  SRAM buffer
	/* assume that the box coordinates are valid */
	for (row = 0; row <= 239; row++){
		for (col = 0; col <= 319; col++){
			offset = (row << 9) + col;						// compute offset
			*(pixel_buffer + offset) = (short)(image[row][col]);		// set pixel value
		}
	}
}

/***************************************************************
	Function to load and dispaly a number (28x28 box) on SDRAM in the middle of the screen (100,100) to (127,127)
*************************************************************/
void VGA_load_number_sdram(short int image[][28]){
	int offset, row, col;
	int i = 0, j = 0;
	volatile short * pixel_buffer = (short *)SDRAM_BASE;	//  SRAM buffer
	/* assume that the box coordinates are valid */
	for (row = 106; row <= 146; row++){
		for (col = 146; col <= 173; col++){
			offset = (row << 9) + col;						// compute offset
			*(pixel_buffer + offset) = (short)(image[i][j++]);		// set pixel value
		}
		i++;
	}
}

/***************************************************************
	Function to draw a a string of text (NULL terminated)
	(x, y): coordinate of first character in the character buffer
	resolution of character buffer = 80x60
*************************************************************/
void VGA_text(int x, int y, char * text_ptr){
	int offset;
	char * character_buffer = (char *)FPGA_CHAR_BASE;	// VGA character buffer

	/* compute offset and set character */
	offset = (y << 7) + x;
	while (*(text_ptr)){// NULL character = end of array
		*(character_buffer + offset) = *(text_ptr);
		++text_ptr;
		++offset;
	}
}

//Draws a colored box from (top_x to bot_x) and (top_y to bot_y)
void VGA_load_sdram(int top_x,int bot_x, int top_y, int bot_y,short color){
	int offset, row, col;
	volatile short * pixel_buffer = (short *)SDRAM_BASE;	//  SRAM buffer
	/* assume that the box coordinates are valid */
	for (row = top_x; row <= bot_x; row++){
		for (col = top_y; col <= bot_y; col++){
			offset = (row << 9) + col;						// compute offset
			*(pixel_buffer + offset) = color;		// set pixel value
		}
	}
}

//Draws a 28x28 array based on the coords given (top_x to top_x+27) and (top_y to top_y+27)
void VGA_loadInit(int top_x,int top_y, short int img[][28]){
	int offset, row, col;
	int i = 0, j = 0;
	volatile short * pixel_buffer = (short *)SDRAM_BASE;	//  SRAM buffer
	/* assume that the box coordinates are valid */
	for (row = top_x; row < top_x+28; row++)
	{
		j=0;
		for (col = top_y; col < top_y+28; col++)
		{
			offset = (row << 9) + col;						// compute offset
			*(pixel_buffer + offset) = img[i][j++];		// set pixel value
		}
		i++;
	}
}

//Draws a 28x28 array based on the coords given (top_x to top_x+27) and (top_y to top_y+27)
void VGA_loadImage(int top_x,int top_y, short int img[][28]){
	int offset, row, col;
	int i = 0, j = 0;
	volatile short * pixel_buffer = (short *)SDRAM_BASE;	//  SRAM buffer
	/* assume that the box coordinates are valid */
	for (row = top_x; row < top_x+28; row++)
	{
		for (col = top_y; col < top_y+28; col++)
		{
			offset = (row << 9) + col;						// compute offset
			*(pixel_buffer + offset) = img[i][j++];		// set pixel value
		}
		i++;
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
/**
 * Updates the global var networkInput by pulling a 28x28 box that goes from (106,146) to (133,173) (the center of the box; where the yellow box is drawn)
 */
void updateInput(){
	int o_set = 0, row, col;
	int i = 0, j = 0;
	volatile short * fpga_chip = (short *) FPGA_ONCHIP_BASE;	//  on-chip buffer
	//106+27, 146+27
	for (row = 106; row <= 133; row++){
		for (col = 146; col <= 173; col++){
			o_set = (row << 9) + col;						// compute offset
			initIMG[i][j++] = *(fpga_chip+o_set);
		}
		i++;
	}
}

/**
 * Converts the RRRR RGGG GGGB BBBB (16 bit short) into a 8 bit grayscale value (0-255 is the amount of gray present) to pass to the classifier
*/
void RAMtoClassifier(short int ramIMG[][28]){
    int offset, row, col, red, blue, green, THRESHOLD;
    short gray, testThr;
	volatile int * SW_ptr = SW_BASE;
	//Upscale the value by three to extend the range from (0 to 41) to (0 to 255) (256/42 = 6)
	int scaleVal = 6;
    short *pixel_buffer = (short *)SDRAM_BASE; // pixel buffer
    for (row = 0; row <= 27; row++){
        for (col = 0; col <= 27; col++)
				{
            offset = (row << 9) + col;
            gray = findAverage(ramIMG[row][col]);
			testThr = gray;
            gray = gray*scaleVal;
			if (gray > 31){ // 63 is overflow. Green can overflow since it is 6 bits
				red = 0x1f;
				blue = BLUE; //1f
				green = 0x3f; //consider changing this to 63 (0x3f) if there is a purple tint **DEBUG THIS IF ANY ISSUES**
			} else {
				red = gray&0x1f;
				green = gray&0x3f;
				blue = gray&0x1f;
			}
			THRESHOLD = (*SW_ptr);
			testThr = (red << 11) | (green << 6) | blue;
			if (gray < THRESHOLD)
			{
				gray = 0;
				testThr = 0;
			}
			else {
				gray = 0xFF;
				testThr = 0xFFFF;
			}
			networkInput[row][col] = (char) gray;
			testThresh[row][col] = testThr;
			}
		}
}

/**
 * Computes the average value of RRRR RGGG GGGB BBBB
 * Range from: 0 to 41
*/
short findAverage(short img){
    short int red, green, blue, avg;
    red = (img & RED) >> 11;
    green = (img & GREEN) >> 5;
    blue = img & BLUE;
    avg = (red + green + blue) / 3;
    return avg;
}

/**
 * Shifts the average value (0 to 41) to the 6 bit Green field (0 to 63)
*/
void displayConversion(){
    int row, col;
    short gray;
    for (row = 0; row <= 27; row++){
        for (col = 0; col <= 27; col++){
            gray = findAverage(initIMG[row][col]);
            conversion[row][col] = gray << 5;
        }
    }
}
