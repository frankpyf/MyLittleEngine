#pragma once
//*******copied from Hazel/Core/Log, modified***********

#include "glm/gtx/string_cast.hpp"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include "spdlog/spdlog.h"
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace engine {

	class Log
	{
	public:
		static void Init();

		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
	return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
{
	return os << glm::to_string(quaternion);
}

// Core log macros
#define MLE_CORE_TRACE(...)    ::engine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define MLE_CORE_INFO(...)     ::engine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define MLE_CORE_WARN(...)     ::engine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define MLE_CORE_ERROR(...)    ::engine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define MLE_CORE_CRITICAL(...) ::engine::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define MLE_TRACE(...)         ::engine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define MLE_INFO(...)          ::engine::Log::GetClientLogger()->info(__VA_ARGS__)
#define MLE_WARN(...)          ::engine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define MLE_ERROR(...)         ::engine::Log::GetClientLogger()->error(__VA_ARGS__)
#define MLE_CRITICAL(...)      ::engine::Log::GetClientLogger()->critical(__VA_ARGS__)
