// Copyright (c) 2015 Cesanta Software Limited
// All rights reserved

#include <iostream>

extern "C"
{
#include "mongoose.h"
}
#include <string>

static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;

static void ev_handler(struct mg_connection *nc, int ev, void *p) {
  if (ev == MG_EV_HTTP_REQUEST) {
    struct http_message *hm = (struct http_message *) p;
    std::string uri = std::string(hm->uri.p, (hm->uri.p)+(hm->uri.len));
    std::string query_string = std::string(hm->query_string.p, (hm->query_string.p)+(hm->query_string.len));
    std::cout<<"URI: "<<uri<<std::endl;
    std::cout<<"QUERY_STRING: "<<query_string<<std::endl;
    if ( uri == "/query" and query_string.substr(0, 6) == "query=" ) {
        std::string query = query_string.substr(7);
        std::cout<<"TODO: handle "<<query << std::endl;
        std::string data =  "{\"suggestions\":[{\"value\":\"Andorra\",\"data\":\"AD\"},{\"value\":\"Alaska\",\"data\":\"AS\"}]}\n";
        /* Send headers */
          mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
          mg_printf_http_chunk(nc, data.c_str(), data.size());
          mg_send_http_chunk(nc,"",0);//send empty chunk, the end of response
    } else {
        mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
    }
  }
}

int main(void) {
  struct mg_mgr mgr;
  struct mg_connection *nc;

  mg_mgr_init(&mgr, NULL);
  nc = mg_bind(&mgr, s_http_port, ev_handler);

  // Set up HTTP server parameters
  mg_set_protocol_http_websocket(nc);
  s_http_server_opts.document_root = "../web";      // Serve current directory
  s_http_server_opts.enable_directory_listing = "yes";

  printf("Starting web server on port %s\n", s_http_port);
  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return 0;
}
