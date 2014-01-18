#include "http.h"

#include <iostream>
#include <sstream>
#include <string>
#include <string.h>

// For send/recv
#include <sys/socket.h>
#include <sys/types.h>

#define BUF_SIZE 1024 * 16

using namespace std;

namespace obsequi {

void HttpRequest::Print() {
  cout << "##### HEADER #######" << endl;
  cout << "Method: " << method_ << endl;
  cout << "URI: " << request_uri_ << endl;
  cout << "Version: " << http_version_ << endl;
  for (auto str : headers_) {
    cout << str << endl;
  }
  cout << content_ << endl;
  cout << "##### END #######" << endl;
}

HttpRequest HttpRequest::parse(int fd) {
  char buffer[BUF_SIZE];
  ssize_t len = recv(fd, buffer, BUF_SIZE - 1, 0);
  buffer[len] = '\0';

  //cout << "START " << len << " ###########################" << endl;
  //cout << string(buffer, len) << endl;
  //cout << "END   ################################" << endl;

  const char* ptr;
  const char* curr = buffer;
  const char* content = NULL;
  vector<string> headers;

  while ((ptr = strstr(curr, "\r\n")) != NULL) {
    if (ptr - buffer > len) {  // Exceeded buffer size.
      cerr << "BUFFER OVERFLOW" << endl;
      return HttpRequest();
    }
    if (ptr - curr == 0) {  // End of header.
      content = ptr + 2;
      if (headers.size() == 0) return HttpRequest();

      size_t l1 = headers[0].find_first_of(' ');
      string method(headers[0], 0, l1);

      size_t l2 = headers[0].find_first_of(' ', l1 + 1);
      string request_uri(headers[0], l1 + 1, l2 - (l1 + 1));

      string http_version(headers[0], l2 + 1);
      headers.erase(headers.begin());

      return HttpRequest(method, request_uri, http_version, headers,
                         content, len - (content - buffer));
    }
    headers.push_back(string(curr, ptr - curr));
    curr = ptr + 2;
  }
  cerr << "END OF BUFFER" << endl;
  return HttpRequest();
}


void HttpResponse::Send(int fd) {
  stringstream buf;

  buf << "HTTP/1.0 200 OK\r\n";
  buf << "Content-type:" << type << "\r\n";
  buf << "Content-Length:" << content.size() << "\r\n";
  buf << "\r\n";
  buf << content.c_str();

  send(fd, buf.str().c_str(), buf.str().size(), 0);
}

}  // namespace obsequi
