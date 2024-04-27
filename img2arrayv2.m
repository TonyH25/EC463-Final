% Code to convert a 24-bit image file to a 2D array bg_img (16-bit image)
% Dr. Hau Ngo

clc; clear all; close all;
filename = 'Numbers';  %%%% make sure that the right filename 
extention= '.png';      %%%% is showing here  
fn = strcat (filename, extention)% 24-bit input image,
fid = fopen('img.h','a+'); %append to the end of the file
img = imread (fn);
img = imresize(img, [240 320]);
%img2 = im2double(img .* 255);
figure, imshow(img);
title('original image');
img_size = size(img);

imgR = bitshift (img(:,:,1), -3); % Red (5 msb)
imgG = bitshift (img(:,:,2), -2); % Green (6 msb)
imgB = bitshift (img(:,:,3), -3); % Blue (5 msb)

% Red = bits (15:11), Green = bits (10:5), Blue = bits (4:0)
img_result = (uint16(imgR)*2^11) +  (uint16(imgG)*2^5) + (uint16(imgB)); 
figure, imshow(img_result, []); %should show gray scale image
title('16-bit image R(15:11), G(10:5), B(4:0)');

% output pixel values to 2d array in bg_image.h file %
img_result = img_result';
fprintf(fid, 'short int ');
fprintf(fid, '%s[', filename);
fprintf(fid, '%d][', img_size(1));
fprintf(fid, '%d] = {', img_size(2));
fprintf(fid, ' %d,', img_result (1:img_size(2), 1:(img_size(1)-1)));
fprintf(fid, ' %d };\n', img_result (img_size(2), img_size(1)));
fprintf(fid, ' %d,', img_result (1:img_size(2), 1:img_size(1) ) );
fprintf(fid, ' };\n');

fclose(fid);