function Parsing_Weights
    clc;
    fout = fopen("weights_fp.h",'w');
    
    for currentImage = 0:9
        myfile = 'weights_';
        myfile = [myfile num2str(currentImage)];
        fid = fopen(myfile);
        fprintf(fout, '%s', ' float weights');
        fprintf(fout, '%d', currentImage);
        fprintf(fout, '%s', '[28][28] = { ');
        weight = fread (fid, [28 28] , 'float' );
        weight = weight';
        for i = 1:numel(weight)
            fprintf(fout, '%f', weight(i));
            if i ~= 784
                fprintf(fout, '%s', ',');
            end
        end
        fprintf(fout, '%s\n', '};');
        fclose(fid);
    end
    fclose(fout) ; 
end
