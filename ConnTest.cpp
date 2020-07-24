#include "Connector.h"
#include "muduo/base/Logging.h"
#include "EventLoop.h"
#include <unistd.h>
#include <thread>
void test(Connector &conn)
{
    sleep(2);
    int fd = conn.connect();

    LOG_WARN << "fd:" << fd;
}

void quit(EventLoop &loop)
{
    loop.quit();
}
int main()
{
    EventLoop loop;
    std::string name("conn");
    Connector conne(&loop, InetAddress("127.0.0.1", 13000), name);
    std::thread a(test, std::ref(conne));
    loop.runAfter(10, std::bind(quit, std::ref(loop)));
    loop.loop();
    if (a.joinable())
    {
        a.join();
    }

    return 0;
}