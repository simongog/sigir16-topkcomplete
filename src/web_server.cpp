// Copyright (c) 2015 Cesanta Software Limited
// All rights reserved

#include "topkcomp/index.hpp"
#include <iostream>
#include <string>

extern "C"
{
#include "mongoose.h"
}

using namespace topkcomp;

typedef INDEX_TYPE t_index;

static std::string s_http_port("8000");
static struct mg_serve_http_opts s_http_server_opts;
static t_index topk_index;

static void ev_handler(struct mg_connection *nc, int ev, void *p) {
  if (ev == MG_EV_HTTP_REQUEST) {
    struct http_message *hm = (struct http_message *) p;
    std::string uri = std::string(hm->uri.p, (hm->uri.p)+(hm->uri.len));
    std::string query_string = std::string(hm->query_string.p, (hm->query_string.p)+(hm->query_string.len));
    if ( uri == "/topcomp" and query_string.substr(0, 6) == "query=" ) {
        std::string query = query_string.substr(6);
        std::string data;
        auto result_list = topk_index.top_k(query, 10);
        if ( result_list.empty() ){
            data =  "{\"suggestions\":[\"value\":\"\",\"data\":\"\"]}\n";
        } else {
            data =  "{\"suggestions\":[";
            for (size_t i=0; i<result_list.size(); ++i) {
                if (i>0) data += ",";
                data += "{\"value\":\"" + result_list[i].first + "\",";
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
  }
  if ( argc > 2 ) {
    s_http_port = argv[2];
  }
  
  if ( sdsl::load_from_file(topk_index, index_file) ) {
      std::cout<<"Index load from file " << index_file << std::endl;
  } else {
    // TODO: generate index, if it does not yet exist
    std::cerr<<"Could not load index from file " << index_file << std::endl;
    return 1;
  }

  struct mg_mgr mgr;
  struct mg_connection *nc;

  mg_mgr_init(&mgr, NULL);
  nc = mg_bind(&mgr, s_http_port.c_str(), ev_handler);

  // Set up HTTP server parameters
  mg_set_protocol_http_websocket(nc);
  s_http_server_opts.document_root = "../web";      // Serve current directory
  s_http_server_opts.enable_directory_listing = "yes";

  printf("Starting web server on port %s\n", s_http_port.c_str());
  
  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}
