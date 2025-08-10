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

int sleepValue = 10;

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

void printCommandUsage() {
    printOnConsole("-------------------------------Command usage-------------------------------\n");
    printOnConsole("Blockwise reversal     : ./a.out <input_file> 0 <block_size>\n");
    printOnConsole("Full file reversal     : ./a.out <input_file> 1\n");
    printOnConsole("Partial range reversal : ./a.out <input_file> 2 <start_index> <end_index>\n");
    printOnConsole("---------------------------------------------------------------------------\n\n");
}

long long isBlockSizeValid(const char *c) {
    long long maxBlockSize = LLONG_MAX;
    char* endptr;  // endptr will point to the first invalid character after the number
    long long blocksize = strtoll(c, &endptr, 10);
    if (*endptr != '\0') {
        printOnConsole("Invalid block size, block size must be an integer\n");
        _exit(1);
    }

    if(blocksize<=0) {
        printOnConsole("Block size should be positive\n");
        _exit(1);
    }

    // to avoid overflow 
    if(blocksize>maxBlockSize) {
        printOnConsole("Overflow: Block size very large\n");
        _exit(1);
    }

    return blocksize;
}

void isIndexValid(const char *startIndex, const char *endIndex, off_t filesize) {
    // need to convert them into long long and then need to compare that 0 <= start <= end < eof
    char* endptr;  // endptr will point to the first invalid character after the number
    long long maximum = LLONG_MAX;
    long long startInd = strtoll(startIndex,&endptr,10);
    if (*endptr != '\0') {
        printOnConsole("Start index should be an integer\n");
        _exit(1);
    }
    long long endInd = strtoll(endIndex,&endptr,10);
    if(*endptr != '\0') {
        printOnConsole("End index should be an integer\n");
        _exit(1);
    }

    if(startInd<0 || endInd<0) {
        printOnConsole("Index should be positive\n");
        _exit(1);
    }

    // to avoid overflow
    if(startInd>maximum || endInd>maximum) {
        printOnConsole("Overflow: Index size is very large\n");
        _exit(1);
    }

    // start index should be less than the file size 
    if(startInd>=filesize) {
        printOnConsole("Start index must be less than the file size\n");
        _exit(1);
    }

    // end should be less than the file size 
    if(endInd>=filesize) {
        printOnConsole("End index must be less than the file size\n");
        _exit(1);
    }

    // check 
    if(endInd<startInd) {
        printOnConsole("End index must be greater than the start index\n");
        _exit(1);
    }
}

