#pragma once
#ifndef MYLIB_H
#define MYLIB_H
#include <windows.h>
#include <string>
#include <iostream>

#include <fstream>   //文件流
#include <iostream>
using namespace std;


/*
// 数据下载
BOOL HttpDownload(const char* pszDownloadUrl, BYTE** ppDownloadData, DWORD* pdwDownloadDataSize);

*/



// 创建并保存文件
BOOL SaveToFile(const char* pszFileName, BYTE* pData, DWORD dwDataSize);

struct Result {
	string code;  //响应码，1 代表成功; 0 代表失败
	string msg;//响应信息 描述字符串
	string sign;
	int    time;
	string data;//返回的数据
};


struct Result_ {
	string expirationTime;  //时间
	string statusCode;//状态码
	string deduct;//扣除
};



//取随机数
ULONG64 GetRandom(ULONG64  a, ULONG64  b);
//结束当前进程
void EndCurrentProcess();

//单码模式
class Buff
{
public:
	Buff(void);
	//~Buff(void);
public:
	//秒转时间
	static string  SecondstoTime(int Seconds)
	{
		string  Hour, minute, second, day;
		second = to_string(Seconds % 60);
		minute = to_string(Seconds % 3600 / 60);
		Hour = to_string(Seconds % (24 * 3600) / 3600);
		day = to_string(Seconds / (24 * 3600));
		string time_s = Hour + "小时" + minute + "分钟" + second + "秒";
		if (day != "0") {
			time_s = day + "天" + Hour + "小时" + minute + "分钟" + second + "秒";
		}
		return time_s;
	}
	//解析卡密类型   1小时卡 2六时卡 3半天卡  4天卡  5周卡  6半月卡  7月卡  8季卡  9半年卡  10年卡   11永久卡
	static string ReturntoCardType(string type);

	//下载更新文件   url 下载链接    ppDownloadData 返回的文件数据    pdwDownloadDataSize 返回文件大小
	bool DownloadUpdataFile(string url, BYTE** ppDownloadData, DWORD* pdwDownloadDataSize,string &error);

	//下载云下发文件  url 下载链接    ppDownloadData 返回的文件数据    pdwDownloadDataSize 返回文件大小  必须在登录以后使用
	bool DownloadCloudFile(string url,   BYTE** ppDownloadData, DWORD* pdwDownloadDataSize, string &error);

	//获取机器码
	static string GetMachineCode();
	//初始化  time 参数  真 效验本地时间和服务器时间差 要要求上下不能超过60秒    假 不效验时间
	bool initialize(string sname, string versionID, string machineCode, string uuid,string key ,bool time_, string &error);
	//获取服务器时间戳   用于数据包时效判断 为了避免本地时间和服务器时间不一致 故而采用先获取 服务器时间然后计算
	bool GetTimestamp(string & info);
	//获取专码     根据IP计算出专属编码，可用于配置文件名等，从而减少特征
	bool GetSpecialCode(int len, string &code);
	//获取软件公告
	bool GetNotice(string&Notice);
	//获取最新版本号
	bool GetNewVersion(string&Version);
	//检测是否最新版  1 最新版, 2 不是最新版
	bool IsNewVersion( string&status);
	//获取更新地址
	bool GetUpdateAddress(string cardKey, string &info);
	//获取程序数据
	bool GetAppData(string &info);
	//获取版本数据
	bool GetVersionData( string &info);
	//单码登录
	bool SingleLogin(string cardKey, string &Response, int &expirationTime);
	//单码二次登录
	bool SingleTwologin(string cardKey, string statusCode, string &Response, int &expirationTime);
	//获取到期时间
	bool GetExpireTime( string &expiretime);
	//获取到期时间_分钟
	bool GetExpireTime_minute( string &expiretime);
	//解绑机器码
	bool UnbindMachineCode(string cardKey, string NewMachineCode, string &info, string &deduct);
	//解绑IP
	bool UnbindIP(string cardKey, string NewIP, string &info, string &deduct);

	//获取远程变量数据
	bool GetRemoteVariables( string varNumber, string varName, string &info);


	//获取云变量数据
	bool GetCloudVarInfo( string varNumber, string varName, string &info);



	//设置用户数据
	bool SetUserdata( string userdata, string &info);
	//获取用户数据
	bool GetUserdata( string &info);
	//添加黑名单 IP会自动添加到黑名单
	bool AddBlacklist(string &info, string cardKey = "null", string NewMachineCode = "null", string remarks = "null");

	//查询是否使用  0 未使用 1已使用
	bool QueryWhetherTouse(string cardKey, string &info);

	//查询卡密类型    1小时卡 2六时卡 3半天卡  4天卡  5周卡  6半月卡  7月卡  8季卡  9半年卡  10年卡   11永久卡
	bool QueryCardType(string cardKey, string & info);

	//检测用户状态  
	bool GetUserStatus(string &info);
	//获取本机IP
	bool GetlocalIP(string & info);
	//获取错误返回释义
	string ReturntoDefinition_(string number);
	//调用远程JS算法
	bool RunJScode(string jsName,string params, string &info);
	//合法性检测  - 此函数不联网 必须在登录以后使用建议周期调用
	bool LegitimacyTesting();


	//查询剩余点数
	bool GetRemainingPoints( string &info);
	//扣除卡密点数
	bool subCardPoints(int dropCount,  string &info);
	//获取登录信息
	bool GetLogininfo_card(string machineCode,string &cardKey, string &statusCode,string &version_);
	

private:
	string sname;
	string versionID;
	string machineCode;
	string uuid;
	string key;
	bool initial;
	bool loginStatus;
	int   expirationTime;
	string cardKey;//保存临时卡密
	string statusCode;//保存状态码
	unsigned __int64   loginTime;//登录时间

