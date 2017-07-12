/*
 * TicketAdapter.cpp
 *
 *  Created on: 2016年12月1日
 *      Author: wong
 */

#include "TicketAdapter.h"

#include <cstring>
#include <sstream>
#include <curl/curl.h>

#include "base/Logging.h"

#include "CCMS.h"
#include "BATNetSDKAPI.h"
#include "SZTBTicketAPI.h"



TicketAdapter::TicketAdapter(const std::string& sn, const std::string& dbServer)
: m_hFront(-1)
, m_hAnaly(-1)
, m_hCurl(NULL)
, m_dbServer(dbServer)
{
    BATNetSDK_Init(CCMS_DEVTYPE_ADAPTER808, (char*)sn.c_str(), true);
    curl_global_init(CURL_GLOBAL_ALL);
    m_hCurl = curl_easy_init();
    curl_easy_setopt(m_hCurl, CURLOPT_WRITEFUNCTION, TicketAdapter::CurlCB);
    curl_easy_setopt(m_hCurl, CURLOPT_WRITEDATA, this);
}

TicketAdapter::~TicketAdapter()
{
    if (m_hFront != -1)
    {
        BATNetSDK_DeleteObj(m_hFront);
    }
    if (m_hAnaly != -1)
    {
        BATNetSDK_DeleteObj(m_hAnaly);
    }
    BATNetSDK_Release();

    curl_easy_cleanup(m_hCurl);
    curl_global_cleanup();
}

int TicketAdapter::StartFrontServer(const std::string& ip, uint16_t port)
{
    LOG_INFO << "StartFrontServer " << ip << " " << port;
    CCMS_NETADDR addr = { {0}, 0};
    memcpy(addr.chIP, ip.c_str(), ip.length());
    addr.nPort = port;
    m_hFront = BATNetSDK_CreateServerObj(&addr);
    BATNetSDK_SetMsgCallBack(m_hFront, FrontRecvCB, this);
    BATNetSDK_SetConnCallBack(m_hFront, FrontConnCB, this);
    BATNetSDK_SetDataFlowCallBack(m_hFront, FrontDataFlowCB, this);
    BATNetSDK_Start(m_hFront);
    return 0;
}

int TicketAdapter::StartAnalyServer(const std::string& ip, uint16_t port)
{
    LOG_INFO << "StartAnalyServer " << ip << " " << port;
    CCMS_NETADDR addr = { {0}, 0};
    memcpy(addr.chIP, ip.c_str(), ip.length());
    addr.nPort = port;
    m_hAnaly = BATNetSDK_CreateServerObj(&addr);
    BATNetSDK_SetMsgCallBack(m_hAnaly, AnalyRecvCB, this);
    BATNetSDK_Start(m_hAnaly);
    return 0;
}

int TicketAdapter::OnFrontRecv(int sessionId, int msgId, const char* buf, int len)
{
    LOG_TRACE << msgId << " " << len << " " << buf;
    switch (msgId)
    {
    case CCMS_BASESCHEDULEDATA_MSG:
    case CCMS_BASESCHEDULE_DETAILDATA_MSG:
    case CCMS_BASESCHEDULE_PRICEDATA_MSG:
    case CCMS_SCHEDULEDATA_MSG:
    case CCMS_SCHEDULE_DETAILDATA_MSG:
    case CCMS_SENDSCHEDULEDATA_MSG:
    case CCMS_SENDSCHEDULE_DETAILDATA_MSG:
    case CCMS_ORDERUPLOADDATA_MSG:
    case CCMS_ORDERUPLOAD_TICKETDATA_MSG:
        if (len < SZTBTicket_GetPackBufLen(msgId))
        {
            LOG_ERROR << "error len = " << len << ". msgId=" << msgId;
        }
        else
        {
            SendToAnaly(msgId, buf, len);
        }
        break;
    default:
        LOG_ERROR << "error msg = " << msgId << " " << len << " " << buf;
        break;
    }
    return 0;
}

