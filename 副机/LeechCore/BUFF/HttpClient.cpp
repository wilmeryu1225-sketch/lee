#include "HttpClient.h"
#include <tchar.h>
#define __HTTP_ACCEPT_TYPE "*/*"

Http::Http(void) {
	_hOpen = NULL;
	_hConnection = NULL;
	_hRequest = NULL;
	nPort = 0;

	m_hCompleteEvent = NULL;
	m_hCancelEvent = NULL;

	m_dwConnectTimeOut = 60 * 1000;
	m_dwContext = 0;
}
wstring s2ws(const string& s)
{
	string curLocale = setlocale(LC_ALL, NULL);  //curLocale="C"
	setlocale(LC_ALL, "chs");
	const char* source = s.c_str();
	size_t charNum = s.size() + 1;
	cout << "s.size():" << s.size() << endl;         //7：多字节字符串"ABC我们"有7个字节

	wchar_t* dest = new wchar_t[charNum];
	mbstowcs_s(NULL, dest, charNum, source, _TRUNCATE);
	wstring result = dest;
	delete[] dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}

bool Http::CreateConnect(LPCTSTR lpszUrl, LPCTSTR szuser, LPCTSTR szpassword)
{
	TCHAR szScheme[INTERNET_MAX_URL_LENGTH] = { 0 };
	TCHAR szHostName[INTERNET_MAX_URL_LENGTH] = { 0 };
	TCHAR szUrlPath[INTERNET_MAX_URL_LENGTH] = { 0 };
	//DWORD dwAccessType;
	//LPCTSTR lpszProxy;



	// 解析Url
	BOOL bRet = ParseURL(lpszUrl, szScheme, INTERNET_MAX_URL_LENGTH, szHostName, INTERNET_MAX_URL_LENGTH, nPort, szUrlPath, INTERNET_MAX_URL_LENGTH);
	if (!bRet)
		return FALSE;
	m_hCompleteEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);     // 创建事件句柄
	m_hCancelEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == m_hCompleteEvent || NULL == m_hCancelEvent)
	{
		CloseRequest();
		return FALSE;
	}
	bool ret = true;
	try
	{
		//初始化 打开一个网络会话
		_hOpen = ::InternetOpenA(IE8_USER_AGENT, // agent
			INTERNET_OPEN_TYPE_PRECONFIG, // access type
			NULL, // proxy
			NULL, // proxy by pass
			0); // flags
		if (_hOpen == NULL) {
			CloseRequest();
			return false;
		}


/*
		LPCTSTR lpctstr;
#ifdef UNICODE
		// 如果是 Unicode 编译模式，需要将 char* 转换为 wchar_t*
		printf("szHostName:%ws\n", szHostName);
#else
		// 如果是非 Unicode 编译模式，const char* 已经是 LPCTSTR 的一种形式
		printf("szHostName:%s\n", szHostName);
#endif // UNICODE*/
	
			

		//建立到远程服务器的连接
		_hConnection = ::InternetConnect(_hOpen, szHostName, nPort, szuser, szpassword, INTERNET_SERVICE_HTTP, 0, (DWORD_PTR)this);
		if (NULL == _hConnection)
		{
			if (::GetLastError() != ERROR_IO_PENDING)
			{
				CloseRequest();
				return FALSE;//连接失败
			}
		}






		if (::InternetAttemptConnect(NULL) != ERROR_SUCCESS) {
			CloseRequest();
			return FALSE;//连接失败
		}
	}
	catch (const char *szm)
	{
		ret = false;
		if (_hOpen || _hOpen != INVALID_HANDLE_VALUE)
			::InternetCloseHandle(_hOpen);
		if (_hConnection || _hConnection != INVALID_HANDLE_VALUE)
			::CloseHandle(_hConnection);
		printf("异常信息%s\n", szm);
	}



	//printf("建立网络链接：%d\n", (DWORD)_hConnection);
	return ret;
}

