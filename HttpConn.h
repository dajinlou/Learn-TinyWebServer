#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
#include "SqlConnectionPool.h"

#include <map>
#include <string>
#include <mysql/mysql.h>
#include <arpa/inet.h>
#include <sys/stat.h>

using namespace std;

enum METHOD
{
    GET = 0,
    POST,
    HEAD,
    PUT,
    DELETE,
    TRACE,
    OPTIONS,
    CONNECT,
    PATH
};
enum CHECK_STATE // 主状态机的状态
{
    CHECK_STATE_REQUESTLINE = 0,
    CHECK_STATE_HEADER,
    CHECK_STATE_CONTENT
};

enum HTTP_CODE
{
    NO_REQUEST,
    GET_REQUEST,
    BAD_REQUEST,
    NO_RESOURCE,
    FORBIDDEN_REQUEST,
    FILE_REQUEST,
    INTERNAL_ERROR,
    CLOSED_CONNECTION
};
enum LINE_STATUS // 从状态机的状态
{
    LINE_OK = 0,
    LINE_BAD,
    LINE_OPEN
};

class HttpConn
{

public:
    HttpConn(/* args */);
    ~HttpConn();
    // 初始化套接字地址
    void init(int sockfd, const sockaddr_in &addr, char *, int, int, string user, string passwd, string sqlname);
    // 关闭http连接
    void close_conn(bool real_close = true);
    void process();
    // 读取浏览器发来的全部数据
    bool read_once();
    // 响应报文写入函数
    bool write();

    sockaddr_in *get_address();

    // 同步线程初始化数据库读取表
    void initmysql_result(SqlConnectionPool *connPool);

private:
    void init();

    // 从m_read_buf读取，并处理请求报文
    HTTP_CODE process_read();

    //*向m_write_buf写入响应报文
    bool process_write(HTTP_CODE ret);

    // 主状态机解析报文中的请求行数据
    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);
    HTTP_CODE do_request();

    /*m_start_line是已解析的字符*/
    /*get_line用于将指针往后偏移，指向未处理的字符*/
    char *get_line();

    // 从状态机读取一行，分析是请求报文的哪一部分
    LINE_STATUS parse_line();

    void unmap();

    // 根据响应报文的格式，生成对应的8个部分，以下函数均由do_request调用
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    static const int FILENAME_LEN = 200;       // 设置读取文件的名称m_rea_file大小
    static const int READ_BUFFER_SIZE = 2048;  // 设置读缓冲区m_read_buf大小
    static const int WRITE_BUFFER_SIZE = 1024; // 设置写缓冲区m_write_buf大小

public:
    static int m_epollfd;
    static int m_user_count;
    MYSQL *mysql;
    int m_state; // 读为0，写为1
    int timer_flag;
    int improv;

private:
    int m_sockfd;
    sockaddr_in m_address;
    char m_read_buf[READ_BUFFER_SIZE]; // 存储读取的请求报文
    long m_read_idx;                   // 缓冲区中m_read_buf中数据的最后一个字节的下一个位置
    long m_checked_idx;                // m_read_buf读取的位置m_checked_idx
    int m_start_line;                  // m_read_buf中已经解析的字符个数

    char m_write_buf[WRITE_BUFFER_SIZE]; // 存储发送的响应报文数据
    int m_write_idx;                     // 指示buffer中的长度

    CHECK_STATE m_check_state; // 主状态机的状态
    METHOD m_method;

    /*以下是解析报文中对应的6个变量*/
    char m_real_file[FILENAME_LEN];
    char *m_url;
    char *m_version;
    char *m_host;
    long m_content_length;
    bool m_linger;

    char *m_file_address;
    struct stat m_file_stat;
    struct iovec m_iv[2]; // io向量机制iovec
    int m_iv_count;
    int cgi;             // 是否启用的POST
    char *m_string;      // 存储请求头数据
    int bytes_to_send;   // 剩余发送的字节
    int bytes_have_send; // 已发送的字节
    char *doc_root;      // 网站根目录

    map<string, string> m_users;
    int m_TRIGMode; // 触发模式，=1时为ET触发模式
    int m_close_log;

    char sql_user[100];
    char sql_passwd[100];
    char sql_name[100];
};

#endif // HTTPCONNECTION_H