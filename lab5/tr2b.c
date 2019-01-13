#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void checker (int arg_num, char * chararr[]){
    if (arg_num != 3){
        fprintf(stderr, "Number of arguments is incorrect. (Expect 2 but get %d)\n", arg_num-1);
        exit(1);
    }
    char * from = chararr[1];
    char * to = chararr[2];

    if (strlen(from)!=strlen(to) ){
        fprintf(stderr, "The length of from and to should be the same.\n");
        exit(1);
    }

    //duplications:
    int i = 0;
    int j;
    int length = strlen(from);
    for (; i < length; i++){
        for (j = i+1 ; j < length; j++){
            if(from[i]==from[j]){
                fprintf(stderr, "Unable to handle duplicated bytes in from.\n");
                exit(1);   //ther is duplicateds
            }
        }
    }
}

int search(char target, int range, char * dict){
    int i = 0;
    for (; i < range; i++){
        if(dict[i] == target){
            return i;
        }
    }
    return -1;
}

int main(int argc, char * argv[]){
    checker(argc,argv);

    // obtain variables
    char * from = argv[1];
    char * to  = argv[2];
    int length = strlen(from);

    // record std in
    char temp;
    while(1){
        temp = getchar();
        if(ferror(stdin)){
            fprintf(stderr, "Error while inputting string.\n");
            exit(1);
        }

        if(feof(stdin)){ // end of input
            break;
        }

        int index = search(temp, length, from);
        if(index != -1){
            putchar(to[index]);
        }
        else{
            putchar(temp);
        }

        if (ferror(stdout)){
            fprintf(stderr, "Error while outputting string.\n");
            exit(1);
        }
    }
    return 0;
}
