#include "BoostServer.h"
#include "HttpServerConnection.h"
#include "../Utils/Config.h"
#include "../Utils/Log.h"

BoostServer::BoostServer(Config* config) : mConfig(config)
{

}
BoostServer::~BoostServer() {
	if (mAcceptor) {
		delete mAcceptor;
		mAcceptor = nullptr;
	}
	if (mIoc) {
		delete mIoc;
		mIoc = nullptr;
	}
}

/**
 * 利用io_context设置ip地址 端口；  
 * 设置接受端acceptor 包含io_context 
 * acceptor.open
 * acceptor.set_option
 * acceptor.bind
 * acceptor.listen
 * 设置接收连接回调函数setOnAccept 异步接受连接 有连接就调用onAccept
 * 多线程方式启动io_context 
 */
void BoostServer::start()
{
	int threadNum = std::max<int>(1, mConfig->getThreadNum());
	unsigned short port = mConfig->getPort();
	LOGI("BoostServer::start() ip=%s,port=%d,threadNum=%d", mConfig->getIp() , port, threadNum);

	boost::asio::ip::address address = net::ip::make_address(mConfig->getIp()); //address 不仅包含ip还包含协议之类的
	tcp::endpoint endpoint{ address, port };// 

	mIoc = new net::io_context{ threadNum };// 不使用智能指针
	mAcceptor = new tcp::acceptor(*mIoc);

	beast::error_code ec;
	mAcceptor->open(endpoint.protocol(), ec); //打开协议
	if (ec) 
	{
		LOGE("acceptor.open error: %s", ec.message().data());
		return;
	}
	mAcceptor->set_option(net::socket_base::reuse_address(true), ec);//为什么需要端口重用的一个设置 为了防止端口占用
	if (ec)
	{
		LOGE("acceptor.set_option error: %s", ec.message().data());
		return;
	}
	mAcceptor->bind(endpoint, ec);
	if (ec)
	{
		LOGE("acceptor.bind error: %s", ec.message().data());
		return;
	}
	mAcceptor->listen(net::socket_base::max_listen_connections, ec);
	if (ec)
	{
		LOGE("acceptor.listen error: %s", ec.message().data());
		return;
	}
	setOnAccept(); //设置接收连接回调 因为是异步网络库，所以需要设置回调函数

	std::vector<std::thread> ts;//创建一个线程的容器
	ts.reserve(threadNum - 1);//容量

	for (auto i = 0; i < threadNum - 1; ++i)
	{
		ts.emplace_back([this] { //push_back
			std::thread::id threadId = std::this_thread::get_id(); 
			LOGI("ioc sub threadId=%d", threadId);
			mIoc->run();//子线程run threadNum-1次  将mIoc实例在每个线程单独run
		});
	}
	/*
	std::thread 是一个非可复制对象，不能用拷贝构造，但可以用移动构造。
	如果用 push_back，需要先创建一个 std::thread 临时对象，然后将其移动到 std::vector 中。
	而使用 emplace_back 时，线程对象会直接在容器内构造，完全避免了额外的移动操作。
	*/ 
	std::thread::id threadId = std::this_thread::get_id();
	LOGI("ioc main threadId=%d", threadId);
	mIoc->run();//主线程run 1次

}

void BoostServer::setOnAccept()
{
	mAcceptor->async_accept(boost::asio::make_strand(*mIoc),
		beast::bind_front_handler(&BoostServer::onAccept, this));//异步接受连接 有连接就调用onAccept
}
/**
 * 无错误创建新http连接
 * 添加连接
 * 设置断开连接回调函数 cbDisconnection
 * 启动连接
 * 创建新连接后原有监听失效，需要重新设置异步监听
 * 
 */
void BoostServer::onAccept(beast::error_code ec, tcp::socket socket)
{
	if (ec)
	{
		LOGE("onAccept error: %s", ec.message().data());
		return;
	}
	else
	{
		HttpServerConnection* conn = new HttpServerConnection(this, socket); //创建新连接
		if (this->addConn(conn)) { //添加连接
			conn->setDisconnectionCallback(BoostServer::cbDisconnection, this);//设置断开连接回调函数 cbDisconnection
			conn->run();//启动连接
		}
		else {
			delete conn; //添加失败，释放连接
			conn = nullptr;
		}

	}
	setOnAccept();// 创建新连接后原有监听失效，需要重新设置异步监听
}

bool BoostServer::addConn(HttpServerConnection* conn) {
	m_connMap_mtx.lock(); //加锁
	if (m_connMap.find(conn->getSession()) != m_connMap.end()) { /**< 找到连接 连接已存在 返回false  */
		m_connMap_mtx.unlock();
		return false;
	}
	else {
		m_connMap.insert(std::make_pair(conn->getSession(), conn)); //将新的连接插入到连接映射中，键是会话 ID，值是连接对象指针
		m_connMap_mtx.unlock();
		return true;
	}
}
bool BoostServer::removeConn(std::string session) {
	m_connMap_mtx.lock();
	std::map<std::string, HttpServerConnection*>::iterator it = m_connMap.find(session);
	if (it != m_connMap.end()) {
		m_connMap.erase(it);
		m_connMap_mtx.unlock();
		return true;
	}
	else {
		m_connMap_mtx.unlock();
		return false;
	}
}
HttpServerConnection* BoostServer::getConn(std::string session) {

	m_connMap_mtx.lock();
	std::map<std::string, HttpServerConnection*>::iterator it = m_connMap.find(session);
	if (it != m_connMap.end()) {
		m_connMap_mtx.unlock();
		return it->second;
	}
	else {
		m_connMap_mtx.unlock();
		return nullptr;
	}
}
void BoostServer::cbDisconnection(void* arg, std::string session) {//session是连接的名称 用来唯一标识连接
	BoostServer* server = (BoostServer*)arg;
	server->handleDisconnection(session);

}
bool BoostServer::sendData(char* data, int size) { //暂时没用到
	bool result = false;

	m_connMap_mtx.lock();
	std::map<std::string, HttpServerConnection*>::iterator it;
	if (m_connMap.size() > 0) {
		result = true;
	}
	//LOGI("ʵʱ���߿ͻ�������=%lld,size=%d", m_connMap.size(),size);

	for (it = m_connMap.begin(); it != m_connMap.end(); it++) {
		HttpServerConnection* conn = it->second;
	}
	m_connMap_mtx.unlock();
	return result;
}

void BoostServer::handleDisconnection(std::string session) {
	LOGI("session=%s,disconnection", session.data());
	HttpServerConnection* conn = this->getConn(session);//根据session找到连接示例
	this->removeConn(session);//删除连接
	if (conn) {
		delete conn;
		conn = nullptr;
	}


}
std::string BoostServer::generateSession()
{
	std::string numStr;
	numStr.append(std::to_string(rand() % 9 + 1));
	numStr.append(std::to_string(rand() % 10));
	numStr.append(std::to_string(rand() % 10));
	numStr.append(std::to_string(rand() % 10));
	numStr.append(std::to_string(rand() % 10));
	numStr.append(std::to_string(rand() % 10));
	numStr.append(std::to_string(rand() % 10));
	numStr.append(std::to_string(rand() % 10));
	//int num = stoi(numStr);

	return numStr;
}