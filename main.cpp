#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "Logger.h"
using namespace yanlong::utility;

// 多线程测试函数
void threadFunc(int threadId)
{
    for (int i = 0; i < 50; ++i)
    {
        LOG_INFO("Thread %d: iteration %d, value=%d", threadId, i, i * 2);
        LOG_WARN("Thread %d: warning message %d", threadId, i);

        // 模拟一些工作
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

int main()
{
    try
    {
        // 初始化日志对象
        Logger::instance()->open("./test.log");

        // 设置日志级别为INFO
        Logger::instance()->setLevel(Logger::Level::INFO);

        // 设置日志最大文件大小为1MB
        Logger::instance()->setMax(1 * 1024 * 1024);

        // 单线程测试
        std::cout << "=== Single thread test ===" << std::endl;
        LOG_DEBUG("hello world");  // 不会输出（级别低于INFO）
        LOG_DEBUG("name=%s age=%d", "jack", 18);  // 不会输出
        LOG_INFO("this is info message");
        LOG_WARN("this is warn message");
        LOG_ERROR("this is error message");
        LOG_FATAL("this is fatal message");

        // 测试格式化输出
        LOG_INFO("User %s logged in", "admin");
        LOG_WARN("Disk usage: %d%%", 85);
        LOG_ERROR("Connection to %s:%d failed", "localhost", 8080);

        // 多线程测试
        std::cout << "\n=== Multi-thread test ===" << std::endl;
        std::cout << "Starting 5 threads, each writing 50 logs..." << std::endl;

        std::vector<std::thread> threads;
        for (int i = 0; i < 5; ++i)
        {
            threads.emplace_back(threadFunc, i);
        }

        // 等待所有线程结束
        for (auto& t : threads)
        {
            t.join();
        }

        std::cout << "All threads finished!" << std::endl;
        std::cout << "Check test.log for output." << std::endl;

        // 测试文件轮转
        std::cout << "\n=== Testing log rotation ===" << std::endl;
        Logger::instance()->setMax(1024);  // 设置为1KB，方便测试轮转

        for (int i = 0; i < 200; ++i)
        {
            LOG_INFO("Rotation test: iteration %d, data: %s", i,
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
        }

        std::cout << "Log rotation test completed. Check for rotated files." << std::endl;

    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
