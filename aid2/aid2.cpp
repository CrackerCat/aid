#include <stdio.h>
#include <iostream>
#include<map>
#include <future>
#include "iOSDeviceInfo.h"
#include "iOSApplication.h"
#include "ATH.h"
#include "GenerateRS_client.h"
#include "Logger.h"
#include "aid2.h"

using namespace std;
using namespace aid2;
static map<string, AMDeviceRef> gudid;
static bool gautoAuthorize = true;
static char* gipaPath;
static future<void> gfutListen;
static CFRunLoopRef grunLoop;

// 连接回调指针函数指针变量
static ConnectCallbackFunc ConnectCallback = nullptr;
// 断开连接回调函数指针变量
static DisconnectCallbackFunc  DisconnectCallback = nullptr;
static AuthorizeDeviceCallbackFunc DoPairCallback = nullptr;




void device_notification_callback(struct AMDeviceNotificationCallbackInformation* CallbackInfo)
{
	AMDeviceRef deviceHandle = CallbackInfo->deviceHandle;
	switch (CallbackInfo->msgType)
	{
		case ADNCI_MSG_CONNECTECD:
		{
			if((ConnectMode)AMDeviceGetInterfaceType(deviceHandle)== ConnectMode::USB)
			{
				string udid = getUdid(deviceHandle);
				if (udid.length() > 24) {  //只有取到udid
					gudid[udid] = deviceHandle;
					auto appleInfo = iOSDeviceInfo(deviceHandle);
					// 连接回调
					if (ConnectCallback) { // 连接回调
						ConnectCallback(udid.c_str(),
							appleInfo.DeviceName().c_str(),
							appleInfo.ProductType().c_str(),
							appleInfo.DeviceEnclosureColor().c_str(),
							appleInfo.MarketingName().c_str(),
							appleInfo.TotalDiskCapacity()
						);
					}
					logger.log("Start Device.");
					logger.log("Device %p connected,udid:%s", deviceHandle, udid.c_str());
					if (gautoAuthorize)   //自动授权
					{
						auto ret = AuthorizeDeviceEx(deviceHandle);
						if (iOSDeviceInfo::DoPairCallback)
							iOSDeviceInfo::DoPairCallback(udid.c_str(), ret ? AuthorizeReturnStatus::AuthorizeSuccess : AuthorizeReturnStatus::AuthorizeFailed);
						if (gipaPath) {
							InstallApplicationEx(deviceHandle, gipaPath);
						}
					}
				}
			}
			else
			{
				logger.log("不处理USB以外连接设备 %p 进行connect.", deviceHandle);
			}
			break; 
		}
		case ADNCI_MSG_DISCONNECTED:
		{
			string udid;
			for (auto it : gudid)
			{
				if (it.second == deviceHandle)
				{
					udid = it.first;
					break;
				}
			}
			if (udid.length() > 24) {  //只有取到udid
				if ((ConnectMode)AMDeviceGetInterfaceType(gudid.at(udid)) == ConnectMode::USB)  //只有usb 设置才discount
				{
					gudid.erase(udid);
					if (DisconnectCallback) {
						DisconnectCallback(udid.c_str());
					}
					logger.log("Device %p disconnected,udid:%s", deviceHandle, udid.c_str());
				}
				else
				{
					logger.log("不处理USB以外设备Device %p disconnected.", deviceHandle);
				}
			}
			else
			{
				logger.log("Device %p disconnected,udid:NULL.", deviceHandle);
			}
			break;
		}
		case ADNCI_MSG_UNKNOWN:
			logger.log("Unsubscribed");
			break;
		default:
			logger.log("Unknown message %d\n", CallbackInfo->msgType);
			break;
	}
}

bool StartListen(bool autoAuthorize,const char* ipaPath)
{
	gautoAuthorize = autoAuthorize;
	gfutListen = async(launch::async, [autoAuthorize] {
		void* subscribe = nullptr;
		int ret = AMDeviceNotificationSubscribe(device_notification_callback, 0, 0, 0, &subscribe);
		if (ret) {
			logger.log("StartListen( %s ) failed.", autoAuthorize?"True":"False");
			return;
		}
		grunLoop = CFRunLoopGetCurrent();
		CFRunLoopRun();
		AMDeviceNotificationUnsubscribe(subscribe);
		CFRelease(subscribe);
		return;
	});
	if (ipaPath) {
		auto len = strlen(ipaPath);
		gipaPath = (char*)malloc(len + 1);
		memcpy(gipaPath, ipaPath, len + 1);
	}
	logger.log("StartListen( %s ) success.", autoAuthorize ? "True" : "False");
	return true;
}

bool StopListen()
{
	CFRunLoopStop(grunLoop);
	gfutListen.wait();
	free(gipaPath);
	logger.log( "StopListen success." );
	return true;
}

