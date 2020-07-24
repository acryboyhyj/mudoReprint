#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

string format(const char *fmt, ...)
{
    int len;
    string str;
    va_list args;
    char buffer[4 * 1024];

    va_start(args, fmt);

    if ((len = vsnprintf(buffer, sizeof(buffer), fmt, args)) > 0)
    {
        if (len < sizeof(buffer))
        {
            str = buffer;
        }
        else
        {
            int maxsz = len + 1;
            char *buffer = (char *)malloc(maxsz);

            if (buffer)
            {
                len = vsnprintf(buffer, maxsz, fmt, args);

                if (len > 0 && len < maxsz)
                    str = buffer;

                free(buffer);
            }
        }
    }

    va_end(args);

    return str;
}

int test(int argc, char **argv)
{
    cout << format("字符串格式化：%s-%d-%.2f", "1", 2, 3.88) << endl;

    return 0;
}