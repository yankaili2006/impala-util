#include <iostream>
#include <fstream>
#include "TCPServer.h"

TCPServer tcp;
pthread_t msg_t;


int save_file(string &msg, string &receive_file) {
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
    recf.open(receive_file);
    recf << msg;
    recf.close();
  }
  return 0;
}

int run_cmd(string &privpy_cmd) {
  cout << "run_cmd:" << privpy_cmd << endl;
  FILE *fp = NULL;

  fp = popen(privpy_cmd.c_str(), "r");
  if(!fp) {
    perror("popen error");
    return -1;
  }
  char   buf[1024];
  memset(buf, '\0', sizeof(buf) );
  fgets(buf, sizeof(buf), fp);
  cout << "run_cmd out:" << buf << endl;
  pclose(fp);
  return 0;
}

int send_result(string &result_file) {
  cout << "start reply:" << endl;
  ifstream replyf;
  replyf.open(result_file);
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

    char szBuf[256] = {0};
    time_t timer = time(NULL);
    strftime(szBuf, sizeof(szBuf), "%Y%m%d%H%M%S", localtime(&timer));
    string receive_file = "receive_" + string(szBuf) +".txt";
    string replay_file = "reply_" + string(szBuf) +".txt";
    string privpy_cmd = "/root/privpy/bazel-bin/client -code_file="+receive_file+" -impala=true >"+ replay_file;
    cout << "receive_file:" << receive_file << ", replay_file:" << replay_file << ", privpy_cmd:" << privpy_cmd << endl;
    if (!tcp.recv_q.empty()){
      msg = tcp.recv_q.front();
      cout << "raw message:" << msg << endl;
      tcp.recv_q.pop();

      if(save_file(msg, receive_file) !=0) {
        cout << "save file error" <<endl;
        continue;
      }
      if(run_cmd(privpy_cmd) != 0) {
        cout << "run_cmd error" << endl;
        continue;
      }
      if(send_result(replay_file)){
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