bool AuthorizeDeviceEx(void* deviceHandle)
{
	iOSDeviceInfo appleInfo((AMDeviceRef)deviceHandle);

	if (!appleInfo.DoPair()) {
		logger.log("信认失败或没有通过。");
		return false;
	};

	aidClient* client = aidClient::NewInstance((AMDeviceRef)deviceHandle);
	string remote_grappa = string();
	string grappa = string();

	try
	{
		string udid = getUdid((AMDeviceRef)deviceHandle);
		ATH ath = ATH(udid);  //new出同步实例
		
		if (!client->GenerateGrappa(remote_grappa))  //Grappa数据生成
		{
			logger.log("udid:%s,RemoteGetGrappa failed.", udid.c_str());
			delete client;
			return false;
		}
		logger.log("udid:%s,RemoteGetGrappa success.", udid.c_str());

		if (!ath.SyncAllowed()) {  //允许同步
			logger.log("udid:%s,SyncAllowed message read failed.", udid.c_str());
			delete client;
			return false;
		}
		logger.log("udid:%s,SyncAllowed message read success.", udid.c_str());
		
		//请求生成同步信息
		if (!ath.RequestingSync(remote_grappa)) {
			logger.log("udid:%s,RequestingSync failed.", udid.c_str());
			delete client;
			return false;
		};
		logger.log("udid:%s,RequestingSync success.", udid.c_str());
		// 获取请求同步状态数据
		if (!ath.ReadyForSync(grappa)) {
			logger.log("udid:%s,ReadyForSync message read failed.\n", udid.c_str());
			delete client;
			return false;
		}
		logger.log("udid:%s,ReadyForSync message read success.", udid.c_str());
		if (!client->GenerateRs(grappa))	//调用远程服务器指令生成afsync.rs和afsync.rs.sig文件
		{
			logger.log("udid:%s,GenerateRs failed.", udid.c_str());
			delete client;
			return false;
		}
		logger.log("udid:%s,GenerateRs success.", udid.c_str());
		//模拟itunes完成同步指令
		if (!ath.FinishedSyncingMetadata())
		{
			logger.log("udid:%s,FinishedSyncingMetadata failed.", udid.c_str());
			delete client;
			return false;

		}
		logger.log("udid:%s,FinishedSyncingMetadata success.", udid.c_str());
		//读取同步状态
		if (!ath.SyncFinished())
		{
			logger.log("udid:%s,SyncFinished message read SyncFailed.", udid.c_str());
			delete client;
			return false;

		}
		logger.log("udid:%s,SyncFinished message read ok.", udid.c_str());
		delete client;
		return true;
	}
	catch (const char* e)
	{
		logger.log(e);
		delete client;
		return false;
	}
}


bool AuthorizeDevice(const char * udid) {
	int i = 0;
	map<string, AMDeviceRef>::iterator iter;
	for (;;) {
		this_thread::sleep_for(chrono::milliseconds(2));
		iter = gudid.find(udid);
		if (iter == gudid.end()){
			if (i++ >= 30) {
				logger.log("设备没有插入，初始化失败。");
				return false;
			}
		}
		else {
			break;
		}
	}
	auto deviceHandle = iter->second;
	return AuthorizeDeviceEx(deviceHandle);
}


bool InstallApplicationEx(void* deviceHandle, const char* ipaPath)
{
	iOSDeviceInfo appleInfo((AMDeviceRef)deviceHandle);

	if (!appleInfo.DoPair()) {
		if (iOSApplication::InstallCallback) iOSApplication::InstallCallback("fail", 100);
		logger.log("信认失败或没有通过。");
		return false;
	};

	iOSApplication iosapp = iOSApplication((AMDeviceRef)deviceHandle);

	auto retInstall = iosapp.Install(ipaPath);
	if (iOSApplication::InstallCallback)
		iOSApplication::InstallCallback(retInstall ? "success" : "fail", 100);
	return retInstall;
}

bool InstallApplication(const char* udid, const char* ipaPath) {
	auto iter = gudid.find(udid);
	if (iter == gudid.end())
	{
		logger.log("设备没有插入，初始化失败。");
		return false;
	}
	auto deviceHandle = iter->second;
	return InstallApplicationEx(deviceHandle, ipaPath);
}


//注册授权回调函数，授权过程中需要配对信息和授权结果通过回调函数通知
void RegisterAuthorizeCallback(AuthorizeDeviceCallbackFunc callback) {
	iOSDeviceInfo::DoPairCallback = callback;
}

//注册安装回调函数，授权过程中需要配对信息和授权结果通过回调函数通知
void RegisterInstallCallback(InstallApplicationFunc callback) {
	iOSApplication::InstallCallback = callback;
}

void RegisterConnectCallback(ConnectCallbackFunc callback)
{
	ConnectCallback = callback;
}

void RegisterDisconnectCallback(DisconnectCallbackFunc callback)
{
	DisconnectCallback = callback;
}
