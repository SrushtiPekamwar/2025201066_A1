#include<stdio.h>
#include<unistd.h> // contains read, write, close sys calls
#include<cstring>
#include<cstdlib>
#include<fcntl.h> // contains open sys call
#include<errno.h>
#include<libgen.h> // for basename function
#include<sys/stat.h>
#include<sys/types.h> // mode_t
#include<iostream>

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
    char* endptr;  // endptr will point to the first invalid character after the number.
    long long blocksize = strtoll(c, &endptr, 10);
    if (*endptr != '\0') {
        printOnConsole("Invalid block size, block size must be an integer\n");
        _exit(1);
    }

    // block size must be positive value
    if(blocksize<=0) {
        printOnConsole("Block size should be positive\n");
        _exit(1);
    }

    // to avoid overflow 
    if(blocksize>maxBlockSize) {
        printOnConsole("Block size is too large\n");
        _exit(1);
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
    return (int)flag;
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
        if (errno==ENOENT || errno==EACCES) {
            printOnConsole(strerror(errno));
            printOnConsole("\n");
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
    // add condition that error while reading 
}

// directory creation with read, write and execute permissions 
void createDirectory(const char *directoryName) {
    struct stat stats;

    // Check if the directory exists
    if (stat(directoryName,&stats)==0) {
        // st_mode returns true if it is a directory 
        printOnConsole(directoryName);
        printOnConsole(" already exists\n");
        // printf("Permissions: %o\n", stats.st_mode & 0777); 
        return;
    }

    // Create directory and give user permissions of read, write and execute
    else {
        mkdir(directoryName, 0700);
        printOnConsole("Directory 'Assignment1' created successfully\n");
    }
}

// function to perform block wise reversal
void performBlockwiseReversal(int inputFileDesc, int outputFileDesc, long long blockSize, off_t fileSize) {
    // allocating in the heap so that there won't be any stack overflow 
    char *buffer = (char*)malloc(blockSize);
    off_t offset = 0; 

    while (offset<fileSize) {
        ssize_t bytesToRead = blockSize;

        // when the last remaining characters are less than the block size then it will just consider the remaining characters
        // this will also handle when the block size is greater than the file size 
        if (offset+blockSize>fileSize) {
            bytesToRead = fileSize-offset;
        }

        // Move to correct offset in input file
        lseek(inputFileDesc, offset, SEEK_SET);
        // Read the block and pass into the buffer block
        ssize_t totalBytesRead = read(inputFileDesc, buffer, bytesToRead);
        // Reverse the contents of the current block
        for(ssize_t i=0;i<blockSize/2;++i) {
            char temp = buffer[i];
            buffer[i] = buffer[blockSize-i-1];
            buffer[blockSize-i-1] = temp;
        }
        // Write to the output file
        write(outputFileDesc, buffer, totalBytesRead);
        // update the offset value 
        offset += totalBytesRead;
    }
    free(buffer);
}

// file creation with read and write permissions 
int createOuputFile(const char *directoryName, const char *filepath, long long flag) {
    char *copyOfPath = strdup(filepath);

    // converting flag from long long to string
    char buffer[64]; 
    snprintf(buffer,sizeof(buffer),"%lld",flag);

    const char* outputFileName = basename(copyOfPath);   // base name of the file 
    const char* suffix = "_";
    // Prepare full output path: "Assignment1/flag_<input_file_name>"
    char fullPath[1024];
    snprintf(fullPath, sizeof(fullPath), "%s/%s%s%s", directoryName, buffer, suffix, outputFileName);

    // Create file with read, wirte permissions to the users 
    // O_CREAT creates the file if it doesn’t exist, O_WRONGLY opens the file in write only and
    // O_TRUNC if the file exists then this clears the content 
    return open(fullPath, O_CREAT | O_WRONLY | O_TRUNC, 0600); // read and write permissions
}

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

            char *endptr;
            long long blocksize = isBlockSizeValid(argv[3]);

            // the property of stroll is that it will read only till the digits are present 
            // if it is 123ds then it will read till 123 and then endptr will not return 
            if(blocksize==-1) {
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

                struct stat fileStat;
                if (fstat(originalFileDesc, &fileStat) == -1) {
                    perror("\nError with fstat");
                    return 1;
                }
                // off_t is 64 bits and hence can store till ~8 exa bytes
                off_t originalFileSize = fileStat.st_size;  

                // directory creation 
                const char * directName = "Assignment1";
                createDirectory(directName);

                // descriptor for output file
                int outputFileDesc = createOuputFile(directName,filepath,(long long)flag);

                // Do block wise reversal 
                performBlockwiseReversal(originalFileDesc, outputFileDesc, blocksize, originalFileSize);

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