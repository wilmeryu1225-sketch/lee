


#include "buffApi.h"
#include "HttpClient.h"
#include <sstream>
#include <iomanip>
#include <time.h>
#include <sys/timeb.h>
#include <atlstr.h>
#include "Rc4.h"
//using namespace std;




#define RequestURL      _T("https://www.buffdata.cn")

//#define RequestURL      _T("http://127.0.0.1:8090")

#define 引号 "\""

#define CloudFlag       "/api/validate/CloudFlag"   //获取云下发标识
#define JavaScript_     "/api/validate/javascript"//单码调用远程JS
#define Timestamp       "/api/validate/timestamp"
#define Special         "/api/validate/Special"
#define singleLogin     "/api/validate/login"
#define singleTwoLogin  "/api/validate/twologin"
#define notice          "/api/validate/notice"
#define version         "/api/validate/version"
#define isVersion       "/api/validate/isversion"
#define Expiretime      "/api/validate/expiretime"
#define Expiretimes     "/api/validate/expiretimes"
#define Updatacode      "/api/validate/updatacode"
#define Updataip        "/api/validate/updataip"
#define Variable        "/api/validate/variable"
#define CloudVar        "/api/validate/CloudVar"
#define userData        "/api/validate/setuserdata"
#define GetuserData     "/api/validate/getuserdata"
#define Blacklist       "/api/validate/blacklist"
#define Updata          "/api/validate/updata"
#define appData         "/api/validate/appdata"
#define varData         "/api/validate/vardata"
						
#define isUse           "/api/validate/isuse"
#define Status          "/api/validate/statusA"
#define Localip         "/api/validate/localip"
#define cardType        "/api/validate/cardType"

#define queryPoints     "/api/validate/queryPoints"
#define subPoints       "/api/validate/subPoints"
#define cardLgininfo    "/api/validate/cardLgininfo"


#define user_CloudFlag  "/api/validate/user/CloudFlag"   //获取云下发标识
#define JavaScript_user "/api/validate/user/javascript"//注册码调用远程JS
#define user_reg        "/api/validate/user/reg"
#define user_rec        "/api/validate/user/recharge"
#define user_login      "/api/validate/user/login"
#define user_expire     "/api/validate/user/expiretime"
#define user_expires    "/api/validate/user/expiretimes"
#define user_updatacode "/api/validate/user/updatacode"
#define user_updataip   "/api/validate/user/updataip"
#define user_var        "/api/validate/user/variable"
#define user_CloudVar   "/api/validate/user/CloudVar"
#define user_setdata    "/api/validate/user/setuserdata"
#define user_getdata    "/api/validate/user/getuserdata"
#define user_status     "/api/validate/user/statusA"

#define user_MailCode   "/api/validate/GetMailCode"
#define user_password   "/api/validate/user/updata/password"

#define max_Valid        120
#define mix_Valid        -120
#define notInitiali    "zrSz9cq8u68="// "未初始化";

int g_ErrorTime = 0;
string g_key;
//=================================================== Base64编码   开始=================================================================
#pragma region 
static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(const char * bytes_to_encode, int in_len) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';

	}

	return ret;

}

std::string base64_decode(std::string const& encoded_string) {
	int in_len = (int)encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = (int)base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = (int)base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}
#pragma endregion
//=================================================== Base64编码   结束=================================================================

//=================================================== Buff工具类   开始=================================================================
#pragma region 
// 使用异或运算进行加密
void xorEncrypt(BYTE* data, int length, char key) {
	for (int i = 0; i < length; ++i) {
		data[i] ^= key;
	}
}
// XOR 加密函数
void xorEncryptDecrypt(BYTE* data, size_t dataLength, const char* key, size_t keyLength) {
	for (size_t i = 0; i < dataLength; ++i) {
		// 对每个字节与密钥的对应字节进行异或操作
		data[i] = data[i] ^ key[i % keyLength];
	}
}
//获取本地时间
int GetlocalTime(int ErrorTime) {
	time_t localTime = time(NULL);
	return (int)localTime- ErrorTime;
}

void RandomSeed()
{
	struct timeb timer;
	//srand((int)time(0));  // 产生随机种子  把0换成NULL也行
	//srand((unsigned)timer.time * 1000 + timer.millitm);//毫秒种子
	ftime(&timer);
	srand((unsigned)timer.time * 1000 + timer.millitm);//毫秒种子
}
int Random(int max, int mix)
{
	return rand() % (mix - max + 1) + max;
}
string  GetRandomText(int length)
{
	//置随机数种子();
	RandomSeed();
	string strRand;
	string strTmp = "1234567890QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm";
	for (int i = 0; i < length; i++)
	{
		int j = rand() % 40;
		strRand += strTmp.at(j);
	}
	return strRand;
}



string string_To_UTF8(const string &str) {
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	wchar_t* pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
	ZeroMemory(pwBuf, nwLen * 2 + 2);
	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.length(), pwBuf, nwLen);
	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
	char* pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);
	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
	string retStr(pBuf);
	delete[]pwBuf;
	delete[]pBuf;

	pwBuf = NULL;
	pBuf = NULL;
	return retStr;
}
string UTF8_To_string(const string &str) {
	int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t* pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
	memset(pwBuf, 0, nwLen * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), pwBuf, nwLen);
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
	char* pBuf = new char[nLen + 1];
	memset(pBuf, 0, nLen + 1);
	WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
	string retStr = pBuf;
	delete[]pBuf;
	delete[]pwBuf;

	pBuf = NULL;
	pwBuf = NULL;
	return retStr;
}
string GetText_left(const std::string& text, size_t len) {
	return (text.substr(0, len));
}
string GetText_right(const std::string& text, size_t len) {
	if (text.empty()) return "";
	if (len >= text.size()) return text;
	return (text.substr(text.length() - len));
}
string GetText_center(const std::string& text, size_t start, size_t len) {
	return (text.substr(start - 1, len));
}
string dec2hex(int i, int width)
{
	stringstream ioss;     //定义字符串流
	string s_temp;         //存放转化后字符
	ioss << setiosflags(ios::uppercase) << hex << i;      //以十六制形式输出
	ioss >> s_temp;
	if (width > (int)s_temp.size())
	{
		string s_0(width - s_temp.size(), '0');      //位数不够则补0
		s_temp = s_0 + s_temp;                            //合并
	}
	string s = s_temp.substr(s_temp.length() - width, s_temp.length());    //取右width位
	return s;
}
static vector<string> splitEx(const string& src, string separate_character)
{
	vector<string> strs;
	int separate_characterLen = (int)separate_character.size();//分割字符串的长度,这样就可以支持如“,,”多字符串的分隔符 
	int lastPosition = 0, index = -1;
	while (-1 != (index = (int)src.find(separate_character, lastPosition)))
	{
		strs.push_back(src.substr(lastPosition, index - lastPosition));
		lastPosition = index + separate_characterLen;
	}
	string lastString = src.substr(lastPosition);//截取最后一个分隔符后的内容 
	if (!lastString.empty())
		strs.push_back(lastString);//如果最后一个分隔符后还有内容就入队 
	return strs;
}
string to_sendJson(string data, string data2) {

	string json = "{\"data\":\"" + base64_encode(data.c_str(),data.length()) + "\","
		"\"data2\":\"" + base64_encode(data2.c_str(), (int)data2.length()) 
		+ "\"}";

/*
	string json = "{\"data\":\"" + data + "\","
		"\"data2\":\"" +data2 + "\","
		"\"data3\":\"" +data3
		+ "\"}";*/

	return json;
}
string toJson(string str) {
	string json = "{";
	vector<string>dataArr = splitEx(str, "&");
	int len = (int)dataArr.size();
	for ( int i = 0; i < len; i++)
	{
		vector<string>item = splitEx(dataArr[i], "=");
		int item_len = (int)item.size();
		if (item_len == 1)
		{
			json = json + 引号 + item[0] + 引号 + ":" + 引号 + 引号 + ",";
		}
		else
		{
			json = json + 引号 + item[0] + 引号 + ":" + 引号 + item[1] + 引号 + ",";
		}
	}
	json = GetText_left(json, json.length() - 1);
	json = json + "}";
	return json;
}

