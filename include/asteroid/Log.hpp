#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#ifndef NDEBUG

#define AST_TRACE SPDLOG_TRACE
#define AST_DEBUG SPDLOG_DEBUG
#define AST_INFO SPDLOG_INFO
#define AST_WARN SPDLOG_WARN
#define AST_ERROR SPDLOG_ERROR
#define SDL_ERROR() SPDLOG_ERROR("{}", SDL_GetError())

#else

#define AST_TRACE(...) (void)0
#define AST_DEBUG(...) (void)0
#define AST_INFO(...) (void)0
#define AST_WARN(...) (void)0
#define AST_ERROR(...) (void)0
#define SDL_ERROR() (void)0

#endif