//发送网络请求
bool Http::Request(const char * mapUrl, const char * lpData, string &error, requestType type)
{
	bool bflag = false;
	try
	{
		switch (type)
		{
		case Http::reqGet:
			bflag = RequestGet(mapUrl);
			break;
		case Http::reqPost:
			bflag = RequestPost(mapUrl, lpData, error);//发送POST请求
			break;
		case Http::reqPostMultipartsFormdata:
			break;
		case Http::download:
			break;
		default:
			break;
		}
		if (!bflag) {
			CloseRequest();
			_hRequest = NULL;
		}
	}
	catch (const char *szme)
	{
		CloseRequest();
		_hRequest = NULL;
		printf("异常信息%s\n", szme);
	}
	catch (...) {
		CloseRequest();
		_hRequest = NULL;
	}
	return bflag;
}

//获取返回信息
unsigned long Http::Response(unsigned char * buf, unsigned long len)
{
	unsigned long nread = 0;
	try {
		if (!_hRequest)
			throw "connection failed...";

		if (!::InternetReadFile(_hRequest, buf, len, &nread))
			throw "response failed...";

	}
	catch (const char *szm) {
		::InternetCloseHandle(_hRequest);
		_hRequest = NULL;
		printf("异常信息%s\n", szm);

	}
	catch (...) {
		::InternetCloseHandle(_hRequest);
		_hRequest = NULL;

	}

	return nread;
}

BOOL Http::AddReqHeaders(LPCTSTR lpHeaders)
{
	if (NULL == _hRequest || NULL == lpHeaders)
		return FALSE;
	return ::HttpAddRequestHeaders(_hRequest, lpHeaders, sizeof(lpHeaders), HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);
}




//发送POST请求
bool Http::RequestPost(const char *mapurl, const char * lpData, string &error)
{
	LPCSTR rgpszAcceptTypes[] = { "*/*", NULL };
	//static LPCSTR szAcceptType = (__HTTP_ACCEPT_TYPE);
	//static LPCSTR szContentType = ("Content-Type: application/x-www-form-urlencoded\r\n");
	static LPCSTR szContentType = ("Content-Type: application/json;charset=UTF-8");
	unsigned char *buf = NULL;
	unsigned long len = 0;

	if (!_hConnection || _hConnection == INVALID_HANDLE_VALUE)
		return false;

	DWORD dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
	if (INTERNET_DEFAULT_HTTPS_PORT == nPort)
		dwFlags |= INTERNET_FLAG_SECURE;
	//用于创建一个 HTTP 请求句柄，以发送 HTTP 请求并接收服务器的响应
	_hRequest = ::HttpOpenRequestA(_hConnection, __HTTP_VERB_POST, // HTTP Verb
		mapurl, // Object Name    函数方法路径  后端 PostMapping 的路径名
		"HTTP/1.1", // Version  或 HTTP_VERSIONA
		NULL, // Reference
		rgpszAcceptTypes, // Accept Type															
		dwFlags,
		(DWORD_PTR)this); // context call-back point

	if (!_hRequest || _hRequest == INVALID_HANDLE_VALUE)
		return false;
	//printf("创建网络请求：%d\n", (DWORD)_hRequest);
	//DWORD dwFlags2 = 0;
/*
	DWORD dwError = 0;
	DWORD dwBuffLen = sizeof(dwFlags);
	InternetQueryOption(_hRequest, INTERNET_OPTION_SECURITY_FLAGS,
		(LPVOID)&dwFlags, &dwBuffLen);

	dwFlags |= SECURITY_FLAG_IGNORE_REVOCATION;
	InternetSetOption(_hRequest, INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)&dwFlags, sizeof(dwFlags));*/


	//关闭证书检查
	DWORD dwFlags2 = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
		SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
		SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
	InternetSetOption(_hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags2, sizeof(dwFlags2));




	//AddReqHeaders("Accept: application/json, text/plain, */*");
	//AddReqHeaders("Content-Type: application/json;charset=UTF-8 ");
	//AddReqHeaders("Accept-Language: zh-cn");
	//AddReqHeaders("Connection: Close");

	BOOL bRet = ::HttpSendRequestA(_hRequest, szContentType, (DWORD)strlen(szContentType), (LPVOID)lpData, (DWORD)strlen(lpData));
	if (!bRet)
	{
		if (::GetLastError() != ERROR_IO_PENDING) {}
		printf("发送请求失败：%d\n", GetLastError());


		error = to_string(GetLastError());
		return FALSE;
	}
	return true;
}

