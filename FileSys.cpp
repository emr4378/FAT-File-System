/**
 * The main File System class. All the file system magic happens here.
 *
 * @author: Eduardo Rodrigues - emr4378
 */
 

using namespace std;

#include "FileSys.h"

/**
 * Opens a file system and loads in the Boot Record, FAT and root directory
 *
 * If file isn't filesystem, it shouldn't open. Determines if
 * file is file system but looking at where boot record should be; if
 * it loads in correctly then probably a file system.
 *
 * @param name string containing the name of the filesystem to be opened
 * @return int 0 if file system loads in right, -1 if not (not file system)
 */
int FileSys::openFileSys(string name) {
	int ret = -1;
	int i;
	file = fopen(name.c_str(), "r+");
	if (file != NULL) {
		boot = new BootRecord();
		readBootRecord(boot);

		if ((boot->clusterSize < (MIN_CLUSTER_SIZE * 1024) 
			|| boot->clusterSize > (MAX_CLUSTER_SIZE * 1024)
			|| boot->size < (MIN_FILE_SIZE * 1024 * 1024)
			|| boot->size > (MAX_FILE_SIZE * 1024 * 1024)
			|| boot->size < boot->clusterSize)) {
			ret = -1;
		} else {
			sysName = name;
			entriesPerTable = (boot->clusterSize)/DT_ENTRY_SIZE;
			numClusters = (boot->size)/(boot->clusterSize);
			fileAllocationTable = new int[numClusters];
			directoryTable.resize(entriesPerTable);

			readFAT(fileAllocationTable, boot->FAT);
			readDirectoryTable(&directoryTable, boot->rootDir);

			printInfo();
			ret = 0; 
		}
	}

	return ret;
}

/**
 * Creates a file system. Saves the inital boot record, FAT and 
 * root directory table to a file named after file system.
 *
 * @param name string containing name of file system
 * @param fSize int the total size of the file system
 * @param cSize int the size of the clusters in the file system
 * @return int 0 if file system is created, -1 otherwise
 */
int FileSys::createFileSys(string name, int fSize, int cSize) {
	int ret = -1;
	int i;

	file = fopen(name.c_str(), "w+");
	if (file != NULL) {
		boot = new BootRecord();
		boot->clusterSize = cSize * 1024;
		boot->size = fSize * 1024 * 1024;
		boot->FAT = 1;
		boot->rootDir = 2;
		
		sysName = name;
		entriesPerTable = (boot->clusterSize)/DT_ENTRY_SIZE;
		numClusters = (boot->size)/(boot->clusterSize);
		fileAllocationTable = new int[numClusters];

		fileAllocationTable[0] = 0xFFFF;
		fileAllocationTable[1] = 0xFFFF;
		fileAllocationTable[2] = 0xFFFF;
		for (i = 3; i < numClusters; i++) {
			fileAllocationTable[i] = 0x0000;
		}
		directoryTable.resize(entriesPerTable);
		for (i = 0; i < entriesPerTable; i++) {
			directoryTable[i].name[0] = 0x00;
		}

		writeBootRecord(boot);
		writeFAT(fileAllocationTable, boot->FAT);
		writeDirectoryTable(&directoryTable, boot->rootDir);

		findUsedClusterCount();

		cout << "ls" << endl;

		printDirectoryTable(directoryTable);

		ret = 0;
	}
	return ret;
}

/**
 * This beast of a function will compress a directory table to the smallest
 * amount of clusters it will take. I used to call it whenever a file was
 * removed, but now she be disabled. As far as I know it worked, but whatevs.
 *
 * @param table pointer to the directory table to compress
 * @param cluster int index in FAT where the table is located (starts)
 */
void FileSys::compressDirectoryTable(vector<DirectoryTableEntry> *table, int cluster) {
	/*
	int tempSize = getDirectoryFileCount(*table);
	int mult = ceil((float)tempSize / (float)entriesPerTable);
	if (tempSize == 0) {
		mult = 1;
	}
	
	int tempCluster = cluster;
	int count = 1;
	int oldCluster = cluster;

	while (tempCluster != 0xFFFF) {
		oldCluster = tempCluster;
		tempCluster = fileAllocationTable[tempCluster];
		if (count > mult) {
			fileAllocationTable[oldCluster] = 0x0000;
		} else if (count == mult) {
			fileAllocationTable[oldCluster] = 0xFFFF;
		}
		count++;
	}

	table->resize( entriesPerTable * mult );
*/
}

