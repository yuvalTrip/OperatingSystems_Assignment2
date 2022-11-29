
#include "shellCode.h"

//This function reset the char array that contains the path to the current work directory
void reset_path()
{
    char path[MAX_LENGTH];
    getcwd(path, sizeof(path));
}

int main(int argc, char *argv[])
{
    char *input;
    size_t size = 256;

    while (1)
    {
        /**
         * @brief Changes of Part 2
         * the printf row was the one we were asked to change in part 2 of the assignment
         */
        //printf("%s\n", get_prompt());
        //Print the path and the prompt
//        printf("%s %s\n", get_path(), get_prompt());
        while (1)
        {
            input = (char *) malloc(size);
            if (input == NULL)
            { // malloc unsuccessful
                exit(1);
            }
            //Get an input from the user
            read(STDIN_FILENO, input, 256);

            //Execute the command
            int code = input_check(input);

            //Exit if the code is 1
            if (code == 1)
            {
                free(input);
                exit(0);
            }
        }
    }
}


int caseDir()
{
    //Setting the directory and the struct of the entries
    DIR *directory;
    struct dirent *entry;
    //Open the directory
    directory = opendir(".");
    if (directory == NULL)
    {
        puts("Cannot open the directory");
        return 1;
    } else
    {
        puts("Directory is opened!\n");
        //Read the names of the entries in the directory
        while (entry = readdir(directory))
        {
            printf("%s\n", entry->d_name);
        }
        //Close the directory
        closedir(directory);
        return 0;
    }
}