function Parsing_image(start,finish)
    
    myfile = 't10k-images.idx3-ubyte';

    fid = fopen(myfile) ; 
    fout = fopen("Image.h",'w');
    % read magic number, num of images, num or rows, num of cols
    magic_num = fread (fid, 4, 'uint32', 'b' );  % read and  print 4 numbers in command window
    numOfImage = magic_num(2);

    for currentImage = 1:finish
    % read and display first image
    im1 = fread (fid, [28 28], 'uint8' );
    im1 = im1';
    if currentImage >= start
        fprintf(fout, '%s', 'unsigned char Image');
        fprintf(fout, '%d', currentImage);
        fprintf(fout, '%s', '[28][28] = {');
        for i = 1:numel(im1)
            fprintf(fout, '%d', im1(i));
            if i ~= 784
                fprintf(fout, '%s', ',');
            end
        end
        fprintf(fout, '%s\n', '};');
    end
    end

    fclose(fout);
    fclose(fid) ; 
end