/**
 * Writes given directory table vector to the file.
 *
 * @param table pointer to directory table vector
 * @param cluster the cluster in the FAT where the table starts
 */
void FileSys::writeDirectoryTable(vector<DirectoryTableEntry> *table, int cluster) {
	int offset;
	int i = 0;
	int j = 0;
	int clusterSize = boot->clusterSize;

	do {
		offset =  clusterSize * cluster;
		fseek(file, offset, SEEK_SET);

		fwrite(&((*table)[i * entriesPerTable]), 128, entriesPerTable, file);
		cluster = fileAllocationTable[cluster];
		i++;
	} while(cluster != 0xFFFF);
}

/**
 * Reads a directory table from the file and stores it in given 
 * directory table vector
 *
 * @param table pointer to directory table vector
 * @param cluster the cluster in the FAT where the table starts
 */
int FileSys::readDirectoryTable(vector<DirectoryTableEntry> *table, int cluster) {
	int offset;
	int i = 0;
	int j = 0;
	int clusterSize = boot->clusterSize;

	do {
		table->resize((i + 1)*entriesPerTable);
		offset =  clusterSize * cluster;
		fseek(file, offset, SEEK_SET);
		fread(&((*table)[i * entriesPerTable]), 128, entriesPerTable, file);
		cluster = fileAllocationTable[cluster];
		i++;
	} while(cluster  != 0xFFFF);
	return table->size();
}

/**
 * Writes the given FAT to the file.
 *
 * @param fat pointer to FAT array
 * @param cluster the cluster in the FAT where the table is located
 */
void FileSys::writeFAT(int *fat, int cluster) {
	int offset =  boot->clusterSize * cluster;
	if (offset < boot->size) {
		fseek(file, offset, SEEK_SET);
		fwrite(fat, numClusters * sizeof(int), 1, file);
	}
}

/**
 * Reads the FAT from the file to the FAT array given.
 *
 * @param fat pointer to FAT array
 * @param cluster the cluster in the FAT where the table is located
 */
void FileSys::readFAT(int *fat, int cluster) {
	int offset =  boot->clusterSize * cluster;
	if (offset < boot->size) {
		fseek(file, offset, SEEK_SET);
		fread(fat, numClusters * sizeof(int), 1, file);
	}
}

/**
 * Writes the Boot Record to the file.
 * @param boot pointer to the Boot Record
 */
void FileSys::writeBootRecord(BootRecord *boot) {
	fseek(file, 0, SEEK_SET);
	fwrite(boot, BOOT_RECORD_SIZE, 1, file); 
}

/**
 * Writes the Boot Record to the file.
 * @param boot pointer to the Boot Record
 */
void FileSys::readBootRecord(BootRecord *boot) {
	fseek(file, 0, SEEK_SET);
	fread(boot, BOOT_RECORD_SIZE, 1, file);
}

/**
 * Finds the actual number of files/directories in the given directory.
 * Different from the directory size; size is the total amount of entries
 * it can hold before needing a resize.
 *
 * @param table the Directory Table whose files should be counted
 * @return the total number of files
 */
int FileSys::getDirectoryFileCount(vector<DirectoryTableEntry> table) {
	int ret = 0;
	int i;
	for (i = 0; i < table.size(); i++) {
		if (table[i].name[0] != (char)0x00 && table[i].name[0] != (char)0xFF) {
			ret++;
		}
	}

	return ret;
}


/**
 * Finds the total number of used clusters in the File allocation Table (FAT).
 * A cluster is deemed used if it's value isn't 0 (0x0000).
 *
 * @return the number of used clusters
 */
int FileSys::findUsedClusterCount() {
	int i;
	usedClusters = 0;

	for (i = 0; i < numClusters; i++) {
		if (fileAllocationTable[i] != 0x0000) {
			usedClusters++;
		}
	}

	return usedClusters;
}

