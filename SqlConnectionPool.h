#ifndef SQLCONNECTIONPOOL_H
#define SQLCONNECTIONPOOL_H

#include "Locker.h"
#include "Log.h"

#include <mysql/mysql.h>
#include <list>

using namespace std;

class SqlConnectionPool
{
public:
    static SqlConnectionPool *GetInstance();
    MYSQL *GetConnection();              // 获取数据库连接
    bool ReleaseConnection(MYSQL *conn); // 释放连接
    int GetFreeConn();                   // 获取连接
    int DestroyPool();                   // 销毁所有连接

    void init(string url,string user,string password,string dbName,int port,int maxConn,int close_log);

private:
    SqlConnectionPool(/* args */);
    ~SqlConnectionPool();

public:
    string m_url;          // 主机地址
    string m_port;         // 数据库端口号
    string m_user;         // 登录数据库用户名
    string m_password;     // 密码
    string m_databaseName; // 使用数据库名
    int m_close_log;       // 日志开关

private:
    int m_maxConn;  // 最大连接数
    int m_curConn;  // 当前已使用的连接数
    int m_FreeConn; // 当前空闲的连接数

    Locker lock;
    list<MYSQL *> connList; // 连接池
    Sem reserve;
};

//利用RAII原则    资源获取即初始化
class ConnectionRAII
{

public:
    ConnectionRAII(MYSQL **conn,SqlConnectionPool *connPool);
    ~ConnectionRAII();

    private:
    MYSQL *m_conRAII;
    SqlConnectionPool *m_poolRAII;
};

#endif // SQLCONNECTIONPOOL_H