//发送GET请求
bool Http::RequestGet(const char *mapurl)
{
	static LPCSTR szAcceptType = (__HTTP_ACCEPT_TYPE);
	//static LPCSTR szContentType = ("Content-Type: application/x-www-form-urlencoded\r\n");
	static LPCSTR szContentType = ("Content-Type: application/json;charset=UTF-8");
	unsigned char *buf = NULL;
	unsigned long len = 0;
	if (!_hConnection || _hConnection == INVALID_HANDLE_VALUE)
		return false;
	DWORD dwFlags = 0;
	if (INTERNET_DEFAULT_HTTPS_PORT == nPort)
		dwFlags |= INTERNET_FLAG_SECURE;
	//用于创建一个 HTTP 请求句柄，以发送 HTTP 请求并接收服务器的响应
	_hRequest = ::HttpOpenRequestA(_hConnection, __HTTP_VERB_GET, // HTTP Verb
		mapurl, // Object Name    函数方法路径  后端 PostMapping 的路径名
		HTTP_VERSIONA, // Version  或 HTTP_VERSIONA
		NULL, // Reference
		&szAcceptType, // Accept Type															
		INTERNET_FLAG_NO_CACHE_WRITE |
		dwFlags,
		(DWORD_PTR)this); // context call-back point
	if (!_hRequest || _hRequest == INVALID_HANDLE_VALUE)
		return false;
	//DWORD dwFlags2 = 0;
	DWORD dwError = 0;
	DWORD dwBuffLen = sizeof(dwFlags); InternetQueryOption(_hRequest, INTERNET_OPTION_SECURITY_FLAGS,
		(LPVOID)&dwFlags, &dwBuffLen);
	dwFlags |= SECURITY_FLAG_IGNORE_REVOCATION;
	InternetSetOption(_hRequest, INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)&dwFlags, sizeof(dwFlags));

	//AddReqHeaders("Accept: application/json, text/plain, */*");
	//AddReqHeaders("Content-Type: */* ");
	//AddReqHeaders("Accept-Language: zh-cn");
	//AddReqHeaders("Connection: Close");
	BOOL bRet = ::HttpSendRequestA(_hRequest, NULL, 0, NULL, 0);
	if (!bRet)
	{
		if (::GetLastError() != ERROR_IO_PENDING)
			printf("错误%d\n", GetLastError());
		return FALSE;
	}
	return true;
}







BOOL Http::ParseURL(LPCTSTR lpszUrl, LPTSTR lpszScheme, DWORD dwSchemeLength, LPTSTR lpszHostName, DWORD dwHostNameLength, WORD & nPort, LPTSTR lpszUrlPath, DWORD dwUrlPathLength)
{

	URL_COMPONENTS stUrlComponents = { 0 };
	stUrlComponents.dwStructSize = sizeof(URL_COMPONENTS);
	stUrlComponents.lpszScheme = lpszScheme;
	stUrlComponents.dwSchemeLength = dwSchemeLength;
	stUrlComponents.lpszHostName = lpszHostName;
	stUrlComponents.dwHostNameLength = dwHostNameLength;
	stUrlComponents.lpszUrlPath = lpszUrlPath;
	stUrlComponents.dwUrlPathLength = dwUrlPathLength;

	BOOL bRet = ::InternetCrackUrl(lpszUrl, 0, 0, &stUrlComponents);
	if (bRet)
	{
		nPort = stUrlComponents.nPort;
	}
	return bRet;
}

