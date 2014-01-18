#include <memory>
#include "server.h"
#include "http.h"
#include <iostream>
#include "protoson.h"
#include "obsequi.pb.h"
#include "board.h"
#include "score-board.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
using namespace obsequi;

#define BUF_SIZE 1024 * 1024

void serveIndexHtml(const HttpRequest& req, HttpResponse* resp) {
  int fd = open("html/obsequi.html", O_RDONLY);
  char buffer[BUF_SIZE];
  int size = read(fd, buffer, BUF_SIZE);
  close(fd);
  resp->SetHtmlContent(string(buffer, size));
}

void serveIndexJs(const HttpRequest& req, HttpResponse* resp) {
  int fd = open("html/obsequi.js", O_RDONLY);
  char buffer[BUF_SIZE];
  int size = read(fd, buffer, BUF_SIZE);
  close(fd);
  resp->SetJsContent(string(buffer, size));
}

void processXhr(const HttpRequest& req, HttpResponse* resp) {
  proto::Request ob_req;
  assert(protoson::fill_message(&ob_req, req.content_));
  cout << req.content_ << endl;

  const proto::Board& board = ob_req.board();
  Board b(board.rows(), board.cols());
  for (int i = 0; i < board.blocks_size(); i++) {
    b.SetBlock(board.blocks(i).row(), board.blocks(i).col());
  }
  b.Print();
  b.PrintInfo();
 
  proto::Response ob_resp;
  int score;
  obsequi::is_game_over_expensive(b, *b.GetOpponent(), &score);
  ob_resp.set_score(score);

  proto::ScoreMetrics* metrics;
  int min_moves, max_opp_moves, max_opp_moves_real;

  metrics = ob_resp.mutable_h_metrics();
  obsequi::score_board_get_metrics(b, metrics);
  if (board.whos_turn() == proto::HORIZONTAL) {
    obsequi::score_board_curr_player(b,
        &min_moves, &max_opp_moves, &max_opp_moves_real);
  } else {
    obsequi::score_board_next_player(b,
        &min_moves, &max_opp_moves, &max_opp_moves_real);
  }
  metrics->set_moves_lower_bound(min_moves);
  metrics->set_opp_moves_upper_bound(max_opp_moves);
  metrics->set_opp_moves_upper_bound2(max_opp_moves_real);

  metrics = ob_resp.mutable_v_metrics();
  obsequi::score_board_get_metrics(*b.GetOpponent(), metrics);
  if (board.whos_turn() == proto::HORIZONTAL) {
    obsequi::score_board_next_player(*b.GetOpponent(),
        &min_moves, &max_opp_moves, &max_opp_moves_real);
  } else {
    obsequi::score_board_curr_player(*b.GetOpponent(),
        &min_moves, &max_opp_moves, &max_opp_moves_real);
  }
  metrics->set_moves_lower_bound(min_moves);
  metrics->set_opp_moves_upper_bound(max_opp_moves);
  metrics->set_opp_moves_upper_bound2(max_opp_moves_real);

  cout << protoson::get_json(ob_resp) << endl;
  resp->SetJsonContent(protoson::get_json(ob_resp));
}


//The function we want to make the thread run.
void handle(const TcpConnection& conn) {
  HttpRequest req = HttpRequest::parse(conn.fd);
  HttpResponse resp;
  cout << "## " << req.method_ << " " << req.request_uri_ << endl;
  //req.Print();
  //cout << req.content_ << endl;

  if (req.method_ == "GET" && req.request_uri_ == "/") {
    serveIndexHtml(req, &resp);
  } else if (req.method_ == "GET" && req.request_uri_ == "/obsequi.js") {
    serveIndexJs(req, &resp);
  } else if (req.method_ == "POST" && req.request_uri_ == "/xhr") {
    processXhr(req, &resp);
  }
  resp.Send(conn.fd);
}

int main() {
  TcpServer server(handle);
  server.Run(8000, 2);
}
