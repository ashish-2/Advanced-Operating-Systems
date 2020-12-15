//UT EID: ama7677
//Name: Ashish M. Agarwal


#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char *argv[]){

    //if no files are specified
	if(argc < 2){
		printf("wzip: file1 [file2 ...]\n");
		exit(1);
	}


	char c;

	//compress first file
	FILE *fi = fopen(argv[1], "r");
		if(fi == NULL){
			fprintf(stdout, "wzip: cannot open file\n");
			exit(1);
		}

		c = fgetc(fi);        //get first character
		char prev = (char) c; //assign it to prev
		int count = 1;        //make count as 1

        //loop over the file character by character
		while ((c = fgetc(fi)) != EOF)
    	{
        	char cur = (char) c;   //get current character
            

            //compare current character with previous character, if they are the same then increment count else reset count and prev
        	if(cur != prev){
        		fwrite(&count, sizeof(int), 1, stdout);
    			fwrite(&prev, sizeof(char), 1, stdout);
        		prev = cur;
        		count = 1;
        	}else{
        		count++;
        	}
    	}
    	
		fclose(fi);

	//compress further files if present
	int i=2;
    //loop over the next files one by one
	while(i < argc){
		fi = fopen(argv[i], "r");
		if(fi == NULL){
			fprintf(stdout, "wzip: cannot open file\n");
			exit(1);
		}


		while ((c = fgetc(fi)) != EOF)
    	{
        	char cur = (char) c;
        	if(cur != prev){
        		fwrite(&count, sizeof(int), 1, stdout);
    			fwrite(&prev, sizeof(char), 1, stdout);
        		prev = cur;
        		count = 1;
        	}else{
        		count++;
        	}
    	}
    	
		fclose(fi);
		i++;
	}
    //output last character that was left out due to EOF condition
	fwrite(&count, sizeof(int), 1, stdout);
    fwrite(&prev, sizeof(char), 1, stdout);
    return 0;
}