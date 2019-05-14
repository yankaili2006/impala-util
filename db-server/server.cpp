#include <iostream>
#include <fstream>
#include "TCPServer.h"

TCPServer tcp;

void * loop(void * m) {

  pthread_detach(pthread_self());
  ofstream recf;
  ifstream replyf;

  while (1) {
    bool isend = false;
    while(tcp.isRunning()) {
      string str = tcp.getMessage();

      if(str.find("file:") == 0) {
				recf.open(RECEICE_FILE_NAME);
				str = str.substr(5);
				tcp.clean();
				
				cout << "receive file" << endl;
			}

			if (str.rfind("filend:") == (str.length() - 7) ) {
				str = str.substr(0, str.length() - 7);
				tcp.clean();
				isend = true;
				cout << "receive end" << endl;
			}

			if (str != "") {
				cout << "Message:" << str << endl;
				recf << str;
				tcp.clean();
			}

			if(isend) {
				recf.close();
				break;
			}
			sleep(1);
		}

		if (tcp.isRunning()) {
			FILE *fp = NULL;
			char   buf[1024];
			memset(buf, '\0', sizeof(buf) );

			fp = popen(PRIVPY_CLIENT_CMD, "r");
			if(!fp) {
				perror("popen");
				exit(1);
			}
			fgets(buf, sizeof(buf), fp);
			printf("the popen out:%s\n", buf);
			pclose(fp);
		}

		if(tcp.isRunning()) {
			string line;
			replyf.open(REPLY_FILE_NAME);
			tcp.Send("file:");
			while(!replyf.eof()) {
				getline(replyf, line);
				if(line.empty()) {
					continue;
				}
				cout << "to reply:" << line << endl;
				tcp.Send(line + "\n");
				tcp.clean();
			}
			tcp.Send("filend:");
			replyf.close();
		}
		usleep(1000);
	}
}

int main()
{
	pthread_t msg;
	tcp.setup(SERVICE_PORT);
	if( pthread_create(&msg, NULL, loop, (void *)0) == 0) {
		tcp.receive();
	}

	return 0;
}
