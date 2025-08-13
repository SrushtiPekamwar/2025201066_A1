# Advanced Operating Systems — Assignment 1 (Q1)

## Overview of question 1

In Question 1, we have implemented **three different types of file reversal techniques** using **system calls only**.     
New directory is created with permissions `700` which has read, write, execute permissions for the user only.
In this new directory the output file is generated and stored where permissions of the output file are set to `600` that is read and write permissions for the user only.

1. Blockwise reversal (Flag 0)
2. Full file reversal (Flag 1)
3. Partial range reversal (Flag 2) 

---

## Compilation

```bash
g++ Q1.cpp -o q1.out
````

---

## Usage

**General command format:**

```bash
./q1.out inputfile.txt flag [additional_arguments]
```

**Modes and Examples:**
| **Flag** | **Mode**                            | **Command Example**                                              |
| -------- | ----------------------------------- | ---------------------------------------------------------------- |
| **0**    | Blockwise reversal     | `./q1.out <input_file> 0 <block_size>`  |
| **1**    | Full file reversal     | `./q1.out <input_file> 1`       |
| **2**    | Partial range reversal | `./q1.out <input_file> 2 <start_index> <end_index>`  |

**Output format:**

```
Assignment1/<flag>_<inputFilename>
```

---

## Workflow

**Reversal Logic**

   * **Flag 0 (Block-wise):**

        1. We will read the file sequentially using read() in terms of blocks size.
        2. For each block, we will reverse that block's content and then using write() we will write this reversed block to the output file.
        3. We will continue this until the entire file is processed.

   * **Flag 1 (Full file):**

        1. Using lseek() we will move the file pointer to the end of the input file.
        2. Then we will read the input file backwards using read() and read one block at a time. For each block we have read from the end, we will reverse that single block and then using write() we will write the reversed block sequentially into the output file.
        3. We will continue this until the entire file is processed.

   * **Flag 2 (Partial range):**

        1. First we will reverse from index 0 to startIndex-1 using the complete file reversal function.
        2. For start_index to end_index we will read from the input file sequentially and then write to the output file sequentially without any processing.
        3. Then from end_index+1 to eof, using lseek() we will move the file pointer to the end of the input file and then read blockwise, reverse it and then using write(), write this reversed contents to the output file sequentially.

---

## Example Output

```
srushtipekamwar@Srushtis-MacBook-Air 2025201066_A1 % g++ 2025201066_A1_Q1.cpp -o q1.out
srushtipekamwar@Srushtis-MacBook-Air 2025201066_A1 % ./q1.out input.txt 0 3            
--------------------Mode: Blockwise reversal--------------------
Block size: 3
File size: 1000000 bytes
Assignment1 directory already exists
Progress: 100.00%%                                                                                                     
srushtipekamwar@Srushtis-MacBook-Air 2025201066_A1 % ./q1.out input.txt 1  
--------------------Mode: Full file reversal--------------------
File size: 1000000 bytes
Assignment1 directory already exists
Progress: 100.00%%                                                                                                     
srushtipekamwar@Srushtis-MacBook-Air 2025201066_A1 % ./q1.out input.txt 2 6 103
--------------------Mode: Partial range reversal--------------------
Start index: 6
End index: 103
File size: 1000000 bytes
Assignment1 directory already exists
Progress: 100.00%%                                     
```

---
---
---
# **Advanced Operating Systems — Assignment 1 (Q2)**

## **Overview of question 2**

In this program we have verified the correctness of the file reversal outputs which were generated in question 1. Here also we have taken help of system calls like read, write, open, close, lseek, stat, mkdir. 
In this program we also validate the permissions for input/output file and the directory. And also check whether the file sizes of the input and output files are same.

---

## **Compilation**

```bash
g++ Q2.cpp -o q2.out
```

---

## **Usage**

**General command format:**

```
./q2.out <new_file_path> <old_file_path> <directory_name> <flag> [additional_arguments]
```

### **Modes and Examples**

| **Flag** | **Mode**                            | **Command Example**                                              |
| -------- | ----------------------------------- | ---------------------------------------------------------------- |
| **0**    | Blockwise reversal verification     | `./q2.out Assignment1/0_input.txt input.txt Assignment1 0 <block_size>`  |
| **1**    | Full file reversal verification     | `./q2.out Assignment1/1_input.txt input.txt Assignment1 1`       |
| **2**    | Partial range reversal verification | `./q2.out Assignment1/2_input.txt input.txt Assignment1 2 <start_index> <end_index>` |

---

## **Workflow**

### Verification Logic

#### **Flag 0 — Blockwise Reversal Verification**

1. Using the read() we will read both the input and the output files sequentially and store them in the buffer.
2. We will then reverse the contents of the output buffer.
3. And then we will compare the contents of both the buffers, if they are same then processing was successful.

---

#### **Flag 1 — Full File Reversal Verification**

1. Using read() we will read the input file from the start.
2. Using lseek() we will move the file pointer to the end of the file of the output file and then read the contents of the output file from the back, store this in the buffer and reverse its contents.
2. We will now compare the contents of both the buffers, if they are same then processing was successful.

---

#### **Flag 2 — Partial Range Reversal Verification**

1. For 0 to start_index-1 we will read the input file from the front. The output file will be read from the start_index-1 block wise and we will use lseek to go to start_index-1. We will reverse the contents of the output buffer and then compare both the buffers, if they don't match then we will return false.
2. For start_index to end_index we will read both the files sequentially and then compare, if they don't match then return false.
3. For end_index+1 to the EOF we will read the input file from end_index+1 and using lseek() we will go to the EOF-1 in the output file. We will reverse the contents of the output buffer and then compare both the input and the output buffer. It they don't match then return false.
4. If everything matched then that means the processing was successful.

---