	bool GetCloudFileflag(string url, string &code, string& encrypt);

};



//注册码模式
class Buff_user 
{
public:
	Buff_user(void);
	//~Buff(void);
public:
	//秒转时间
	static string  SecondstoTime(int Seconds)
	{
		string  Hour, minute, second, day;
		second = to_string(Seconds % 60);
		minute = to_string(Seconds % 3600 / 60);
		Hour = to_string(Seconds % (24 * 3600) / 3600);
		day = to_string(Seconds / (24 * 3600));
		string time_s = Hour + "小时" + minute + "分钟" + second + "秒";
		if (day != "0") {
			time_s = day + "天" + Hour + "小时" + minute + "分钟" + second + "秒";
		}
		return time_s;
	}


	//下载更新文件   url 下载链接    ppDownloadData 返回的文件数据    pdwDownloadDataSize 返回文件大小
	bool DownloadUpdataFile(string url, BYTE** ppDownloadData, DWORD* pdwDownloadDataSize, string &error);

	//下载云下发文件  url 下载链接    ppDownloadData 返回的文件数据    pdwDownloadDataSize 返回文件大小  必须在登录以后使用
	bool DownloadCloudFile(string url, BYTE** ppDownloadData, DWORD* pdwDownloadDataSize, string &error);


	//获取机器码
	static string GetMachineCode();
	//初始化
	bool initialize(string sname, string versionID, string machineCode, string uuid, string key,bool time_, string &error);
	//获取时间戳   用于数据包时效判断 为了避免本地时间和服务器时间不一致 故而采用先获取 服务器时间然后计算
	bool GetTimestamp(string & info);
	//获取专码     根据IP计算出专属编码，可用于配置文件名等，从而减少特征
	bool GetSpecialCode(int len,string &info);
	//获取软件公告
	bool GetNotice(string&info);
	//获取最新版本号
	bool GetNewVersion(string&info);
	//检测是否最新版  1 最新版, 2 不是最新版
	bool IsNewVersion( string&info);
	//获取更新地址
	bool GetUpdateAddress(string username, string &info);
	//获取程序数据
	bool GetAppData(string &info);
	//获取版本数据
	bool GetVersionData( string &info);
	//注册码用户_注册
	bool User_reg(string username, string password,string nickname,string mail ,string qq,string machineCode,string referralCode,string &info);
	//注册码用户_充值
	bool User_Recharge(string username, string cardKey, string referralCode, string &info);
	//注册码用户_登录
	bool User_Login(string username, string password, string machineCode, string &Response, int &expirationTime);
	//获取到期时间
	bool GetExpireTime( string &expiretime);
	//获取到期时间_分钟
	bool GetExpireTime_minute( string &expiretime);
	//解绑机器码
	bool UnbindMachineCode(string username, string password, string NewMachineCode, string &info);
	//解绑IP
	bool UnbindIP(string username, string password, string NewIP, string &info);
	//获取远程变量数据
	bool GetRemoteVariables( string varNumber, string varName, string &info);

	//获取云变量数据
	bool GetUserCloudVarData( string varNumber, string varName, string &info);


	//设置用户数据
	bool SetUserdata( string userdata, string &info);
	//获取用户数据
	bool GetUserdata( string &info);
	//添加黑名单 IP会自动添加到黑名单
	bool AddBlacklist(string &info, string username = "null", string NewMachineCode = "null", string remarks = "null");
	//查询是否使用  0 未使用 1已使用
	//bool QueryWhetherTouse(string username, string &info);
	//检测用户状态  
	bool GetUserStatus( string &info);
	//获取本机IP
	bool GetlocalIP(string & info);
	//获取错误返回释义
	string ReturntoDefinition_(string number);
	//获取邮箱验证码     返回 邮箱UUID  注意：每次调用需要 间隔时间120秒 否则返回错误
	bool GetMailcode(string username,  string mail, string &info);

	//更新密码
	bool UpdataPassword(string username, string newPassword, string mail, string mailUuid, string mailCode, string &info);

	//调用远程JS算法
	bool RunJScode(string jsName, string params, string & info);
	//合法性检测  - 此函数不联网 必须在登录以后使用建议周期调用
	bool LegitimacyTesting();


private:
	string sname;
	string versionID;
	
	string machineCode;
	string uuid;
	bool   initial;
	string key;
	string RequestURL ;
	bool   loginStatus;
	int    expirationTime;
	string statusCode;//登录后的状态码
	string username;//保存登录后的帐号
	unsigned __int64   loginTime;//登录时间


	bool GetCloudFileflag( string url, string& code, string& encrypt);
};

class CInfoFile
{
public:

	//读取登陆信息
	void ReadLogin(string path, string &key)
	{
		ifstream ifs; //创建文件输入对象
		ifs.open(path.c_str()); //打开文件
		char buf[1024] = { 0 };
		ifs.getline(buf, sizeof(buf)); //读取一行内容
		key = string(buf);			 //char *转换为CString
		ifs.close(); //关闭文件
	}

	//写入配置
	void WritePwd(string path, const char* key, const char* statusCode)
	{
		ofstream ofs;	 //创建文件输出对象
		ofs.open(path.c_str()); //打开文件
		ofs << key << endl; //name写入文件         卡密
		ofs << statusCode << endl;
		ofs.close();	//关闭文件
	}






};
#endif