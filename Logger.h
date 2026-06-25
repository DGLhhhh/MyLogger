#pragma once

#include <string>
#include <fstream>

namespace yanlong {
    namespace utility {

// 日志宏定义，方便使用
/*
* 原来的写法是：Logger::instance()->log(Logger::DEBUG, __FILE__, __LINE__, "name=%s age=%d", "jack", 18);
* 定义宏后写法：LOG_DEBUG("name=%s age=%d", "jack", 18);   【见main.cpp】
*/
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
            void open(const std::string & filename);
            void close();
			// log函数用于记录日志，参数level表示日志级别，file表示文件名，line表示行号，format表示日志内容的格式化字符串，...表示可变参数列表
            void log(Level level, const char* file, int line, const char* format, ...);

			// max函数用于设置日志文件的最大字节数，当日志文件超过该大小时，会进行日志文件的轮转操作(日志翻滚)
            void setMax(int bytes);

			// level函数用于设置日志级别，只有大于等于该级别的日志才会被记录
            void setLevel(int level);

        private:
            Logger();
            ~Logger();

			// rotate翻滚函数用于进行日志文件的轮转操作，将当前日志文件重命名为备份文件，并创建一个新的日志文件
            void rotate();

        private:
			std::string m_filename;     // 存日志的文件名
			std::ofstream m_fout;       // 日志文件输出流
            int m_max;
			int m_len;                  // 当前日志文件的长度
            int m_level;
			static const char* s_level[LEVEL_COUNT];  // 日志级别字符串数组
			static Logger* m_instance;  // 单例实例指针,全局唯一
        };
    }
}
