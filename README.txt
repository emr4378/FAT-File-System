Operating Systems I Project
Linux-based FAT File System and Shell

Eduardo Rodrigues - emr4378

---------------
--File System--
---------------

File system is comprised of three elements; a file allocation table, a directory table and a boot record.

Boot record - contains basic information about the file system such as the cluster size, the disc size (total filesystem size), and the location of the root directory table entry in the file allocation table. Always resides at address 0 (first entry in file allocation table).

Directory table - list of files in the system. Each entry will consist of: filename, starting FAT index, size (bytes), and creation date. Each entry is exactly 128 bytes.

File allocation table - A list of clusters. A cluster stores a memory address; unless it's 0 (empty) or 0xFFFF (end of file cluster), the value is the index of the next cluster in the chain. The number of clusters is (total disk size)/(cluster size). The index used to access an entry in this table, multiplied by the cluster size, yields the position in the actual file system where the file's data is stored.

---------------
-----Shell-----
---------------

The shell is started in the following manner:

./os1shell filesystem

where filesystem is the name of the file system to be used. If the file system does not exist, the shell will prompt the user to input parameters to set up the initial file system.

When a file system is created, the mount point is "/" in the real file system. However, the actual file system file will be located in the same directory "./os1shell" is run from. Additionally, relative paths update accordingly; "./" refers to the fake file system, while "../" refers to "/" in the real file system.

The shell supports the following commands:
ls
touch
cp
mv
rm
df
cat

If a real Linux command is enterred and not supported by the shell, the shell simply forwards the command to the terminal and executes it normally. Therefore, the shell maintains full terminal functionality.

Addtionally, files can be moved to and from fake file system to real filesystem regardless of file type using the "cp" and "mv" commands.

To exit the shell, end standard input (Ctrl-D) or end the process (Ctrl-C).

Sample commands:
cp /home/emr4378/Desktop/a.txt .
mv /home/emr4378/Desktop/a.txt /Eduardo_FS/a_2.txt
rm ./a.txt
mv a_2.txt a.txt
cat a.txt
