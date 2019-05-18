#include "TCPServer.h" 

string TCPServer::Message;
bool TCPServer::isrunning;
queue<string> TCPServer::recv_q;
mutex TCPServer::m;
condition_variable TCPServer::cond_var;

void* TCPServer::Task(void *arg) {
  int n = 0, read_size = 0;
  int newsockfd = (long)arg;
  char msg[MAXPACKETSIZE];
  pthread_detach(pthread_self());

  while(1) {
    n = recv(newsockfd, msg + read_size, MAXPACKETSIZE, 0);
    if(n == 0) {
      break;
    }
    cout << "tcp recv:" << msg + read_size << endl;
    read_size += n;

    if (strstr(msg, "filend:") != nullptr) {
      msg[read_size] = 0;

      std::unique_lock<std::mutex>lock(m);
//      cout << "tcp receive script file:" << msg << endl;
      recv_q.push(string(msg));
      cond_var.notify_one();

      memset(msg, 0x00, sizeof(msg));
      read_size = 0;
    }
  }
  return 0;
}

void TCPServer::setup(int port) {
  sockfd=socket(AF_INET,SOCK_STREAM,0);
  memset(&serverAddress,0,sizeof(serverAddress));
  serverAddress.sin_family=AF_INET;
  serverAddress.sin_addr.s_addr=htonl(INADDR_ANY);
  serverAddress.sin_port=htons(port);
  cout << "server port:" << port << endl;
  bind(sockfd,(struct sockaddr *)&serverAddress, sizeof(serverAddress));
  listen(sockfd,5);
}

string TCPServer::receive() {
  string str;
  while(1) {
    socklen_t sosize  = sizeof(clientAddress);
    newsockfd = accept(sockfd,(struct sockaddr*)&clientAddress,&sosize);
    str = inet_ntoa(clientAddress.sin_addr);
    pthread_create(&serverThread, NULL, &Task, (void *)newsockfd);
  }
}

void TCPServer::Send(string msg) {
  send(newsockfd, msg.c_str(), msg.length(), 0);
  cout << "send over:" << msg << ", msg.length:" << msg.length() << endl;
  if (newsockfd > 0) {
    shutdown(newsockfd, 2);
    cout << "shutdown fd:" << newsockfd << endl;
  }
}

void TCPServer::detach() {
  pthread_kill(serverThread, SIGKILL);
  if (sockfd >= 0)
    shutdown(sockfd, 2);
//  if (newsockfd >= 0)
//    shutdown(newsockfd);
}