int TicketAdapter::OnFrontConn(int sessionId, const char* sn, int status)
{
    LOG_INFO << "OnFrontConn status = " << status;
    std::string url = "http://" + m_dbServer + "/KeGuanJu/api/deviceInfo/statusInfo?sn=" + sn + "&status=";
    switch (status)
    {
    case 2: // CS_Login
        url += "1";
        break;
    case 1: // CS_Disconnect
    case 3: // CS_Logout
    case 4: // CS_Timeout
        url += "0";
        break;
    default:
        url.clear();
        break;
    }
    if (!url.empty())
    {
        LOG_INFO << "OnFrontConn " << url.c_str();
        curl_easy_setopt(m_hCurl, CURLOPT_URL, url.c_str());
        curl_easy_perform(m_hCurl);
    }
    return 0;
}

int TicketAdapter::OnFrontDataFlow(int sessionId, const char* sn, int recvbytes)
{
    std::stringstream url;
    url << "http://" << m_dbServer << "/KeGuanJu/api/deviceInfo/dataFlow?sn="
        << sn << "&last_minute_transmit_size=" << recvbytes;
    LOG_INFO << "OnFrontDataFlow " << url.str();
    curl_easy_setopt(m_hCurl, CURLOPT_URL, url.str().c_str());
    curl_easy_perform(m_hCurl);
    return 0;
}

int TicketAdapter::OnAnalyRecv(int sessionId, int msgId, const char* buf, int len)
{
    LOG_TRACE << msgId << " " << len << " " << buf;
    switch (msgId)
    {
    case CCMS_BASESCHEDULEDATA_RESPMSG:
    case CCMS_BASESCHEDULE_DETAILDATA_RESPMSG:
    case CCMS_BASESCHEDULE_PRICEDATA_RESPMSG:
    case CCMS_SCHEDULEDATA_RESPMSG:
    case CCMS_SCHEDULE_DETAILDATA_RESPMSG:
    case CCMS_SENDSCHEDULEDATA_RESPMSG:
    case CCMS_SENDSCHEDULE_DETAILDATA_RESPMSG:
    case CCMS_ORDERUPLOADDATA_RESPMSG:
    case CCMS_ORDERUPLOAD_TICKETDATA_RESPMSG:
        {
            CCMS_TICKET_RESP package;
            if (SZTBTicket_ParseTicketResp(buf, len, &package) != 0)
            {
                LOG_ERROR << "ParseTicketResp error. " << msgId << " " << len;
            }
            else
            {
                SendToFront(msgId, package);
            }
        }
        break;
    default:
        LOG_ERROR << "error msgId=" << msgId << " " << len;
        break;
    }
    return 0;
}

int TicketAdapter::SendToFront(int msgId, const T_CCMS_TICKET_RESP& package)
{
    const std::string& sn = ARRAY2STR(package.chFServerSN);
    LOG_INFO << "SendToFront " << sn << " " << msgId;
    if (sn.empty())
    {
        return -1;
    }
    int len = SZTBTicket_GetPackBufLen(msgId);
    if (len < 0)
    {
        return -1;
    }
    char* buf = new char[len];
    int ret = SZTBTicket_PackTicketResp(&package, buf, &len);
    if (ret != 0)
    {
        SAFE_DELETEA(buf);
        return -1;
    }
    ret = BATNetSDK_SendBySN(m_hFront, sn.c_str(), msgId, buf, len);
    SAFE_DELETEA(buf);
    return ret;
}

int TicketAdapter::SendToAnaly(int msgId, const char* buf, int len)
{
    LOG_INFO << "SendToAnaly " << msgId;
    return BATNetSDK_SendByDevType(m_hAnaly, CCMS_DEVTYPE_ANALYSIS, msgId, buf, len);
}

size_t TicketAdapter::OnCurlResponse(void* buffer, size_t size, size_t nmemb)
{
    LOG_INFO << std::string((char*)buffer, size * nmemb).c_str();
    return size * nmemb;
}
