#include <err.h>
#include <sys/types.h>

#include "default_consts.h"
#include "http_head.h"
#include "responce_parse.h"
#include "libevent/event2/bufferevent.h"
#include "libevent/event2/buffer.h"
#include "libevent/event2/event.h"
#include "libevent/event2/listener.h"
#include "libevent/evrpc.h"
#include "server.h"
#include "../logger/logger.h"


namespace {

std::string root_dir;

struct client_connection {
    struct event_base* evbase = nullptr;
    struct bufferevent* buf_ev = nullptr;
    struct evbuffer* output_buffer = nullptr;

    ~client_connection() {
        logger.info("Close client");
        if (evbase != nullptr) {
            event_base_free(evbase);
            evbase = nullptr;
        }
        if (output_buffer != nullptr) {
            evbuffer_free(output_buffer);
            output_buffer = nullptr;
        }
    }
};


void buffered_on_read(struct bufferevent* bev, void* arg) {
    logger.info("start buffer_event reading");

    auto* client = (client_connection*)arg;
    http_t http_request;
    int response_id;

    char response[RESPONSE_BUFSIZE];
    logger.info("http parsing");

    response_id = http_parse(&http_request, bufferevent_get_input(bev), root_dir.c_str());
    switch (response_id) {
        case NOT_ALLOWED_HTTP_METHOD :
            create_response("405", "Not Implemented", response);
            logger.info("405 ERROR");
            break;
        case FILE_NOT_EXIST :
            create_response("404", "Not found", response);
            logger.info("404 ERROR");
            break;
        case FILE_IS_EXECUTABLE :
            create_response("500", "Internal server error", response);
            logger.info("500 ERROR");
            break;
        case ALL_OK :
            create_response("200", "OK", response);
            logger.info("200 OK");
            break;
        case ESCAPING_ROOT :
            logger.info("403 ERROR");
            create_response("403", "Forbidden", response);
            break;
        case PARSE_ERROR :
            logger.info("PARSE ERROR");
            create_response("400", "Bad request", response);
            break;
        case INDEX_FILE_NOT_EXIST:
            logger.info("INDEX ERROR");
            create_response("403", "Forbidden", response);
            break;
        default:
            logger.info("DEFAULT 500 ERROR");
            create_response("500", "Internal server error", response);
            break;
    }

    if (response_id == ALL_OK) {
        logger.info("Here");
        sprintf(response + strlen(response), "Content-Length: %lu\r\n", http_request.filesize);
        sprintf(response + strlen(response), "Content-Type: %s\r\n\r\n", http_request.filetype);
        logger.info(http_request.filesize);
    }
    logger.info(fmt::format("response:\n{}", std::string(response)));

    if (evbuffer_add(client->output_buffer, response, strlen(response)) != 0) {
        logger.info("Error in client buffer filling");
    }
    logger.info("client buffer filled");
    if (strcmp(http_request.method, "GET") == 0 && response_id == ALL_OK) {
        int fd = open(http_request.filename, O_RDONLY, 0);
        if (evbuffer_add_file(client->output_buffer, fd, 0, static_cast<int64_t>(http_request.filesize)) != 0) {
            logger.error("File can't be added in answer");
        }
        logger.info("File is added in answer");
    }
    if(strcmp(http_request.method, "HEAD") == 0){
        logger.info(fmt::format("HEAD request{}", response));
    }
    bufferevent_disable(bev, EV_READ);
    bufferevent_enable(bev, EV_WRITE);
    /* Send the results to the client.  This actually only queues the results for sending.
     * Sending will occur asynchronously, handled by libevent. */

    if (bufferevent_write_buffer(bev, client->output_buffer) != 0) {
        logger.info("bufferevent error");
    }

    //struct timeval delay = { 1e };
}

void buffered_on_write(struct bufferevent* bev, void* arg) {
    logger.info("Answer has written");
    bufferevent_free(bev);
}

void eventcb (struct bufferevent* buf_ev, short events, void* arg) {
    if(events & BEV_EVENT_ERROR)
        logger.error( "Error with object bufferevent" );
    if(events & (BEV_EVENT_EOF | BEV_EVENT_ERROR))
        bufferevent_free( buf_ev );
}

void server_job_function(const std::shared_ptr<struct client_connection>& client) {
    logger.info("start task client");
    event_base_dispatch(client->evbase);
    logger.info("end task client");
}

void on_accept(struct evconnlistener *listener,
               evutil_socket_t client_fd, struct sockaddr */*client_addr*/,
               int /*addr_len*/,
               void *arg) {
    logger.info("On accept start");

    auto client = std::make_shared<struct client_connection>();

    if ((client->output_buffer = evbuffer_new()) == nullptr) {
        logger.error("Can't allocate client out buffer");
        throw std::runtime_error("Can't allocate client out buffer");
    }

    if ((client->evbase = event_base_new()) == nullptr) {
        logger.error("Can't create client event_base");
        throw std::runtime_error("Can't create client event_base");
    }

    if (evutil_make_socket_nonblocking(client_fd) != 0) {
        logger.error("Can't make client socket non_blocking");
        throw std::runtime_error("Can't make client socket non_blocking");
    }
    client->buf_ev = bufferevent_socket_new(client->evbase, client_fd,
                                            BEV_OPT_CLOSE_ON_FREE);
    if (client->buf_ev == nullptr) {
        logger.error("Can't create client buffer_event");
        throw std::runtime_error("Can't create client buffer_event");
    }

    bufferevent_setcb(client->buf_ev, buffered_on_read, buffered_on_write,
                      eventcb, client.get());

    if (bufferevent_enable(client->buf_ev, EV_READ)) {
        logger.error("Can't enable client buffer_event");
        throw std::runtime_error("Can't enable client buffer_event");
    }

    auto* tp = (thread_pool::thread_pool*)arg;

    tp->submit(std::bind(server_job_function, client));

    logger.info("Task added");
}

}

server::server(const struct config& config_):
    config(config_),
    evbase_accept(nullptr),
    evlistener(nullptr),
    thread_pool(config_.thread_limit - 1) {
    root_dir = config_.document_root;
    logger.info("Server created");
}

server::~server() {
    logger.info("Stopping socket listener event loop");
    if (event_base_loopexit(evbase_accept, nullptr) != 0) {
        logger.error("Error shutting down server");
    }

    evconnlistener_free(evlistener);
    evlistener = nullptr;
    event_base_free(evbase_accept);
    evbase_accept = nullptr;

    logger.info("Server shutdown");
}

void server::create_connection() {
    struct sockaddr_in sin;

    evbase_accept = event_base_new();
    if (!evbase_accept) {
        logger.error("Error creating event_base");
        throw std::runtime_error("Can't create event_base");
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(config.port);

    evlistener = evconnlistener_new_bind(evbase_accept, on_accept,
                                         (void *)&thread_pool,
            (LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE),
            -1, (struct sockaddr *)&sin, sizeof(sin));
    if (!evlistener) {
        logger.error("Error creating evconnlistener");
        throw std::runtime_error("Can't create evconnlistener");
    }

    logger.info("Connection created");
    //evconnlistener_set_error_cb( listener, accept_error_cb );
}

void server::run() {
    logger.info("Server running");

    if (!evbase_accept) {
        logger.error("Server isn't connected");
        throw std::runtime_error("Error server connection");
    }

    event_base_dispatch(evbase_accept);

    logger.info("Server stopped");
}
