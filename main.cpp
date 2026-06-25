#include <iostream>
#include <string>

#include "Logger.h"
using namespace yanlong::utility;

int main()
{
	// 初始化日志对象
	Logger::instance()->open("./test.log");

	// 设置日志级别为INFO，低于该级别的日志将不会输出,不会记录到日志文件中
	Logger::instance()->setLevel(Logger::Level::INFO);

	// 设置日志最大文件大小为1MB，超过该大小将会切割日志文件
	Logger::instance()->setMax(1024);

	LOG_DEBUG("hello world");
	LOG_DEBUG("name=%s age=%d", "jack", 18);
	LOG_INFO("this is info message");
	LOG_WARN("this is warn message");
	LOG_ERROR("this is error message");

	return 0;
}
