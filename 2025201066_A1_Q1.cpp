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


#include<unistd.h>
#include<cstring>


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
    printOnConsole("\n-------------------------------Command usage-------------------------------\n");
    printOnConsole("Blockwise reversal     : ./a.out <input_file> 0 <block_size>\n");
    printOnConsole("Full file reversal     : ./a.out <input_file> 1\n");
    printOnConsole("Partial range reversal : ./a.out <input_file> 2 <start_index> <end_index>\n");
    printOnConsole("---------------------------------------------------------------------------\n\n");
}

int main(int argc, char *argv[]) {
    if(argc>3 || argc<1) {
        printOnConsole("Wrong number of arguments\n");
        printCommandUsage();
        return 1;
    }
    int flag = *argv[2];
    const char *filename = argv[1];
}