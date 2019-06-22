#pragma once

#include <string>

/*Buffer of line in request*/
const size_t LINE_BUFSIZE = 256;
/*Buffer of response page*/
const size_t RESPONSE_BUFSIZE = 1024;
/*Buffer of request page*/
const size_t REQUEST_BUFSIZE = 1024;
/*Buffer of DATE header*/
const size_t TIME_BUFSIZE = 1024;
/* Port to listen on. */
const size_t DEFAULT_SERVER_PORT = 80;
/* Number of worker threads.*/
const size_t DEFAULT_THREADS_LIMIT = 196;
/* Number of worker threads.*/
const size_t DEFAULT_CPU_LIMIT = 4;

const std::string DEFAULT_DOCUMENT_ROOT = ".";

const std::string CONFIG_PATH = "/etc/httpd.conf";