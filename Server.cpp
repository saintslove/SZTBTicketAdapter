/*
 * Server.cpp
 *
 *  Created on: 2016年12月1日
 *      Author: wong
 */

#include <unistd.h>

#include "Config.h"
#include "ConfigDefine.h"
#include "LogHelper.h"
#include "TicketAdapter.h"

int main()
{
    CONFIG->Init(CONFIG_PATH);
    LogHelper logHelper(CONFIG->GetInt(LOG_LEVEL), CONFIG->GetStr(LOG_PATH));

    TicketAdapter ticketAdapter(CONFIG->GetStr(DEVSN)/*"100200000001"*/,
        CONFIG->GetStr(DBAPI_SERVER));
    ticketAdapter.StartAnalyServer(CONFIG->GetStr(LISTEN_IP_FOR_ANALY),
        CONFIG->GetInt(LISTEN_PORT_FOR_ANALY)/*"0.0.0.0", 10001*/);
    ticketAdapter.StartFrontServer(CONFIG->GetStr(LISTEN_IP_FOR_FEP),
        CONFIG->GetInt(LISTEN_PORT_FOR_FEP)/*"0.0.0.0", 10002*/);

    pause();
    return 0;
}


