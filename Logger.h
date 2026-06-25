#pragma once

#pragma once
#include <string>
#include <fstream>
#include <mutex>          // 添加互斥锁头文件

namespace yanlong {
    namespace utility {

        // 日志宏定义
#define LOG_DEBUG(format, ...) \
    Logger::instance()->log(Logger::DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_INFO(format, ...) \
    Logger::instance()->log(Logger::INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_WARN(format, ...) \
    Logger::instance()->log(Logger::WARN, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) \
    Logger::instance()->log(Logger::ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_FATAL(format, ...) \
    Logger::instance()->log(Logger::FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)

        class Logger
        {
        public:
            enum Level
            {
                DEBUG = 0,
                INFO,
                WARN,
                ERROR,
                FATAL,
                LEVEL_COUNT
            };

            static Logger* instance();
            void open(const std::string& filename);
            void close();
            void log(Level level, const char* file, int line, const char* format, ...);
            void setMax(int bytes);
            void setLevel(int level);

        private:
            Logger();
            ~Logger();
            Logger(const Logger&) = delete;
            Logger& operator=(const Logger&) = delete;

            void rotate();

        private:
            std::string m_filename;
            std::ofstream m_fout;
            int m_max;
            int m_len;
            int m_level;

            // 添加互斥锁，用于多线程安全
            std::mutex m_mutex;

            static const char* s_level[LEVEL_COUNT];
            static Logger* m_instance;
        };
    }
}
