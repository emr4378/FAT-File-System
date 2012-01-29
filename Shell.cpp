/**
 * The Shell class. Contains all the shell command magic.
 * Handles all the user input and contact to the FileSys.
 *
 * @author: Eduardo Rodrigues - emr4378
 */

using namespace std;

#include "Shell.h"

/**
 * Constructor
 */
Shell::Shell(char *name) {
	char *dir = (char*)malloc(sizeof(char)*FILENAME_MAX);
	int fileSysUse = -1;
	string tempName;

	getcwd(dir, sizeof(char)*FILENAME_MAX);
	filePath = new string(dir);
	oldFilePath = new string(dir);
	fakeFilePath = new string();
	free(dir);

	fileSystem = new FileSys();

	if (name != NULL) {
		tempName = name;
		getAbsoluteFromRelativePath(tempName, &tempName);
		if (checkIfExists(tempName)) {
			fileSysUse = fileSystem->openFileSys(name);
		} else {
			fileSysUse = createFileSystem(name);
		}
	}
	
	if (fileSysUse == 0) {
		chdir("/");
		*fakeFilePath = "/";
		*fakeFilePath += name;
		*filePath = *fakeFilePath;
	}

	cout << "Shell Created" << endl;
}

/**
 *
 * @return -1 if no filesystem created, 0 otherwise */
int Shell::createFileSystem(string name) {
	int ret = -1;
	int num;
	bool condMet;
	string input;
	int fileSize;
	int clusterSize;

	do {
		condMet = true;
		cout << "Are you sure you want to create a new file system[Y]? ";
		getline(cin, input);
		if (input != "" && input[0] != 'Y' && input[0] != 'N') {
			condMet = false;
			cout << "Please enter Y or N" << endl;
		}
	} while (!condMet && !cin.eof());

	if (input[0] == 'N') {
		ret = -1;
	} else {
		do {
			condMet = true;
			cout << "Enter the maximum size for this file system in MB [10]: ";
			getline(cin, input);
			if (input == "") {
				num = 10;
			} else {
				num = atoi(input.c_str());
				if (num > MAX_FILE_SIZE || num < MIN_FILE_SIZE) {
					condMet = false;
					cout << "Please enter integer between " << MIN_FILE_SIZE;
					cout << " and " << MAX_FILE_SIZE << endl;
				}
			}
		} while (!condMet && !cin.eof());

		fileSize = num;

		do {
			condMet = true;
			cout << "Enter the cluster size for this file system in KB [8]:";
			getline(cin, input);
			if (input == "") {
				num = 8;
			} else {
				num = atoi(input.c_str());
				if (num > MAX_CLUSTER_SIZE || num < MIN_CLUSTER_SIZE) {
					condMet = false;
					cout << "Please enter integer between " << MIN_CLUSTER_SIZE;
					cout << " and " << MAX_CLUSTER_SIZE << endl;
				}
			}
		} while (!condMet && !cin.eof());

		clusterSize = num;

		ret = fileSystem->createFileSys(name, fileSize, clusterSize);
	}

	return ret;
}

/**
 * Constantly prompts the user for input
 * until Ctrl+D or Ctrl+C is pressed
 */
void Shell::prompt() {
	char input[256];
	do {
		cout << "os1shell->";
		cin.getline(input, 256);
		runCommand(input);
		
	} while (!cin.eof());
	cout << endl;
}

/**
 * Tokenizes the user command and runs it
 *
 * If any of the parameters make use of the fake filesystem, it uses 
 * the fake filesystems commands. Otherwise, it just passes the command
 * to exec
 *
 * The cd command is a special exception; it isn't passed anywhere. It is handled
 * locally to provide the same functionality as a normal cd command.
 * 
 * @param cmdline string for of entire command
 */
