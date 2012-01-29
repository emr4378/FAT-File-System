#ifndef SHELL_H
#define SHELL_H

#include <iostream>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string>

#include "FileSys.h"

class Shell {
	public:
		Shell(char *name);
		~Shell();
		void prompt();
		bool checkIfExists(string path);
		bool checkIfDirExists(string path);
	private:
		string *fakeFilePath;
		string *filePath;
		string *oldFilePath;
		FileSys *fileSystem;

		void getTokens(string orig, string delims, string tokens[]);
		int runCommand(string cmdline);
		int changeDirectory(string tokens[]);
		void getAbsoluteFromRelativePath(string relPath, string *absPath);
		int createFileSystem(string name);
		int runFakeCommand(string tokens[]);
		int runRealCommand(string tokens[]);
		bool isCommandSupported(string cmd);
};
#endif
