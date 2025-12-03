#pragma once

#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H
#include <windows.h>
#include <iostream>
#include <string>
#include <wininet.h>
#include <vector>


#pragma comment(lib, "wininet.lib")
using namespace std;
#define __HTTP_VERB_GET    "GET"
#define __HTTP_VERB_POST   "POST"
#define __HTTP_ACCEPT "Accept: */*\r\n"

//#define __HTTP_ACCEPT "Accept: application/json, text/plain, */*\r\n"
#define  AGENT  ("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 2.0.50727)")

//#define IE8_USER_AGENT "Mozilla/4.0 (compatible; )"

// IE8 on Windows 7
#define     IE8_USER_AGENT ("Mozilla/4.0 (Buff-Verify)")

// 数据下载
BOOL HttpDownload(string  pszDownloadUrl, BYTE** ppDownloadData, DWORD* pdwDownloadDataSize, string &error);

// 创建并保存文件
BOOL SaveToFile(const char* pszFileName, BYTE* pData, DWORD dwDataSize);
class Http {
public:
	Http(void);
	//~Http(void);

	enum requestType {reqGet, reqPost, reqPostMultipartsFormdata, download};
public:
	//创建连接
	bool CreateConnect(LPCTSTR lpszUrl, LPCTSTR szuser = NULL, LPCTSTR szpassword = NULL);
	bool Request(const char *mapUrl, const char * lpData, string &error, requestType type = reqPost);
	unsigned long Response(unsigned char *buf, unsigned long len);
	BOOL AddReqHeaders(LPCTSTR lpHeaders);                                          // 添加一个或多个HTTP请求头函数
	BOOL GetRespBodyData(CHAR * lpBuf, DWORD dwLen, DWORD& dwRecvLen);              // 获取HTTP响应消息体数据函数
	void CloseRequest();                                                            // 关闭HTTP请求函数



private:
	virtual bool RequestPost(const char *mapurl, const char * lpData, string &error);             //发送POST请求

	virtual bool RequestGet(const char *mapurl);                                    //发送GET请求





private:
	// 解析Url函数(协议，主机名，端口，文件路径)
	BOOL ParseURL(LPCTSTR lpszUrl, LPTSTR lpszScheme, DWORD dwSchemeLength, LPTSTR lpszHostName, DWORD dwHostNameLength, WORD& nPort, LPTSTR lpszUrlPath, DWORD dwUrlPathLength);
private:
	HINTERNET _hOpen; // internet open handle
	HINTERNET _hConnection; // internet connection handle
	HINTERNET _hRequest; // internet request handle
	std::string _szaddress;
	HANDLE    m_hCompleteEvent;             // 操作完成事件句柄
	HANDLE    m_hCancelEvent;               // 外部取消事件句柄

	DWORD     m_dwConnectTimeOut;           // 连接超时时间
	DWORD     m_dwContext;                  // 当前操作上下文
	WORD       nPort;
	std::vector<string> m_arrRespHeader;   // Http响应头数组

};

#endif // HTTPCLIENT_H