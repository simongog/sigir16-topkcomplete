// Copyright (c) 2015 Cesanta Software Limited
// All rights reserved

#include "topkcomp/index.hpp"
#include <iostream>
#include <string>
#include <sstream>

extern "C"
{
#include "mongoose.h"
}

// -- Helper function to escape output
// http://stackoverflow.com/questions/7724448/simple-json-string-escape-for-c/33799784#33799784
std::string escape_json(const std::string &s) {
    std::ostringstream o;
    for (auto c = s.cbegin(); c != s.cend(); c++) {
        if (*c == '"' || *c == '\\' || ('\x00' <= *c && *c <= '\x1f')) {
            o << "\\u"
              << std::hex << std::setw(4) << std::setfill('0') << (int)*c;
        } else {
            o << *c;
        }
    }
    return o.str();
}
// --

using namespace topkcomp;
using namespace sdsl;

typedef INDEX_TYPE t_index;

static std::string s_http_port("8000");
static struct mg_serve_http_opts s_http_server_opts;
static t_index topk_index;

static void ev_handler(struct mg_connection *nc, int ev, void *p) {
  if (ev == MG_EV_HTTP_REQUEST) {
    struct http_message *hm = (struct http_message *) p;
    std::string uri = std::string(hm->uri.p, (hm->uri.p)+(hm->uri.len));

    if ( uri == "/topcomp" ) {
        std::string prefix = "";
        size_t k           = 10;
        char prefix_buf[128];
        int prefix_len = mg_get_http_var(&(hm->query_string), "q", prefix_buf, 128); 
        if ( prefix_len > 0 ) {
            prefix = std::string(prefix_buf, prefix_buf+prefix_len);
        }
        char k_buf[16];
        int k_len = mg_get_http_var(&(hm->query_string), "k", k_buf, 16); 
        if ( k_len > 0 ) {
            k = std::stoull(std::string(k_buf, k_buf+k_len));
        }

        std::string data;
        auto result_list = topk_index.top_k(prefix, k);
        if ( result_list.empty() ){
            data =  "{\"suggestions\":[\"value\":\"\",\"data\":\"\"]}\n";
        } else {
            data =  "{\"suggestions\":[";
            for (size_t i=0; i<result_list.size(); ++i) {
                if (i>0) data += ",";
                data += "{\"value\":\"" + escape_json( result_list[i].first ) + "\",";
                data +=  "\"data\":\""+ std::to_string(i)+ "\"}";
            }
            data += "]}\n";
        }
        /* Send headers */
        mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
        mg_printf_http_chunk(nc, data.c_str(), data.size());
        mg_send_http_chunk(nc,"",0);//send empty chunk, the end of response
    } else {
        mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
    }
  }
}



int main(int argc, char* argv[]){
  const std::string index_name = INDEX_NAME;
  const std::string index_file = std::string(argv[1])+"."+INDEX_NAME+".sdsl";
  std::cout<<"index file="<<index_file<<std::endl;
  
  if ( argc < 2 ) {
      std::cout << "Usage: ./" << argv[0] << " file [port]" << std::endl;
      std::cout << "  file: File for which an index file exists." << std::endl;
      std::cout << "  port: Webserver port. Default 8000." << std::endl;
      return 1;
  }
  if ( argc > 2 ) {
    s_http_port = argv[2];
  }
  
  generate_index_from_file(topk_index, argv[1], index_file, index_name);

  struct mg_mgr mgr;
  struct mg_connection *nc;

  mg_mgr_init(&mgr, NULL);
  nc = mg_bind(&mgr, s_http_port.c_str(), ev_handler);

  // Set up HTTP server parameters
  mg_set_protocol_http_websocket(nc);
  s_http_server_opts.document_root = "../web";
  s_http_server_opts.enable_directory_listing = "no";

  printf("Starting web server on port %s\n", s_http_port.c_str());
  
  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}
