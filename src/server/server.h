#pragma once


#include "../thread_pool/thread_pool.h"
#include "config.h"
#include "http_head.h"
#include "libevent/event2/util.h"

class server {
public:
    explicit server(const config& config_);
    ~server();

    server(const server&) = delete;
    server(server&&) = delete;
    server& operator=(const server&) = delete;
    server& operator=(server&&) = delete;

    void create_connection();
    void run();

private:
    friend void on_accept(struct evconnlistener *listener,
            evutil_socket_t fd, struct sockaddr *,
                    int, void *arg);

    struct config config;
    struct event_base* evbase_accept;
    struct evconnlistener* evlistener;
    thread_pool::thread_pool thread_pool;
};