/*
string removeSpecificText(string str,string s) {
	
	int len = s.length();

	int j = (int)str.find(s, 0);
	while (j != -1) {
		str.replace(j, len, "");
		j = (int)str.find(s, 0);
	}
	return str;
}*/

//判断IP是否合法
bool isIPAddressValid(const char* pszIPAddr)
{
	if (!pszIPAddr) return false; //若pszIPAddr为空  
	char IP1[100], cIP[4];
	int len = (int)strlen(pszIPAddr);
	int i = 0, j = len - 1;
	int k, m = 0, n = 0, num = 0;
	//去除首尾空格(取出从i-1到j+1之间的字符):  
	while (pszIPAddr[i++] == ' ');
	while (pszIPAddr[j--] == ' ');
	for (k = i - 1; k <= j + 1; k++)
	{
		IP1[m++] = *(pszIPAddr + k);
	}
	IP1[m] = '\0';
	char *p = IP1;

	while (*p != '\0')
	{
		if (*p == ' ' || *p<'0' || *p>'9') return false;
		cIP[n++] = *p; //保存每个子段的第一个字符，用于之后判断该子段是否为0开头  

		int sum = 0;  //sum为每一子段的数值，应在0到255之间  
		while (*p != '.'&&*p != '\0')
		{
			if (*p == ' ' || *p<'0' || *p>'9') return false;
			sum = sum * 10 + *p - 48;  //每一子段字符串转化为整数  
			p++;
		}
		if (*p == '.') {
			if ((*(p - 1) >= '0'&&*(p - 1) <= '9') && (*(p + 1) >= '0'&&*(p + 1) <= '9'))//判断"."前后是否有数字，若无，则为无效IP，如“1.1.127.”  
				num++;  //记录“.”出现的次数，不能大于3  
			else
				return false;
		};
		if ((sum > 255) || (sum > 0 && cIP[0] == '0') || num > 3) return false;//若子段的值>255或为0开头的非0子段或“.”的数目>3，则为无效IP  

		if (*p != '\0') p++;
		n = 0;
	}
	if (num != 3) return false;
	return true;
}


ULONG GetMac_()
{
	//获取机器码
	char m_Volume[256];//卷标名  
	char m_FileSysName[256];
	DWORD   m_SerialNum;//序列号  
	DWORD   m_FileNameLength;
	DWORD   m_FileSysFlag;
	::GetVolumeInformationA("C:\\",
		m_Volume,
		256,
		&m_SerialNum,
		&m_FileNameLength,
		&m_FileSysFlag,
		m_FileSysName,
		256);
	return m_SerialNum;
}
Result_ ParsingData2(string str) {
	Result_ ret;
	str.replace(0, 7, "");
	//去除")"并返回
	int j = (int)str.find(")", 0);
	while (j != -1) {
		str.replace(j, 1, "");
		j = (int)str.find(")", 0);
	}
	vector<string> strs = splitEx(str, ",");
	int len = (int)strs.size();
	if (len<3) {
		ret.statusCode = str;
		return ret;
	}
	for (int i = 0; i < len; i++) {

		if (strs[i].find("expirationTime") != -1)
		{
			ret.expirationTime = strs[i].replace(0, 15, "");
			continue;
		}
		if (strs[i].find("statusCode") != -1)
		{
			ret.statusCode = strs[i].replace(0, 12, "");
			continue;
		}
		if (strs[i].find("deduct") != -1)
		{
			ret.deduct = strs[i].replace(0,8, "");
			continue;
		}
	}
	return ret;




}
Result ParsingData(string str, string sign,int time) {
	Result ret;
	vector<string> strs = splitEx(str, "|");
	int len = (int)strs.size();
	//printf("长度：%d\n", len);
	if (len<5) {
		if (str.find("对不起,操作失败,请联系管理") != -1) {
			ret.code = "0";
			ret.msg = "执行方法时发生异常,请联系管理";
			ret.sign = sign;
			ret.time = time;
			return ret;
		}

		ret.code = "0";
		ret.msg = str;
		ret.sign = sign;
		ret.time = time;
		return ret;
	}

		if (strs[0].find("code") != -1) {
			ret.code = strs[0].replace(0, 5, "");
		}
		if (strs[1].find("msg") != -1) {
			string str = strs[1].replace(0, 4, "");
			ret.msg = str;
			
		}

		if (strs[2].find("sign") != -1) {
			string str = strs[2].replace(0, 5, "");
			ret.sign = str;
			
		}

		if (strs[3].find("time") != -1) {
			string str = strs[3].replace(0, 5, "");
			ret.time = atoi(str.c_str());
			
		}

		if (strs[4].find("data") != -1) {  
			string str = strs[4].replace(0, 5, "");
			if (len > 5) {
				string obj_text= str;
				for (int j = 0; j < len - 4; j++) {
					obj_text = obj_text + strs[4 + j] + "|";
				}
				ret.data = obj_text;
			}
			else {
				ret.data = str;

			}
		
		}
	return ret;
}

Result Send(LPCSTR mapUrl, string sendText,string sign) {
	Result result;
	result.code = "0";
	Http http;
	string error;
	bool networkStatus = true;
	for (int i=0;i<5;i++)
	{
		if (!http.CreateConnect(RequestURL)) {
			result.msg = "网络初始化失败...";
			result.sign = sign;
			result.time = GetlocalTime(g_ErrorTime);
			http.CloseRequest();
			networkStatus = false;
			Sleep(5000);
			goto repeat;
		}
		repeat:
		//printf("请求路径：%s\n", mapUrl);
		if (!http.Request(mapUrl, sendText.c_str(), error)) {
			result.msg = "发送请求失败,编号：" + error;
			result.sign = sign;
			result.time =  GetlocalTime(g_ErrorTime);
			http.CloseRequest();
			networkStatus = false;
			Sleep(5000);
		}
		else
		{
			networkStatus = true;
			break;
		}
	}
	if (!networkStatus) {
		return result;
	}


	char buf[1024] = "\0";
	DWORD len;
	http.GetRespBodyData(buf, 1024, len);
	//printf("返回数据：%s\n",buf);
	string data= UTF8_To_string(buf);
	
	vector<string> strs = splitEx(data, "|");
	 len = (DWORD)strs.size();
	string status;
	if (len==2) {
		//printf("左边文本:%s\n", strs[0].c_str());
		status=strs[0];
		if (!status.compare("200")) {//返回成功 - 加密
			Help::Rc4 rc4;
			string key_ = g_key + sign;
			string str = rc4.Decrypt(strs[1].c_str(), key_.c_str());
			//printf("解密数据：%s\n", UTF8_To_string(str).c_str());
			result = ParsingData(UTF8_To_string(str), sign,  GetlocalTime(g_ErrorTime));
		}
		else//返回信息错误
		{
			result.code = "0";
			result.msg = strs[1];
			result.sign = sign;
			result.time =  GetlocalTime(g_ErrorTime);
			result.data = buf;
			
		}
	}
	else
	{
		result.code = "0";
		result.msg = data;
		result.sign = sign;
		result.time =  GetlocalTime(g_ErrorTime);
		
	}





	//printf("code: %s    msg:%s  sign:%s   data:%s  \n", result.code.c_str(), result.msg.c_str(), result.sign.c_str(), result.data.c_str());
/*
	if (!result.code.compare("1")) {//成功标志
		http.CloseRequest();
		return result;
	}*/

	/*
		printf("转换：%s\n", str.c_str());
		cout << "测试" << unicodeToCHI2(str).c_str() << endl;
		Result info = GetReturnData(str);
		printf("code: %s    msg:%s   data:%s  \n", info.code.c_str(), info.msg.c_str(), info.data.c_str());
	*/
	http.CloseRequest();
	return result;
}
#pragma endregion
//=================================================== Buff工具类   结束=================================================================


