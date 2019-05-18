#include <iostream>
#include <fstream>
#include "TCPServer.h"

TCPServer tcp;
pthread_t msg_t;

int save_file(string &msg) {
  int pos = msg.find("file:");
  if( pos >= 0) {
    msg = msg.substr(pos + 5);
  }
  pos = msg.rfind("filend:");
  if ( pos >= 0) {
    msg = msg.substr(0, pos);
  }

  if (msg != "") {
    ofstream recf;
    cout << "raw file Message:" << msg << endl;
    recf.open(RECEICE_FILE_NAME);
    recf << msg;
    recf.close();
  }
  return 0;
}

int run_cmd() {
  cout << "run_cmd:" << PRIVPY_CLIENT_CMD << endl;
  FILE *fp = NULL;

  fp = popen(PRIVPY_CLIENT_CMD, "r");
  if(!fp) {
    perror("popen error");
    return -1;
  }
  char   buf[1024];
  memset(buf, '\0', sizeof(buf) );
  fgets(buf, sizeof(buf), fp);
  if (strlen(buf) == 0) {
    cout << "execute command failed" << endl;
    pclose(fp);
    return -1;
  }
  cout << "run_cmd out:" << buf << endl;
  pclose(fp);
  return 0;
}

int send_result() {
  cout << "start reply:" << endl;
  ifstream replyf;
  replyf.open(REPLY_FILE_NAME);
  string result = "file:";

  string line;
  while(!replyf.eof()) {
    getline(replyf, line);
    if(line.empty()) {
      continue;
    }
    result += line + "\n";
  }
  result += "filend:";
  tcp.Send(result);
  cout << "reply over:" << result << endl;
  replyf.close();
  return 0;
}

void* loop(void * m) {
  pthread_detach(pthread_self());
  pthread_t process_t;
  while (1) {
      {
      	std::unique_lock<std::mutex> lock(tcp.m);
      	tcp.cond_var.wait(lock, [=] { return !tcp.recv_q.empty();});
      }
      string msg = "";
      if (!tcp.recv_q.empty()){
        msg = tcp.recv_q.front();
        cout << "raw message:" << msg << endl;
        tcp.recv_q.pop();

        if(save_file(msg) !=0) {
          cout << "save file error" <<endl;
          continue;
        }
        if(run_cmd() != 0) {
          cout << "run_cmd error" << endl;
          continue;
        }
        if(send_result()){
          cout << "send_result error" <<endl;
          continue;
        }
      }
    sleep(1);
  }
}

void sig_exit(int s) {
  cout << "signal:" << s << endl;
  pthread_kill(msg_t, SIGKILL);
  tcp.detach();
  exit(0);
}

int main()
{
  signal(SIGINT, sig_exit);
  signal(SIGTERM, sig_exit);
  signal(SIGHUP, sig_exit);

	tcp.setup(SERVICE_PORT);
	if(pthread_create(&msg_t, NULL, loop, (void *)0) == 0) {
		tcp.receive();
	}

	return 0;
}
