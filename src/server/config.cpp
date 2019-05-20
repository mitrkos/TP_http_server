#include <fstream>


#include "config.h"
#include "../logger/logger.h"


namespace {

const std::string CPU_LIMIT = "cpu_limit";
const std::string THREAD_LIMIT = "thread_limit";
const std::string DOCUMENT_ROOT = "document_root";

}

void config::parse_config(const std::string& config_path) {
    logger.info("parse config");
    std::ifstream input(config_path);
    if (input) {
        std::string option;
        while (input >> option) {
            if (option == CPU_LIMIT) {
                input >> cpu_limit;
                if (cpu_limit == 0) {
                    throw std::runtime_error("Too low cpu_limit");
                }
            } else if (option == THREAD_LIMIT) {
                input >> thread_limit;
                if (thread_limit == 0) {
                    throw std::runtime_error("Too low cpu_limit");
                }
            } else if (option == DOCUMENT_ROOT) {
                input >> document_root;
            }
            getline(input, option); // pass end of line
        }
    } else {
        logger.error("parse config failed: ");
        throw std::runtime_error("Can't open config file");
    }

    logger.info(fmt::format("config parsed:\ncpu_limit: {}\nthread_limit: {}\ndocument_root: {}",
                            cpu_limit,
                            thread_limit,
                            document_root));
}