//解析返回释义
string ReturntoDefinition(string number) {
	if (!number.compare("400")) {
		return "成功";
	}
	else if (!number.compare("-1")) {
		return "软件不存在";
	}
	else if (!number.compare("-2")) {
		return "当前软件已停用";
	}
	else if (!number.compare("-3")) {
		return "版本不存在";
	}
	else if (!number.compare("-4")) {
		return "程序版本不是最新";
	}
	else if (!number.compare("-5")) {
		return "版本已停用";
	}
	else if (!number.compare("-6")) {
		return "您已被添加到黑名单";
	}
	else if (!number.compare("-7")) {
		return "远程变量不存在";
	}
	else if (!number.compare("-8")) {
		return "获取公告失败";
	}
	else if (!number.compare("-9")) {
		return "未设置更新地址";
	}
	else if (!number.compare("-10")) {
		return "未设置程序数据";
	}
	else if (!number.compare("-11")) {
		return "未设置版本数据";
	}
	else if (!number.compare("-12")) {
		return "还未登录";
	}
	else if (!number.compare("-13")) {
		return "用户数据设置成功";
	}
	else if (!number.compare("-14")) {
		return "添加成功";
	}
	else if (!number.compare("-15")) {
		return "强制下线";
	}
	else if (!number.compare("-16")) {
		return "未设置软件版本";
	}
	else if (!number.compare("-17")) {
		return "未设置用户数据";
	}
	else if (!number.compare("-18")) {
		return "已被顶下线";
	}
	else if (!number.compare("-19")) {
		return "未在绑定的 IP 登录";
	}
	else if (!number.compare("-20")) {
		return "未在绑定电脑登录";
	}
	else if (!number.compare("-21")) {
		return "未开启机器码验证,无需转绑";
	}
	else if (!number.compare("-22")) {
		return "未开启IP地址验证,无需转绑";
	}
	else if (!number.compare("-23")) {
		return "重绑次数超过限制";
	}
	else if (!number.compare("-24")) {
		return "登录版本不一致";
	}
	else if (!number.compare("-25")) {
		return "登录用户已达上限";
	}
	else if (!number.compare("-26")) {
		return "注册码卡密不能用于单码登陆";
	}
	else if (!number.compare("-27")) {
		return "单码卡密不能用于充值";
	}
	else if (!number.compare("-28")) {
		return "卡密已退卡无法使用";
	}
	else if (!number.compare("-100")) {
		return "卡密不存在";
	}
	else if (!number.compare("-101")) {
		return "卡密已删除";
	}
	else if (!number.compare("-102")) {
		return "卡密已被封停";
	}
	else if (!number.compare("-103")) {
		return "卡密已到期";
	}
	else if (!number.compare("-104")) {
		return "卡密未使用";
	}
	else if (!number.compare("-105")) {
		return "卡密已使用";
	}
	else if (!number.compare("-106")) {
		return "未查询到卡密信息";
	}
	else if (!number.compare("-107")) {
		return "机器码一致 无需转绑";
	}
	else if (!number.compare("-108")) {
		return "ip一致无需转绑";
	}
	else if (!number.compare("-200")) {
		return "充值卡密不存在";
	}
	else if (!number.compare("-201")) {
		return "充值卡密与账号不符";
	}
	else if (!number.compare("-202")) {
		return "充值卡密已使用";
	}
	else if (!number.compare("-300")) {
		return "用户名已存在";
	}
	else if (!number.compare("-301")) {
		return "昵称已存在";
	}
	else if (!number.compare("-302")) {
		return "邮箱已存在";
	}
	else if (!number.compare("-303")) {
		return "用户编号设置错误";
	}
	else if (!number.compare("-304")) {
		return "用户不存在";
	}
	else if (!number.compare("-305")) {
		return "密码不正确";
	}
	else if (!number.compare("-306")) {
		return "新密码输入错误";
	}
	else if (!number.compare("-307")) {
		return "用户已被封停";
	}
	else if (!number.compare("-308")) {
		return "用户已到期";
	}
	else if (!number.compare("-309")) {
		return "账号已删除";
	}
	else if (!number.compare("-310")) {
		return "注册配置不存在";
	}
	else if (!number.compare("-311")) {
		return "未开启注册功能";
	}
	else if (!number.compare("-312")) {
		return "用户状态正常";
	}
	else if (!number.compare("-313")) {
		return "注册用户达到上限";
	}
	else if (!number.compare("-314")) {
		return "充值成功!填写的推荐人不存在";
	}
	else if (!number.compare("-315")) {
		return "充值成功!填写推荐人获赠时间失败";
	}
	else if (!number.compare("-316")) {
		return "充值成功!添加推荐信息失败";
	}
	else if (!number.compare("-317")) {
		return "用户名格式不正确";
	}
	else if (!number.compare("-318")) {
		return "密码格式不正确";
	}
	else if (!number.compare("-319")) {
		return "QQ格式不正确";
	}
	else if (!number.compare("-320")) {
		return "机器码格式不正确";
	}
	else if (!number.compare("-321")) {
		return "邮箱格式不正确";
	}
	else if (!number.compare("-322")) {
		return "昵称格式不正确";
	}
	else if (!number.compare("-323")) {
		return "邮箱验证码不存在";
	}
	else if (!number.compare("-324")) {
		return "邮箱验证码不正确";
	}
	else if (!number.compare("-325")) {
		return "邮箱和账户不匹配";
	}
	else if (!number.compare("-401")) {
		return "作者使用时间已到期";
	}
	else if (!number.compare("-402")) {
		return "操作频繁,请重试!";
	}
	else if (!number.compare("-403")) {
		return "系统禁止登录!";
	}
	else if (!number.compare("-404")) {
		return "网络链接失败";
	}
	else if (!number.compare("-405")) {
		return "错误的参数,请检查参数是否正确.";
	}
	else if (!number.compare("-406")) {
		return "已被顶下线";
	}
	else if (!number.compare("-407")) {
		return "状态码有误";
	}
	else if (!number.compare("-408")) {
		return "注册用户达到系统上限";
	}
	else if (!number.compare("-409")) {
		return "该功能为付费功能，将软件改为收费即可使用";
	}
	else if (!number.compare("-410")) {
		return "生成一机一码失败";
	}
	else if (!number.compare("-411")) {
		return "作者余额不足";
	}
	else if (!number.compare("-412")) {
		return "点卡不足";
	}
	else {
		return number;
	}
}

//解析卡密类型   1小时卡 2六时卡 3半天卡  4天卡  5周卡  6半月卡  7月卡  8季卡  9半年卡  10年卡   11永久卡
string Buff::ReturntoCardType(string type) {
	if (type=="1") {
		return "小时卡";
	}
	else if (type == "2") {
		return "六时卡";
	}
	else if (type == "3") {
		return "半天卡";
	}
	else if (type == "4") {
		return "天卡";
	}
	else if (type == "5") {
		return "周卡";
	}
	else if (type == "6") {
		return "半月卡";
	}
	else if (type == "7") {
		return "月卡";
	}
	else if (type == "8") {
		return "季卡";
	}
	else if (type == "9") {
		return "半年卡";
	}
	else if (type == "10") {
		return "年卡";
	}
	else if (type == "11") {
		return "永久卡";
	}
	else {
		return type;
	}
	
}


//取随机数
ULONG64 GetRandom(ULONG64  a, ULONG64  b)
{
	ULONG64 t1 = __rdtsc();
	ULONG64  v = (((((t1 * (a ^ t1) + t1 * (b * a)) * 0x17) / 7) % (b - a + 1)) + a);
	return v;
}

//结束当前进程
void EndCurrentProcess() {
	TerminateProcess(GetCurrentProcess(), 0);
	while (1)
		malloc(4096);
}

//=================================================== Buff单码类   开始=================================================================
#pragma region 


Buff::Buff() {
	this->initial = false;
}
string Buff::GetMachineCode()
{
	ULONG Mac = GetMac_();
	//MD5
	HCRYPTPROV hProv;
	HCRYPTPROV hHash = NULL;
	DWORD Send_size = 4;
	string Md5;
	if (::CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
		{
			if (CryptHashData(hHash, (const BYTE*)&Mac, Send_size, 0))
			{
				CryptGetHashParam(hHash, HP_HASHVAL, NULL, &Send_size, 0);
				PBYTE hnnn = (PBYTE)malloc(Send_size);
				if (CryptGetHashParam(hHash, HP_HASHVAL, hnnn, &Send_size, 0) && hnnn != NULL)
				{
					string tempStr;
					for (DWORD i = 0; i < Send_size; i++)
					{
						//tempStr.Format("%02X", *(hnnn + i));
						tempStr = dec2hex(*(hnnn + i), 2);
						Md5 += tempStr;
					}
					free(hnnn);

					CryptDestroyHash(hHash);
					CryptReleaseContext(hProv, 0);

				}
				else
				{
					CryptDestroyHash(hHash);
					CryptReleaseContext(hProv, 0);
				}
			}
			else
			{
				CryptDestroyHash(hHash);
				CryptReleaseContext(hProv, 0);
			}
		}
		else
		{
			CryptReleaseContext(hProv, 0);
		}
	}
	return Md5;
}


