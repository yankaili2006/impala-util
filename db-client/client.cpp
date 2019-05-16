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
	tcp.setup("10.18.0.19", SERVICE_PORT);
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
		string msg = tcp.receive();
    cout << "receive result:" << msg << endl;
    int pos = msg.find("file:");
    if( pos >= 0) {
      msg = msg.substr(pos + 5);
    }
    pos = msg.rfind("filend:");
    if ( pos >= 0) {
      msg = msg.substr(0, pos);
    }

		if (msg != "") {
			cout << "Message:" << msg << endl;
			outf << msg;
			outf.close();
			break;
		}
		sleep(1);
	}
	cout << "to exit" << endl;

	tcp.exit();
	return 0;
}
