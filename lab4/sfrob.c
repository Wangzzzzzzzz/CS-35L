#include <stdio.h>
#include <stdlib.h>

const char decoder = '*';
const int ARRAY_SIZE = 10;
const int CHAR_SIZE = 10;

int frobcmp(char const * a, char const * b){
    if (a == NULL || b == NULL){
        fprintf(stderr, "Invalid inputs: character array cannot be NULL");
        exit(1);
    }
    int i = 0;
    while(a[i] == b[i]){
        //increment until one of them hits the space
        if (a[i] == ' ' || b[i] == ' '){
            return 0;
        }
        i++;
    }
    if (a[i] != ' ' && b[i] != ' '){
        int diff = ((a[i]) ^ decoder) < ((b[i]) ^ decoder) ? -1 : 1;
        return diff;
    }
    else if(a[i] == ' '){
        return -1;
    }
    else{
        return 1;
    }
}

// frobcmp does not work directly, need to use this function to do comparision
int qsort_fcmp (const void * left, const void * right){
    return ( frobcmp( *(char const **)left, *(char const **)right ) );
}

void free_space(char ** wordlist, int list_size){
    int index;
    for(index = 0; index < list_size; index++){
        free(wordlist[index]);
    }
    free(wordlist);
}

int main(){
    char ** wordlist = (char **)malloc(ARRAY_SIZE * sizeof(char *));
    char temp_input;

    // track the whole word list
    int list_size = ARRAY_SIZE;  // capacity of the wordlist
    // how much of the wordlist is occupied
    // increment when a char pointer is initialized
    int occupied_size = 0;

    // track word
    int wordlen = 0;     // how much of the char array is filled
    int wordsize = 0;    // char array capacity

    // track which character pointer are we currently working on
    int index = 0;

    if (wordlist == NULL){
        fprintf(stderr, "Memory allocation to wordlist failed.");
        exit(1);
    }

    while(1){
        temp_input = getchar();

        if (ferror(stdin)){
            free_space(wordlist,occupied_size);
            fprintf(stderr, "Cannot read characters");
            exit(1);
        }

        if (feof(stdin)){
            break;
        }

        // change size of the character array (word)
        if (wordlen >= wordsize){
            if (wordsize == 0){ // char array not initialized
                wordlist[index] = (char *)malloc(CHAR_SIZE * sizeof(char));
                if (wordlist[index] == NULL){
                    free_space(wordlist,occupied_size);
                    fprintf(stderr, "Cannot allocate memory for character arrary");
                    exit(1);
                }
                wordsize += CHAR_SIZE;
                occupied_size++;
            }
            else{ // array is initialized
                wordlist[index] = (char *)realloc(wordlist[index], (wordsize + CHAR_SIZE) * sizeof(char));
                if (wordlist[index] == NULL){
                    free_space(wordlist,occupied_size);
                    fprintf(stderr, "Cannot reallocate memory for character arrary");
                    exit(1);
                }
                wordsize += CHAR_SIZE;
            }
        }

        // change the size of the wordlist
        if (occupied_size >= list_size){
            wordlist = (char **)realloc(wordlist, (list_size + ARRAY_SIZE) * sizeof(char *));
            if(wordlist == NULL){
                free_space(wordlist,occupied_size);
                fprintf(stderr, "Cannot reallocate memory for wordlist");
                exit(1);
            }
            list_size += ARRAY_SIZE;
        }

        // put the new char in
        wordlist[index][wordlen] = temp_input;
        wordlen++;

        if (temp_input == ' '){
            index++;
            wordsize = 0;
            wordlen = 0;
        }

    }

    if (wordlen != 0 && wordlist[index][wordlen-1] != ' '){
        // check if we are putting sp on invalid space 
        if (wordlen >= wordsize){
            wordlist[index] = (char *)realloc(wordlist[index], (wordsize + CHAR_SIZE) * sizeof(char));
            if (wordlist[index] == NULL){
                free_space(wordlist,occupied_size);
                fprintf(stderr, "Cannot reallocate memory for character arrary");
                exit(1);
            }
            wordsize += CHAR_SIZE;
        }
        wordlist[index][wordlen] = ' ';
        index++;
        wordsize = 0;
        wordlen = 0;
    }

    qsort(wordlist,index,sizeof(char *), qsort_fcmp);

    // output result
    int i=0;
    int j;
    for(; i < occupied_size; i++){
        for(j=0; 1; j++){
            int result = putchar(wordlist[i][j]);
            if(result == EOF){
                free_space(wordlist,occupied_size);
                fprintf(stderr, "Fail to output the character");
                exit(1);
            }
            if(wordlist[i][j]==' '){
                // if we just output space successfully, stop the inner loop.
                break;
            }
        }
    }

    // free the free
    free_space(wordlist,occupied_size);
    return 0;
}