//初始化 time 参数  真 效验本地时间和服务器时间差 要要求上下不能超过60秒    假 不效验时间   默认真
bool Buff::initialize(string sname, string versionID, string machineCode, string uuid, string key, bool time_,string &error) {
	this->sname = sname;
	this->versionID = versionID;
	this->machineCode = machineCode;
	this->uuid = uuid;
	this->key= key;
	g_key = key;
	if (sname.empty()) {
		error = "软件名不能为空";
		return false;
	}
	if (versionID.empty()) {
		error = "版本号不能为空";
		return false;
	}
	if (machineCode.empty()) {
		error = "机器码不能为空";
		return false;
	}
	if (uuid.empty()) {
		error = "软件uuid不能为空";
		return false;
	}
	if (key.empty()) {
		error = "软件密钥不能为空";
		return false;
	}
	this->initial = true;
	if (time_) {
		string info;
		if (!GetTimestamp(info)) {
			error = info;
			this->initial = false;
			return false;
		}
		int localTime =  GetlocalTime(g_ErrorTime);
		g_ErrorTime = localTime - atoi(info.c_str());
/*
		if (Valid < mix_Valid || Valid>max_Valid) {
			error = "本地时间和服务器时间相差过大，请将时间同步到北京时间!";
			this->initial = false;
			return false;
		}*/
	}
	return true;
}

//下载更新文件
bool Buff::DownloadUpdataFile(string url, BYTE ** ppDownloadData, DWORD * pdwDownloadDataSize, string &error)
{
	
	return HttpDownload(url.c_str(), ppDownloadData, pdwDownloadDataSize, error);
	//return true;
}