int Shell::runCommand(string cmdline) {
	int ret = 0;
	string tokens[256];
	int i;
	int j;
	bool usingFake = false;
	getTokens(cmdline, " \n", tokens);
	bool leastOnePath = false;
	if (!tokens[0].empty()) {
		if (tokens[0] == "cd") {
			changeDirectory(tokens);
		} else {

			if (isCommandSupported(tokens[0])) {
				i = 1;
				while (!tokens[i].empty()) {
					if ((tokens[i][0] == '/' 
								|| (tokens[i].size() >= 1 
									&& tokens[i][0] != '-'))) {
						string newArg;
						getAbsoluteFromRelativePath(tokens[i], &newArg);
						tokens[i] = newArg;
						leastOnePath = true;
					}

					if (fakeFilePath->empty() == false && 
						(tokens[i].substr(0, 
							fakeFilePath->size()) == *fakeFilePath)) {
						usingFake = true;
					}
					i++;
				}
				if (!leastOnePath) {
					tokens[1] = *filePath;
					tokens[2].clear();

					if (fakeFilePath->empty() == false && 
						!tokens[1].empty() &&
						tokens[1].substr(0, 
							fakeFilePath->size()) == * fakeFilePath) {
						usingFake = true;
					}
				} 
			}

			if (usingFake) {
				ret = runFakeCommand(tokens);
				if (ret == -2) {
					cout << "ERROR: Not enough space in filesystem" << endl;
				} else if (ret < 0) {
					cout << tokens[0] << ": error with command" << endl;
				}
			} else {
				ret = runRealCommand(tokens);
			}
		}
	}
	return ret;
}

bool Shell::isCommandSupported(string cmd) {
	string cmds[] = {"ls", "touch", "cp", "mv", "rm", "df", "cat"};
	int i;
	bool ret = false;
	for (i = 0; i < 7 && ret == false; i++) {
		if (cmd == cmds[i]) {
			ret = true;
		}
	}

	return ret;
}

/**
 * Runs a real command; call fork() on exec() on it
 *
 * @param tokens string array containing tokenized version of command
 * @returns 0 if command runs fine; -1 if there's an error
 */
int Shell::runRealCommand(string tokens[]) {
	int ret = -1;
	int i;
	char* args[256];
	cout << "Command: ";
	i = 0;
	while (!tokens[i].empty()) {
		args[i] = (char*)malloc(sizeof(char)*tokens[i].length());
		strcpy(args[i], tokens[i].c_str());
		cout << args[i] << " ";
		i++;
	}
	args[i] = NULL;
	cout << endl;

	pid_t pID = fork();
	if (pID == 0) {
		//success,  Child stuffs
		execvp(*args, args);
		cout << "Not a valid command" << endl;
		exit(0);
	} else if (pID < 0) {
		//failure :(
		exit(-1);
	} else {
		//parent stuffs
		int status;
		pid_t tpid;
		do {
			tpid = wait(&status);
		} while (tpid != pID);

		while (i>0) {
			free(args[i]);
			i--;
		}

		ret = 0;
	}

	return ret;
}

/**
 * Runs a fake command; calls the appropriate methods in the FileSys
 * One runs supported commands: ls, touch, cp, mv, rm, df, cat
 *
 * @param tokens string array containing tokenized version of command
 * @returns 0 if command runs fine; -1 if there's an error
 */
int Shell::runFakeCommand(string tokens[]) {
	int ret = -1;
	int i;
	string cmd = tokens[0];
	if (cmd == "ls") {
		fileSystem->printDirectoryTable();
		ret = 0;
	} else if (cmd == "touch") {
		if (tokens[1].size() > fakeFilePath->size() + 1) {
			ret = fileSystem->createFile(tokens[1].substr(fakeFilePath->size() + 1));
		}
	} else if (cmd == "cp" || cmd == "mv") {
		string sourceFile;
		string destFile;

		i = 1;
		while (!tokens[i].empty() && destFile.empty()) {
			if (tokens[i][0] != '-') {
				if (sourceFile.empty()) {
					sourceFile = tokens[i];
				} else {
					destFile = tokens[i];
				}
			}
			i++;
		}
		if (destFile.empty()) {
			ret = -1;
		} else {
			bool sourceInFake = false;
			bool destInFake = false;
			if (tokens[1].find(*fakeFilePath) == 0) {
				if (tokens[1].size() > fakeFilePath->size() + 1) {
					sourceFile = sourceFile.substr(fakeFilePath->size() + 1);
				} else {
					sourceFile = "";
				}
				sourceInFake = true;
			}

			if (tokens[2].find(*fakeFilePath) == 0) {
				if (tokens[2].size() >= fakeFilePath->size() + 1) {
					destFile = destFile.substr(fakeFilePath->size() + 1);
					destInFake = true;
				} else {
					destFile = "";
				}
				destInFake = true;
			}
		
			if (cmd == "cp") {
				ret = fileSystem->copyFile(sourceFile, destFile, 
											sourceInFake, destInFake);
			} else if (cmd == "mv") {
				ret = fileSystem->moveFile(sourceFile, destFile, 
											sourceInFake, destInFake);
			}
		}
	} else if (cmd == "rm") {
		if (tokens[1].size() > fakeFilePath->size() + 1) {
			ret = fileSystem->removeFile(tokens[1].substr(fakeFilePath->size() + 1));
		}
	} else if (cmd == "df") {
		fileSystem->printInfo(6, NULL);
		ret = 0;
	} else if (cmd == "cat" && !tokens[1].empty()) {
		if (tokens[1].size() > fakeFilePath->size() + 1) {
			ret = fileSystem->printFile(tokens[1].substr(fakeFilePath->size() + 1));
		}
	} else {
		cout << cmd << " command not supported by fake filesystem." << endl;
		ret = -1;
	}

	return ret;
}

