/******************************************************************************
 * @brief     µÍ¹¦ºÄ¿ØÖÆÃüÁî
 *
 * Copyright (c) 2020, <morro_luo@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs: 
 * Date           Author       Notes 
 * 2021/03/07     Morro        
 ******************************************************************************/
#include "cli.h"
#include "pm.h"
#include <stdlib.h>

/* 
 * @brief   µÍ¹¦ºÄ¿ØÖÆÃüÁî
 * @example pm 0 - ½ûÓÃµÍ¹¦ºÄ       
 * @example pm 1 - ÆôÓÃµÍ¹¦ºÄ   
 */ 
int do_cmd_pm(struct cli_obj *cli, int argc, char *argv[])
{
    bool value;
    if (argc != 2) {
        cli->print(cli, "Command format error\r\n");
    }
    value = atoi(argv[1]);
    if (value) {
        pm_enable();
        cli->print(cli, "Lowpower enable...\r\n");
    } else {
        pm_disable();
        cli->print(cli, "Lowpower disable...\r\n");
    }
        
    return 0;
}cmd_register("pm", do_cmd_pm, "Low power control command");
