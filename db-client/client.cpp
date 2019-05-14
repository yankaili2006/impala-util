#include <iostream>
#include <fstream>
#include <signal.h>
#include "TCPClient.h"
#include "unistd.h"

TCPClient tcp;

void sig_exit(int s) {
	tcp.exit();
	exit(0);
}


int main(int argc, char *argv[]) {
	signal(SIGINT, sig_exit);

	string infname = "test.txt";
	int o;
	const char *optstring = "f:";
	while((o = getopt(argc, argv, optstring)) != -1) {
		switch (o) {
			case 'f':
				cout << "opt is f, filename:" << optarg << endl;
				infname = optarg;
				break;
			case '?':
				cout << "Usage:" << endl;
				cout << argv[0] << " -f <filename>" << endl;	
		}
	}
	ifstream in(infname);
	if (! in.is_open()) {
		cout << "error open file:" << infname << endl;
		exit(0);
	}

	string line;
	tcp.setup(, SERVICE_PORT);
	tcp.Send("file:");
	while(!in.eof()) {
		getline(in, line);
		if(line.empty()) {
			continue;
		}
		tcp.Send(line+"\n");
	}

	in.close();
	tcp.Send("filend:");
	cout << "send over" << endl;

	string outfname = "out_" + infname;
	ofstream outf(outfname);
	if(!outf) {
		cout << "error out file :" << outfname << endl;
	}

	bool isend = false;
	while(1) {
		string str = tcp.receive();
		if(str.find("file:") == 0) {
			str = str.substr(5);
			cout << "receive file" << endl;
		}

		if (str.rfind("filend:") == (str.length() - 7) ) {
			str = str.substr(0, str.length() - 7);
			isend = true;
			cout << "receive end" << endl;
		}

		if (str != "") {
			cout << "Message:" << str << endl;
			outf << str;
		}
		else {
			// cout << "Message not expected:" << str << endl;
		}

		if(isend) {
			outf.close();
			break;
		}
		sleep(1);
	}

	cout << "to exit" << endl;

	tcp.exit();
	return 0;
}
