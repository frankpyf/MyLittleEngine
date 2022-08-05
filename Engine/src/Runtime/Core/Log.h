#pragma once
#include "spdlog/spdlog.h"

namespace Engine {
    class Log {
    public:
        static void Init();

        static std::shared_ptr<spdlog::logger> GetRuntimeLogger() { return runtime_logger_; }
        static std::shared_ptr<spdlog::logger> GetClientLogger() { return client_logger_; }
    private:
        static std::shared_ptr<spdlog::logger> runtime_logger_;
        static std::shared_ptr<spdlog::logger> client_logger_;
    };
}

#define RUNTIME_ERROR(...) ::Engine::Log::GetRuntimeLogger()->error(__VA_ARGS__)