/*
 * ConfigDefine.h
 *
 *  Created on: 2016年12月7日
 *      Author: wong
 */

#ifndef CONFIGDEFINE_H_
#define CONFIGDEFINE_H_

// 配置文件的路径
#define CONFIG_PATH "./SZTBTicketAdapter.conf"

// 配置文件中用到的key值定义
#define LOG_LEVEL   "log_level"     // 日志级别
#define LOG_PATH    "log_path"      // 日志路径
#define DEVSN       "devsn"         // 设备SN
#define LISTEN_IP_FOR_FEP       "listen_ip_for_fep"     // 用于前置机连接的监听IP
#define LISTEN_PORT_FOR_FEP     "listen_port_for_fep"   // 用于前置机连接的监听端口
#define LISTEN_IP_FOR_ANALY     "listen_ip_for_analy"   // 用于分析服务器连接的监听IP
#define LISTEN_PORT_FOR_ANALY   "listen_port_for_analy" // 用于分析服务器连接的监听端口
#define DBAPI_SERVER            "dbapi_server"          // 数据库API服务器

#endif /* CONFIGDEFINE_H_ */
