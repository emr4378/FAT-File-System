/**
 * The main file class. Contains a main method (no way!)
 * Just gets everything going.
 *
 * @author: Eduardo Rodrigues - emr4378
 */

#include <iostream>
#include <stdio.h>
using namespace std;

#include "Shell.h"

int main( int argc, char ** argv) {
	Shell *shell;
	if (argc == 1) {
		//no filesystem given
		shell = new Shell(NULL);
		shell->prompt();
	} else if (argc == 2) {
		//filesystem given
		shell = new Shell(argv[1]);
		shell->prompt();
	} else {
		cout << "usage: os1shell [file-system-name]\n";
	}
	delete shell;
	return 0;
}
