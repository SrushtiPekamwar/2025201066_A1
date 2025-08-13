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
    printOnConsole("---------------------------------------------------Command usage---------------------------------------------------\n");
    printOnConsole("Blockwise reversal verification     : /a.out <newfilepath> <oldfilepath> <directory> 0 <block_size>\n");
    printOnConsole("Full file reversal verification     : /a.out <newfilepath> <oldfilepath> <directory> 1\n");
    printOnConsole("Partial range reversal verification : /a.out <newfilepath> <oldfilepath> <directory> 2 <start_index> <end_index>\n");
    printOnConsole("-------------------------------------------------------------------------------------------------------------------\n\n");
}

long long isBlockSizeValid(const char *c) {
    long long maxBlockSize = LLONG_MAX;
    char* endptr;  // endptr will point to the first invalid character after the number
    long long blocksize = strtoll(c, &endptr, 10);
    if (*endptr!='\0') {
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
    if (*endptr!='\0') {
        printOnConsole("Start index should be an integer\n");
        _exit(1);
    }
    long long endInd = strtoll(endIndex,&endptr,10);
    if(*endptr!='\0') {
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
        std::cout << "Start index must be less than " << (long long)filesize << std::endl;
        _exit(1);
    }

    // end should be less than the file size 
    if(endInd>=filesize) {
        std::cout << "End index must be less than " << (long long)filesize << std::endl;
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
    long long flag = strtoll(c,&endptr,10);

    // Check if flag is a valid integer
    if (*endptr!='\0') {
        printOnConsole("Flag value must be an integer either 0, 1 or 2\n\n");
        printCommandUsage();
        _exit(1);
    }

    // Check if flag is in allowed range
    if (flag!=0 && flag!=1 && flag!=2) {
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

void oldFileValidation(const char* filePath) {
    // open in read only mode 
    int fileDesc = open(filePath,O_RDONLY);
    if (fileDesc==-1) {
        if (errno==ENOENT) {
            printOnConsole(filePath);
            printOnConsole(" :No such file exists\n");
            _exit(1);
        }
        if(errno==EACCES) {
            printOnConsole("You don't have required permissions to access this file\n");
            close(fileDesc);
            _exit(1);
        }
    }
}

void newFileValidation(const char* newfilePath, const char* oldfilePath, const char* flag) {
    // open in read only mode 
    int fileDesc = open(newfilePath,O_RDONLY);
    if (fileDesc==-1) {
        if (errno==ENOENT) {
            printOnConsole(newfilePath);
            printOnConsole(" :No such file exists\n");
            _exit(1);
        }
        if(errno==EACCES) {
            printOnConsole("You don't have required permissions to access this file\n");
            close(fileDesc);
            _exit(1);
        }
    }

    // let's say the file exists but then its not the valid output file 
    // like the output file should be of the format flag_input.txt 

    // Copy and normalize output file name
    char* copyOfPath = strdup(newfilePath);
    const char* outputFileName = basename(copyOfPath);

    // Copy and normalize old file name
    char* copyOfOldPath = strdup(oldfilePath);
    char oldModifiedfile[1024];
    snprintf(oldModifiedfile, sizeof(oldModifiedfile), "%s_%s", flag, copyOfOldPath);

    // Compare
    if (strcmp(outputFileName,oldModifiedfile)!=0) {
        printOnConsole("\033[1;33mWarning: input file exists, but it is not the valid output file for the given input file\n\033[0m");
    } 
}

void directoryValidation(const char *directoryName, const char* newFilepath) {
    struct stat stats;

    // extracting the directory name from the file path
    char filePath2[1024];
    strncpy(filePath2,newFilepath,sizeof(filePath2));
    filePath2[sizeof(filePath2)-1]='\0';
    char *directoryNameExtracted = dirname(filePath2);

    if(strcmp(directoryName,directoryNameExtracted)!=0) {
        printOnConsole("Output file's directory name does not match with the given directory name\n");
        _exit(1);
    }
    // Check if the directory exists
    else if (stat(directoryName,&stats)==0) {
        printOnConsole(directoryName);
        printOnConsole(" directory is created?: Yes\n");
        return;
    }
    else {
        printOnConsole(directoryName);
        printOnConsole(" directory is created?: No\n");
        _exit(1);
    }
}

void progressBar(float totalProgress) {
    printOnConsole("\r"); 
    printOnConsole("Progress: ");
    char buffer[16];
    // we are just storing the string in the buffer instead of printing it to the screen with the help of snprintf
    snprintf(buffer,sizeof(buffer),"%.2f%%",totalProgress);
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

// check if the contents have been correctly processed for the blockwise reversal
bool isBlockwiseReversalValid(int inputFileDesc, int outputFileDesc, long long blockSize, off_t fileSize) {
    // allocating in the heap so that there won't be any stack overflow 
    char* buffer1 = (char*)malloc(blockSize);
    char* buffer2 = (char*)malloc(blockSize);
    if(buffer1==NULL || buffer2==NULL) {
        printOnConsole("Memory allocation was unsuccessful\n");
        _exit(1);
    }

    off_t currOffset = 0;
    while (currOffset<fileSize) {
        ssize_t inputBytesRead = read(inputFileDesc,buffer1,(ssize_t)blockSize);
        ssize_t outputBytesRead = read(outputFileDesc,buffer2,(ssize_t)blockSize);

        if(inputBytesRead==0 && outputBytesRead==0) {
            free(buffer1);
            free(buffer2);
            return true;
        }
        else if((inputBytesRead==0 && outputBytesRead!=0)||(inputBytesRead!=0 && outputBytesRead==0)) {
            free(buffer1);
            free(buffer2);
            return false;
        }

        // Reversing the contents of the output file block
        singleBlockReversal(buffer2,outputBytesRead);

        // if the last read blocks are not of equal sizes then also we need to return 
        if(outputBytesRead!=inputBytesRead) {
            free(buffer1);
            free(buffer2);
            return false;
        }

        // compare both the blocks 
        for(ssize_t i=0;i<inputBytesRead;++i) {
            if(buffer1[i]!=buffer2[i]) {
                free(buffer1);
                free(buffer2);
                return false;
            }
        }
        // Current offset value updation
        currOffset+=inputBytesRead;
    }
    free(buffer1);
    free(buffer2);
    return true;
}

// check if the contents have been correctly processed for the complete file reversal
bool isFileReversalValid(int inputFileDesc, int outputFileDesc, off_t fileSize) {
    off_t offset = fileSize;
    int blocksize = 4096;

    // allocating in the heap so that there won't be any stack overflow 
    char *buffer1 = (char*)malloc(blocksize);
    char *buffer2 = (char*)malloc(blocksize);
    if(buffer1==NULL || buffer2==NULL) {
        printOnConsole("Memory allocation was unsuccessful\n");
        close(inputFileDesc);
        close(outputFileDesc);
        _exit(1);
    }
    
    off_t index = 0;
    while(index<fileSize) {
        size_t n = blocksize;

        // read the input file from the front
        size_t readBytes = read(inputFileDesc,buffer1,n);

        // set the fd to the end and read the output file from back
        lseek(outputFileDesc,fileSize-index-readBytes,SEEK_SET);
        read(outputFileDesc,buffer2,readBytes);

        // reversing the single buffer block
        singleBlockReversal(buffer2,readBytes);

        index+=readBytes;

        // compare the blocks
        for(size_t i=0;i<readBytes;++i) {
            if(buffer1[i]!=buffer2[i]) return false; 
        }
    }
    free(buffer1);
    free(buffer2);
    return true;
}

// check if the contents have been correctly processed for the partial file reversal
bool isPartialReversalValid(int inputFileDesc, int outputFileDesc, off_t fileSize, const char* startInd, const char* endInd) {
    ssize_t blockSize = 4096;
    char* buffer1 = (char*)malloc(blockSize);
    char* buffer2 = (char*)malloc(blockSize);
    if(buffer1==NULL || buffer2==NULL) {
        printOnConsole("Memory allocation was unsuccessful\n");
        if(inputFileDesc>=0) close(inputFileDesc);
        if(outputFileDesc>=0) close(outputFileDesc);
        _exit(1);
    }

    char *endptr;
    long long startIndex = strtoll(startInd,&endptr,10);
    long long endIndex = strtoll(endInd,&endptr,10);

    // check the correctness from 0 to startIndex-1
    off_t low = 0;
    off_t high = startIndex - 1;
    while(low<=startIndex-1) {
        ssize_t bytesRead = blockSize;
        if(low+bytesRead>startIndex) {
            bytesRead = startIndex-low;
        }
        // read the input file from the front 
        lseek(inputFileDesc,low,SEEK_SET);
        read(inputFileDesc,buffer1,bytesRead);

        // read the output file from the back
        lseek(outputFileDesc,high-bytesRead+1,SEEK_SET);
        read(outputFileDesc,buffer2,bytesRead);
        singleBlockReversal(buffer2,bytesRead);

        // compare the input and the output buffer
        for(ssize_t i=0;i<bytesRead;++i) {
            if(buffer1[i]!=buffer2[i]) {
                free(buffer1);
                free(buffer2);
                return false;
            }
        }
        low +=bytesRead;
        high-=bytesRead;
    }

    // startIndex to endIndex the content should be same
    ssize_t i = startIndex;
    while(i<=endIndex) {
        ssize_t bytesRead = blockSize;
        if(i+bytesRead>endIndex) { // i+bytesRead-1 > endIndex
            bytesRead = endIndex+1-i;
        }

        if(lseek(inputFileDesc,i,SEEK_SET)==-1) {
            printOnConsole("Error while seeking the input file\n");
            free(buffer1);
            free(buffer2);
            _exit(1);
        }
        if(read(inputFileDesc,buffer1,bytesRead)==-1) {
            printOnConsole("Error while reading the input file\n");
            free(buffer1);
            free(buffer2);
            _exit(1);
        }

        if(lseek(outputFileDesc,i,SEEK_SET)==-1) {
            printOnConsole("Error while seeking the input file\n");
            free(buffer1);
            free(buffer2);
            _exit(1);
        }
        if(read(outputFileDesc,buffer2,bytesRead)==-1) {
            printOnConsole("Error while reading the input file\n");
            free(buffer1);
            free(buffer2);
            _exit(1);
        }

        for(ssize_t j=0;j<bytesRead;++j) {
            if(buffer1[j]!=buffer2[j]) {
                free(buffer1);
                free(buffer2);
                return false;
            }
        }
        i+=bytesRead;
    }
    // endIndex+1 to EOF check check whether the contents are reversed properly 
    low = endIndex+1;
    high = fileSize-1;
    while(low<=fileSize-1) {
        ssize_t bytesRead = blockSize;
        if(low+bytesRead>fileSize) {
            bytesRead = fileSize-low;
        }

        // input file traversing
        if(lseek(inputFileDesc,low,SEEK_SET)==-1) {
            printOnConsole("Error while seeking the input file\n");
            free(buffer1);
            free(buffer2);
            _exit(1);
        }
        if(read(inputFileDesc,buffer1,bytesRead)==-1) {
            printOnConsole("Error while reading the input file\n");
            free(buffer1);
            free(buffer2);
            _exit(1);
        }

        // output file traversing
        if(lseek(outputFileDesc,high-bytesRead+1,SEEK_SET)==-1) {
            printOnConsole("Error while seeking the input file\n");
            free(buffer1);
            free(buffer2);
            _exit(1);
        }
        if(read(outputFileDesc,buffer2,bytesRead)==-1) {
            printOnConsole("Error while reading the input file\n");
            free(buffer1);
            free(buffer2);
            _exit(1);
        }

        // reverse the output buffer 
        singleBlockReversal(buffer2,bytesRead);
        // comparing both the buffers
        for(ssize_t i=0;i<bytesRead;++i) {
            if(buffer1[i]!=buffer2[i]) {
                free(buffer1);
                free(buffer2);
                return false;
            }
        }
        low+=bytesRead;
        high-=bytesRead;    
    }
    free(buffer1);
    free(buffer2);
    return true;
}

// check the permissions for file and directory
void checkPermissions(const char* filepath, const char *type) {
    // ls -l myfile.txt checking the permissions through cli
    struct stat st;
    if(stat(filepath,&st)==-1) {
        printOnConsole(filepath);
        perror(" :Error while calculating the file stats");
        printOnConsole("\n");
        _exit(1);
    }

    std::cout << "User has read permissions on " << type << " " << filepath << ": " << ((st.st_mode & S_IRUSR) ? "Yes" : "No") << std::endl;
    std::cout << "User has write permissions on " << type << " " << filepath << ": " << ((st.st_mode & S_IWUSR) ? "Yes" : "No") << std::endl;
    std::cout << "User has execute permissions on " << type << " " << filepath << ": " << ((st.st_mode & S_IXUSR) ? "Yes" : "No") << std::endl;

    std::cout << "Group has read permissions on " << type << " " << filepath << ": " << ((st.st_mode & S_IRGRP) ? "Yes" : "No") << std::endl;
    std::cout << "Group has write permissions on " << type << " " << filepath << ": " << ((st.st_mode & S_IWGRP) ? "Yes" : "No") << std::endl;
    std::cout << "Group has execute permissions on " << type << " " << filepath << ": " << ((st.st_mode & S_IXGRP) ? "Yes" : "No") << std::endl;

    std::cout << "Others have read permissions on " << type << " " << filepath << ": " << ((st.st_mode & S_IROTH) ? "Yes" : "No") << std::endl;
    std::cout << "Others have write permissions on " << type << " " << filepath << ": " << ((st.st_mode & S_IWOTH) ? "Yes" : "No") << std::endl;
    std::cout << "Others have execute permissions on " << type << " " << filepath << ": " << ((st.st_mode & S_IXOTH) ? "Yes" : "No") << std::endl;
    std::cout << std::endl;
}

void isFileSizeSame(const char* newFile, const char* oldFile) {
    struct stat stats1, stats2;
    if (stat(newFile,&stats1)==-1) {
        printOnConsole(newFile);
        printOnConsole(" :Error while getting stats\n");
        _exit(1);
    }
    if (stat(oldFile,&stats2)==-1) {
        printOnConsole(oldFile);
        printOnConsole(" :Error while getting stats\n");
        _exit(1);
    }

    // compare whether the sizes of both the files are same 
    if(stats1.st_size==stats2.st_size) {
        printOnConsole("Size of both the files is same?: Yes\n");
        return;
    }
    else {
        printOnConsole("Size of both the files is same?: No\n");
        return;
    }
}

int createOutputFile(const char *directoryName, const char *filepath, long long flag) {
    // "Assignment1/flag_<input_file_name>"
    char completePath[1024];
    char *copyOfPath = strdup(filepath);
    const char* outputFileName = basename(copyOfPath);   // base name of the file 
    snprintf(completePath,sizeof(completePath),"%s/%lld_%s",directoryName,flag,outputFileName);

    int outputFileDesc = open(completePath,O_RDONLY);
    if(outputFileDesc==-1) {
        printOnConsole("Error while creating the output file\n");
        _exit(1);
    }
    return outputFileDesc;
}

int main(int argc, char *argv[]) {
    if(argc<5) {
        printOnConsole("Number of arguments, fewer than expected\n");
        printCommandUsage();
        return 1;
    }

    else if(argc>7) {
        printOnConsole("Number of arguments, greater than expected\n");
        printCommandUsage();
        return 1;
    }

    // flag is integer but we get string type from the console 
    long long flag = isFlagValid(argv[4]);

    //-------------------------------flag 0 - block-wise reversal done---------------------------
    if(flag==0) {
        printOnConsole("-------------------------Mode: Blockwise reversal-------------------------\n");
        if(argc!=6) {
            printOnConsole("Wrong number of arguments for flag 0\n");
            printCommandUsage();
            return 1;
        }
        else {
            // ./a.out <newfilepath> <oldfilepath> <directoryName> <flag> <blockSize>
            const char* newFilepath = argv[1];
            const char* oldFilepath = argv[2];
            const char* directoryName = argv[3];
            const char* flagString = argv[4];
            long long blocksize = isBlockSizeValid(argv[5]);

            // validations for the files and the directory 
            directoryValidation(directoryName,newFilepath);
            oldFileValidation(oldFilepath);
            newFileValidation(newFilepath,oldFilepath,flagString);

            // file descriptors
            int newfileDesc = open(newFilepath,O_RDONLY);
            int oldfileDesc = open(oldFilepath,O_RDONLY);

            // check whether the contents have been correctly processed
            struct stat fileStat;
            fstat(oldfileDesc,&fileStat);
            off_t fileSize = fileStat.st_size; // calculating the size of the file 

            bool correct = isBlockwiseReversalValid(oldfileDesc,newfileDesc,blocksize,fileSize);
            if(correct==true) {
                printOnConsole("Contents have been correctly processed? : Yes\n");
            }
            else {
                printOnConsole("Contents have been correctly processed? : No\n");
            }

            // check whether the file sizes are same 
            isFileSizeSame(newFilepath,oldFilepath);

            // check permissions for the files
            std::cout << std::endl;
            checkPermissions(newFilepath,"file");
            checkPermissions(oldFilepath,"file");
            checkPermissions(directoryName,"directory");

            close(newfileDesc);
            close(oldfileDesc);
        }
    }

    //-------------------------------flag 1 - complete file reversal---------------------------
    // For flag 1: ./a.out Assignment1/1_input.txt input.txt Assignment1 1
    else if(flag==1) {
        printOnConsole("--------------------Mode: Full file reversal--------------------\n");
        if(argc!=5) {
            printOnConsole("Wrong number of arguments for flag 1\n");
            printCommandUsage();
            return 1;
        }
        else {
            const char* newFilepath = argv[1];
            const char* oldFilepath = argv[2];
            const char* directoryName = argv[3];
            const char* flagString = argv[4];

            // validations for the files and the directory 
            directoryValidation(directoryName,newFilepath);
            oldFileValidation(oldFilepath);
            newFileValidation(newFilepath,oldFilepath,flagString);

            // file descriptors
            int newfileDesc = open(newFilepath,O_RDONLY);
            int oldfileDesc = open(oldFilepath,O_RDONLY);

            // check whether the contents have been correctly processed
            struct stat fileStat;
            fstat(oldfileDesc,&fileStat);
            off_t fileSize = fileStat.st_size; // calculating the size of the file 

            bool correct = isFileReversalValid(oldfileDesc,newfileDesc,fileSize);
            if(correct==true) {
                printOnConsole("Contents have been correctly processed? : Yes\n");
            }
            else {
                printOnConsole("Contents have been correctly processed? : No\n");
            }

            // check whether the file sizes are same 
            isFileSizeSame(newFilepath,oldFilepath);

            // check permissions for the files
            std::cout << std::endl;
            checkPermissions(newFilepath,"file");
            checkPermissions(oldFilepath,"file");
            checkPermissions(directoryName,"directory");

            close(newfileDesc);
            close(oldfileDesc);
        }
    }

    //-------------------------------flag 2 is used to do the reversal in the given range of index---------------------------
    else if(flag==2) {
        printOnConsole("--------------------Mode: Partial range reversal--------------------\n");
        if(argc!=7) {
            printOnConsole("Wrong number of arguments for flag 2\n");
            printCommandUsage();
            return 1;
        } 
        else {
            // ./a.out <newfilepath> <oldfilepath> <directoryName> <flag> <startIndex> <endIndex>
            const char* newFilepath = argv[1];
            const char* oldFilepath = argv[2];
            const char* directoryName = argv[3];
            const char* flagString = argv[4];
            const char* startIndex = argv[5];
            const char* endIndex = argv[6];

            // validations for the files and the directory 
            directoryValidation(directoryName,newFilepath);
            oldFileValidation(oldFilepath);
            newFileValidation(newFilepath,oldFilepath,flagString);

            // file descriptors
            int newfileDesc = open(newFilepath,O_RDONLY);
            int oldfileDesc = open(oldFilepath,O_RDONLY);

            // check whether the contents have been correctly processed
            struct stat fileStat;
            fstat(oldfileDesc,&fileStat);
            off_t fileSize = fileStat.st_size; // calculating the size of the file 

            // index validation 
            isIndexValid(startIndex,endIndex,fileSize);

            // check whether the contents have been correctly processed
            bool correct = isPartialReversalValid(oldfileDesc,newfileDesc,fileSize,startIndex,endIndex);
            if(correct==true) {
                printOnConsole("Contents have been correctly processed? : Yes\n");
            }
            else {
                printOnConsole("Contents have been correctly processed? : No\n");
            }

            // check whether the file sizes are same 
            isFileSizeSame(newFilepath,oldFilepath);

            // check permissions for the files
            std::cout << std::endl;
            checkPermissions(newFilepath,"file");
            checkPermissions(oldFilepath,"file");
            checkPermissions(directoryName,"directory");

            close(newfileDesc);
            close(oldfileDesc);
        }
    }
    return 0;
}