/**
 * Finds the next available cluster in the File Allocation Table (FAT).
 * A cluster is deemed free if it's value is 0 (0x0000).
 *
 * @return an available FAT/cluster index; 0xFFFF if no clusters are free
 */
int FileSys::findNextFreeCluster() {
	int i;
	int ret = 0xFFFF;

	for (i = 1; i < numClusters && ret == 0xFFFF; i++) {
		if (fileAllocationTable[i] == 0x0000) {
			ret = i;
		}
	}

	return ret;
}

/**
 * Finds the Directory Table index of a file by it's filename.
 *
 * @param name string containing name of file
 * @return index of file if it exists; -1 otherwise
 */
int FileSys::findIndexForFile(string name) {
	int i;
	int index = -1;
	for (i = 0; i < directoryTable.size() && index == -1; i++) {
		if (directoryTable[i].name == name) {
			index = i;
		}
	}
	return index;
}

/**
 * The "touch" functionality of filesystem.
 *
 * Creates a 0 byte file, with timestamp and given name
 * and adds it to the first available spot in the directory
 * table and first available spot in the FAT.
 * Will not create file if file of same name is found.
 *
 * @param name string containing name of the file to be created
 * @return directory index of file if created; -2 if out of clusters, -1 otherwise
 */
int FileSys::createFile(string name) {
	int ret = -2;
	int i;
	int cluster = findNextFreeCluster();
	int index = -1;
	bool unique = true;

	if (cluster != 0xFFFF) {
		ret = -1;
		for (i = 0; i < directoryTable.size() && unique; i++) {
			if (index == -1 &&
				(directoryTable[i].name[0] == (char)0x00 || 
				directoryTable[i].name[0] == (char)0xFF)) {
				index = i;
			}
			if (directoryTable[i].name == name) {
				unique = false;
			}
		}
		if (unique) {
			//new file at cluster; filled here in case directory table must grow
			fileAllocationTable[cluster] = 0xFFFF;
			if (index == -1) {
				//if directory table is filled
				int dirCluster = boot->rootDir;
				while (fileAllocationTable[dirCluster] != 0xFFFF) {
					dirCluster = fileAllocationTable[dirCluster];
				}
			
				fileAllocationTable[dirCluster] = findNextFreeCluster();
				if (fileAllocationTable[dirCluster] != 0xFFFF) {
					dirCluster = fileAllocationTable[dirCluster];
					fileAllocationTable[dirCluster] = 0xFFFF;
					int dirTableSize = directoryTable.size();
					directoryTable.resize(dirTableSize+entriesPerTable);
					for (i = dirTableSize; i < directoryTable.size(); i++) {
						directoryTable[i].name[0] = 0x00;
					}
					index = dirTableSize;
				} else {
					ret = -2;
				}
			}

			if (index != -1 && ret != -2) {
				strcpy(directoryTable[index].name, name.c_str());
				directoryTable[index].index = cluster;
				directoryTable[index].size = 0;
				directoryTable[index].type = 0x00;
				directoryTable[index].creation = time(NULL);

				writeFAT(fileAllocationTable, boot->FAT);
				writeDirectoryTable(&directoryTable, boot->rootDir);

				ret = index;
			} else {
				readFAT(fileAllocationTable, boot->FAT);
				readDirectoryTable(&directoryTable, boot->rootDir);
			}
		}
	}

	return ret;
}

/**
 * The "cp" functionality of the filesystem.
 * 
 * Copies source param to dest param. At least one should be local
 * (inside this FileSys), otherwise this shouldn't be called
 * 
 * @param source string containing file to be copied
 * @param dest string containing destination of file
 * @param sourceInFileSys boolean indicating if source is local (in FileSys)
 * @param destInFileSys boolean indicating if dest is local (in FileSys)
 * @return int If both files are external, -2. If file can't be found, -1. Otherwise 0
 */
