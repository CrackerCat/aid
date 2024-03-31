#include <iostream>
#include <Windows.h>
#include "aid2/aid2.h"

char gudid[41];

// 设置成自动授权 事件通知回调函数
void ReadAuthorizeInfo(const char* udid,  AuthorizeReturnStatus ReturnFlag)
{
	std::cout << "iOS设备，udid:" << udid ;
	switch (ReturnFlag)
	{
	case AuthorizeReturnStatus::AuthorizeDopairingLocking:
		std::cout << "，请打开密码锁定，进入ios主界面。" << std::endl;
		break;
	case AuthorizeReturnStatus::AuthorizeDopairingTrust:
		std::cout << "，请在设备端按下“信任”按钮。" << std::endl;
		break;
	case AuthorizeReturnStatus::AuthorizeDopairingNotTrust:
		std::cout << "，使用者按下了“不信任”按钮。" << std::endl;
		break;
	default:
		std::cout << (ReturnFlag == AuthorizeReturnStatus::AuthorizeSuccess ? " 授权成功" : " 授权失败") << std::endl;
		break;
	}
}

// 插入连接事件
void Connecting(const char* udid, const char* DeviceName, const char* ProductType, const char* DeviceEnclosureColor, const char* MarketingName, long long TotalDiskCapacity)
{
	std::cout << "Connecting udid:" << udid << ',' << DeviceName << ',' << ProductType << ", " << DeviceEnclosureColor << ", " << MarketingName << ", " << TotalDiskCapacity << std::endl;
	strcpy_s(gudid, strlen(udid) + 1, udid);
}


// 断开事件
void Distinct(const char* udid)
{
	std::cout << "Distinct udid:" << udid << std::endl;
}


//安装事件通知 回调函数
void InstallApplicationInfo(const char* status, const int percent) {
	std::cout << "[" << percent << "%]," << status << "." << std::endl;
	if (strcmp(status, "success") == 0)
		MessageBox(NULL, L"安装ipa包成功", L"成功", MB_OK | MB_ICONASTERISK);
	if (strcmp(status, "fail") == 0)
		MessageBox(NULL, L"安装ipa包失败", L"失败", MB_OK | MB_ICONERROR);
}




//自动授权代码示例
void autoDo(char* ipaPath)
{
	StartListen(true,ipaPath);
	std::cout << "按回车键停止..." << std::endl;
	std::cin.get();  // 阻止主线程退出
	StopListen();
}
//下面代码是不自动授权
void Do(char * ipaPath)
{
	StartListen(false);
	auto ret = AuthorizeDevice(gudid);
	std::cout << "iOS设备，udid:" << gudid << (ret ? " 授权成功" : " 授权失败") << std::endl;
	//auto retInstall = InstallApplication(gudid, ipaPath);
	//std::cout << "iOS设备，udid:" << gudid << " ipa包：" << ipaPath << (retInstall ? " 安装成功" : " 安装失败") << std::endl;
	std::cout << "按回车键停止..." << std::endl;
	std::cin.get();  // 阻止主线程退出
	StopListen();
}


int main(int argc, char* argv[], char* envp[])
{
	//RegisterAuthorizeCallback(ReadAuthorizeInfo); //注册信任,结果通知事件回调函数
	RegisterInstallCallback(InstallApplicationInfo);  //安装ipa 回调函数
	RegisterConnectCallback(Connecting);		// 连接回调函数
	RegisterDisconnectCallback(Distinct);		// 断开事件
	Do(argv[1]);
	//autoDo(argv[1]);
}