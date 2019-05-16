#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;

#define MAXPACKETSIZE 4096

class TCPServer {
  public:
		int sockfd, newsockfd, n, pid;
		struct sockaddr_in serverAddress;
		struct sockaddr_in clientAddress;
		pthread_t serverThread;
		char msg[MAXPACKETSIZE];
		static string Message;
		static bool isrunning;
    static queue<string> recv_q;
    static mutex m;
    static condition_variable cond_var;

		void setup(int port);
		string receive();
		string getMessage();
		bool isRunning();
		void Send(string msg);
		void detach();
		void clean();

	private:
    static void * Task(void * argv);
};

#endif
