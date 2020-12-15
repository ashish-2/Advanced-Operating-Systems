//UT EID: ama7677
//Name: Ashish M. Agarwal


#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[]){

	//if no files are mentioned just exit
	if(argc == 1){
		return 0;
	}

	int i=1;

	//loop over all mentioned files one by one
	while(i < argc){
		FILE *fi = fopen(argv[i], "r");
		if(fi == NULL){
			fprintf(stdout, "wcat: cannot open file\n");
			exit(1);
		}

		//create a buffer to use with fgets
		char buffer[64];

		//loop over current file and get it contents with the help of fgets and print it
		while(fgets(buffer, 63, fi) != NULL){
			printf("%s", buffer);
		}
		fclose(fi);
		i++;
	}
	
	return 0;
}