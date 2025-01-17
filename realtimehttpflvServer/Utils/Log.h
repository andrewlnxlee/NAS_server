#ifndef LOG_H
#define LOG_H
#include <time.h>
#include <string>


static std::string getTime() {
    const char* format = "%Y-%m-%d %H:%M:%S";
    time_t t = time(nullptr);

    char tc[64];
    strftime(tc, sizeof(tc), format, localtime(&t));

    std::string timeStr = tc;
    return timeStr;
}
// add by lhx


#define LOGI(format, ...)  fprintf(stderr,"[I]%s [%d %s()] " format "\n", getTime().data(),__LINE__,__func__ ,##__VA_ARGS__)
#define LOGE(format, ...)  fprintf(stderr,"[E]%s [%d %s()] " format "\n",getTime().data(),__LINE__,__func__ ,##__VA_ARGS__)

#endif //LOG_H