int FileSys::copyFile(string source, string dest, 
						bool sourceInFileSys, bool destInFileSys) {
	//struct dirent dirp;
	//DIR *dir;
	string fileName;
	int i;
	int ret = -2;
	if (source == dest) {
		ret = -3;
	} else {
		ret = -1;
		/*
			//apparently this doesn't work in Solaris; there is no d_type in the
			//dirent struct. AWESOME.

			if (source[source.size()-1] == '*') {
			source.erase(source.end()-1);
			cout << "Copy all " << endl;

			if (!sourceInFileSys) {
				dir = opendir(source.c_str());
				if (dir != NULL) {
					while ((dirp = readdir(dir)) != NULL) {
						if (dirp->d_type == 8 && dirp->d_name != "") {
							fileName = source;
							fileName.append(dirp->d_name);
							copyFile(fileName, dest, false, destInFileSys);
						}
					}
				}
			} else {
				int dirTableSize = getDirectoryFileCount(directoryTable);
				for (i = 0; i < dirTableSize; i++) {
					fileName = dest;
					fileName.append(directoryTable[i].name);
					copyFile(directoryTable[i].name, fileName, true, destInFileSys);
				}
			}
			//copy all
		} else {*/

		if (source[dest.size()-1] == '/' || source.empty()) {
			//directory source
			source.append(dest.substr(dest.find_last_of('/') + 1));
		}
		if (dest[source.size()-1] == '/' || dest.empty()) {
			//directory destination
			dest.append(source.substr(source.find_last_of('/') + 1));
		}
		if (!destInFileSys && sourceInFileSys) {
			//internal (fake/the FileSys) to external (real)
			ret = copyFileInToExt(source, dest);
		} else if (!sourceInFileSys && destInFileSys) {
			//external (real) to internal (fake/the FileSys)
			ret = copyFileExtToIn(source, dest);
		} else if (sourceInFileSys && destInFileSys) {
			//internal to internal
			ret = copyFileInternally(source, dest);
		}
	}
	return ret;
}

/**
 * A sub-component of the "cp" functionality of the filesystem.
 *
 * Copies an internal file system to the  external file (real).
 *
 * Should NEVER be called by anything other than copyFile()
 *
 * @param source string containing the name of the source file
 * @param dest string containing the full path name of the destination file
 * @return int If error, -1; otherwise 0
 */
int FileSys::copyFileInToExt(string source, string dest) {
	int ret = -1;
	string fileName;
	FILE *outerFile;
	int index;
	int offset;
	int j;
	int cluster;
	int sourceIndex;
	int destIndex;
	int sourceCluster;
	int destCluster;
	int leftOver;
	int i = 0;
	int clusterSize = boot->clusterSize;
	void *clusterData = malloc(clusterSize);

	//internal (fake/the FileSys) to external (real)
	outerFile = fopen(dest.c_str(), "w+");
	if (outerFile != NULL) {
		index = findIndexForFile(source);	
		if (index != -1) {
			cluster = directoryTable[index].index;
			leftOver = directoryTable[index].size % clusterSize;

			while (fileAllocationTable[cluster] != 0xFFFF) {
				offset = clusterSize * cluster;
				fseek(outerFile, (clusterSize * i), SEEK_SET);
				fseek(file, offset, SEEK_SET);
				fread(clusterData, clusterSize, 1, file);
				fwrite(clusterData, clusterSize, 1, outerFile);
				
				cluster = fileAllocationTable[cluster];
				i++;
			}

			if (leftOver != 0) {
				clusterData = realloc(clusterData, leftOver);
				offset = clusterSize * cluster;
				fseek(outerFile, (clusterSize * i), SEEK_SET);
				fseek(file, offset, SEEK_SET);
				fread(clusterData, leftOver, 1, file);
				fwrite(clusterData, leftOver, 1, outerFile);
			}

			ret = 0;
		}
		fclose(outerFile);
	}

	free(clusterData);

	return ret;
}

/**
 * A sub-component of the "cp" functionality of the filesystem.
 *
 * Copies an external file (real) to the internal file system.
 *
 * Should NEVER be called by anything other than copyFile()
 *
 * @param source string containing the full path name of the source file
 * @param dest string containing the name of the destination file
 * @return int -1 if error, -2 if out of clusters, 0 otherwise
 */
