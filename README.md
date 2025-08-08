# 2025201066_A1

# Creating 5GB size file 
truncate -s 5G bigfile

git add .
git commit -m "type the message"
git push origin main 

🔧 System & Library Calls — Cheat Sheet
📖 1. read(int fd, void *buf, size_t count)
Reads bytes from a file descriptor into a buffer.
fd: file descriptor (e.g., returned by open)
buf: buffer to read into
count: max number of bytes to read
✅ Returns number of bytes read, or -1 on error
🔹 Use: To read data from input files (especially in chunks or blocks).

🖊️ 2. write(int fd, const void *buf, size_t count)
Writes bytes to a file descriptor.
fd = 1: stdout, fd = 2: stderr
✅ Returns number of bytes written, or -1 on error
🔹 Use: Output to console or write reversed data to files.

🧭 3. lseek(int fd, off_t offset, int whence)
Moves the file pointer to a desired location.
whence can be:
SEEK_SET: from beginning
SEEK_CUR: from current position
SEEK_END: from end
✅ Returns new offset or -1 on error
🔹 Use: For partial or reverse reads/writes. E.g., reading file from end to start.

📂 4. stat(const char *pathname, struct stat *statbuf)
Gets metadata about a file (size, permissions, etc.)
Populates a struct stat which includes file size, type, etc.
🔹 Use: To find file size before reversing or validating input.

💾 5. fflush(FILE *stream)
Flushes buffered output. Typically used with printf() or fwrite().
❌ Not used in system calls-based code, since you are using write().
🔹 Ignore it unless you're using FILE* streams (which you're not supposed to).

⚠️ 6. perror(const char *msg)
Prints the last error message (based on errno) to stderr.
Automatically appends error string to your message.
🔹 Use: For debugging errors after read, write, open, etc.

🏗️ 7. mkdir(const char *pathname, mode_t mode)
Creates a new directory.
mode sets permissions (e.g., 0777 for full access).
🔹 Use: To create your Assignment1/ output folder if it doesn't exist.

🗃️ 8. open(const char *pathname, int flags[, mode_t mode])
Opens a file and returns a file descriptor.
flags: O_RDONLY, O_WRONLY, O_RDWR, O_CREAT, O_TRUNC, etc.
If creating a file, provide mode (e.g., 0644)
🔹 Use: To open input file for reading or output file for writing.

📴 9. close(int fd)
Closes an open file descriptor.
✅ Returns 0 on success, -1 on error
🔹 Use: Always close files when done to avoid resource leaks.