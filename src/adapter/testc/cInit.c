#include "main.h"
#include "plugin.h"

static int testc_init(void);
static int testc_destroy(void);
static int testc_start(void);
static int testc_stop(void);

static bool flag_stop = false;

//  定义NNGclient结构体内容
const brickPluginRegister_TypeDef testc = {

    .init = testc_init,
    .destroy = testc_destroy,
    .start = testc_start,
    .stop = testc_stop,

    .moduleName = "testc",
    .moduleVersion = "ST03.001",
};

brickRouteClient_TypeDef *cClient = NULL;

static int testc_init(void)
{
    printf("testc_init is running\n");

    cClient = route_client_setDefault(&testc);
    assert(cClient != NULL);

    return 0;
}

static int testc_destroy(void)
{
    int ret;
    if ((ret = route_client_free(cClient)) != 0)
    {
        ERROR_ASSERT();
        return -1;
    }

    printf("testc_destroy.\n");
    return 0;
}

static int testc_start(void)
{
    char recv_str[2048] = {0};
    int ret, count = 0;

    printf("testc_start running\n");

    while (true)
    {
        if (flag_stop)
        {
            printf("testc_start is endding.\n");
            return 0;
        }

        if ((ret = route_client_recv(cClient, recv_str)) < 0)
        {
            continue;
        }

        printf("%d: [%d] %s\n", count, ret, recv_str);
        count++;
    }

    return 0;
}


static int testc_stop(void)
{
    printf("testc_stop is running\n");
    flag_stop = true;
    return 0;
}
