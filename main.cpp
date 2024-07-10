#include "Config.h"
#include "WebServer.h"

int main(int argc, char *argv[])
{
    Stu_sqlInfo sqlInfo;
    sqlInfo.user = "root";
    sqlInfo.password = "123456";
    sqlInfo.databaseName = "test_db";

    // 命令行解析
    Config config;
    config.parse_arg(argc, argv);

    WebServer server;
   
    // 初始化
    server.init(config.PORT, sqlInfo, config.LOGWrite, config.OPT_LINGER, config.TRIGMode, config.sql_num, config.thread_num,
                config.close_log, config.actor_model);
    
    // 日志
    server.log_write();
    // 数据库
    server.sql_pool();
    // 线程池
    server.thread_pool();
    // 触发模式
    server.trig_mode();

    // 监听
    server.eventListen();
    cout << "运行成功" << endl;
    // 运行
    server.eventLoop();
   
    return 0;
}