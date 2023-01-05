#include "main.h"
#include "plugin.h"

static int testb_init(void);
static int testb_destroy(void);
static int testb_start(void);
static int testb_stop(void);

static bool flag_stop = false;

//  定义模块结构体内容
const brickPluginRegister_TypeDef testb = {

    .init = testb_init,
    .destroy = testb_destroy,
    .start = testb_start,
    .stop = testb_stop,

    .moduleName = "testb",
    .moduleVersion = "ST03.005",
};

brickRouteClient_TypeDef *bClient = NULL;

static int testb_init(void)
{
    printf("testb_init running\n");

    bClient = route_client_setDefault(&testb);
    assert(bClient != NULL);

    return 0;
}

static int testb_destroy(void)
{
    int ret;
    if ((ret = route_client_free(bClient)) != 0)
    {
        ERROR_ASSERT();
        return -1;
    }

    printf("testb_destroy.\n");
    return 0;
}

static int testb_start(void)
{
    char recv_str[2048] = {0};
    int ret, count = 0;

    printf("testb_start running\n");

    while (true)
    {
        if (flag_stop)
        {
            printf("testb_start is endding.\n");
            return 0;
        }

        if ((ret = route_client_recv(bClient, recv_str)) < 0)
        {
            ERROR("route_client_recv error!\n");
            continue;
        }
        if ((ret = route_client_send(bClient, recv_str, strlen(recv_str))) != 0)
        {
            ERROR("route_client_send error!\n");
        }

        printf("testb_start: count: %d\n", count);
        count++;
    }

    return 0;
}

static int testb_stop(void)
{
    printf("testb_stop is running\n");
    flag_stop = true;
    return 0;
}