BOOL Http::GetRespBodyData(CHAR * lpBuf, DWORD dwLen, DWORD & dwRecvLen)
{
	if (NULL == _hRequest || NULL == lpBuf || dwLen <= 0)
		return FALSE;

	INTERNET_BUFFERSA stInetBuf = { 0 };
	BOOL bRet;
	dwRecvLen = 0;
	memset(lpBuf, 0, dwLen);
	stInetBuf.dwStructSize = sizeof(stInetBuf);
	stInetBuf.lpvBuffer = lpBuf;
	stInetBuf.dwBufferLength = dwLen;

	bRet = ::InternetReadFileExA(_hRequest, &stInetBuf, 0, (DWORD_PTR)this);
	if (!bRet)
	{
		if (::GetLastError() != ERROR_IO_PENDING)
			return FALSE;

		//bRet = WaitForEvent(REQUEST_COMPLETE_EVENT, INFINITE);
		//if (!bRet)
		return FALSE;
	}
	else
	{
		//bRet = WaitForEvent(USER_CANCEL_EVENT, 0);
		//if (!bRet)
		return FALSE;
	}

	dwRecvLen = stInetBuf.dwBufferLength;

	return TRUE;


}

// 关闭HTTP请求函数
void Http::CloseRequest()
{
	if (m_hCompleteEvent != NULL)
	{
		::CloseHandle(m_hCompleteEvent);
		m_hCompleteEvent = NULL;
	}

	if (m_hCancelEvent != NULL)
	{
		::CloseHandle(m_hCancelEvent);
		m_hCancelEvent = NULL;
	}

	if (_hOpen)
	{
		::InternetCloseHandle(_hOpen);
		_hOpen = NULL;
	}

	if (_hConnection)
	{
		::InternetCloseHandle(_hConnection);
		_hConnection = NULL;
	}

	if (_hRequest)
	{
		::InternetCloseHandle(_hRequest);
		_hRequest = NULL;
	}


	m_arrRespHeader.clear();

	m_dwContext = 0;

}




BOOL UrlCrack(LPTSTR pszUrl, LPTSTR pszScheme, LPTSTR pszHostName, LPCTSTR pszUserName, LPCTSTR pszPassword, LPTSTR pszUrlPath, LPCTSTR pszExtraInfo, DWORD dwBufferSize)
{
	BOOL bRet = FALSE;
	URL_COMPONENTS uc = { 0 };

	// 初始化变量中的内容
	RtlZeroMemory(&uc, sizeof(uc));
/*
	RtlZeroMemory(pszScheme, dwBufferSize);
	RtlZeroMemory(pszHostName, dwBufferSize);
	RtlZeroMemory(pszUserName, dwBufferSize);
	RtlZeroMemory(pszPassword, dwBufferSize);
	RtlZeroMemory(pszUrlPath, dwBufferSize);
	RtlZeroMemory(pszExtraInfo, dwBufferSize);*/

	uc.dwStructSize = sizeof(URL_COMPONENTS);
	uc.lpszScheme = pszScheme;
	uc.dwSchemeLength = INTERNET_MAX_URL_LENGTH;
	uc.lpszHostName = pszHostName;
	uc.dwHostNameLength = INTERNET_MAX_URL_LENGTH;
	uc.lpszUrlPath = pszUrlPath;
	uc.dwUrlPathLength = INTERNET_MAX_URL_LENGTH;

	// 将长度填充到结构中
	/*uc.dwStructSize = sizeof(uc);
	uc.dwSchemeLength = dwBufferSize - 1;
	uc.dwHostNameLength = dwBufferSize - 1;
	uc.dwUserNameLength = dwBufferSize - 1;
	uc.dwPasswordLength = dwBufferSize - 1;
	uc.dwUrlPathLength = dwBufferSize - 1;
	uc.dwExtraInfoLength = dwBufferSize - 1;

	uc.lpszScheme = pszScheme;
	uc.lpszHostName = pszHostName;
	uc.lpszUserName = pszUserName;
	uc.lpszPassword = pszPassword;
	uc.lpszUrlPath = pszUrlPath;
	uc.lpszExtraInfo = pszExtraInfo;*/

	// 分解URL地址
	bRet = InternetCrackUrl(pszUrl, 0, 0, &uc);
	if (FALSE == bRet)
	{
		return bRet;
	}
	return bRet;
}



