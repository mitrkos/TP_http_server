#pragma once

/*Buffer of line in request*/
#define  LINE_BUFSIZE 256
/*Buffer of response page*/
#define RESPONSE_BUFSIZE 1024
/*Buffer of request page*/
#define REQUEST_BUFSIZE 1024
/*Buffer of DATE header*/
#define TIME_BUFSIZE 1024
/* Port to listen on. */
#define DEFAULT_SERVER_PORT 80
/* Number of worker threads.*/
#define DEFAULT_THREADS_LIMIT 196
/* Number of worker threads.*/
#define DEFAULT_CPU_LIMIT 4

#define DEFAULT_DOCUMENT_ROOT "."

#define CONFIG_PATH "/etc/httpd.conf"