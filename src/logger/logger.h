#pragma once


#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"


class logger {
public:
    static logger& instance() {
        static logger l;
        return l;
    }

    void off() {
        is_on = false;
    }

    logger(logger const&) = delete;
    logger(logger&&) = delete;
    void operator=(logger const&) = delete;
    void operator=(logger&&) = delete;

    template <typename T>
    void info(const T& message) {
        if (is_on) console->info(message);
    }
    template <typename T>
    void warn(const T& message) {
        if (is_on) console->warn(message);
    }
    template <typename T>
    void error(const T& message) {
        if (is_on) console->error(message);
    }
    template <typename T>
    void critical(const T& message) {
        if (is_on) console->critical(message);
    }
    template <typename T>
    void debug(const T& message) {
        if (is_on) console->debug(message);
    }

private:
    logger() {
        auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        spdlog::init_thread_pool(8192, 1);
        console = std::make_shared<spdlog::async_logger>("async_logger", sink, spdlog::thread_pool(),
                                                         spdlog::async_overflow_policy::block);
        console->set_pattern("[%H:%M:%f] [%^%l%$] [thread %t] %v");
        console->set_level(spdlog::level::debug);

    }

    std::shared_ptr<spdlog::logger> console;
    bool is_on = true;
};


static logger& logger = logger::instance();
