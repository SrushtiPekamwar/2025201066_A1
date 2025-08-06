#include<iostream>
#include<stdio.h>
#include<unistd.h> // contains read, write, close sys calls
#include<cstring>
#include<cstdlib>
#include<fcntl.h> // contains open sys call
#include<errno.h>
#include<libgen.h> // for basename function
#include<sys/stat.h>
#include<sys/types.h> // mode_t

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
    const long long maxBlockSize = LLONG_MAX;
    char* endptr;
    long long blocksize = strtoll(c, &endptr, 10);
    if (*endptr != '\0') {
        printOnConsole("Invalid block size, block size must be an integer\n");
        return -1;
    }

    if(blocksize<0) {
        printOnConsole("Block size should be positive\n");
        return -1;
    }

    if(blocksize>maxBlockSize) {
        printOnConsole("Block size is too large\n");
        return -1;
    }

    return blocksize;
}

// function to check whether the value of flag is valid
int isFlagValid(const char *c) {
    char* endptr;
    long long flag = strtoll(c, &endptr, 10);

    // Check if input is a valid integer
    if (*endptr != '\0') {
        printOnConsole("Flag value must be an integer either 0, 1 or 2\n\n");
        printCommandUsage();
        _exit(1);
    }

    // Check if flag is in allowed range
    if (flag != 0 && flag != 1 && flag != 2) {
        printOnConsole("Invalid flag value, valid flags values are 0, 1, or 2\n\n");
        printCommandUsage();
        _exit(1);
    }
    return flag;
}

// function to print the integers by converting it into string 
void printInteger(long long number) {
    char buffer[64]; 
    snprintf(buffer,sizeof(buffer),"%lld",number);
    printOnConsole(buffer);
}

// checks whether file exists or its has required permissions 
void fileValidation(int fileDesc) {
    if (fileDesc == -1) {
        if (errno == ENOENT) {
            perror("File does not exist");
            _exit(1);
        } else if (errno == EACCES) {
            perror("You don't have required permissions to access this file");
            _exit(1);
        }
    }
}

// function to read from the file 
void readContentsOfFile(int fileDesc) {
    char buffer[1024];
    ssize_t infoRead;
    while ((infoRead = read(fileDesc, buffer, sizeof(buffer))) > 0) {
        write(1, buffer, infoRead);
    }

    if (infoRead == -1) {
        printOnConsole("Error in reading of the file\n");
        close(fileDesc);
        _exit(1);
    }
}

// directory creation with read, write and execute permissions 
void createDirectory(const char *directoryName) {
    struct stat stats;

    // Check if the directory exists
    if (stat(directoryName, &stats) == 0) {
        if(S_ISDIR(stats.st_mode)) {
            printOnConsole(directoryName);
            printOnConsole(" already exists");
            // check what permissions are present for the directory
            printf("Permissions: %o\n", stats.st_mode & 0777); 
            return;
        } 
        else {
            printOnConsole(directoryName);
            perror(" exists but is not a directory");
            _exit(1);
        }
    }

    // Create directory and give user permissions of read, write and execute
    else if (mkdir(directoryName, 0700) == -1) {
        perror("Error while creating directory named 'Assignment1'");
        _exit(1);
    } 
    else {
        printOnConsole("Directory 'Assignment1' created successfully.\n");
        // check what permissions are present for the directory
        printf("Permissions: %o\n", stats.st_mode & 0777);  
    }
}

// file creation with read and write permissions 

int main(int argc, char *argv[]) {
    if(argc<3) {
        printOnConsole("Number of arguments are fewer than expected\n");
        printCommandUsage();
        return 1;
    }

    else if(argc>5) {
        printOnConsole("Number of arguments are greater than expected\n");
        printCommandUsage();
        return 1;
    }

    // flag is integer but we get string type from the console 
    long long flag = isFlagValid(argv[2]);

    //-------------------------------flag 0 is used to do block-wise reversal---------------------------
    if(flag==0) {
        if(argc!=4) {
            printOnConsole("Wrong number of arguments\n");
            printCommandUsage();
            return 1;
        }
        else {
            // ./a.out <input_file> 0 <block_size>
            const char* filepath = argv[1];
            char* pathCopy = strdup(filepath); 
            // we can get the file name using basname 
            char* originalFilename = basename(pathCopy);

            char *endptr;
            long long blocksize = isBlockSizeValid(argv[3]);

            // the property of stroll is that it will read only till the digits are present 
            // if it is 123ds then it will read till 123 and then endptr will not return 
            if(blocksize==-1) {
                return 1;
            }
            else if(blocksize==0) {
                printOnConsole("Block size should be greater than 0\n");
                return 1;
            }
            // block size is valid and now perform the operation 
            else {
                printOnConsole("Flag: ");
                printInteger(flag);
                printOnConsole("\nBlock size: ");
                printInteger(blocksize);
                printOnConsole("\n");

                // open the file in read only 
                int originalFileDesc = open(filepath, O_RDONLY); 

                // validating the file 
                fileValidation(originalFileDesc);
                printOnConsole("File opened successfully\n");

                // reading and printing the contents of the file
                // readContentsOfFile(originalFileDesc);

                /* create new file and then reverse the content in the another file
                before creating new file check whether the folder exists or not 
                then create new file with the format Assignment1/0_<input_file_name>
                now do blockwise reversal */

                struct stat fileStat;
                if (fstat(originalFileDesc, &fileStat) == -1) {
                    perror("\nError with fstat");
                    return 1;
                }
                // off_t is 64 bits and hence can store till ~8 exa bytes
                off_t originalFileSize = fileStat.st_size;  

                // directory creation 
                createDirectory("Assignment1");

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

    return 0;
}