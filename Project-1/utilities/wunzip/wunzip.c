//UT EID: ama7677
//Name: Ashish M. Agarwal


#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char *argv[]){

	//if no input files are provided
	if(argc < 2){
		printf("wunzip: file1 [file2 ...]\n");
		exit(1);
	}

	int n;	//to store 4 byte integer from the zip file that mentions the number of times a character occurs
	char c;	//to store 1 byte character form the zip file that follows the number of times it occurs (a character byte follows 4 bytes of integer)


	int i=1;

	//start uncompressing the files one by one
	while(i < argc){
		FILE *fi = fopen(argv[i], "rb"); //open file in binary read mode

		//if error while opening file
		if(fi == NULL){
			fprintf(stdout, "wzip: cannot open file\n");
			exit(1);
		}

		//loop as long as integer is read out (since the character follows the integer as they occur in pairs i.e; 4 byte integer followed by 1 byte character)
		
		while(fread(&n, sizeof(int), 1, fi) == 1){	//now n has the count of the number of times the character following it occurs
			
			fread(&c, sizeof(char), 1, fi);			// c stores the character that occurs n times

			//print the character c the number of times it occurs i.e; n times
			for(int i=1; i<=n; i++){
				printf("%c", c);
			}
		}
    	
		fclose(fi);
		i++;
	}

	return 0;
}