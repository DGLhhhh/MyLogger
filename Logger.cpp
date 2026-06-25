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
    // 加锁保护
    std::lock_guard<std::mutex> lock(m_mutex);

    m_filename = filename;
    m_fout.open(filename, std::ios::app);
    if (m_fout.fail())
    {
        throw std::logic_error("open log file failed: " + filename);
    }
    m_fout.seekp(0, std::ios::end);
    m_len = static_cast<int>(m_fout.tellp());
}

void Logger::close()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_fout.is_open())
    {
        m_fout.close();
    }
}

void Logger::log(Level level, const char* file, int line, const char* format, ...)
{
    // 检查日志级别
    if (m_level > level)
    {
        return;
    }

    // 加锁保护整个日志写入过程
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_fout.fail())
    {
        throw std::logic_error("open log file failed: " + m_filename);
    }

    // 获取时间戳
    time_t ticks = time(NULL);
    struct tm tmbuf;
    localtime_s(&tmbuf, &ticks);
    struct tm* ptm = &tmbuf;

    char timestamp[32];
    memset(timestamp, 0, sizeof(timestamp));
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", ptm);


    // 安全地获取级别字符串
    const char* levelStr = "UNKNOWN";
    if (level >= 0 && level < LEVEL_COUNT)
    {
        levelStr = s_level[level];
    }

    // 写入日志前缀
    int len = 0;
    const char* fmt = "%s %s %s:%d ";
    len = snprintf(NULL, 0, fmt, timestamp, levelStr, file, line);
    if (len > 0)
    {
        char* buffer = new char[len + 1];
        snprintf(buffer, len + 1, fmt, timestamp, levelStr, file, line);
        buffer[len] = '\n';
        m_fout << buffer;
        delete[] buffer;
        m_len += len;
    }

    // 写入日志内容
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
    m_fout.flush();

    // 检查是否需要轮转
    if (m_max > 0 && m_len >= m_max)
    {
        rotate();
    }
}

void Logger::rotate()
{
    // 注意：rotate 在 log 函数中被调用，此时已经持有锁
    // 不需要再次加锁，避免死锁

    if (!m_fout.is_open())
    {
        return;
    }

    m_fout.close();

    time_t ticks = time(NULL);
    struct tm tmbuf;
    localtime_s(&tmbuf, &ticks);
    struct tm* ptm = &tmbuf;

    char timestamp[32];
    memset(timestamp, 0, sizeof(timestamp));
    strftime(timestamp, sizeof(timestamp), ".%Y-%m-%d_%H-%M-%S", ptm);

    std::string filename = m_filename + timestamp;
    if (rename(m_filename.c_str(), filename.c_str()) != 0)
    {
        char errBuf[128] = { 0 };
        strerror_s(errBuf, sizeof(errBuf), errno);
        throw std::logic_error("rename log file failed: " + std::string(errBuf));
    }

    // 重新打开文件
    m_fout.open(m_filename, std::ios::app);
    if (m_fout.fail())
    {
        throw std::logic_error("open log file failed: " + m_filename);
    }
    m_len = 0;
}

void Logger::setMax(int bytes)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_max = bytes;
}

void Logger::setLevel(int level)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_level = level;
}
