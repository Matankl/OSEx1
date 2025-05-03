   
/*
 * File: hello.c
 */
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char *string, *string_so_far;
    int i, length;     length = 0;
    for(i=0; i<argc; i++) {
        length += strlen(argv[i])+1;
        string = malloc(length+1);
 
        /*  * Copy the string built so far. */
        if(string_so_far != (char *)0)
            strcpy(string, string_so_far);
        else *string = '\0';
        strcat(string, argv[i]);
        if(i < argc-1) strcat(string, " ");
        string_so_far = string;
    }
    printf("You entered: %s\n", string_so_far);
    return (0);
}

// the leak is in the malloc, the string is being allocated in every iteration of the loop, but it is never freed
// string = malloc(length+1);

/*
the fix is to free the string after the strcpy

    if (string_so_far != NULL) {
        // Copy old contents into the new buffer
        strcpy(string, string_so_far);
        // Free the old buffer
        free(string_so_far);



*/