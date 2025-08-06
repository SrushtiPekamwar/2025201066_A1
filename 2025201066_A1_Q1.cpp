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
#include<unistd.h>
#include<cstring>
#include<cstdlib>
#include<iostream>

// function to print onto the console, this is just like the wrapper of printf 
void printOnConsole(const char *msg) {
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
                // block size is valid and now perform the operation 
            }
        }
    }

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

    else if(flag==2) {
        if(argc!=5) {
            printOnConsole("Wrong number of arguments\n");
            printCommandUsage();
            return 1;
        } 
        else {
            // get the filename, flag, start index and the end index
        }
    }

    else {
        printOnConsole("Invalid flag value. Valid flags are 0,1,2\n");
        printCommandUsage();
        return 1;
    }

    return 0;
}