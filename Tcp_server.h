#ifndef __Tcp_server_h__
#define __Tcp_server_h__

#include <uv.h>
#include <cstdlib>
#include <cstring>
#include "Common.h"

typedef std::string (*request_cb)(std::string content);

class Tcp_server
{
    uv_loop_t *uv_loop = nullptr;
    request_cb request;

public:    
    int run(short port_no, request_cb request);

private:
    static void uv_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf); 
    static void uv_after_write(uv_write_t *req, int status);
    static void uv_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
    static void on_new_connection(uv_stream_t *server, int status);
};

int Tcp_server::run(short port_no, request_cb request)
{
    uv_loop = uv_default_loop();
    uv_tcp_t server;
    uv_tcp_init(uv_loop, &server);

    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", port_no, &addr);

    server.data = (void*)request;

    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
    int r = uv_listen((uv_stream_t*)&server, 128, on_new_connection);
    if (r) {
        log("Listen error ", r);
        return 1;
    }

    return uv_run(uv_loop, UV_RUN_DEFAULT);
}


void Tcp_server::uv_alloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) 
{
    buf->base = (char*)malloc(suggested_size);
    buf->len = suggested_size;
}

void Tcp_server::uv_after_write(uv_write_t *req, int status) 
{
    if (status) {
        log( "Write error", status);
    }
    
    free(req);
}

void Tcp_server::uv_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) 
{
    if (nread < 0) 
    {
        if (nread != UV_EOF) 
        {
            log( "Read error", nread);
            uv_close((uv_handle_t*) client, NULL);
        }
    } 
    else if (nread > 0) 
    {
        
        std::string content;
        if(client->data)
        {
            request_cb request = (request_cb)client->data;
            std::string response = request(content);
            if(!response.empty())
            {
                uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
                uv_buf_t wrbuf = uv_buf_init(buf->base, nread);
                uv_write(req, client, &wrbuf, 1, uv_after_write);
            }
        }

        
    }

    if (buf->base) 
        free(buf->base);
}

void Tcp_server::on_new_connection(uv_stream_t *server, int status) 
{
    if (status < 0) {
        log("New connection error", status);
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));

    uv_tcp_init(uv_loop, client);
    client->data = server->data;
    
    if (uv_accept(server, (uv_stream_t*) client) == 0) 
    {
        log("Accepted new connection");
        uv_read_start((uv_stream_t*)client, uv_alloc, uv_read);
    } 
    else 
        uv_close((uv_handle_t*) client, NULL);
}


#endif
