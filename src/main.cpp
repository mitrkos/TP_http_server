#include "server/server.h"
#include "logger/logger.h"


int main() {
    logger.off();
    config server_config;
    server_config.parse_config();
    server file_server(server_config);
    file_server.create_connection();
    file_server.run();

    return 0;
}
