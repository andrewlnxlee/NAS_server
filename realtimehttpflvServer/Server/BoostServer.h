#ifndef BOOSTSERVER_H
#define BOOSTSERVER_H

#include "boost.h"
#include <map>

#include <mutex> //add by lhx   error: 'mutex' in namespace 'std' does not name a type

class Config;
class HttpServerConnection;

class BoostServer
{
public:
	BoostServer(Config* config);
	~BoostServer();
public:
	void start();

	bool addConn(HttpServerConnection* conn); //添加连接
	bool removeConn(std::string session); //删除连接
	HttpServerConnection* getConn(std::string session); //获取连接
	bool sendData(char* data, int size); //发送数据

	static void cbDisconnection(void* arg, std::string session);
	std::string generateSession();
private:
	void handleDisconnection(std::string session);

	void setOnAccept(); //设置接收连接回调
	void onAccept(beast::error_code ec, tcp::socket socket); //接收连接回调函数 

private:
	Config* mConfig; 
	net::io_context* mIoc; //io上下文 boost库开发经常用 
	tcp::acceptor*   mAcceptor;//接收器
	std::map<std::string, HttpServerConnection*> m_connMap;// <session,conn> 维护所有被创建的连接的容器
	std::mutex								     m_connMap_mtx; //锁


};


#endif //BOOSTSERVER_H