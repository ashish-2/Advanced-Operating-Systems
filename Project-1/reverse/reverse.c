//UT EID: ama7677
//Name: Ashish M. Agarwal


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


//structure of doubly linked list (Doubly linked list is used because it is easy to traverse backwards)
struct Node { 
    struct Node* prev;
    char *line; 
    struct Node* next; 
}; 


//main starts here...
int main(int argc, char *argv[]){

    //if more than two filenames have been passed
    if(argc > 3){
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }
    
    
    //create a buffer to use with getline function
    size_t buffer_size = 128;
    char *buffer = malloc(buffer_size * sizeof(char));

    if(buffer == NULL){
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    //create head of doubly linked list
    struct Node* head = NULL;
    head = (struct Node*)malloc(sizeof(struct Node));
    if(head == NULL){
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }


    //if no input file is specified take input from stdin and print output to stdout
    if(argc == 1){
        getline(&buffer, &buffer_size, stdin);
        //create character array to store contents of buffer (since buffer gets overwritten continuously)
        char *c1 = malloc(buffer_size * sizeof(char));
        if(c1 == NULL){
            fprintf(stderr, "malloc failed\n");
            exit(1);
        }
            strcpy(c1, buffer); //copy contents of buffer to character array
            head->line = c1;    //store address of char array in node
            head->prev = NULL;
            head->next = NULL;


    struct Node* pre = head;
    

    while(getline(&buffer, &buffer_size, stdin) != -1)
    {
        //create new node to store current line
        struct Node* cur = (struct Node*)malloc(sizeof(struct Node));
        if(cur == NULL){
        fprintf(stderr, "malloc failed\n");
        exit(1);
        }

        pre->next = cur;    //make next pointer of previous node point to current node
        cur->prev = pre;    //make prev pointer of current node point to previous node
        char *c = malloc(buffer_size * sizeof(char));   //create char array to store contents of buffer
        if(c == NULL){
        fprintf(stderr, "malloc failed\n");
        exit(1);
        }
        strcpy(c, buffer);  //copy contents of buffer to char array
        cur->line = c;      //store address of char array in current node
        cur->next = NULL;

        pre = cur;          //make current node as pre for next iteration
    }

    struct Node *temp = pre;    //create a pointer to pre node, pre node is the last node in the list. So now temp points to end of list.

    //traverse the linked list backwards and print to standard output
    while(temp != NULL){
        fprintf(stdout, "%s", temp->line);
        temp = temp->prev;
    }
    fflush(stdout);
    free(buffer);


    //delete linked list
        
        while(head != NULL){
            temp = head;
            head = head->next;
            free(temp);
            temp = NULL;
        }
    }

    //if input file is speciified
    if(argc > 1){

        //create a file pointer to given file in read mode
        FILE *fi = fopen(argv[1], "r");
        if(fi == NULL){
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[1]);
            exit(1);
        }

    size_t buffer_size = 128;
    char *buffer = malloc(buffer_size * sizeof(char));

    if(buffer == NULL){
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    struct Node* head = NULL;
    head = (struct Node*)malloc(sizeof(struct Node));
    if(head == NULL){
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    getline(&buffer, &buffer_size, fi);
    char *c1 = malloc(buffer_size * sizeof(char));
    if(c1 == NULL){
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }
    strcpy(c1, buffer);
    head->line = c1;
    head->prev = NULL;
    head->next = NULL;


    struct Node* pre = head;
    

    while(getline(&buffer, &buffer_size, fi) != -1)
    {
        struct Node* cur = (struct Node*)malloc(sizeof(struct Node));
        if(cur == NULL){
        fprintf(stderr, "malloc failed\n");
        exit(1);
        }

        pre->next = cur;
        cur->prev = pre;
        char *c = malloc(buffer_size * sizeof(char));
        if(c == NULL){
        fprintf(stderr, "malloc failed\n");
        exit(1);
        }
        strcpy(c, buffer);
        cur->line = c;
        cur->next = NULL;

        pre = cur;


    }


    struct Node *temp = pre;

    //if output file is not specified, print to standard output
    if(argc == 2){
        while(temp != NULL){
        fprintf(stdout, "%s", temp->line);
        temp = temp->prev;
        }
    }

    //if output file is specified
    if(argc == 3){
        //error handling if both files are the same
        if(!(strcmp(argv[1], argv[2]))){
            fprintf(stderr, "reverse: input and output file must differ\n");
            exit(1);
        }

        //create a file pointer to given file in write mode
        FILE *fo = fopen(argv[2], "w");
        if(fo == NULL){
            exit(1);
        }
        //inode check for hard link
        struct stat buf1;
        stat(argv[1],&buf1);

        struct stat buf2;
        stat(argv[2],&buf2);

        //if inode numbers are same, the files are hard linked
        if(buf1.st_ino == buf2.st_ino){
            fprintf(stderr, "reverse: input and output file must differ\n");
            exit(1);
        }
        

        //traverse the doubly linked list backwards and print out each line to the output file 
        while(temp != NULL){
        fprintf(fo, "%s", temp->line);
        temp = temp->prev;
        }

        fclose(fo);

        //delete linked list
        
        while(head != NULL){
            temp = head;
            head = head->next;
            free(temp);
            temp = NULL;
        }
    }
    

    fflush(stdout);
    fclose(fi);
    free(buffer);
    }
    
    return 0;
}
