function Parsing_labels(start,finish)

    myfile = 't10k-labels.idx1-ubyte';

    fid = fopen(myfile) ; 
    fout = fopen("Label.h",'w');
    % read magic number, num of images, num or rows, num of cols
    magic_num2 = fread (fid, 2, 'uint32', 'b' );  % read and  print 4 numbers in command window
    numOfImage = magic_num2(2);

    fprintf(fout, '%s', 'unsigned char Labels[] = { ');
    for currentImage = 1:finish
        im1 = fread (fid, 1 , 'uint8' );
        if currentImage >= start
            fprintf(fout, '%d', im1);
            if currentImage ~= finish
                fprintf(fout, '%s', ', ');
            end
        end
    end
    fprintf(fout, '%s\n', '};');

    fclose(fout);
    fclose(fid) ; 
end