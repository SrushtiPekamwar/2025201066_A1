# 2025201066_A1

# Creating 5GB size file 
truncate -s 5G bigfile

🧭 3. lseek(int fd, off_t offset, int whence)
Moves the file pointer to a desired location.
whence can be:
SEEK_SET: from beginning
SEEK_CUR: from current position
SEEK_END: from end
✅ Returns new offset or -1 on error
🔹 Use: For partial or reverse reads/writes. E.g., reading file from end to start.