int FileSys::copyFileExtToIn(string source, string dest) {
	int ret = -1;
	string fileName;
	FILE *outerFile;
	int index;
	int offset;
	int j;
	int cluster;
	int sourceIndex;
	int destIndex;
	int sourceCluster;
	int destCluster;
	int i = 0;
	int clusterSize = boot->clusterSize;
	void *clusterData = malloc(clusterSize);
	
	//external (real) to internal (fake/the FileSys)
	outerFile = fopen(source.c_str(), "r");
	if (outerFile != NULL) {
		removeFile(dest); //if dest already exists, delete/overwrite
		index = createFile(dest);
		if (index >= 0) {
			cluster = directoryTable[index].index;

			fseek(outerFile, 0, SEEK_END);
			directoryTable[index].size = ftell(outerFile);
			fseek(outerFile, 0, SEEK_SET);

			while (!feof(outerFile) && cluster != 0xFFFF) {
				offset = clusterSize * cluster;
				fseek(outerFile, (clusterSize * i), SEEK_SET);
				fseek(file, offset, SEEK_SET);
				fread(clusterData, clusterSize, 1, outerFile);
				fwrite(clusterData, clusterSize, 1, file);

				fileAllocationTable[cluster] = 0xFFFF;
				if (!feof(outerFile)) {
					fileAllocationTable[cluster] = findNextFreeCluster();
					cluster = fileAllocationTable[cluster];
				}
				i++;
			}

			if (!feof(outerFile)) {
				//OH SHIT, outta room baby!
				//undo all the changes
				readFAT(fileAllocationTable, boot->FAT);
				readDirectoryTable(&directoryTable, boot->rootDir);
				removeFile(index);
				ret = -2;
			} else {
				writeFAT(fileAllocationTable, boot->FAT);
				writeDirectoryTable(&directoryTable, boot->rootDir);
				ret = 0;
			}
		} else {
			ret = index;
		}
		fclose(outerFile);
	}

	free(clusterData);

	return ret;
}

/**
 * A sub-component of the "cp" functionality of the filesystem.
 *
 * Copies an internal file of the filesystem to the filesystem. Basically
 * iterates through all the clusters of a file and copies them to the next
 * available cluster.
 *
 * Should NEVER be called by anything other than copyFile()
 *
 * @param source string containing the name of the source file
 * @param dest string containing the name of the destination file
 * @return int -1 if error, -2 if out of clusters, 0 otherwise
 */
int FileSys::copyFileInternally(string source, string dest) {
	int ret;
	int i;
	int sourceIndex;
	int destIndex;
	int sourceCluster;
	int destCluster;
	int clusterSize = boot->clusterSize;
	void *clusterData = malloc(clusterSize);

	sourceIndex = findIndexForFile(source);
	if (sourceIndex != -1) {
		removeFile(dest); //if dest already exists, delete/overwrite
		destIndex = findIndexForFile(dest);
		if (destIndex == -1) {
			destIndex = createFile(dest);
		}
		if (destIndex >= 0) {
			destCluster = directoryTable[destIndex].index;
			sourceCluster = directoryTable[sourceIndex].index;
			directoryTable[destIndex].size = directoryTable[sourceIndex].size;

			while(sourceCluster != 0xFFFF && destCluster != 0xFFFF) {
				fseek(file, (clusterSize * sourceCluster), SEEK_SET);
				fread(clusterData, clusterSize, 1,  file);
				fseek(file, (clusterSize * destCluster), SEEK_SET);
				fwrite(clusterData, clusterSize, 1,  file);

				sourceCluster = fileAllocationTable[sourceCluster];
			
				fileAllocationTable[destCluster] = 0xFFFF;
				if (sourceCluster != 0xFFFF) {
					fileAllocationTable[destCluster] = findNextFreeCluster();
					destCluster = fileAllocationTable[destCluster];
				}
			}

			if (sourceCluster != 0xFFFF) {
				//OH SHIT, outta room baby!
				//undo all the changes
				readFAT(fileAllocationTable, boot->FAT);
				readDirectoryTable(&directoryTable, boot->rootDir);
				removeFile(destIndex);
				ret = -2;
			} else {
				writeFAT(fileAllocationTable, boot->FAT);
				writeDirectoryTable(&directoryTable, boot->rootDir);
				ret = 0;
			}
		} else {
			ret = destIndex;
		}
	}

	free(clusterData);
	
	return ret;
}

