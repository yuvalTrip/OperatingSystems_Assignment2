
#include "shellCode.h"

//This function reset the char array that contains the path to the current work directory
void reset_path() {
    char path[MAX_LENGTH];
    getcwd(path, sizeof(path));
}

int main(int argc, char *argv[]) {
    char *input;
    size_t size = 256;

    while (1) {
        input = (char *) malloc(size);
        if (input == NULL) { // malloc unsuccessful
            exit(1);
        }
        //Get an input from the user
        read(STDIN_FILENO, input, 256);

        //Execute the command
        int code = input_check(input);

        //Exit if the code is 1
        if (code == 1) {
            free(input);
            exit(0);
        }
    }
}

