/*
* 存在问题：（1）多线程不安全，多个线程同时调用 log() 方法可能会导致日志内容混乱。
*               ===> 解决方案：可以使用互斥锁（mutex）来保护 log() 方法，确保同一时间只有一个线程可以写入日志。
*           （2）使用了snprintf和vsnprintf来计算日志内容的长度，这可能会导致性能问题，尤其是在高频率的日志记录场景下。
*               ===> 解决方案：可以使用固定大小的缓冲区来存储日志内容，避免频繁的内存分配和释放。
*                               优先使用 C++ 的 std::string 和 RAII，避免手动内存管理。
*           （3）localtime 是非线程安全的标准 C 函数。strerror(errno) 同样内部使用全局静态字符串缓冲区，多线程调用会发生字符串覆盖，线程不安全，VS 标记废弃
*               ===> 解决方案：替换为Windows编译器支持的安全函数 localtime_s，替换为安全函数 strerror_s 
* 		    （4）还可以完善相关日志系统，添加删除旧日志文件等等，异步日志等等。
* 
*/
#include "Logger.h"
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdexcept>
using namespace yanlong::utility;

const char* Logger::s_level[LEVEL_COUNT] =
{
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

Logger* Logger::m_instance = NULL;

Logger::Logger() : m_max(0), m_len(0), m_level(DEBUG)
{
}

Logger::~Logger()
{
    close();
}

Logger* Logger::instance()
{
    if (m_instance == NULL)
        m_instance = new Logger();
    return m_instance;
}

void Logger::open(const std::string& filename)
{
    m_filename = filename;
    m_fout.open(filename, std::ios::app);
    if (m_fout.fail())
    {
        throw std::logic_error("open log file failed: " + filename);
    }
	// seekp() 设置写入位置的偏移量，std::ios::end 表示从文件末尾开始计算偏移量
    m_fout.seekp(0, std::ios::end);
	// tellp() 返回当前写入位置的偏移量，即文件的长度
    m_len = m_fout.tellp();
}

void Logger::close()
{
    m_fout.close();
}

void Logger::log(Level level, const char* file, int line, const char* format, ...)
{
    if (m_level > level)
    {
        return;
    }

    if (m_fout.fail())
    {
        throw std::logic_error("open log file failed: " + m_filename);
    }

    time_t ticks = time(NULL);
    // struct tm* ptm = localtime(&ticks);
	/*Windows 下 localtime_s 参数：(输出tm缓冲区, 输入time_t*)*/
    struct tm tmbuf;
    localtime_s(&tmbuf, &ticks);
    struct tm* ptm = &tmbuf;
	
    char timestamp[32];
    memset(timestamp, 0, sizeof(timestamp));
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", ptm);

    int len = 0;
    const char* fmt = "%s %s %s:%d ";
    len = snprintf(NULL, 0, fmt, timestamp, s_level[level], file, line);
    if (len > 0)
    {
        char* buffer = new char[len + 1];
        snprintf(buffer, len + 1, fmt, timestamp, s_level[level], file, line);
        buffer[len] = 0;
        m_fout << buffer;
        delete[] buffer;
        m_len += len;
    }

    va_list arg_ptr;
    va_start(arg_ptr, format);
    len = vsnprintf(NULL, 0, format, arg_ptr);
    va_end(arg_ptr);
    if (len > 0)
    {
        char* content = new char[len + 1];
        va_start(arg_ptr, format);
        vsnprintf(content, len + 1, format, arg_ptr);
        va_end(arg_ptr);
        content[len] = 0;
        m_fout << content;
        delete[] content;
        m_len += len;
    }

    m_fout << "\n";
	// flush 刷新缓冲区，确保日志流及时写入文件
    m_fout.flush();

    if (m_max > 0 && m_len >= m_max)
    {
        rotate();
    }
}

void Logger::setMax(int bytes)
{
    m_max = bytes;
}

void Logger::setLevel(int level)
{
    m_level = level;
}

void Logger::rotate()
{
    close();
    time_t ticks = time(NULL);
    // struct tm* ptm = localtime(&ticks);
	/*Windows 下 localtime_s 参数：(输出tm缓冲区, 输入time_t*)*/
    struct tm tmbuf;
	localtime_s(&tmbuf, &ticks);
	struct tm* ptm = &tmbuf;
	
    char timestamp[32];
    memset(timestamp, 0, sizeof(timestamp));
    strftime(timestamp, sizeof(timestamp), ".%Y-%m-%d_%H-%M-%S", ptm);
    std::string filename = m_filename + timestamp;
    if (rename(m_filename.c_str(), filename.c_str()) != 0)
    {
        // throw std::logic_error("rename log file failed: " + std::string(strerror(errno)));
		char errBuf[128] = { 0 };
		strerror_s(errBuf, sizeof(errBuf), errno);
		throw std::logic_error("rename log file failed: " + std::string(errBuf));
    }
    open(m_filename);
}
