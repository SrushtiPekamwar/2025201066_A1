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

int sleepValue = 1000;

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

// function to check whether the given block size is valid or not 
long long isBlockSizeValid(const char *c) {
    long long maxBlockSize = LLONG_MAX;
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
        printOnConsole("Block size overflow\n");
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
        if (errno==ENOENT) {
            printOnConsole(strerror(errno));
            printOnConsole("\n");
            _exit(1);
        }
        if(errno==EACCES) {
            printOnConsole("You don't have required permissions to access this file\n");
            _exit(1);
        }
    }
}

// directory creation with read, write and execute permissions 
void createDirectory(const char *directoryName) {
    struct stat stats;
    // Check if the directory exists
    if (stat(directoryName,&stats)==0) {
        printOnConsole(directoryName);
        printOnConsole(" already exists\n");
        return;
    }
    // Create directory and give user permissions of read, write and execute
    else {
        mkdir(directoryName, 0700);
        printOnConsole("Directory 'Assignment1' created successfully\n");
    }
}

// updation progresss bar
void progressBar(int totalProgress) {
    printOnConsole("\r"); 
    for (int i=0;i<50;++i) {
        if (i<(totalProgress*50)/100) {
            printOnConsole("#");
        } else {
            printOnConsole("=");
        }
    }
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "] %d%%", totalProgress);
    printOnConsole(buffer);
    fflush(stdout);
}

// function to perform block wise reversal
void performBlockwiseReversal(int inputFileDesc, int outputFileDesc, long long blockSize, off_t fileSize) {
    // allocating in the heap so that there won't be any stack overflow 
    off_t progress = 0;
    char* buffer = (char*)malloc(blockSize);
    off_t currOffset = 0;
    if(buffer==NULL) {
        printOnConsole("Memory allocation was unsuccessful\n");
        _exit(1);
    }

    while (currOffset<fileSize) {
        ssize_t blocksRead = blockSize;

        // this will also handle when the last block size is greater than the file size 
        if (currOffset+blockSize>fileSize) {
            blocksRead = fileSize-currOffset;
        }

        if(lseek(inputFileDesc,currOffset,SEEK_SET)==-1) {
            printOnConsole("Error while seeking the input file");
            _exit(1);
        }
        if(read(inputFileDesc,buffer,blocksRead)==-1) {
            printOnConsole("Error while reading the input ile");
            _exit(1);
        }
        // Reversing the contents of the current block
        for(ssize_t i=0;i<blocksRead/2;++i) {
            char temp = buffer[i];
            buffer[i] = buffer[blocksRead-i-1];
            buffer[blocksRead-i-1] = temp;
        }
        if(write(outputFileDesc,buffer,blocksRead)==-1) {
            printOnConsole("Error while writing to the ouptut file");
            _exit(1);
        }
        // Current offset value updation
        currOffset += blocksRead;
        progress += blocksRead;

        int totalProgress = (progress*100)/fileSize;
        progressBar(totalProgress);
        usleep(sleepValue);
    }
    free(buffer);
}

// file creation with read and write permissions 
int createOuputFile(const char *directoryName, const char *filepath, long long flag) {
    // "Assignment1/flag_<input_file_name>"
    char completePath[1024];
    char *copyOfPath = strdup(filepath);
    const char* outputFileName = basename(copyOfPath);   // base name of the file 
    snprintf(completePath, sizeof(completePath), "%s/%lld_%s", directoryName,flag,outputFileName);

    int outputFileDesc = open(completePath, O_CREAT | O_WRONLY | O_TRUNC, 0600); // read and write permissions
    if(outputFileDesc==-1) {
        printOnConsole("Error while creating the output file");
        _exit(1);
    }
    return outputFileDesc;
}

// file to reverse the whole file 
void reverseTheFile(int inputFileDesc, int outputFileDesc, off_t filesize) {
    // we need to move the pointer to the end
    off_t offset = filesize;
    off_t index = filesize;
    int blocksize = 5;
    off_t progress = 0;

    // allocating in the heap so that there won't be any stack overflow 
    char *buffer = (char*)malloc(blocksize);
    if(buffer==NULL) {
        printOnConsole("Memory allocation was unsuccessful\n");
        _exit(1);
    }
    
    while(index>0) {
        size_t n = blocksize;
        if(index-blocksize<=0) {
            n = index;
        }
        if(lseek(inputFileDesc,index-n,SEEK_SET)==-1) {
            printOnConsole("Error while seeking the input file");
            _exit(1);
        }

        if(read(inputFileDesc,buffer,n)==-1) {
            printOnConsole("Error while reading the input file");
            _exit(1);
        }
        for(int i=0;i<n/2;++i) {
            char temp = buffer[i];
            buffer[i] = buffer[n-i-1];
            buffer[n-i-1] = temp;
        }
        if(write(outputFileDesc,buffer,n)==-1) {
            printOnConsole("Error while writing to the output file");
            _exit(1);
        };
        progress+=n;
        index -= n;
        int totalProgress = (progress*100)/filesize;
        progressBar(totalProgress);
        usleep(sleepValue);
    }
    free(buffer);
}

// reverse the file when start and end index are given

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

            // open the file in read only 
            int originalFileDesc = open(filepath, O_RDONLY); 
            if(originalFileDesc==-1) {
                printOnConsole("Erorr while opening the input file");
                _exit(1);
            }

            // validating the file 
            fileValidation(originalFileDesc);

            // directory creation 
            const char *directName = "Assignment1";
            createDirectory(directName);

            // descriptor for output file
            int outputFileDesc = createOuputFile(directName,filepath,(long long)flag);
            if(outputFileDesc==-1) {
                printOnConsole("Error while creating the output file");
                close(originalFileDesc);
                _exit(1);
            }

            struct stat fileStat;
            fstat(originalFileDesc,&fileStat);
            // off_t is 64 bits and hence can store till ~8 exa bytes
            off_t originalFileSize = fileStat.st_size; 
            if(originalFileSize<0) {
                printOnConsole("Error while calculating the file size");
                close(originalFileDesc);
                close(outputFileDesc);
                _exit(1);
            }
            if(blocksize>originalFileSize) {
                printOnConsole("Warning: Block size is greater than the file size\n");
            } 

            // Do block wise reversal 
            performBlockwiseReversal(originalFileDesc, outputFileDesc, blocksize, originalFileSize);

            // close the file 
            close(originalFileDesc);
            close(outputFileDesc);
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
            // ./a.out <input_file> 1
            const char* filepath = argv[1];
            int originalFileDesc = open(filepath, O_RDONLY); 
            if(originalFileDesc==-1) {
                printOnConsole("Error while opening the input file");
                _exit(1);
            }
            // validating the file 
            fileValidation(originalFileDesc);

            // directory creation
            const char * directName = "Assignment1";
            createDirectory(directName);

            // descriptor for output file
            int outputFileDesc = createOuputFile(directName,filepath,(long long)flag);
            if(outputFileDesc==-1) {
                printOnConsole("Error while creating the output file");
                close(originalFileDesc);
                _exit(1);
            }

            // file size 
            struct stat fileStat;
            fstat(originalFileDesc,&fileStat);
            off_t originalFileSize = fileStat.st_size;  
            if(originalFileSize<0) {
                printOnConsole("Error while calculating the file size");
                close(originalFileDesc);
                close(outputFileDesc);
                _exit(1);
            }

            // reverse the file 
            reverseTheFile(originalFileDesc, outputFileDesc, originalFileSize);

            // close the file 
            close(originalFileDesc);
            close(outputFileDesc);
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