#include <string.h>
#include <stdlib.h>

/**
 * Function count how many spaces in gen string
 * @param str
 * @return
 */

int howManySpaces(char *str) {
    int space = 0;
    int i = 0;
    while (i <= str[i]) {
        if (str[i] == ' ') {
            space++;
        }
        i++;
    }
    return space;
}

/**
 * Function Create array of strings fro user input
 * Example :
parsing the string: "cat file.c > output.txt &"
results:arguments = {"cat","file.c",">","output.txt","&"},
 */
char **inputParser(char *string) {
    int space_num = howManySpaces(string);
    string[strcspn(string, "\n")] = 0;
    char **argv;
    argv = malloc(space_num * sizeof(char *));
    int i = 0;
    argv[i] = strtok(string, " ");//separate string by spaces
    while (argv[i] != NULL) {
        argv[++i] = strtok(NULL, " ");
    }
    return argv;
}

//char *string =  "Mark,John,Mathew";
//    char *argv[10];
//    int i=0;
//
//    argv[i] = strtok(string,",");
//
//    while(argv[i]!=NULL)
//    {
//    argv[++i] = strtok(NULL,",");
//    }

// ls -v ls
// *[1] = "ls"
// *[2] = "-v"

// ls | DIR | ip
//ls|dir
//ls|||||dir