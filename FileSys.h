#ifndef FILESYS_H
#define FILESYS_H

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <time.h>
#include <math.h>

#define MAX_FILE_SIZE 50 //MB
#define MIN_FILE_SIZE 5 //MB
#define MIN_CLUSTER_SIZE 8 //KB
#define MAX_CLUSTER_SIZE 16 //KB
#define DT_ENTRY_SIZE 128 //Bytes
#define BOOT_RECORD_SIZE 16 //Bytes

/**
 * Should reside at address 0 in FileSys 
 * Also should only be 16 bytes (BOOT_RECORD_SIZE)
 */
struct BootRecord {
	unsigned int clusterSize; //the size of the cluster, in bytes
	unsigned int size; //the total size of the disk, in bytes
	unsigned int rootDir; //index to the cluster storing the root directory
	unsigned int FAT; //index to the cluster storing the FAT table
};

/**
 * Should be 128 bytes (DT_ENTRY_SIZE)
 */
struct DirectoryTableEntry {
	char name[112]; //Filname; first byte signifies free(0x00) or deleted(0xFF)
	unsigned int index; //index of first cluster
	unsigned int size; //size of file, in bytes (0 for directories)
	unsigned int type; //File(0x00) or Directory(0xFF)
	unsigned int creation; //create date of file (unix epoch format)
};

class FileSys {
	public:
		~FileSys();
		int openFileSys(string name);
		int createFileSys(string name, int fSize, int cSize);
		
		void printFAT(int width);
		void printDirectoryTable();
		void printDirectoryTable(vector<DirectoryTableEntry> table);
		int createFile(string name);
		int copyFile(string source, string dest, 
						bool sourceInFileSys, bool destInFileSys);
		int moveFile(string source, string dest, 
						bool sourceInFileSys, bool destInFileSys);
		int removeFile(string name);
		void showStructure();
		int printFile(string name);
		void printInfo(int width, vector<DirectoryTableEntry> *table);
		void printInfo();

	private:
		void writeDirectoryTable(vector<DirectoryTableEntry> *table, int cluster);
		int readDirectoryTable(vector<DirectoryTableEntry> *table, int cluster);
		void writeFAT(int *fat, int cluster);
		void readFAT(int *fat, int cluster);
		void writeBootRecord(BootRecord *boot);
		void readBootRecord(BootRecord *boot);
		int findNextFreeCluster();
		int findUsedClusterCount();
		int findIndexForFile(string name);
		int removeFile(int index);
		int copyFileInternally(string source, string dest);
		int copyFileInToExt(string source, string dest);
		int copyFileExtToIn(string source, string dest);
		int getDirectoryFileCount(vector<DirectoryTableEntry> table);
		void compressDirectoryTable(vector<DirectoryTableEntry> *table, int cluster);
	
		
		string sysName;
		FILE *file;
		int usedClusters;
		int entriesPerTable;
		int numClusters;
		int *fileAllocationTable;
		vector<DirectoryTableEntry> directoryTable;
		BootRecord *boot;
};
#endif
