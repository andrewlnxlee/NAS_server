#include "Config.h"
#include <fstream>
#include <json/json.h>
#include "Log.h"


Config::Config(const char* file) : mFile(file)
{
    std::ifstream ifs(mFile, std::ios::binary);

    if (!ifs.is_open()) {
        LOGE("open %s error", file);
        return;
    }
    else {
        Json::CharReaderBuilder builder;
        builder["collectComments"] = true;
        JSONCPP_STRING errs;
        Json::Value root;

        if (parseFromStream(builder, ifs, &root, &errs)) {

            this->ip = root["ip"].asString();
            this->port = root["port"].asInt();
            this->threadNum = root["threadNum"].asInt();

            state = true;
        }
        else {
            LOGE("parse %s error", file);
        }
        ifs.close();
    }
}

Config::~Config()
{

}

void Config::show() {

    //printf("config.file=%s\n", mFile.data());

}