/**
 * Converts a relative path to it's absolute path equivalent.
 * It parses the entire thing, removes all ".."'s and "."'s
 * @param relPat the path string to be converted
 * @param absPath pointer to the absolute path string; will be stored there
 */
void Shell::getAbsoluteFromRelativePath(string relPath, string *absPath) {
	int j;
	string tempStr;
	string subToks[256];
	string newArg;
	struct stat st_buf;

	tempStr = relPath;
	if (relPath[0] != '/') {
		newArg = *filePath;
	} else {
		newArg = "/";
	}
	getTokens(tempStr, "/", subToks);
	
	j = 0;
	while (!subToks[j].empty()) {
		if (subToks[j] == "..") {
			int num = newArg.find_last_of("/");
			newArg = newArg.substr(0, num);
		} else if (subToks[j] != ".") {
			if (newArg[newArg.length()-1] != '/') {
				newArg += "/";
			}
			newArg += subToks[j];
		} 
		if (newArg.length() <= 0) {
			newArg += "/";
		}
		
		j++;
	}
	if (newArg[newArg.size()-1] != '/' && newArg != *fakeFilePath) {
		stat(newArg.c_str(), &st_buf);
		if (S_ISDIR(st_buf.st_mode)) {
			newArg += "/";
		}
	}
	*absPath = newArg;
}

/**
 * Changes the directory. Basically the "cd" command.
 * @param tokens a string array; the tokenized form of the command
 */
int Shell::changeDirectory(string tokens[]) {
	int i;
	int j;
	i = 1;
	string absPath;
	while (!tokens[i].empty() && absPath.empty()) {
		if (tokens[i][0] != '-') {
			getAbsoluteFromRelativePath(tokens[i], &absPath);
			*filePath = absPath;
			i++;
		}
	}

	if (checkIfDirExists(*filePath) || 
		(*filePath == *fakeFilePath && fakeFilePath->empty() == false)){
			*oldFilePath = *filePath;
			chdir((*filePath).c_str());
	} else {
		cout << "cd: " << *filePath << ": No such directory" << endl;
		*filePath = *oldFilePath;
		return -1;
	}

	return 0;
}

/**
 * Checks if a file or directory exists
 * @param path the file/directory to check
 * @return true if exists; false otherwise
 */
bool Shell::checkIfExists(string path) {
	struct stat info;
	bool ret;
	
	if ( stat(path.c_str(), &info) < 0 ) {
		ret = false;
	} else {
		ret = true;
	}
	
	return ret;
}

bool Shell::checkIfDirExists(string path) {
	bool ret = false;
	if (opendir(path.c_str()) != NULL) {
		ret = true;
	}
	return ret;
}

/**
 * Tokenizes a string
 * @param orig the string to be tokenized
 * @param a string containing all the delimeter characters
 * @param tokens a string array in which all the tokens will be stored
 */
void Shell::getTokens(string orig, string delims, string tokens[]) {
	int i;
	int j;
	int lastPos;
	int c;
	string origDup;
	string token;

	origDup = orig;
	j = 0;
	lastPos = 0;
	i = 0;

	while ( ( j = origDup.find_first_of(delims, lastPos) ) != -1 ) {
		token = origDup.substr(lastPos, j-lastPos);
		if (!token.empty()) {
			tokens[i] = token;
			i++;
		}
		lastPos = j + 1;
	}
	token = origDup.substr(lastPos);
	tokens[i] = token;
}

/**
 * The deconstructor
 */
Shell::~Shell() {
	delete fileSystem;
	delete filePath;
	delete oldFilePath;
}