// 使用字符分割
void Stringsplit(const string& str, const char split, vector<string>& res)
{
	if (str == "")		return;
	//在字符串末尾也加入分隔符，方便截取最后一段
	string strs = str + split;
	size_t pos = strs.find(split);

	// 若找不到内容则字符串搜索函数返回 npos
	while (pos != strs.npos)
	{
		string temp = strs.substr(0, pos);
		res.push_back(temp);
		//去掉已分割的字符串,在剩下的字符串中进行分割
		strs = strs.substr(pos + 1, strs.size());
		pos = strs.find(split);
	}
}

//
bool isNumber(string s) {
	if (s == "")return false;
	for (char c : s) {
		if (!isdigit(c)) {
			return false;
		}
	}
	return true;
}
void replaceChar(std::string &str, char toReplace, const std::string &replacement) {
	size_t startPos = 0;
	while ((startPos = str.find(toReplace, startPos)) != std::string::npos) {
		str.erase(startPos, 1);
		str.insert(startPos, replacement);
		startPos += replacement.length();
	}
}
// 从响应信息头信息中获取数据内容长度大小
bool GetDownloadFileSize(string pResponseHeader, DWORD* FileSize) {
	//printf("开始\n  %s\n", pResponseHeader.c_str());
	vector<string> strList;
	Stringsplit(pResponseHeader, '\n', strList);
	int len = (int)strList.size();
	string size_s = "";
	//printf("数量：%d\n", len);
	for (int i = 0; i < len; i++) {
		//printf("解析：%s\n", strList[i].c_str());
		if (strList[i].find("Content-Length:") != -1)
		{
			size_s = strList[i].replace(0, 16, "");
			replaceChar(size_s, '\r', ""); // 将\r替换为空字符串
			break;
		}
	}
	//printf("结果：%s\n", size_s.c_str());
	*FileSize = 0;
	if (!isNumber(size_s))return false;
	*FileSize = stoi(size_s);
	//printf("返回大小：%d\n", stoi(size_s));
	return true;

}

BOOL GetContentLength(char* pResponseHeader, DWORD* pdwContentLength)
{
	printf("开始\n  %ws\n", pResponseHeader);



	int i = 0;
	char szContentLength[MAX_PATH] = { 0 };
	DWORD dwContentLength = 0;
	char szSubStr[] = "Content-Length: ";
	RtlZeroMemory(szContentLength, MAX_PATH);

	// 在传入字符串中查找子串
	char* p = strstr(pResponseHeader, szSubStr);
	if (NULL == p)
	{
		return FALSE;
	}
	printf("11111");

	p = p + 18;// lstrlen(szSubStr);

	// 如果找到了就提取出里面的纯数字
	while (('0' <= *p) && ('9' >= *p))
	{
		szContentLength[i] = *p;
		p++;
		i++;
	}

	// 字符串转数字
	dwContentLength = atoi(szContentLength);
	*pdwContentLength = dwContentLength;
	return TRUE;
}
void printProgressBar(float progress) {
	int barWidth = 50;  // 进度条的宽度
	std::cout << "[";
	int pos = barWidth * progress;  // 计算当前进度的位置
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) {
			std::cout << "█";  // 已完成部分，使用块字符填充
		}
		else {
			std::cout << " ";  // 未完成部分，使用空格
		}
	}
	std::cout << " ] " << int(progress * 100.0) << " %\r";  // 输出百分比并在同一行更新
	std::cout.flush();  // 刷新输出
}