int isFlagValid(const char *c) {
    char* endptr;
    long long flag = strtoll(c, &endptr, 10);

    // Check if flag is a valid integer
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

void printInteger(long long number) {
    char buffer[64]; 
    snprintf(buffer,sizeof(buffer),"%lld",number);
    printOnConsole(buffer);
}

void fileValidation(int fileDesc) {
    if (fileDesc == -1) {
        if (errno==ENOENT) {
            printOnConsole("No such file exists\n");
            _exit(1);
        }
        if(errno==EACCES) {
            printOnConsole("You don't have required permissions to access this file\n");
            _exit(1);
        }
    }
}

void createDirectory(const char *directoryName) {
    struct stat stats;
    // Check if the directory exists
    if (stat(directoryName,&stats)==0) {
        printOnConsole(directoryName);
        printOnConsole(" directory already exists\n");
        return;
    }
    // Create directory and give user permissions of read, write and execute
    else {
        mkdir(directoryName, 0700);
        printOnConsole("Directory 'Assignment1' created successfully\n");
    }
}

void progressBar(float totalProgress) {
    printOnConsole("\r"); 
    printOnConsole("Progress: ");
    char buffer[16];
    // we are just storing the string in the buffer instead of printing it to the screen with the help of snprintf
    snprintf(buffer, sizeof(buffer), "%.2f%%", totalProgress);
    printOnConsole(buffer);
    fflush(stdout);
}

void singleBlockReversal(char *buffer, ssize_t bytesRead) {
    for(ssize_t i=0;i<bytesRead/2;++i) {
        char temp = buffer[i];
        buffer[i] = buffer[bytesRead-i-1];
        buffer[bytesRead-i-1] = temp;
    }
}

void performBlockwiseReversal(int inputFileDesc, int outputFileDesc, long long blockSize, off_t fileSize) {
    // allocating in the heap so that there won't be any stack overflow 
    char* buffer = (char*)malloc(blockSize);
    if(buffer==NULL) {
        printOnConsole("Memory allocation was unsuccessful\n");
        if(inputFileDesc>=0) close(inputFileDesc);
        if(outputFileDesc>=0) close(outputFileDesc);
        _exit(1);
    }

    off_t currOffset = 0;
    while (currOffset<fileSize) {
        ssize_t bytesRead = read(inputFileDesc,buffer,(ssize_t)blockSize);
        if(bytesRead==-1) {
            printOnConsole("Error while reading the input file\n");
            free(buffer);
            if(inputFileDesc>=0) close(inputFileDesc);
            if(outputFileDesc>=0) close(outputFileDesc);
            _exit(1);
        }

        if(bytesRead==0) break;

        // Reversing the contents of the current block
        singleBlockReversal(buffer,bytesRead);

        if(write(outputFileDesc,buffer,bytesRead)==-1) {
            printOnConsole("Error while writing to the ouptut file\n");
            free(buffer);
            if(inputFileDesc>=0) close(inputFileDesc);
            if(outputFileDesc>=0) close(outputFileDesc);
            _exit(1);
        }
        // Current offset value updation
        currOffset += bytesRead;

        float totalProgress = (currOffset*100.0)/fileSize;
        progressBar(totalProgress);
        // usleep(sleepValue); 
    }
    free(buffer);
}

// file to reverse the whole file 
void reverseTheFile(int inputFileDesc, int outputFileDesc, off_t filesize) {
    // what we are doing is, we are going to the end and then reversing the blocks and then
    // writing this blocks at the starting of the file 
    // we need to move the pointer to the end
    off_t offset = filesize, index = filesize;
    off_t progress = 0;
    int blocksize = 1024;

    // allocating in the heap so that there won't be any stack overflow 
    char *buffer = (char*)malloc(blocksize);
    if(buffer==NULL) {
        printOnConsole("Memory allocation was unsuccessful\n");
        if(inputFileDesc>=0) close(inputFileDesc);
        if(outputFileDesc>=0) close(outputFileDesc);
        _exit(1);
    }
    
    // here we can't take index>=0 because then for the progree+=n we will be adding 0 and then
    // it will lead to infinite loop
    while(index>0) {
        size_t n = blocksize;
        if(index-blocksize<=0) {
            n = index;
        }
        if(lseek(inputFileDesc,index-n,SEEK_SET)==-1) {
            printOnConsole("Error while seeking the input file\n");
            free(buffer);
            if(inputFileDesc>=0) close(inputFileDesc);
            if(outputFileDesc>=0) close(outputFileDesc);
            _exit(1);
        }

        if(read(inputFileDesc,buffer,n)==-1) {
            printOnConsole("Error while reading the input file\n");
            free(buffer);
            if(inputFileDesc>=0) close(inputFileDesc);
            if(outputFileDesc>=0) close(outputFileDesc);
            _exit(1);
        }
        // reversing the single buffer block
        singleBlockReversal(buffer,n);

        if(write(outputFileDesc,buffer,n)==-1) {
            printOnConsole("Error while writing to the output file\n");
            free(buffer);
            if(inputFileDesc>=0) close(inputFileDesc);
            if(outputFileDesc>=0) close(outputFileDesc);
            _exit(1);
        };
        progress+=n;
        index-=n;
        int totalProgress = (progress*100)/filesize;
        progressBar(totalProgress);
        // usleep(10000); 
    }
    free(buffer);
}

// reverse the file when start and end index are given
void partialReversal(int inputFileDesc, int outputFileDesc, off_t fileSize, long long startIndex, long long endIndex) {
    // reverse from 0 to start and then reverse from end to eof
    ssize_t blockSize = 1024;
    char* buffer = (char*)malloc((int)blockSize);
    if(buffer==NULL) {
        printOnConsole("Memory allocation was unsuccessful\n");
        if(inputFileDesc>=0) close(inputFileDesc);
        if(outputFileDesc>=0) close(outputFileDesc);
        _exit(1);
    }

    //-------------------Reverse from 0th index to the startIndex--------------------------
    off_t currOffset = 0;
    while (currOffset<=startIndex) {
        ssize_t bytesRead = blockSize;
        if(currOffset+bytesRead>startIndex+1) {
            bytesRead = startIndex+1-currOffset;
        }

        ssize_t bytesStored = read(inputFileDesc,buffer,bytesRead);
        if(bytesStored==-1) {
            printOnConsole("Error while reading the input file\n");
            free(buffer);
            if(inputFileDesc>=0) close(inputFileDesc);
            if(outputFileDesc>=0) close(outputFileDesc);
            _exit(1);
        }

        if(bytesStored<=0) break;

        // Reversing the contents of the current block
        singleBlockReversal(buffer,bytesStored);

        if(write(outputFileDesc,buffer,bytesStored)==-1) {
            printOnConsole("Error while writing to the ouptut file\n");
            free(buffer);
            if(inputFileDesc>=0) close(inputFileDesc);
            if(outputFileDesc>=0) close(outputFileDesc);
            _exit(1);
        }
        // Current offset value updation
        currOffset += bytesStored;

        float totalProgress = (currOffset*100.0)/fileSize;
        progressBar(totalProgress);
        // usleep(sleepValue); 
    }

    //-----------------Keep the values from startIndex+1 to endIndex-1 as same---------------
    while(currOffset<endIndex) {
        ssize_t bytesRead = blockSize;
        if(currOffset+bytesRead>endIndex) {
            bytesRead = endIndex-currOffset;
        }

        ssize_t bytesStored = read(inputFileDesc,buffer,bytesRead);
        if(bytesStored==-1) {
            printOnConsole("Error while reading the input file\n");
            free(buffer);
            if(inputFileDesc>=0) close(inputFileDesc);
            if(outputFileDesc>=0) close(outputFileDesc);
            _exit(1);
        }

        if(bytesStored==0) break;

        if(write(outputFileDesc,buffer,bytesStored)==-1) {
            printOnConsole("Error while writing to the ouptut file\n");
            free(buffer);
            if(inputFileDesc>=0) close(inputFileDesc);
            if(outputFileDesc>=0) close(outputFileDesc);
            _exit(1);
        }
        // Current offset value updation
        currOffset += bytesStored;

        float totalProgress = (currOffset*100.0)/fileSize;
        progressBar(totalProgress);
        // usleep(sleepValue); 
    }

    //---------------------Reverse the file from endIndex+1 to the EOF--------------------------
    while (currOffset<fileSize) {
        ssize_t bytesRead = read(inputFileDesc, buffer, blockSize);
        if(bytesRead==-1) {
            printOnConsole("Error while reading the input file\n");
            free(buffer);
            if(inputFileDesc>=0) close(inputFileDesc);
            if(outputFileDesc>=0) close(outputFileDesc);
            _exit(1);
        }
        if(bytesRead<=0) break;

        singleBlockReversal(buffer,bytesRead);

        if(write(outputFileDesc,buffer,bytesRead)==-1) {
            printOnConsole("Error while writing output file\n");
            free(buffer);
            if(inputFileDesc>=0) close(inputFileDesc);
            if(outputFileDesc>=0) close(outputFileDesc);
            _exit(1);
        }
        currOffset+=bytesRead;
        progressBar((currOffset*100.0)/fileSize);
    }
    free(buffer);
}

int createOutputFile(const char *directoryName, const char *filepath, long long flag) {
    // "Assignment1/flag_<input_file_name>"
    char completePath[1024];
    char *copyOfPath = strdup(filepath);
    const char* outputFileName = basename(copyOfPath);   // base name of the file 
    snprintf(completePath, sizeof(completePath), "%s/%lld_%s", directoryName,flag,outputFileName);

    int outputFileDesc = open(completePath, O_CREAT | O_WRONLY | O_TRUNC, 0600); // read and write permissions
    if(outputFileDesc==-1) {
        printOnConsole("Error while creating the output file\n");
        _exit(1);
    }
    return outputFileDesc;
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
        printOnConsole("--------------------Mode: Blockwise reversal--------------------\n");
        if(argc!=4) {
            printOnConsole("Wrong number of arguments\n");
            printCommandUsage();
            return 1;
        }
        else {
            // ./a.out <input_file> 0 <block_size>
            const char* filepath = argv[1];
            long long blocksize = isBlockSizeValid(argv[3]);

            // the property of stroll is that it will read only till the digits are present 
            // if it is 123ds then it will read till 123 and then endptr will not return 
            if(blocksize==-1) {
                return 1;
            }

            // open the file in read only mode
            int originalFileDesc = open(filepath, O_RDONLY); 
            if(originalFileDesc==-1) {
                fileValidation(originalFileDesc);
                printOnConsole("Error while opening the input file\n");
                return 1;
            }

            struct stat fileStat;
            fstat(originalFileDesc,&fileStat);
            off_t originalFileSize = fileStat.st_size; 
            if(originalFileSize<0) {
                printOnConsole("Error while calculating the file size\n");
                close(originalFileDesc);
                return 1;
            }
            if(originalFileSize==0) {
                printOnConsole("File is empty\n");
            }

            printOnConsole("Block size: ");
            printOnConsole(argv[3]);
            printOnConsole("\nFile size: ");
            printInteger(originalFileSize);
            printOnConsole(" bytes");
            printOnConsole("\n");

            const char *directName = "Assignment1";
            createDirectory(directName);

            int outputFileDesc = createOutputFile(directName,filepath,(long long)flag);
            if(outputFileDesc==-1) {
                printOnConsole("Error while creating the output file\n");
                close(originalFileDesc);
                return 1;
            }

            if(blocksize>originalFileSize) {
                printOnConsole("Warning: Block size is greater than the file size\n");
            } 

            // Do block wise reversal 
            performBlockwiseReversal(originalFileDesc, outputFileDesc, blocksize, originalFileSize);

            // close the files
            close(originalFileDesc);
            close(outputFileDesc);
        }
    }

    //-------------------------------flag 1 is used to do complete file reversal---------------------------
    else if(flag==1) {
        printOnConsole("--------------------Mode: Full file reversal--------------------\n");
        if(argc!=3) {
            printOnConsole("Wrong number of arguments\n");
            printCommandUsage();
            return 1;
        }
        else {
            const char* filepath = argv[1];
            int originalFileDesc = open(filepath, O_RDONLY); 
            if(originalFileDesc==-1) {
                fileValidation(originalFileDesc);
                printOnConsole("Error while opening the input file\n");
                return 1;
            }

            struct stat fileStat;
            fstat(originalFileDesc,&fileStat);
            off_t originalFileSize = fileStat.st_size; 
            if(originalFileSize<0) {
                printOnConsole("Error while calculating the file size\n");
                close(originalFileDesc);
                return 1;
            } 
            if(originalFileSize==0) {
                printOnConsole("File is empty\n");
            }

            printOnConsole("File size: ");
            printInteger(originalFileSize);
            printOnConsole(" bytes");
            printOnConsole("\n");

            const char * directName = "Assignment1";
            createDirectory(directName);

            int outputFileDesc = createOutputFile(directName,filepath,(long long)flag);
            if(outputFileDesc==-1) {
                printOnConsole("Error while creating the output file\n");
                close(originalFileDesc);
                return 1;
            }

            // reverse the entire file 
            reverseTheFile(originalFileDesc, outputFileDesc, originalFileSize);

            // close the file 
            close(originalFileDesc);
            close(outputFileDesc);
        }
    }

    //-------------------------------flag 2 is used to do the reversal in the given range of index---------------------------
    else if(flag==2) {
        printOnConsole("--------------------Mode: Partial range reversal--------------------\n");
        if(argc!=5) {
            printOnConsole("Wrong number of arguments\n");
            printCommandUsage();
            return 1;
        } 
        else {
            // get the filename, flag, start index and the end index
            // start index can't be greater than end index 
            const char* filepath = argv[1];
            const char* startIndex = argv[3];
            const char* endIndex = argv[4];

            int originalFileDesc = open(filepath, O_RDONLY); 
            if(originalFileDesc==-1) {
                fileValidation(originalFileDesc);
                printOnConsole("Error while opening the input file\n");
                return 1;
            }

            struct stat fileStat;
            fstat(originalFileDesc,&fileStat);
            off_t originalFileSize = fileStat.st_size; 
            if(originalFileSize<0) {
                printOnConsole("Error while calculating the file size\n");
                close(originalFileDesc);
                return 1;
            }
            if(originalFileSize==0) {
                printOnConsole("File is empty\n");
            }
            isIndexValid(startIndex,endIndex,originalFileSize);
            char *endptr;
            long long startInd = strtoll(startIndex,&endptr,10);
            long long endInd = strtoll(endIndex,&endptr,10);
            printOnConsole("Start index: ");
            printInteger(startInd);
            printOnConsole("\nEnd index: ");
            printInteger(endInd);
            printOnConsole("\nFile size: ");
            printInteger(originalFileSize);
            printOnConsole(" bytes");
            printOnConsole("\n");

            const char * directName = "Assignment1";
            createDirectory(directName);

            int outputFileDesc = createOutputFile(directName,filepath,(long long)flag);
            if(outputFileDesc==-1) {
                printOnConsole("Error while creating the output file\n");
                close(originalFileDesc);
                return 1;
            }

            // reverse the first and last portion of the file 
            partialReversal(originalFileDesc,outputFileDesc,originalFileSize,startInd-1,endInd+1);

            // close the file 
            close(originalFileDesc);
            close(outputFileDesc);
        }
    }

    return 0;
}