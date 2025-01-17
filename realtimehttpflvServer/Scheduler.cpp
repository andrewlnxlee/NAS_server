#include "Scheduler.h"
#include <json/json.h>
#include "Server/BoostServer.h"
#include "Utils/CharUtils.h"
#include "Utils/Config.h"
#include "Utils/Log.h"

Scheduler::Scheduler(BoostServer* server, Config* config) :mServer(server), mConfig(config) {

}

Scheduler::~Scheduler() {
    mIsStopLoop = true;


    if (mSendDataThread) {
        mSendDataThread->join();
        delete mSendDataThread;
        mSendDataThread = nullptr;
    }
}

void Scheduler::loop() {
    mIsStopLoop = false;
    mSendDataThread = new std::thread(&sendDataThread, this);
    this->handleGetData();

}
void Scheduler::handleGetData() {

    while (!mIsStopLoop) {

        std::this_thread::sleep_for(std::chrono::milliseconds(30000));//ÐÝÃß30s
    }
}
void Scheduler::sendDataThread(void* arg) {
    Scheduler* scheduler = (Scheduler*)arg;
    scheduler->handleSendData();
}
void Scheduler::handleSendData() {

    uint64_t count = 0;

    while (!mIsStopLoop) {
     
        ++count;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }


}
Config* Scheduler::getConfig() {
    return mConfig;
}