string UTF8_To_string(const string &str);
// 数据下载
BOOL HttpDownload(string  pszDownloadUrl, BYTE** ppDownloadData, DWORD* pdwDownloadDataSize, string &error)
{

	//LPTSTR DownloadUrl = pszDownloadUrl.c_str();

	LPTSTR DownloadUrl;
#ifdef UNICODE
	// 如果是 Unicode 编译模式，需要将 char* 转换为 wchar_t*
	std::wstring wstr = std::wstring(pszDownloadUrl.begin(), pszDownloadUrl.end());
	DownloadUrl = const_cast<LPTSTR>(wstr.c_str());
	//printf("szHostName:%ws\n", DownloadUrl);
#else
	// 如果是非 Unicode 编译模式，const char* 已经是 LPCTSTR 的一种形式
	DownloadUrl = const_cast<LPTSTR>(pszDownloadUrl.c_str());
	//printf("szHostName:%s\n", DownloadUrl);
#endif // UNICODE*/





	// 定义HTTP子变量
	TCHAR szScheme[INTERNET_MAX_URL_LENGTH] = { 0 };
	TCHAR szHostName[INTERNET_MAX_URL_LENGTH] = { 0 };
	TCHAR szUserName[INTERNET_MAX_URL_LENGTH] = { 0 };
	TCHAR szPassword[INTERNET_MAX_URL_LENGTH] = { 0 };
	TCHAR szUrlPath[INTERNET_MAX_URL_LENGTH] = { 0 };
	TCHAR szExtraInfo[INTERNET_MAX_URL_LENGTH] = { 0 };

	// 填充为空
	RtlZeroMemory(szScheme, INTERNET_MAX_URL_LENGTH);
	RtlZeroMemory(szHostName, INTERNET_MAX_URL_LENGTH);
	RtlZeroMemory(szUserName, INTERNET_MAX_URL_LENGTH);
	RtlZeroMemory(szPassword, INTERNET_MAX_URL_LENGTH);
	RtlZeroMemory(szUrlPath, INTERNET_MAX_URL_LENGTH);
	RtlZeroMemory(szExtraInfo, INTERNET_MAX_URL_LENGTH);

	// 拆解URL地址
	if (FALSE == UrlCrack(DownloadUrl, szScheme, szHostName, szUserName, szPassword, szUrlPath, szExtraInfo, MAX_PATH))
	{
		error = "拆解URL地址失败";
		return FALSE;
	}

	// 数据下载
	HINTERNET hInternet = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;
	DWORD dwOpenRequestFlags = 0;
	BOOL bRet = FALSE;
	 char* pResponseHeaderIInfo = NULL;
	DWORD dwResponseHeaderIInfoSize = 2048;
	BYTE* pBuf = NULL;
	DWORD dwBufSize = 64 * 1024;
	BYTE* pDownloadData = NULL;
	DWORD dwDownloadDataSize = 0;
	DWORD dwRet = 0;
	DWORD dwOffset = 0;
	bool download = true;

	do
	{
		// 建立会话
		hInternet = InternetOpenA("WinInetGet/0.1", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
		if (NULL == hInternet)
		{
			error = "建立会话失败";
			break;
		}

		// 建立连接
		hConnect = InternetConnect(hInternet, szHostName, INTERNET_DEFAULT_HTTP_PORT, szUserName, szPassword, INTERNET_SERVICE_HTTP, 0, 0);
		if (NULL == hConnect)
		{
			error = "建立连接失败";
			break;
		}

		// 打开并发送HTTP请求
		dwOpenRequestFlags = INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
			INTERNET_FLAG_KEEP_CONNECTION |
			INTERNET_FLAG_NO_AUTH |
			INTERNET_FLAG_NO_COOKIES |
			INTERNET_FLAG_NO_UI |
			INTERNET_FLAG_RELOAD;
		if (0 < lstrlen(szExtraInfo))
		{
			lstrcat(szUrlPath, szExtraInfo);
		}

		LPCTSTR HTTP_VERB_GET;
         #ifdef UNICODE
         		// 如果是 Unicode 编译模式，需要将 char* 转换为 wchar_t*
         		HTTP_VERB_GET = _T("GET");
         		//printf("szHostName:%ws\n", szHostName);
         #else
         		// 如果是非 Unicode 编译模式，const char* 已经是 LPCTSTR 的一种形式
         		//printf("szHostName:%s\n", szHostName);
         		HTTP_VERB_GET = "GET";
         #endif // UNICODE*/
		 

		// 以GET模式打开请求
		hRequest = HttpOpenRequest(hConnect, HTTP_VERB_GET, szUrlPath, NULL, NULL, NULL, dwOpenRequestFlags, 0);
		if (NULL == hRequest)
		{
			error = "以GET模式打开请求失败";
			break;
		}

		// 发送请求
		bRet = HttpSendRequest(hRequest, NULL, 0, NULL, 0);
		if (FALSE == bRet)
		{
			error = "发送请求失败";
			break;
		}
		// 接收响应的报文信息头(Get Response Header)
		pResponseHeaderIInfo = new  char[dwResponseHeaderIInfoSize];
		if (NULL == pResponseHeaderIInfo)
		{
			break;
		}
		RtlZeroMemory(pResponseHeaderIInfo, dwResponseHeaderIInfoSize);

		// 查询HTTP请求头
		bRet = HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, pResponseHeaderIInfo, &dwResponseHeaderIInfoSize, NULL);
		if (FALSE == bRet)
		{
			error = "查询HTTP请求头失败";
			break;
		}




		// 从字符串中 "Content-Length: " 网页获取数据长度
		bRet = GetDownloadFileSize(pResponseHeaderIInfo, &dwDownloadDataSize);
		//printf("大小：%d\n", dwDownloadDataSize);
		// 输出完整响应头
		//std::cout << pResponseHeaderIInfo << std::endl;
		if (FALSE == bRet)
		{
			error = "获取文件大小失败:1";
			break;
		}
		// 接收报文主体内容(Get Response Body)
		pBuf = new BYTE[dwBufSize];
		if (NULL == pBuf)
		{
			error = "创建缓冲区失败:2";
			break;
		}
		pDownloadData = new BYTE[dwDownloadDataSize];
		if (NULL == pDownloadData)
		{
			error = "创建接收缓冲区失败:3";
			break;
		}
		RtlZeroMemory(pDownloadData, dwDownloadDataSize);
		do
		{
			RtlZeroMemory(pBuf, dwBufSize);

			// 循环读入数据并保存在变量中
			bRet = InternetReadFile(hRequest, pBuf, dwBufSize, &dwRet);
			if (FALSE == bRet)
			{
				//printf("1111\n");
				break;
			}
			RtlCopyMemory((pDownloadData + dwOffset), pBuf, dwRet);
			dwOffset = dwOffset + dwRet;

			int sssss = dwOffset * 100 / dwDownloadDataSize;
			printf("已下载：%d %% \n", sssss);
			//printf("已完成：%d %% \n", sssss);
		} while (dwDownloadDataSize > dwOffset);

		// 返回数据
		*ppDownloadData = pDownloadData;
		*pdwDownloadDataSize = dwDownloadDataSize;
	} while (FALSE);

	// 关闭并释放资源
	if (NULL != pBuf)
	{
		delete[]pBuf;
		pBuf = NULL;
	}
	if (NULL != pResponseHeaderIInfo)
	{
		delete[]pResponseHeaderIInfo;
		pResponseHeaderIInfo = NULL;
	}
	if (NULL != hRequest)
	{
		InternetCloseHandle(hRequest);
		hRequest = NULL;
	}
	if (NULL != hConnect)
	{
		InternetCloseHandle(hConnect);
		hConnect = NULL;
	}
	if (NULL != hInternet)
	{
		InternetCloseHandle(hInternet);
		hInternet = NULL;
	}

	//printf("状态：%s\n", UTF8_To_string((char*)pDownloadData).c_str());

	if (!download)

	{



		if (pDownloadData != NULL)error = UTF8_To_string((char*)pDownloadData);

		return FALSE;

	}
	return bRet;
}





// 创建并保存文件
BOOL SaveToFile(const char* pszFileName, BYTE* pData, DWORD dwDataSize)
{
	// 创建空文件
	HANDLE hFile = CreateFileA(pszFileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}

	DWORD dwRet = 0;

	// 写出数据到文件
	WriteFile(hFile, pData, dwDataSize, &dwRet, NULL);

	// 关闭句柄保存文件
	CloseHandle(hFile);

	return TRUE;
}
