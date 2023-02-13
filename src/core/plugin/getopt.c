/**
 * @file cs_getopt.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-03-08
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "main.h"
#include "plugin_init.h"
#include "version.h"

// help describe.
static void help(void)
{
    printf("option:\n");
    printf("       -h print help information\n");
    printf("       -v print version\n");
	printf("       -V print all plugins version\n");
}

/**
 * @brief  optionGet
 * @note
 * @param  argc:
 * @param  **argv:
 * @retval
 */
int optionGet(int argc, char **argv)
{
    int ret = getopt(argc, argv, "hvVs:");
    if (ret < 0)
    {
        log_printf("optionGet: ret = %d\n", ret);
        return 1;
    }

    switch (ret)
    {
    case 'h':
        help();
        exit(0);
        break;

    case 'v':
        printf("version: %s\n", SOFTWAREVERSION);
        exit(0);
        break;

    case 'V':
        plugin_printfVer();
        exit(0);
        break;

    case 's':
        plugin_init(optarg);
        exit(0);
        break;

    default:
        help();
        exit(0);
        break;
    }

    return 0;
}
