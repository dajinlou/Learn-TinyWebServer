#include "SqlConnectionPool.h"

SqlConnectionPool *SqlConnectionPool::GetInstance()
{
    static SqlConnectionPool connpool;
    return &connpool;
}

MYSQL *SqlConnectionPool::GetConnection()
{
    MYSQL *conn = NULL;
    if(0 == connList.size()){
        return NULL;
    }
    reserve.wait();  //p操作
    lock.lock();
    conn = connList.front();
    connList.pop_front();
    --m_FreeConn;
    ++m_curConn;
    lock.unlock();

    return conn;
}

bool SqlConnectionPool::ReleaseConnection(MYSQL *conn)
{
    if(NULL == conn){ //检查参数的有效性
        return false;
    }
    lock.lock();
    connList.push_back(conn);
    ++m_FreeConn;
    --m_curConn;
    lock.unlock();
    reserve.post();  //释放资源

    return true;
}

//当前空闲的连接数
int SqlConnectionPool::GetFreeConn()
{
    return this->m_FreeConn;
}

int SqlConnectionPool::DestroyPool()
{
    lock.lock();
    if(connList.size()>0){
        list<MYSQL *>::iterator it;
        for(it =connList.begin();it !=connList.end();++it){
            MYSQL *conn = *it;
            mysql_close(conn);   //关闭连接
        }
        m_curConn = 0;
        m_FreeConn = 0;
        connList.clear();
    }

    return 0;
}

void SqlConnectionPool::init(string url, string user, string password, string dbName, int port, int maxConn, int close_log)
{
    m_url = url;
    m_port = port;
    m_user = user;
    m_password = password;
    m_databaseName = dbName;
    m_close_log = close_log;

    for(int i=0;i<maxConn;++i){
        MYSQL *conn = NULL;
        conn = mysql_init(conn);  //初始化数据库
        if(NULL == conn){
            LOG_ERROR("MySQL Error");
            exit(1);
        }
        conn = mysql_real_connect(conn,url.c_str(),user.c_str(),password.c_str(),dbName.c_str(),port,NULL,0); //打开一个连接
        if(NULL == conn){
            LOG_ERROR("MySQL Error");
            exit(1);
        }
        connList.push_back(conn);
        ++m_FreeConn;
    }

    reserve = Sem(m_FreeConn);
    m_maxConn = m_FreeConn;

}

SqlConnectionPool::SqlConnectionPool(/* args */):m_curConn(0),m_FreeConn(0)
{
}

SqlConnectionPool::~SqlConnectionPool()
{
    DestroyPool();
}



ConnectionRAII::ConnectionRAII(MYSQL **conn, SqlConnectionPool *connPool)
{
    *conn = connPool->GetConnection();
    m_conRAII = *conn;
    m_poolRAII = connPool;
}

ConnectionRAII::~ConnectionRAII()
{
    m_poolRAII->ReleaseConnection(m_conRAII);
}