//获取云下发标识   成功返回 文件标志  5分钟内有效且只能使用一次
bool Buff::GetCloudFileflag( string url, string& code, string& encrypt)
{
	if (!this->initial) {
		code = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (!this->loginStatus) {
		code = "请先登录";
		return false;
	}
	if (this->cardKey.length() != 32) {
		code = "未登录";
		return false;
	}
	string aeskey = GetRandomText(16);//
	string params = "sname=" + sname + "&uuid=" + this->uuid + "&cardKey=" + this->cardKey + "&remarks=" + aeskey + "&url=" + url + "&timestamp=" + to_string(GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());
	//printf("加密：%s", EncryptedData.c_str());
	string sendText = to_sendJson(EncryptedData, this->uuid);
	Result result = Send(CloudFlag, sendText.c_str(), aeskey);
	//printf("代码：%s    消息：%s    数据：%s\n", result.code.c_str(), result.msg.c_str(), result.data);
	if (result.sign.compare(aeskey)) {
		code = "非法数据";
		return false;
	}
	int Valid = result.time - GetlocalTime(g_ErrorTime);
	if (Valid < mix_Valid || Valid>max_Valid) {
		code = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		//printf("获取授权:%s\n", result.data.c_str());

		/*	code = result.data;
			return true;*/
		vector<string> strs = splitEx(result.data.c_str(), "#");
		int len = (int)strs.size();
		if (len < 2) {
			code = result.data;
			return false;
		}
		code = strs[0];
		encrypt = strs[1];
		//code = result.data;
		return true;
	}
	code = result.msg;
	return false;


}


bool Buff::DownloadCloudFile(string url, BYTE ** ppDownloadData, DWORD * pdwDownloadDataSize, string &error)
{
	if (!this->initial) {

		error = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (!this->loginStatus)
	{
		error = "请先登录";
		return false;
	}
	if (url.empty()) {
		error = "下载地址不能为空";
		return false;
	}



	string info, encrypt;
	BOOL Results = GetCloudFileflag( url, info, encrypt);
	if (!Results)
	{
		error = info;
		return false;
	}
	string newUrl = url + "&uuid=" + info + "&newfile=1";
	//printf("组合路径：%s\n", newUrl.c_str());
	// HTTP下载 

	if (TRUE == HttpDownload(newUrl.c_str(), ppDownloadData, pdwDownloadDataSize, error))
	{
		//判断开启了加密
		if (encrypt != "0") {
			std::string key = this->statusCode;
			size_t keyLength = key.length();
			int key_ = atoi(encrypt.c_str());
			xorEncryptDecrypt(*ppDownloadData, *pdwDownloadDataSize, key.c_str(), keyLength);
			xorEncrypt(*ppDownloadData, *pdwDownloadDataSize, key_ % 255);
			xorEncrypt(*ppDownloadData, *pdwDownloadDataSize, key_);
		}
		system("cls");
		//std::cout << "已保存文件,长度: " << *pdwDownloadDataSize << " 字节" << std::endl;
		return true;
	}
	//下载更新文件
	return false;

}




//获取时间戳   用于数据包时效判断 为了避免本地时间和服务器时间不一致 故而采用先获取 服务器时间然后计算
bool  Buff::GetTimestamp(string & info) {
	if (!this->initial) {

		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params = "sname=" + this->sname + "&uuid=" + uuid + "&remarks=" + aeskey + "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
	string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(Timestamp, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
	/*int Valid = result.time -  GetlocalTime(g_ErrorTime);
		if (Valid < mix_Valid || Valid>max_Valid) {
			info = "数据包过期";
			return false;
		}*/
		info = result.data;
		return true;
	}

	info = result.msg;
	return false;

}


//获取专码     根据IP计算出专属编码，可用于配置文件名等，从而减少特征
bool Buff::GetSpecialCode(int len, string &code)
{
	if (!this->initial) {
		expirationTime = -1;
		code = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (len<2) {
		len = 2;
	}
	if (len>20) {
		len = 20;
	}
	string aeskey = GetRandomText(16);
	string params = "sname=" + sname + "&uuid=" + uuid + "&status=" + to_string(len) +"&remarks="+ aeskey + "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());
	//printf("加密：%s", EncryptedData.c_str());
	string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(Special, sendText.c_str(),aeskey);
	//printf("代码：%s    消息：%s    数据：%s\n", result.code.c_str(), result.msg.c_str(), result.data);
	if (result.sign.compare(aeskey)) {
		code = "非法数据";
		return false;
	}
	int Valid = result.time -  GetlocalTime(g_ErrorTime);
	if (Valid < mix_Valid || Valid>max_Valid) {
		code = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		code = result.data;
		return true;
	}
	code = result.msg;
	return false;


}
//单码登录
bool Buff::SingleLogin(string cardKey, string &Response, int &expirationTime) {

	if (!this->initial) {
		expirationTime = -1;
		Response = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (cardKey.empty()) {
		expirationTime = -1;
		Response = "卡密不能为空";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params = "cardKey=" + cardKey + "&sname=" + sname  + "&version=" + versionID + "&machineCode=" + machineCode + "&uuid=" + uuid + "&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson((params));
	Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());
	//printf("加密：%s\n", EncryptedData.c_str());
	//string aa = rc4.Decrypt(EncryptedData.c_str(), key.c_str());
	//printf("解密：%s\n", aa.c_str());
	string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(singleLogin, sendText.c_str(), aeskey);
	//printf("代码：%s    消息：%s    数据：%s\n", result.code.c_str(), result.msg.c_str(), result.data);
	if (result.sign.compare(aeskey)) {
	    expirationTime = -1;
		Response = "非法数据";
		return false;
	}
    int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		expirationTime = -1;
		Response = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		Result_ ret= ParsingData2(result.data);
		expirationTime = atoi(ret.expirationTime.c_str());
		Response = ret.statusCode;

		this->loginStatus = true;
		this->cardKey = cardKey;
		this->statusCode = ret.statusCode;;
		this->loginTime = GetTickCount();
		this->expirationTime = expirationTime;
		return true;
	}
	expirationTime = -1;
	Response = result.msg;
	return false;
}

//单码二次登录
bool Buff::SingleTwologin(string cardKey,string statusCode, string &Response, int &expirationTime) {

	if (!this->initial) {
		expirationTime = -1;
		Response = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (cardKey.empty()) {
		expirationTime = -1;
		Response = "卡密不能为空";
		return false;
	}
	if (statusCode.empty()) {
		expirationTime = -1;
		Response = "状态码不能为空";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "cardKey=" + cardKey +"&statusCode="+ statusCode+ "&sname=" + sname + "&version=" + versionID + "&machineCode=" + machineCode + "&uuid=" + uuid + "&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson((params));
	Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());
	//printf("加密：%s\n", EncryptedData.c_str());
	//string aa = rc4.Decrypt(EncryptedData.c_str(), key.c_str());
	//printf("解密：%s\n", aa.c_str());
	string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(singleTwoLogin, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		expirationTime = -1;
		Response = "非法数据";
		return false;
	}
    int Valid = result.time -  GetlocalTime(g_ErrorTime);
	if (Valid < mix_Valid || Valid>max_Valid) {
		expirationTime = -1;
		Response = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		Result_ ret = ParsingData2(result.data);
		expirationTime = atoi(ret.expirationTime.c_str());
		Response = ret.statusCode;

		this->loginStatus = true;
		this->cardKey = cardKey;
		this->statusCode = ret.statusCode;;
		this->loginTime = GetTickCount();
		this->expirationTime = expirationTime;
		return true;
	}
	expirationTime = -1;
	Response = result.msg;
	return false;
}


//合法性检测  - 此函数不联网 必须在登录以后使用建议周期调用
bool Buff::LegitimacyTesting() {
	//效验是否登录
	if (!loginStatus) {
		exit(0);
		EndCurrentProcess();
		while (true) {
			malloc(4096);
		}
		ExitThread(-2);
		ExitProcess(0);
		Sleep(5000000);
	}
	ULONG64 time=   this->expirationTime * 60 - (int)((GetTickCount() - this->loginTime) / 1000);
	if (time<1)
	{
		exit(0);
		EndCurrentProcess();
		while (true) {
			malloc(4096);
		}
		ExitThread(-2);
		ExitProcess(0);
		Sleep(5000000);

	}
	return true;
}
//查询剩余点数
bool Buff::GetRemainingPoints( string & info)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params = "sname=" + (sname)+"&uuid=" + uuid + "&remarks=" + aeskey + "&timestamp=" + to_string( GetlocalTime(g_ErrorTime))+"&cardKey="+ this->cardKey;
	string data = toJson(params);
	Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
	string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(queryPoints, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
	int Valid = result.time -  GetlocalTime(g_ErrorTime);
	if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//扣除卡密点数
bool Buff::subCardPoints( int dropCount, string & info)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (dropCount<1) {
		info = "扣除点数不能为负数";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params = "sname=" + (sname)+"&uuid=" + uuid + "&remarks=" + aeskey + "&timestamp=" + to_string( GetlocalTime(g_ErrorTime)) + "&cardKey=" + this->cardKey +"&dropCount="+to_string(dropCount);
	string data = toJson(params);
	Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
	string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(subPoints, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
	int Valid = result.time -  GetlocalTime(g_ErrorTime);
	if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//获取登录信息
bool Buff::GetLogininfo_card(string machineCode, string & cardKey, string & statusCode,string &version_)
{
	if (!this->initial) {
		cardKey = base64_decode(notInitiali);// "未初始化";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "sname=" + (sname)+"&uuid=" + uuid + "&remarks=" + aeskey + "&timestamp=" + to_string( GetlocalTime(g_ErrorTime)) + "&machineCode=" + machineCode;
	string data = toJson(params);
	Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
	string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(cardLgininfo, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		cardKey = "非法数据";
		return false;
	}
	int Valid = result.time -  GetlocalTime(g_ErrorTime);
	if (Valid < mix_Valid || Valid>max_Valid) {
		cardKey = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		vector<string> strs = splitEx(result.data, "#");
		if (strs.size()<3) {
			cardKey = result.data;
		}
		cardKey = strs[0];
		statusCode = strs[1];
		version_ = strs[2];

		this->loginStatus = true;
		this->cardKey= strs[0];
		this->statusCode = strs[1];
		this->versionID = strs[2];
		return true;
	}
	cardKey = result.msg;
	return false;
}



//获取公告
bool Buff::GetNotice(string & Notice)
{
	if (!this->initial) {
		Notice = base64_decode(notInitiali);// "未初始化";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "sname=" + (sname)+"&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data= toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(notice, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		Notice = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		Notice = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		Notice = result.data;
		return true;
	}
	Notice = result.msg;
	return false;
}
//获取最新版本号
bool Buff::GetNewVersion(string & Version) {
	if (!this->initial) {
		Version = base64_decode(notInitiali);// "未初始化";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
	string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(version, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		Version = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		Version = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		Version = result.data;
		return true;
	}
	Version = result.msg;
	return false;
}
//检测是否最新版  1 最新版, 2 不是最新版
bool Buff::IsNewVersion(string & status)
{
	if (!this->initial) {
		status = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params ="versionID="+ this->versionID + "&sname=" + sname + "&uuid=" + uuid + "&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(isVersion, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		status = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		status = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		status = result.data;
		return true;
	}
	status = result.msg;
	return false;
}
//获取到期时间
bool Buff::GetExpireTime( string &expiretime) {
	if (!this->initial) {
		expiretime = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (!this->loginStatus)
	{
		expiretime = "请先登录";
		return false;
	}
	if (this->cardKey.length() != 32)
	{
		expiretime = "未登录";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params = "cardKey=" + this->cardKey +"&statusCode="+ this->statusCode+ "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(Expiretime, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		expiretime = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		expiretime = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		expiretime = result.data;
		return true;
	}
	expiretime = result.msg;
	return false;
}
//获取到期时间_分钟
bool Buff::GetExpireTime_minute( string & expiretime) {

	if (!this->initial) {
		expiretime = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (!this->loginStatus)
	{
		expiretime = "请先登录";
		return false;
	}
	if (this->cardKey.length() != 32)
	{
		expiretime = "未登录";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params = "cardKey=" + this->cardKey + "&statusCode=" + this->statusCode + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(Expiretimes, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		expiretime = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		expiretime = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		expiretime = result.data;
		return true;
	}
	expiretime = result.msg;
	return false;

}
////解绑机器码
bool Buff::UnbindMachineCode(string cardKey, string NewMachineCode, string &info, string &deduct) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (cardKey.empty()) {
		info = "卡密不能为空";
		return false;
	}
	if (NewMachineCode.empty()) {
		info = "新机器码不能为空";
		return false;
	}
	if (NewMachineCode.length()!=32) {
		info = "新机器码不正确";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "cardKey=" + cardKey  +"&machineCode=" + NewMachineCode+"&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(Updatacode, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		deduct = "0";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		deduct = "0";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		Result_ ret = ParsingData2(result.data);
		info = ret.statusCode;
		deduct = ret.deduct;
		return true;
	}
	info = result.msg;
	deduct = "0";
	return false;

}
//解绑IP
bool Buff::UnbindIP(string cardKey, string NewIP, string & info, string &deduct) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (cardKey.empty()) {
		info = "卡密不能为空";
		return false;
	}
	if (NewIP.empty()) {
		info = "新IP不能为空";
		return false;
	}
	if (!isIPAddressValid(NewIP.c_str())) {
		info = "IP地址不合法";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "cardKey=" + cardKey + "&ip=" + NewIP + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(Updataip, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		deduct = "0";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		deduct = "0";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		Result_ ret = ParsingData2(result.data);
		info = ret.statusCode;
		deduct = ret.deduct;
		return true;
	}
	info = result.msg;
	deduct = "0";
	return false;
}
//获取远程变量数据
bool Buff::GetRemoteVariables( string varNumber, string varName, string & info) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (varNumber.empty()) {
		info = "变量编号不能为空";
		return false;
	}
	if (varName.empty()) {
		info = "变量名不能为空";
		return false;
	}
	if (!this->loginStatus)
	{
		info = "请先登录";
		return false;
	}
	if (this->cardKey.length() != 32)
	{
		info = "未登录";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params = "cardKey=" + this->cardKey + "&statusCode=" + this->statusCode + "&varName="+ varName+"&varNumber="+ varNumber+"&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(Variable, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}

//获取云变量数据
bool Buff::GetCloudVarInfo( string varNumber, string varName, string & info)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (varNumber.empty()) {
		info = "变量编号不能为空";
		return false;
	}
	if (varName.empty()) {
		info = "变量名不能为空";
		return false;
	}
	if (!this->loginStatus)
	{
		info = "请先登录";
		return false;
	}
	if (this->cardKey.length() != 32)
	{
		info = "未登录";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params = "cardKey=" + this->cardKey + "&statusCode=" + this->statusCode + "&varName=" + varName + "&varNumber=" + varNumber + "&sname=" + sname + "&uuid=" + uuid + "&remarks=" + aeskey + "&timestamp=" + to_string(GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
	string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(CloudVar, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
	int Valid = result.time - GetlocalTime(g_ErrorTime);
	if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}




//设置用户数据
bool Buff::SetUserdata( string userdata, string & info) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (userdata.length() > 200) {
		info = "用户数据过长";
		return false;
	}
	if (!this->loginStatus)
	{
		info = "请先登录";
		return false;
	}
	if (this->cardKey.length() != 32)
	{
		info = "未登录";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params = "cardKey=" + this->cardKey + "&statusCode=" + this->statusCode + "&userData=" + userdata + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(userData, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//获取用户数据
bool Buff::GetUserdata( string & info) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (!this->loginStatus)
	{
		info = "请先登录";
		return false;
	}
	if (this->cardKey.length() != 32)
	{
		info = "未登录";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "cardKey=" + this->cardKey + "&statusCode=" + this->statusCode  + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(GetuserData, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//添加黑名单 IP会自动添加到黑名单
bool Buff::AddBlacklist(string & info, string cardKey, string machineCode, string remarks) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "cardKey=" + cardKey + "&machineCode=" + machineCode +"&remarks="+ remarks+ "&sname=" + sname + "&uuid=" + uuid +"&ipAnalysis=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(Blacklist, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//获取更新地址
bool Buff::GetUpdateAddress(string cardKey, string & info) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
/*
	if (cardKey.empty()) {
		info = "卡密不能为空";
		return false;
	}*/

	string aeskey = GetRandomText(16);
	string params = "cardKey=" + cardKey + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(Updata, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//获取程序数据
bool Buff::GetAppData(string & info) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(appData, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//获取版本数据
bool Buff::GetVersionData(string & info) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "versionID="+ this->versionID+"&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(varData, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//查询是否使用  0 未使用 1已使用
bool Buff::QueryWhetherTouse(string cardKey, string & info) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (cardKey.empty()) {
		info = "卡密不能为空";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "cardKey=" + cardKey + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
	string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(isUse, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}


//查询卡密类型    1小时卡 2六时卡 3半天卡  4天卡  5周卡  6半月卡  7月卡  8季卡  9半年卡  10年卡   11永久卡
bool Buff::QueryCardType(string cardKey, string & info) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (cardKey.empty()) {
		info = "卡密不能为空";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params = "cardKey=" + cardKey + "&sname=" + sname + "&uuid=" + uuid + "&remarks=" + aeskey + "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
	string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(cardType, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
	int Valid = result.time -  GetlocalTime(g_ErrorTime);
	if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}




//获取用户状态
bool Buff::GetUserStatus(  string & info) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (!this->loginStatus)
	{
		info = "请先登录";
		return false;
	}
	if (this->cardKey.length() != 32)
	{
		info = "未登录";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "cardKey=" + this->cardKey + "&statusCode="+ this->statusCode+"&machineCode="+ this->machineCode +"&sname=" + sname +"&loginVersion=" + versionID + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(Status, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//获取本机IP
bool Buff::GetlocalIP(string & info) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params =  "sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(Localip, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//解析返回释义
string Buff::ReturntoDefinition_(string number)
{
	return ReturntoDefinition(number);
}

//调用远程JS算法
bool Buff::RunJScode(string jsName, string params, string & info)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (jsName.empty()) {
		info = "JS函数名不能为空";
		return false;
	}
	if (jsName.length()>20) {
		info = "JS函数名过长";
		return false;
	}
	if (!params.empty() || params.length()>0) {
		//处理中文符号 把中文,替换成英文,
		int j = (int)params.find("，", 0);
		while (j != -1) {
			params.replace(j, 1, ",");
			j = (int)params.find("，", 0);
		}
		//分割文本 判断有多少个参数 
		vector<string> strs = splitEx(params, ",");
		if ((int)strs.size() > 20) {
			info = "JS方法参数不能大于20个";
			return false;
		}
	}
	if (!this->loginStatus)
	{
		info = "请先登录";
		return false;
	}
	if (this->cardKey.length() != 32)
	{
		info = "未登录";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params_ = "cardKey=" + this->cardKey + "&sname=" + this->sname +  "&machineCode=" + this->machineCode +"&javascript=" + jsName +"&params="+ params+ "&uuid="+ uuid + "&remarks=" + aeskey + "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params_);
	Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
	string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(JavaScript_, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
	int Valid = result.time -  GetlocalTime(g_ErrorTime);
	if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
#pragma endregion
//=================================================== Buff单码类   结束=================================================================






//=================================================== Buff用户类   开始=================================================================
#pragma region 
Buff_user::Buff_user() {
	this->initial = false;
}
//下载更新文件   url 下载链接    ppDownloadData 返回的文件数据    pdwDownloadDataSize 返回文件大小
bool Buff_user::DownloadUpdataFile(string url, BYTE ** ppDownloadData, DWORD * pdwDownloadDataSize, string & error)
{
	return HttpDownload(url.c_str(), ppDownloadData, pdwDownloadDataSize, error);
	
}
bool Buff_user::DownloadCloudFile(string url, BYTE ** ppDownloadData, DWORD * pdwDownloadDataSize, string & error)
{
	if (!this->initial) {

		error = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (username.empty()) {
		error = "用户名不能为空";
		return false;
	}
	string info, encrypt;
	BOOL Results = GetCloudFileflag( url, info, encrypt);
	if (!Results)
	{
		error = info;
		return false;
	}
	string newUrl = url + "&uuid=" + info + "&newfile=1";
	//printf("组合路径：%s\n", newUrl.c_str());
	// HTTP下载 
	if (TRUE == HttpDownload(newUrl.c_str(), ppDownloadData, pdwDownloadDataSize, error))
	{
		//判断开启了加密
		if (encrypt != "0") {
			std::string key = this->statusCode;
			size_t keyLength = key.length();
			int key_ = atoi(encrypt.c_str());
			xorEncryptDecrypt(*ppDownloadData, *pdwDownloadDataSize, key.c_str(), keyLength);
			xorEncrypt(*ppDownloadData, *pdwDownloadDataSize, key_ % 255);
			xorEncrypt(*ppDownloadData, *pdwDownloadDataSize, key_);
		}
		std::cout << "已保存文件,长度: " << *pdwDownloadDataSize << " 字节" << std::endl;
		return true;
	}
	//下载更新文件
	return false;
}
string Buff_user::GetMachineCode()
{
	ULONG Mac = GetMac_();
	//MD5
	HCRYPTPROV hProv;
	HCRYPTPROV hHash = NULL;
	DWORD Send_size = 4;
	string Md5;
	if (::CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
		{
			if (CryptHashData(hHash, (const BYTE*)&Mac, Send_size, 0))
			{
				CryptGetHashParam(hHash, HP_HASHVAL, NULL, &Send_size, 0);
				PBYTE hnnn = (PBYTE)malloc(Send_size);
				if (CryptGetHashParam(hHash, HP_HASHVAL, hnnn, &Send_size, 0) && hnnn != NULL)
				{
					string tempStr;
					for (DWORD i = 0; i < Send_size; i++)
					{
						//tempStr.Format("%02X", *(hnnn + i));
						tempStr = dec2hex(*(hnnn + i), 2);
						Md5 += tempStr;
					}
					free(hnnn);

					CryptDestroyHash(hHash);
					CryptReleaseContext(hProv, 0);

				}
				else
				{
					CryptDestroyHash(hHash);
					CryptReleaseContext(hProv, 0);
				}
			}
			else
			{
				CryptDestroyHash(hHash);
				CryptReleaseContext(hProv, 0);
			}
		}
		else
		{
			CryptReleaseContext(hProv, 0);
		}
	}
	return Md5;
}
//初始化
bool Buff_user::initialize(string sname, string versionID, string machineCode, string uuid,string key, bool time_, string &error) {
	this->initial = true;
	this->sname = sname;
	this->versionID = versionID;
	this->machineCode = machineCode;
	this->uuid = uuid;
	this->key = key;
	g_key = key;
	if (sname.empty()) {
		error = "软件名不能为空";
		return false;
	}
	if (versionID.empty()) {
		error = "版本号不能为空";
		return false;
	}
	if (machineCode.empty()) {
		error = "机器码不能为空";
		return false;
	}
	if (uuid.empty()) {
		error = "软件uuid不能为空";
		return false;
	}
	if (key.empty()) {
		error = "软件密钥不能为空";
		return false;
	}

	if (time_) {
		string info;
		if (!GetTimestamp(info)) {
			error = info;
			this->initial = false;
			return false;
		}
		int localTime = GetlocalTime(g_ErrorTime);
		g_ErrorTime = localTime - atoi(info.c_str());
	}

	return true;
	//printf("%s\n", this->http.c_str());
}
//调用远程JS算法
bool Buff_user::RunJScode(string jsName, string params, string & info)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (jsName.empty()) {
		info = "JS函数名不能为空";
		return false;
	}
	if (jsName.length() > 20) {
		info = "JS函数名过长";
		return false;
	}
	if (!params.empty() || params.length() > 0) {
		//处理中文符号 把中文,替换成英文,
		int j = (int)params.find("，", 0);
		while (j != -1) {
			params.replace(j, 1, ",");
			j = (int)params.find("，", 0);
		}
		//分割文本 判断有多少个参数 
		vector<string> strs = splitEx(params, ",");
		if ((int)strs.size() > 20) {
			info = "JS方法参数不能大于20个";
			return false;
		}
	}
	if (!this->loginStatus)
	{
		info = "请先登录";
		return false;
	}
	if (this->statusCode.length() != 32)
	{
		info = "未登录";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params_ = "username=" + this->username + "&statusCode="+ statusCode+ "&sname=" + this->sname + "&machineCode=" + this->machineCode + "&javascript=" + jsName + "&params=" + params + "&uuid=" + uuid + "&remarks=" + aeskey + "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params_);
	Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
	string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(JavaScript_user, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
	int Valid = result.time -  GetlocalTime(g_ErrorTime);
	if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}


//合法性检测  - 此函数不联网 必须在登录以后使用建议周期调用
bool Buff_user::LegitimacyTesting() {
	//效验是否登录
	if (!loginStatus) {
		exit(0);
		EndCurrentProcess();
		while (true) {
			malloc(4096);
		}
		ExitThread(-2);
		ExitProcess(0);
		Sleep(5000000);
	}
	ULONG64 time = this->expirationTime * 60 - (int)((GetTickCount() - this->loginTime) / 1000);
	if (time < 1)
	{
		exit(0);
		EndCurrentProcess();
		while (true) {
			malloc(4096);
		}
		ExitThread(-2);
		ExitProcess(0);
		Sleep(5000000);

	}
	return true;
}

bool Buff_user::GetCloudFileflag(string url, string& code, string& encrypt)
{
	if (!this->initial) {
		code = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (this->loginStatus) {
		code = "请先登录";
		return false;
	}
	if (username.empty()) {
		code = "用户名不能为空";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params = "sname=" + this->sname + "&uuid=" + this->uuid + "&username=" + this->username + "&remarks=" + aeskey + "&url=" + url + "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
	string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(user_CloudFlag, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		code = "非法数据";
		return false;
	}
	int Valid = result.time -  GetlocalTime(g_ErrorTime);
	if (Valid < mix_Valid || Valid>max_Valid) {
		code = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		vector<string> strs = splitEx(result.data.c_str(), "#");
		int len = (int)strs.size();
		if (len < 2) {
			code = result.data;
			return false;
		}
		code = strs[0];
		encrypt = strs[1];
		return true;
	}
	code = result.msg;
	return false;
}





//获取时间戳   用于数据包时效判断 为了避免本地时间和服务器时间不一致 故而采用先获取 服务器时间然后计算
bool  Buff_user::GetTimestamp(string & info) {
	if (!this->initial) {

		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "sname=" + sname + "&uuid=" + uuid + "&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(Timestamp, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}

	info = result.msg;
	return false;

}





//获取专码     根据IP计算出专属编码，可用于配置文件名等，从而减少特征
bool Buff_user::GetSpecialCode(int len, string &info)
{
	if (!this->initial) {
		expirationTime = -1;
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (len < 2) {
		len = 2;
	}
	if (len > 20) {
		len = 20;
	}

	string aeskey = GetRandomText(16);
	string params = "sname=" + sname + "&uuid=" + uuid + "&status=" + to_string(len) +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(Special, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}


//获取公告
bool Buff_user::GetNotice(string & info)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "sname=" + (sname)+"&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(notice, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//获取最新版本号
bool Buff_user::GetNewVersion(string & info) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(version, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//检测是否最新版  1 最新版, 2 不是最新版
bool Buff_user::IsNewVersion(string & info)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params = "versionID=" + this->versionID + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(isVersion, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}

//获取更新地址
bool Buff_user::GetUpdateAddress(string username, string & info) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params = "username=" + username + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(Updata, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//获取程序数据
bool Buff_user::GetAppData(string & info) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(appData, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//获取版本数据
bool Buff_user::GetVersionData( string & info) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "versionID=" + this->versionID + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(varData, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//注册码用户_注册
bool Buff_user::User_reg(string username, string password, string nickname, string mail, string qq, string machineCode, string referralCode, string &info)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (username.empty()) {
		info = "用户名不能为空";
		return false;
	}	
	if (password.empty()) {
		info = "密码不能为空";
		return false;
	}
	if (nickname.empty()) {
		info = "昵称不能为空";
		return false;
	}
	if (mail.empty()) {
		info = "邮箱不能为空";
		return false;
	}
	if (machineCode.empty()) {
		info = "机器码不能为空";
		return false;
	}
	if (qq.empty()) {
		qq = "";
	}
	if (referralCode.empty()) {
		referralCode = "";
	}

	string aeskey = GetRandomText(16);
	string params = "username=" + username + "&password="+ password+"&nickname="+ nickname+ "&mail="+ mail+"&qq="+qq+"&referralCode="+ referralCode+"&machineCode="+ machineCode+ "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(user_reg, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		Result_ ret = ParsingData2(result.data);
		int Time = 0;
		if (ret.expirationTime.length()>0) {
		    Time = atoi(ret.expirationTime.c_str());
		}
		info = ret.statusCode;
		if (Time>0)
		{
			info = info + "赠送时长" + ret.expirationTime + "分钟";
		}
		return true;
	}
	info = result.msg;
	return false;
}


//注册码用户_充值
bool Buff_user::User_Recharge(string username, string cardKey, string referralCode, string &info)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (username.empty()) {
		info = "用户名不能为空";
		return false;
	}
	if (cardKey.empty()) {
		info = "卡密不能为空";
		return false;
	}
	if (referralCode.empty()) {
		referralCode = "";
	}

	string aeskey = GetRandomText(16);
	string params = "username=" + username + "&cardKey=" + cardKey + "&referralCode=" + referralCode + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
	string str = rc4.Decrypt(EncryptedData.c_str(), key.c_str());
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(user_rec, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//注册码用户_登录
bool Buff_user::User_Login(string username, string password, string machineCode, string & Response, int & expirationTime)
{
	if (!this->initial) {
		expirationTime = -1;
		Response = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (username.empty()) {
		expirationTime = -1;
		Response = "用户名不能为空";
		return false;
	}
	if (password.empty()) {
		expirationTime = -1;
		Response = "密码不能为空";
		return false;
	}
	if (machineCode.empty()) {
		expirationTime = -1;
		Response = "机器码不能为空";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "username=" + username+"&password="+ password+"&loginVersion="+ versionID  + "&machineCode=" + machineCode + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(user_login, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		Response = "非法数据";
		expirationTime = -1;
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		expirationTime = -1;
		Response = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		Result_ ret = ParsingData2(result.data);
		expirationTime = atoi(ret.expirationTime.c_str());
		Response = ret.statusCode;

		this->loginStatus = true;
		this->username = username;
		this->statusCode = ret.statusCode;
		this->loginTime = GetTickCount();
		this->expirationTime = atoi(ret.expirationTime.c_str());
		return true;
	}
	expirationTime = -1;
	Response = result.msg;
	return false;



}
//获取到期时间
bool Buff_user::GetExpireTime( string & expiretime)
{
	if (!this->initial) {
		expirationTime = -1;
		statusCode = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (this->loginStatus) {
		expiretime = "请先登录";
		return false;
	}
	string aeskey = GetRandomText(16);
	string params = "username=" + this->username + "&statusCode=" + this->statusCode + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(user_expire, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		expiretime = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		expiretime = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		expiretime = result.data;
		return true;
	}
	expiretime = result.msg;
	return false;
}
//获取到期时间_分钟
bool Buff_user::GetExpireTime_minute( string & expiretime)
{
	if (!this->initial) {
		expiretime = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (this->loginStatus) {
		expiretime = "请先登录";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "username=" + this->username + "&statusCode=" + this->statusCode + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(user_expires, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		expiretime = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		expiretime = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		expiretime = result.data;
		return true;
	}
	expiretime = result.msg;
	return false;
}
//解绑机器码
bool Buff_user::UnbindMachineCode(string username, string password, string NewMachineCode, string & info)
{
	if (!this->initial) {
		expirationTime = -1;
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (username.empty()) {
		expirationTime = -1;
		info = "用户名不能为空";
		return false;
	}
	if (password.empty()) {
		expirationTime = -1;
		info = "密码不能为空";
		return false;
	}
	if (NewMachineCode.empty()) {
		expirationTime = -1;
		info = "新机器码不能为空";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "username=" + username +"&password="+ password+ "&machineCode=" + NewMachineCode + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(user_updatacode, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		Result_ ret = ParsingData2(result.data);
		int Time = 0;
		if (ret.expirationTime.length() > 0) {
			Time = atoi(ret.expirationTime.c_str());
		}
		info = ret.statusCode;
		if (Time > 0)
		{
			info = info + "扣除时间：" + ret.expirationTime + "小时";
		}
		return true;
	}
	info = result.msg;
	return false;
}
//解绑IP
bool Buff_user::UnbindIP(string username, string password, string NewIP, string & info)
{
	if (!this->initial) {
		expirationTime = -1;
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (username.empty()) {
		expirationTime = -1;
		info = "用户名不能为空";
		return false;
	}
	if (password.empty()) {
		expirationTime = -1;
		info = "密码不能为空";
		return false;
	}
	if (NewIP.empty()) {
		expirationTime = -1;
		info = "新IP不能为空";
		return false;
	}
	if (!isIPAddressValid(NewIP.c_str())) {
		info = "IP地址不合法";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "username=" + username + "&password=" + password + "&ip=" + NewIP + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(user_updataip, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		Result_ ret = ParsingData2(result.data);
		int Time = 0;
		if (ret.expirationTime.length() > 0) {
			Time = atoi(ret.expirationTime.c_str());
		}
		info = ret.statusCode;
		if (Time > 0)
		{
			info = info + "扣除时间：" + ret.expirationTime + "小时";
		}
		return true;
	}
	info = result.msg;
	return false;
}
//获取远程变量数据
bool Buff_user::GetRemoteVariables(string varNumber, string varName, string & info)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (this->loginStatus) {
		info = "请先登录";
		return false;
	}
	if (varNumber.empty()) {
		info = "变量编号不能为空";
		return false;
	}
	if (varName.empty()) {
		info = "变量名不能为空";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "username=" + this->username + "&statusCode=" + this->statusCode + "&varName=" + varName + "&varNumber=" + varNumber + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(user_var, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}

//获取云变量数据
bool Buff_user::GetUserCloudVarData( string varNumber, string varName, string & info)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (this->loginStatus) {
		info = "请先登录";
		return false;
	}
	if (varNumber.empty()) {
		info = "变量编号不能为空";
		return false;
	}
	if (varName.empty()) {
		info = "变量名不能为空";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "username=" + this->username + "&statusCode=" + this->statusCode + "&varName=" + varName + "&varNumber=" + varNumber + "&sname=" + sname + "&uuid=" + uuid + "&remarks=" + aeskey + "&timestamp=" + to_string(GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
	string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(user_CloudVar, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
	int Valid = result.time - GetlocalTime(g_ErrorTime);
	if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//设置用户数据
bool Buff_user::SetUserdata( string userdata, string & info)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (this->loginStatus) {
		info = "请先登录";
		return false;
	}

	if (userdata.length() > 200) {
		info = "用户数据过长";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "username=" + this->username + "&statusCode=" + this->statusCode + "&userData=" + userdata + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(user_setdata, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//获取用户数据
bool Buff_user::GetUserdata(string & info)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (this->loginStatus) {
		info = "请先登录";
		return false;
	}


	string aeskey = GetRandomText(16);
	string params = "username=" + this->username + "&statusCode=" + this->statusCode + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(user_getdata, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//添加黑名单 IP会自动添加到黑名单
bool Buff_user::AddBlacklist(string & info, string username, string NewMachineCode, string remarks)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "username=" + username + "&machineCode=" + machineCode + "&remarks=" + remarks + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(Blacklist, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}



//检测用户状态  
bool Buff_user::GetUserStatus( string & info)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (this->loginStatus) {
		info = "请先登录";
		return false;
	}


	string aeskey = GetRandomText(16);
	string params = "username=" + this->username + "&statusCode=" + this->statusCode + "&machineCode=" + this->machineCode + "&sname=" + sname + "&loginVersion=" + versionID + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(user_status, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}

//获取本机IP
bool Buff_user::GetlocalIP(string & info) {
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(Localip, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//解析返回释义
string Buff_user::ReturntoDefinition_(string number)
{
	return ReturntoDefinition(number);
}
//获取邮箱验证码     返回 邮箱UUID  注意：每次调用需要 间隔时间120秒 否则返回错误
bool Buff_user::GetMailcode(string username, string mail, string & info)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (username.empty()) {
		info = "用户名不能为空";
		return false;
	}
	if (mail.empty()) {
		info = "邮箱不能为空";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "username=" + username + "&mail=" + mail  + "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(user_MailCode, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
//更新密码
bool Buff_user::UpdataPassword(string username, string newPassword, string mail, string mailUuid, string mailCode, string & info)
{
	if (!this->initial) {
		info = base64_decode(notInitiali);// "未初始化";
		return false;
	}
	if (username.empty()) {
		info = "用户名不能为空";
		return false;
	}
	if (newPassword.empty()) {
		info = "新密码不能为空";
		return false;
	}
	if (mail.empty()) {
		info = "邮箱不能为空";
		return false;
	}
	if (mailUuid.empty()) {
		info = "邮箱UUID不能为空";
		return false;
	}
	if (mailCode.empty()) {
		info = "邮箱验证码不能为空";
		return false;
	}

	string aeskey = GetRandomText(16);
	string params = "username=" + username +"&repassword="+ newPassword+ "&mail=" + mail +"&mailuuid="+ mailUuid+"&mailCode="+ mailCode+ "&sname=" + sname + "&uuid=" + uuid +"&remarks=" + aeskey +  "&timestamp=" + to_string( GetlocalTime(g_ErrorTime));
	string data = toJson(params);
	 Help::Rc4 rc4;
	string EncryptedData = rc4.Encrypt(string_To_UTF8(data).c_str(), key.c_str());//用软件的密钥加密
    string sendText = to_sendJson(EncryptedData, uuid);
	Result result = Send(user_password, sendText.c_str(), aeskey);
	if (result.sign.compare(aeskey)) {
		info = "非法数据";
		return false;
	}
int Valid = result.time -  GetlocalTime(g_ErrorTime);
	 if (Valid < mix_Valid || Valid>max_Valid) {
		info = "数据包过期";
		return false;
	}
	if (!result.code.compare("1")) {//成功标志
		info = result.data;
		return true;
	}
	info = result.msg;
	return false;
}
#pragma endregion
//=================================================== Buff用户类   结束=================================================================
