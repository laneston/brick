# 插件创建

## 文件添加

**第1步**，在路径 /src/adapter 下新建模块文件夹，如 testc. 

**第2步**，在 testb 路径下增加 CMakeList.txt 文件，格式如下：

```
file(GLOB SOURCES_TAR "${CMAKE_CURRENT_LIST_DIR}/*.c")


target_sources(${PROJECT_NAME}
	PRIVATE
	${SOURCES_TAR}
	PUBLIC
	${SOURCES_TAR}
)


target_include_directories(${PROJECT_NAME}
	PUBLIC
	${CMAKE_CURRENT_LIST_DIR}
)
```

**第3步**，修改工程src/adapter目录下的 CMakeList.txt 文件，添加插件子目录指向，语句格式如下：

```
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/testa)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/testb)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/testc)
```

最后只需要在子目录 /src/adapter/testb 下增加所需的 .c .h 文件。

## 定义接口

为主进程对子进程的管理实现提供接口，需要实现主进程的结构体定义，格式如下：

```
    /**
     * @brief  brickPluginRegister
     * @note   The following interfaces are provided for BRICK calls.
     */
    typedef struct brickPluginRegister
    {

        int (*init)(void);      // Used to initialize process resources
        int (*destroy)(void);   // Used to destroy process resources
        int (*start)(void);     // Enable process when BRICK is called
        int (*stop)(void);      // Disable process when BRICK is called
        char moduleName[128];   // Plug in name
        char moduleVersion[64]; // Plug in version

    } brickPluginRegister_TypeDef;
```

具体描述如下：

- 初始化函数 init。 BRICK 启动时首先调用此函数，通常情况下，将一次性资源分配的动作放入其中，例如 NNG 路由初始化的定义。

- 开始函数 start。 BRICK 调用 init 函数后会调用此函数，此处应为进程的执行，在执行完任务后不能主动 return，需由 BRICK 调用 stop 函数进行退出操作。

- 停止函数 stop。 BRICK 调用此函数退出 start 函数内的所执行的动作，并 return 相应的返回值。

- 销毁函数 destroy。 BRICK 调用此函数时，实现本进程的资源回收，获得函数的返回值后，结束此进程。

- 模块名称 moduleName。 模块名称应与其他模块名称区别开，不能重名，最大命名长度为128字节。

- 模块版本信息 moduleVersion。 为字符串格式，最大命名长度为64字节。

*提示：模块文件夹名称、结构体名称、模块名称最好相同，以方便工程管理与阅读。*

**例子**


*/src/adapter/testb/bInit.h*
```
#include "plugin.h"

extern const brickPluginRegister_TypeDef testc;
```

*/src/adapter/testb/cInit.c*
```
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

```


*/src/moduleReg.h*
```
#include "aInit.h"
#include "bInit.h"
#include "cInit.h"
#include "plugin.h"

    brickPluginRegister_regArray moduleRegister[] = {
        &testa,
        &testb,
        &testc,
    };
```

## 配置文件修改

将创建的插件添加至文件 moduleEnable.json 当中，BRICK 启动时会默认根据配置文件内容进行使能相应的插件模块。

```
{
	"testa": "enable",
	"testb": "enable",
	"testc": "enable"
}
```

## 配置路由消息

```
{
	"testa": [
		"testb"
	],
	"testb": [
		"testc"
	],
	"testc": [
		"testb"
	]
}
```

将创建的插件添加至文件 routeMapping.json 当中，然后把当前插件的数据所推送的目标模块名称填入数组中。需要注意的是，模块的数目与种类必须与 moduleEnable.json 中的一致。

*错误例子：*
```
{
	"testa": [
		"testb"
	]
}
```

```
{
	"testa": [
		"testb"
	],
	"testb": [
		"testa",
		"testc"
	]
}
```

然而 BRICK 支持 1 对多或多对多路由通信。

*例如：*

```
{
	"testa": [
		"testb"
	],
	"testb": [
		"testa",
		"testc"
	]
	"testc": [
		"testb"
	],
}
```


# API 接口

## 日志管理

本工程提供日志打印函数:

```
void log_printf(const char *fmt, ...);
```

此函数提供本地日志管理功能（各个进程日志单独保存，并实现日志轮转）或rsyslog日志管理，使用方式与 c 库的 printf()一致。


## 进程间通信

**获得路由句柄(初始化)**


默认初始化，接收状态为阻塞模式。
```
    /**
     * @brief  route_client_setDefault
     * @note   get the route handle.
     * @param  *p_plugin:
     * @retval the obj of the route client.
     */
    extern brickRouteClient_TypeDef *route_client_setDefault(const brickPluginRegister_TypeDef *p_plugin);
```

可设置初始化。参数 flag_nonblock 为 false 时，接收状态为阻塞模式，但可通过配置 timeout 参数设置超时时间，超时结束后接收函数 (route_client_recv) 返回；参数 flag_nonblock 为 true 时，接收状态为非阻塞模式，无论是否获取到数据，接收函数立即返回。

```
    /**
     * @brief  route_client_set
     * @note
     * @param  *p_plugin:
     * @param  timeout: 0~65535 ms
     * @param  flag_nonblock: The receive function returns immediately, set true or false.
     * @retval
     */
    extern brickRoute_TypeDef *route_client_set(const brickPluginRegister_TypeDef *p_plugin, short timeout, bool flag_nonblock);
```

**销毁路由句柄**

当进程销毁时，需对路由句柄进行释放，函数为：

```
    /**
     * @brief  route_client_free
     * @note
     * @param  *t_route: the client handle
     * @retval
     */
    extern int route_client_free(brickRoute_TypeDef *t_route);
```

**发送函数**

```
    /**
     * @brief  route_client_send
     * @note
     * @param  *t_route:
     * @param  *data:
     * @param  len:
     * @retval return 0 if success, or return other if failed.
     */
    extern int route_client_send(brickRoute_TypeDef *t_route, void *data, size_t len);
```

**接收函数**

```
    /**
     * @brief  route_client_recv
     * @note
     * @param  *t_route:
     * @param  *data:
     * @retval return 0 if success, or return other if failed.
     */
    extern int route_client_recv(brickRoute_TypeDef *t_route, void *data);
```




# 执行文件使用方式

```
brick -s [appName/mode]
brick -v
brick -h
```

## 模式(mode)

1. loopback
2. relay

## 执行文件存放位置声明

应为 brick 需要 fock 指定文件，所以需要声明文件存放位置。

声明路径在 plugin.h 文件下：
```
#define __TARGET_DIR__ "/userdata/dgri/brick"
```