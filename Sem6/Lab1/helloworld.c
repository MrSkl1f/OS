#include <syslog.h>
#include <stdlib.h>

int main()
{
    syslog(LOG_INFO, "Hello world");
    return 0;
}