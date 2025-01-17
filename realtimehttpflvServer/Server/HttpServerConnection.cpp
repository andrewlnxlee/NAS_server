#include "HttpServerConnection.h"
#include "BoostServer.h"
#include "../Utils/Log.h"

HttpServerConnection::HttpServerConnection(BoostServer* server, tcp::socket& socket)
    : mSession(server->generateSession()), //随机的生成一个唯一的session
    mServer(server),
    mSocket(std::move(socket)), //左值引用转右值引用才能赋值
    mTimer(mSocket.get_executor(), std::chrono::seconds(10)) //add by lhx
{
    LOGI("");

}
HttpServerConnection::~HttpServerConnection() {
    LOGI("");
    if (fp) {
        fclose(fp);
        fp = nullptr;
    }

    //add by lhx
    if (ffmpegPipe) {
        pclose(ffmpegPipe); // 关闭 FFmpeg 子进程
        ffmpegPipe = nullptr;
    }

    this->mSocket.close();
    this->mTimer.cancel();
}
std::string HttpServerConnection::getSession() {
    return mSession;
}

void HttpServerConnection::run()
{
    http::async_read(//异步可读
        mSocket,
        mTempBuffer,//临时缓冲区
        mHttpRequest,//请求被触发后，将请求数据存储在mHttpRequest中
        [this](beast::error_code ec,
            std::size_t bytes_transferred)  
        {// 回调函数：一个 lambda 表达式，当读取操作完成时将被调用。

            boost::ignore_unused(bytes_transferred);//在某些情况下，您可能会有一个变量（如 bytes_transferred），它在当前的代码逻辑中没有被使用，但您不希望编译器发出未使用变量的警告。使用 boost::ignore_unused 可以解决这个问题
            if (ec) {
                //接收可读数据失败
                LOGE("run error,msg=%s", ec.message().data());
                m_disconnectionCallback(m_arg, mSession);  //如果错误断开连接
            }
            else {
                this->handle();
            }
        });

    /*
    mTimer.async_wait(
        [this](beast::error_code ec)
        {
            if (ec)
            {
            }
        }
    );*/
}
void HttpServerConnection::handle()
{
    mHttpResponse.version(mHttpRequest.version());//设置http响应参数版本号
    mHttpResponse.set(http::field::server, "Boost");//设置服务器类型  

    switch (mHttpRequest.method())//判断请求的方法
    {
    case http::verb::get:
    {

        if (mHttpRequest.target() == "/test.flv")
        {
            mHttpResponse.result(http::status::ok); //返回的状态码  比如说404 not found
            mHttpResponse.keep_alive(true); //一般http是短时间连接 但是我们这里是http-flv,需要保持连接 重要
            mHttpResponse.set(http::field::access_control_allow_origin, "*");//设置跨域
            mHttpResponse.set(http::field::content_type, "video/x-flv");// 设置返回的数据类型
            mHttpResponse.set(http::field::connection, "close");//设置连接类型 无效的？
            mHttpResponse.set(http::field::expires, "-1");//设置过期时间 -1表示一直过期
            mHttpResponse.set(http::field::pragma, "no-cache"); //不缓存
            mHttpResponse.content_length(-1);//一般http是有长度的 但是我们这里是http-flv,没有长度

            http::async_write(//设置完后做一个异步回复
                mSocket,
                mHttpResponse,
                [this](beast::error_code ec, std::size_t)
                {
                    if (ec)
                    {
                        //发送失败
                        LOGE("play flv error,msg=%s", ec.message().data());
                        //this->mSocket.shutdown(tcp::socket::shutdown_send, ec);
                        m_disconnectionCallback(m_arg, mSession);//断开连接 会执行server的handleDisconnection   m_disconnectionCallback = cbDisconnection   
                    }
                    else {
                        //发送成功
                        LOGI("play flv success");
                        // 启动 FFmpeg 子进程，将视频实时转码为 FLV
                        //const char* command = "ffmpeg -i ../data/input.mp4 -c:v libx264 -c:a aac -f flv pipe:1";
                        const char* command = "ffmpeg -i ../data/input.mp4 -c:v libx264 -preset ultrafast -c:a aac -f flv pipe:1";
                        ffmpegPipe = popen(command, "r"); // 启动 FFmpeg 子进程，pipe:1 表示标准输出
                        if (!ffmpegPipe) {
                            LOGE("Failed to start FFmpeg process");
                            m_disconnectionCallback(m_arg, mSession);
                            return;
                        }
                        //const char* filename = "../data/test.flv";
                        // const char* filename = "../data/笑傲江湖天地作合.flv";

                        // fp = fopen(filename, "rb");//fp是类的成员

                        this->keepWrite();

                    }
                });
        }
        else
        {
            mHttpResponse.result(http::status::ok);
            mHttpResponse.keep_alive(false);
            mHttpResponse.result(http::status::not_found);
            mHttpResponse.set(http::field::content_type, "text/plain");
            beast::ostream(mHttpResponse.body()) << "File not found\r\n";

            http::async_write(
                mSocket,
                mHttpResponse,
                [this](beast::error_code ec, std::size_t)
                {
                    if (ec)
                    {
                        //发送失败
                        LOGE("http::async_write error,msg=%s", ec.message().data());
                        //this->mSocket.shutdown(tcp::socket::shutdown_send, ec
                    }
                    else {
                        //发送成功
                        LOGI("http::async_write success,msg=%s", ec.message().data());
                    }
                    m_disconnectionCallback(m_arg, mSession);
                });
        }
        break;
    }
    default:
    {
        mHttpResponse.result(http::status::bad_request);
        mHttpResponse.set(http::field::content_type, "text/plain");
        beast::ostream(mHttpResponse.body())
            << "Invalid request-method '"
            << std::string(mHttpRequest.method_string())
            << "'";
        mHttpResponse.content_length(mHttpResponse.body().size());

        http::async_write(
            mSocket,
            mHttpResponse,
            [this](beast::error_code ec, std::size_t)
            {
                if (ec)
                {
                    //发送失败
                    LOGE("http::async_write error,msg=%s", ec.message().data());
                    //this->mSocket.shutdown(tcp::socket::shutdown_send, ec);
                }
                else {
                    //发送成功
                    LOGI("http::async_write success,msg=%s", ec.message().data());
                }
                m_disconnectionCallback(m_arg, mSession);
            });
        break;
    }

    }

}
void HttpServerConnection::keepWrite() {
    
    char data[5000];//每次5000字节
    int  size;

    size = fread(data, 1, sizeof(data), ffmpegPipe);
    //size = fread(data, 1, sizeof(data), fp);
    if (size > 0) {
        boost::asio::async_write(//异步写入
            mSocket,
            boost::asio::buffer(data, size),
            [this](beast::error_code ec, std::size_t)
            {
                if (ec)
                {
                    //发送失败
                    LOGE("keepWrite error,msg=%s", ec.message().data());
                    m_disconnectionCallback(m_arg, mSession);
                }
                else {
                    //发送成功
                    //LOGI("keepWrite successs");
                    this->keepWrite();//size>0 一直成功 递归调用 一直写数据
                }

            });
    }
    else {
        LOGE("keepWrite error,msg= flv buffer finish");
        //add by lhx
        pclose(ffmpegPipe); // 关闭 FFmpeg 子进程
        ffmpegPipe = nullptr;

        m_disconnectionCallback(m_arg, mSession);//删除连接
    }

}


