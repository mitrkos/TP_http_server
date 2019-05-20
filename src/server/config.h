#pragma once


#include <stdint.h>
#include <string>
#include "default_consts.h"


struct config {
    int port{DEFAULT_SERVER_PORT};
    size_t cpu_limit{DEFAULT_CPU_LIMIT};
    size_t thread_limit{DEFAULT_THREADS_LIMIT};
    std::string document_root{DEFAULT_DOCUMENT_ROOT};

    void parse_config(const std::string& config_path = CONFIG_PATH);
};
