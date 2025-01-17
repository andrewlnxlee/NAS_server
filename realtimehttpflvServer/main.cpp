#include "Server/BoostServer.h"
#include "Scheduler.h"
#include "Utils/Config.h"

/*
* 作者：北小菜
* 个人网站：http://www.any12345.com
* 邮箱：bilibili_bxc@126.com
* QQ：1402990689
* 微信：bilibili_bxc
* 作者-哔哩哔哩主页：https://space.bilibili.com/487906612
* 作者-头条西瓜主页：https://www.ixigua.com/home/4171970536803763
* github开源地址：https://github.com/any12345com/BXC_MediaServer
* gitee开源地址：https://gitee.com/Vanishi/BXC_MediaServer


ffmpeg 播放http-flv视频流
    ffplay -i http://localhost:8080/test.flv

实现浏览器播放HLS和HTTP-FLV视频流-源码下载  http://www.any12345.com/article.html?id=1

*/

int main(int argc, char* argv[])
{
    const char* file = NULL;
    file = "config.json";
    //读取参数
    Config config(file);
    if (!config.state) {
        printf("failed to read config file: %s\n", file);
        return -1;
    }
    
    BoostServer server(&config);
    std::thread([&]() {
        server.start();
    }).detach();
    
    Scheduler sch(&server,&config);
    sch.loop();

    return 0;
}
