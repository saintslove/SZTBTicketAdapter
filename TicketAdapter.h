/*
 * TicketAdapter.h
 *
 *  Created on: 2016年12月1日
 *      Author: wong
 */

#ifndef TICKETADAPTER_H_
#define TICKETADAPTER_H_

#include <stdint.h>
#include <string>
#include <curl/curl.h>

struct T_CCMS_TICKET_RESP;

class TicketAdapter
{
public:
    TicketAdapter(const std::string& sn, const std::string& dbServer);
    virtual ~TicketAdapter();

public:
    int StartFrontServer(const std::string& ip, uint16_t port);
    int StartAnalyServer(const std::string& ip, uint16_t port);

private: // FrontServer
    static int FrontRecvCB(int sessionId, int msgId, const char* buf, int len, void* userdata)
    {
        TicketAdapter* that = reinterpret_cast<TicketAdapter*>(userdata);
        return that->OnFrontRecv(sessionId, msgId, buf, len);
    }
    int OnFrontRecv(int sessionId, int msgId, const char* buf, int len);

    static int FrontConnCB(int sessionId, const char* sn, int status, void* userdata)
    {
        TicketAdapter* that = reinterpret_cast<TicketAdapter*>(userdata);
        return that->OnFrontConn(sessionId, sn, status);
    }
    int OnFrontConn(int sessionId, const char* sn, int status);

    static int FrontDataFlowCB(int sessionId, const char* sn, int recvbytes, void* userdata)
    {
        TicketAdapter* that = reinterpret_cast<TicketAdapter*>(userdata);
        return that->OnFrontDataFlow(sessionId, sn, recvbytes);
    }
    int OnFrontDataFlow(int sessionId, const char* sn, int recvbytes);

private: // AnalyServer
    static int AnalyRecvCB(int sessionId, int msgId, const char* buf, int len, void* userdata)
    {
        TicketAdapter* that = reinterpret_cast<TicketAdapter*>(userdata);
        return that->OnAnalyRecv(sessionId, msgId, buf, len);
    }
    int OnAnalyRecv(int sessionId, int msgId, const char* buf, int len);

private:
    static size_t CurlCB(void* buffer, size_t size, size_t nmemb, void* userp)
    {
        TicketAdapter* that = reinterpret_cast<TicketAdapter*>(userp);
        return that->OnCurlResponse(buffer, size, nmemb);
    }
    size_t OnCurlResponse(void* buffer, size_t size, size_t nmemb);

private:
    int SendToFront(int msgId, const T_CCMS_TICKET_RESP& package);
    int SendToAnaly(int msgId, const char* buf, int len);

private:
    int m_hFront;
    int m_hAnaly;
    CURL* m_hCurl;
    std::string m_dbServer;
};

#endif /* TICKETADAPTER_H_ */
