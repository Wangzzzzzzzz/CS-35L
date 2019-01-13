#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void checker (int arg_num, char * chararr[]){
    if (arg_num != 3){
        char err_message[] = "Number of arguments is incorrect. (Expect 2)\n";
        write(2, err_message, sizeof(err_message));
        exit(1);
    }
    char * from = chararr[1];
    char * to = chararr[2];

    if (strlen(from)!=strlen(to) ){
        char err_message[] = "The length of from and to should be the same.\n";
        write(2, err_message, sizeof(err_message));
        exit(1);
    }

    //duplications:
    int i = 0;
    int j;
    int length = strlen(from);
    for (; i < length; i++){
        for (j = i+1 ; j < length; j++){
            if(from[i]==from[j]){
                char err_message[] = "Unable to handle duplicated bytes in from.\n";
                write(2, err_message, sizeof(err_message));
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

    // record read
    char temp;

    while(read(0, &temp, 1) == 1){
        int index = search(temp, length, from);
        if(index != -1){
            write(1, &to[index], 1);
        }
        else{
            write(1, &temp, 1);
        }
    }
    return 0;
}
