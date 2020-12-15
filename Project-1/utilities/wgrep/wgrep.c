//UT EID: ama7677
//Name: Ashish M. Agarwal


#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char *argv[]){

	//if no command line arguments are passed
	if(argc == 1){
		fprintf(stdout, "wgrep: searchterm [file ...]\n");
		exit(1);
	}

	//allocate buffer to store lines
	size_t buffer_size = 128;
    char *buffer = malloc(buffer_size * sizeof(char));

    if(buffer == NULL){
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

	//if only search term is passed, read from standard input
	if(argc == 2){
		while(getline(&buffer, &buffer_size, stdin) != -1){
			if (strstr(buffer , argv[1] )!= NULL)
      		{
         		printf("%s",buffer);
      		}
		}
		return 0;
	}

	//if files are passed
	if(argc > 2){
		int i=2;

		//loop over files one by one
		while(i < argc){
			FILE *fi = fopen(argv[i], "r");
			if(fi == NULL){
				fprintf(stdout, "wgrep: cannot open file\n");
				exit(1);
			}
			while(getline(&buffer, &buffer_size, fi) != -1){

				//check if search term is present in current line of current file 
				if (strstr(buffer , argv[1] )!= NULL)
      			{
         			printf("%s",buffer);	//print the line that contains the search term
      			}
			}
			fclose(fi);
			i++;
		}
	}
	return 0;
}