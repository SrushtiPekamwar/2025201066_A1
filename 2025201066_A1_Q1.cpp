/* 
command line 
$ g++ Q1.cpp
$ ./a.out <input file name> <flag> [additional arguments]
for flag 0: 
    Usage: ./a.out <input_file> 0 <block_size>
    Output file: Assignment1/0_<input_file_name>
for flag 1: 
    Usage: ./a.out <input_file> 1
    Output file: Assignment1/1_<input_file_name>
for flag 2: 
    Usage: ./a.out <input_file> 2 <start_index> <end_index>
    Output file: Assignment1/2_<input_file_name>
*/

#include<stdio.h>
#include<unistd.h> // contains read, write, close sys calls
#include<cstring>
#include<cstdlib>
#include<iostream>
#include<fcntl.h> // contains open sys call
#include<errno.h>

// function to print onto the console, this is just like the wrapper of printf 
void printOnConsole(const char *msg) {
    // write(int fd, const void *buf, size_t count)
    // ssize_t is the type which is used as return type to many sys calls 
    ssize_t e = write(1,msg,strlen(msg));

    if(e==-1) {
        const char *error = "Failed to print to the console\n";
        // the first arg of write is the file desc where 1 - stdout and 2 is stderr
        write(2,error,strlen(error));
        _exit(1);
    }
}

// function to print the usage of the command 
void printCommandUsage() {
    printOnConsole("-------------------------------Command usage-------------------------------\n");
    printOnConsole("Blockwise reversal     : ./a.out <input_file> 0 <block_size>\n");
    printOnConsole("Full file reversal     : ./a.out <input_file> 1\n");
    printOnConsole("Partial range reversal : ./a.out <input_file> 2 <start_index> <end_index>\n");
    printOnConsole("---------------------------------------------------------------------------\n\n");
}

// function to check whether the given block size is valid integer or not 
long long isBlockSizeValid(const char *c) {
    char* endptr;
    long long blocksize = strtoll(c, &endptr, 10);
    if (*endptr != '\0') {
        printOnConsole("Invalid block size, block size must be an integer\n");
        return -1;
    }
    else if(blocksize<0) {
        printOnConsole("Block size should be positive\n");
        return -1;
    }
    return blocksize;
}

// function to print the integers by converting it into string 
void printInteger(long long number) {
    char buffer[64]; 
    snprintf(buffer,sizeof(buffer),"%lld",number);
    printOnConsole(buffer);
}

// checks whether file exists or its has required permissions 
bool fileValidation(int fileDesc) {
    if (fileDesc == -1) {
        if (errno == ENOENT) {
            printOnConsole("File does not exist\n");
            return false;
        } else if (errno == EACCES) {
            printOnConsole("You don't have required permissions to access this file\n");
            return false;
        }
    }
    return true;
}

bool readContentsOfFile(int fileDesc) {
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(fileDesc, buffer, sizeof(buffer))) > 0) {
        write(1, buffer, bytesRead);
    }

    if (bytesRead == -1) {
        printOnConsole("Error in reading of the file\n");
        close(fileDesc);
        return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    if(argc<3) {
        printOnConsole("Few arguments\n");
        printCommandUsage();
        return 1;
    }

    else if(argc>5) {
        printOnConsole("Too many arguments\n");
        printCommandUsage();
        return 1;
    }

    // flag is integer but we get string type from the console 
    int flag = atoi(argv[2]);

    //-------------------------------flag 0 is used to do block-wise reversal---------------------------
    if(flag==0) {
        if(argc!=4) {
            printOnConsole("Wrong number of arguments\n");
            printCommandUsage();
            return 1;
        }
        else {
            // ./a.out <input_file> 0 <block_size>
            const char* filename = argv[1];
            char *endptr;
            long long blocksize = isBlockSizeValid(argv[3]);

            // the property of stroll is that it will read only till the digits are present 
            // if it is 123ds then it will read till 123 and then endptr will not return 
            if(blocksize==-1) {
                return 1;
            }
            else {
                // print the flag 
                printOnConsole("Flag: ");
                printInteger(flag);
                printOnConsole("\nBlock size: ");
                printInteger(blocksize);
                printOnConsole("\n");

                // block size is valid and now perform the operation 

                // open the file in read only 
                int originalFileDesc = open(filename, O_RDONLY); 

                // validating the file 
                if(fileValidation(originalFileDesc)==false) return 1;
                else {
                    printOnConsole("File opened successfully\n");
                }

                // reading the contents of the file
                if(readContentsOfFile(originalFileDesc)==false) return 1;

                // create new file and then reverse the content in the another file 
                // new file must follow this format Assignment1/0_<input_file_name>


                // close the file 
                close(originalFileDesc);
            }
        }
    }

    // flag 1 is used to do complete file reversal
    else if(flag==1) {
        if(argc!=3) {
            printOnConsole("Wrong number of arguments\n");
            printCommandUsage();
            return 1;
        }
        else {
            // get the filename, flag
        }
    }

    // flag 2 is used to do the reversal in the given range of index
    else if(flag==2) {
        if(argc!=5) {
            printOnConsole("Wrong number of arguments\n");
            printCommandUsage();
            return 1;
        } 
        else {
            // get the filename, flag, start index and the end index
            // start index can't be greater than end index 
        }
    }

    else {
        printOnConsole("Invalid flag value. Valid flags are 0,1,2\n");
        printCommandUsage();
        return 1;
    }

    return 0;
}