/**
 * The "mv" functionality of the filesystem.
 *
 * Removes destination if it exists, then passes all parameters onto copyFile() 
 * then removes the source file
 *
 * @param source string containing file to be copied
 * @param dest string containing destination of file
 * @param sourceInFileSys boolean indicating if source is local (in FileSys)
 * @param destInFileSys boolean indicating if dest is local (in FileSys)
 * @return int -1 if error, -2 if out of clusters, 0 otherwise
 */ 
int FileSys::moveFile(string source, string dest, 
						bool sourceInFileSys, bool destInFileSys) {
	int ret = -1;
	if (source != dest) {
		ret = copyFile(source, dest, sourceInFileSys, destInFileSys);
		if (ret >= 0) {
			if (sourceInFileSys) {
				ret = removeFile(source);
			} else {
				ret = remove(source.c_str());
			}
		}
	}

	return ret;
}

/**
 * The "rm" functionality of the filesystem.
 *
 * Finds the index inside the directory table of a file
 * given by it's file name. Can handle wildcard "*" name.
 *
 * @param name string containing name of the file to be removed
 * @return int 0 if (all) removed successfully, -1 otherwise
 */
int FileSys::removeFile(string name) {
	int ret = 0;
	int i;
	int cluster;
	int oldCluster;
	int index = -1;
	for (i = directoryTable.size()-1; i >= 0 && (name == "*" || index == -1); i--) {
		if (directoryTable[i].name == name ||
			(name == "*" && (directoryTable[i].name[0] != (char)0x00 
					&& directoryTable[i].name[0] != (char)0xFF))) {
			index = i;
			if (removeFile(index) == -1) {
				ret = -1;
			}
		}
	}

	return 0;
}

/**
 * The background "rm" functionality of the filesystem.
 *
 * Removes a file from by setting the first bit of the file
 * name to the deleted flag (0xFF). Removes all of the file's
 * clusters from the FAT.
 *
 * Should NEVER be called by anything other than removeFile(string).
 *
 * @param index the index of the entry in the directory table to be removed
 * @return int 0 if removed successfully, -1 otherwise
 */
int FileSys::removeFile(int index) {
	int ret = -1;
	int cluster;
	int oldCluster;
	if (index != -1) {
		directoryTable[index].name[0] = 0xFF;

		cluster = directoryTable[index].index;
		do {
			oldCluster = cluster;
			cluster = fileAllocationTable[oldCluster];
			fileAllocationTable[oldCluster] = 0x0000;
		} while(cluster != 0xFFFF);

		directoryTable.erase(directoryTable.begin() + index);
		writeFAT(fileAllocationTable, boot->FAT);
		writeDirectoryTable(&directoryTable, boot->rootDir);
		ret = 0;
	}

	return ret;
}

/**
 * The "cat" functionality of the filesystem.
 *
 * Prints the contents of a file to standard output (terminal)
 *
 * @param name string containing name of the file to be printed
 * @return int -1 if error (file not found), otherwise 0
 */
int FileSys::printFile(string name) {
	int ret = -1;
	int index;
	int cluster;
	int leftOver;
	int offset;
	int clusterSize = boot->clusterSize;
	void *clusterData;

	index = findIndexForFile(name);
	if (index != -1) {
		clusterData = malloc(clusterSize);
		cluster = directoryTable[index].index;
		leftOver = directoryTable[index].size % clusterSize;

		while (fileAllocationTable[cluster] != 0xFFFF) {
			offset = clusterSize * cluster;
			fseek(file, offset, SEEK_SET);
			fread(clusterData, clusterSize, 1, file);
			cout << (char*)clusterData;
			cluster = fileAllocationTable[cluster];
		}
		offset = clusterSize * cluster;
		fseek(file, offset, SEEK_SET);
		while (leftOver > 0) {
			cout << (char)fgetc(file);
			leftOver--;
		}

		free(clusterData);
		ret = 0;
	}

	return ret;
}

/**
 * The "df" functionality of the filesystem.
 *
 * Shows the structure of the filesystem
 */
void FileSys::showStructure() {
	findUsedClusterCount();

	cout << left << setw(15) << "Filesystem";
	cout << " ";
	cout << right << setw(10) << "Size";
	cout << " ";
	cout << right << setw(5) << ((boot->clusterSize)/1024) << "K-clusters";
	cout << " ";
	cout << right << setw(8) << "Used" ;
	cout << " ";
	cout << right << setw(9) << "Available";
	cout << " ";
	cout << right << setw(3) << "Use%" ;
	cout << endl;

	cout << left << setw(15) << sysName;
	cout << " ";
	cout << right << setw(10) << boot->size;
	cout << " ";
	cout << right << setw(15) << numClusters;
	cout << " ";
	cout << right << setw(8) << usedClusters;
	cout << " ";
	cout << right << setw(9) << (numClusters-usedClusters);
	cout << " ";
	cout << right << setw(3) << (int)(((double)usedClusters/(double)numClusters)*100) << "%" ;
	cout << endl << endl;
}

/**
 * A function for my benefit.
 * Just prints out the FAT table and current directory table
 */
void FileSys::printInfo() {
	printInfo(6, &directoryTable);
}

/**
 * The "df" functionality of the file system.
 * Prints out the FAT table, the filesystem structure data, and a directory
 *
 * @param width int how many columns the FAT table should be; 6 is awesome
 * @param table pointer to directory table vector to be printed; can be NULL
 */
void FileSys::printInfo(int width, vector<DirectoryTableEntry> *table) {
	printFAT(width);
	showStructure();
	if (table != NULL) {
		printDirectoryTable(*table);
	}
}

/**
 * Prints the File Allocation Table (FAT) to standard output (terminal).
 * 
 * The entries are printed in the format <cluster #>:<cluster value>
 *
 * If the cluster is free, its value is 0 and green
 * If the cluster is reserved, its value is OxFE(65534) and red
 * If the cluster is the last cluster in a chain, its value is 0xFF(65535) and red
 * If the cluster points to another cluster, its value is blue
 * 
 * @param width the number of FAT entries to print per line
 */
void FileSys::printFAT(int width) {
	int i;
	string titleRow;
	string rowDivide;
	string title = "File Allocation Table";
	for (i = 0; i < width; i++) {
		rowDivide += "-----------";
	}
	titleRow = rowDivide;
	
	cout << titleRow.replace(titleRow.length()/2-title.length()/2, 
							title.length(), title);
	for (i = 0; i < numClusters; i++) {
		if (i % width == 0) {
			cout << endl;
		}
		cout << "\033[1;1m" << setw(4) << i << ":";
		if (fileAllocationTable[i] == 0x0000) {
			cout << "\033[0;32m";
		} else if (fileAllocationTable[i] >= 0xFFFE) {
			cout << "\033[0;31m";
		} else {
			cout << "\033[0;34m";
		}
		cout << setw(5) << fileAllocationTable[i];
		cout << "\033[0m" << "|";

	}
	cout << endl << endl;
}

/**
 * The "ls" functionality of the filesystem.
 *
 * Prints out all non-empty and non-deleted entries
 * in the given DirectoryTable array
 * 
 * @param table the DirectoryTableEntry to print (aka the directory)
 */
void FileSys::printDirectoryTable(vector<DirectoryTableEntry> table) {
	int i;
	time_t time;

	cout << setw(5) << "" << left << setw(17) << "Filename";
	cout << left << setw(11) << "Index";
	cout << left << setw(10) << "Size";
	cout << left << setw(10) << "Type";
	cout << left << setw(25) << "Created";

	cout << endl;
	cout << setfill('-') << setw(75) << "";
	cout << setfill(' ');

	for (i = 0; i < table.size(); i++) {
		if (table[i].name[0] != (char)0x00 && table[i].name[0] != (char)0xFF) {
			cout << endl;
			cout << left << setw(5) << i << left << setw(17) << table[i].name;
			cout << right << setw(5) << table[i].index;
			cout << right << setw(10) << table[i].size;
			cout << right << setw(10) << table[i].type;

			time = table[i].creation;
			cout << " ";
			cout << right << setw(30) << ctime(&time);

		
		}
	}
	cout << setfill('-') << setw(75) << "";
	cout << setfill(' ') << endl;;
}

/**
 * Prints the current directory table.
 */
void FileSys::printDirectoryTable() {
	printDirectoryTable(directoryTable);
}

/**
 * Deconstructor
 */
FileSys::~FileSys() {
	delete[] fileAllocationTable;
	delete boot;
	fclose(file);
}
