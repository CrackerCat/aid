#include <stdio.h>
#include <iostream>
#include<map>
#include "aid2.h"
#include "iOSDevice.h"
#include "GenerateRS_client.h"
#include "Logger.h"

using namespace std;
using namespace aid2;
static map<string, iOSDevice *> gudid;
static bool gautoAuthorize = true;
static char* gipaPath;
static map<string,future<void>> gmapfut;
static CFRunLoopRef grunLoop;

// ���ӻص�ָ�뺯��ָ�����
static ConnectCallbackFunc ConnectCallback = nullptr;
// �Ͽ����ӻص�����ָ�����
static DisconnectCallbackFunc  DisconnectCallback = nullptr;

void device_notification_callback(struct AMDeviceNotificationCallbackInformation* CallbackInfo)
{
	AMDeviceRef deviceHandle = CallbackInfo->deviceHandle;
	switch (CallbackInfo->msgType)
	{
		case ADNCI_MSG_CONNECTECD:
		{
			auto apple_device = new iOSDevice(deviceHandle);
			if (apple_device->GetInterfaceType() == ConnectMode::USB)  //	
			{
				string udid = apple_device->udid();
				if (udid.length() > 24) {  //ֻ��ȡ��udid
					gudid[udid] = apple_device;
					// ���ӻص�
					ConnectCallback(udid.c_str(), apple_device->DeviceName().c_str(), apple_device->ProductType().c_str());
					logger.log("Start Device.");
					logger.log("Device %p connected,udid:%s", deviceHandle, udid.c_str());
					if (gautoAuthorize)   //�Զ���Ȩ
					{
						gmapfut[udid] = async(launch::async, [udid, apple_device] {  //��Ȩ�첽ִ��
							auto ret = AuthorizeDevice(udid.c_str());
							string deviceName = apple_device->DeviceName();
							string productType = apple_device->ProductType();
							if(iOSDevice::DoPairCallback)
								iOSDevice::DoPairCallback( udid.c_str(),deviceName.c_str(),productType.c_str(), ret ? AuthorizeReturnStatus::AuthorizeSuccess : AuthorizeReturnStatus::AuthorizeFailed );
							if (gipaPath) {
								auto retInstall = apple_device->InstallApplication(gipaPath);
								if(iOSDevice::InstallApplicationCallback)
									iOSDevice::InstallApplicationCallback(retInstall ? "success" : "fail", 100);
							}
						});
					}
				}
				else {
					delete apple_device;
				}
			}
			else
			{
				delete apple_device;
				logger.log("������USB���������豸 %p ����connect.", deviceHandle);
			}
			break; 
		}
		case ADNCI_MSG_DISCONNECTED:
		{
			string udid;
			for (auto it : gudid)
			{
				if (it.second->DeviceRef() == deviceHandle)
				{
					udid = it.first;
					break;
				}
			}
			if (udid.length() > 24) {  //ֻ��ȡ��udid
				if (gudid[udid]->GetInterfaceType() == ConnectMode::USB)  //ֻ��usb ���ò�discount
				{
					gmapfut[udid].wait();
					delete gudid[udid];
					gmapfut.erase(udid);
					gudid.erase(udid);
					DisconnectCallback(udid.c_str());
					logger.log("Device %p disconnected,udid:%s", deviceHandle, udid.c_str());
				}
				else
				{
					logger.log("������USB�����豸Device %p disconnected.", deviceHandle);
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
	gmapfut["listen"] = async(launch::async, [autoAuthorize] {
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
	gautoAuthorize = autoAuthorize;
	logger.log("StartListen( %s ) success.", autoAuthorize ? "True" : "False");
	return true;
}

bool StopListen()
{
	CFRunLoopStop(grunLoop);
	gmapfut["listen"].wait();
	gmapfut.erase("listen");
	free(gipaPath);
	logger.log( "StopListen success." );
	return true;
}

bool AuthorizeDevice(const char * udid) {
	int i = 0;
	bool ret = true;
	for (;;) {
		this_thread::sleep_for(chrono::milliseconds(2)); 
		if (gudid[udid] != nullptr) {
			break;
		}
		if (i++ >= 30) {
			logger.log( "�豸û�в��룬��ʼ��ʧ�ܡ�" );
			return false;
		}
	}
	auto apple_device = gudid.at(udid);
	if (!apple_device->DoPair()) {
		logger.log("����ʧ�ܻ�û��ͨ����");
		return false;
	};


	//RemoteGetGrappa   async call
	aidClient* client = aidClient::Instance();
	string grappa = string();
	unsigned long grappa_session_id = 0;
	auto rggret = async(std::launch::async, [client, udid, &grappa,&grappa_session_id](){
			return client->RemoteGetGrappa(udid, grappa, grappa_session_id);
		}
	);
	// athostconnect
	auto connRet = apple_device->ATHostConnection();
	logger.log( "ATHostConnection(),����ֵ:%s,udid:%s" , connRet ? "True" : "False",udid);
	if (!connRet) {
		return false;
	}
	//��������һ���첽ȥ��ȡ���ʱ��Ϊ10��
	future<bool> fut = async(std::launch::async, [apple_device]() {return apple_device->ReceiveMessage(10000); });
	apple_device->OpenIOSFileSystem(); //��afc �ļ�ϵͳ
	apple_device->DeleteAfsyncRq();  //ɾ���ֻ����AfsyncRq�ļ�
	// ���ö�ʱ��
	auto startTime = chrono::high_resolution_clock::now();
	chrono::milliseconds durationMs;
	do{
		this_thread::sleep_for(chrono::milliseconds(1));	
		durationMs = chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - startTime);
	} while (!apple_device->athostMessage.SyncAllowed and durationMs.count() < 4000);  //������һ���̶߳�ȡ����ͬ����4��û�ж�ȡʧ�ܴ���
	if (apple_device->athostMessage.SyncAllowed) {  //����ͬ��
		logger.log("udid:%s,SyncAllowed message read success.", udid);
		if (rggret.get())
		{
			logger.log("udid:%s,RemoteGetGrappa success.", udid);
			apple_device->SendSyncRequest(SYNC_KEYBAG, grappa);  //����ͬ������
		}
		else
		{
			logger.log("udid:%s,RemoteGetGrappa failed", udid);
			ret = false;
		}
	}
	else {
		logger.log("udid:%s,SyncAllowed message read failed.", udid);
		ret = false;
	}
	if (ret)
	{
		startTime = chrono::high_resolution_clock::now();
		do{
			this_thread::sleep_for(chrono::milliseconds(1));
			durationMs = chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - startTime);
		} while (!apple_device->athostMessage.ReadyForSync and durationMs.count() < 3000); //�ȱ���һ���̶߳�ȡ����ͬ����3��û�ж�ȡʧ�ܴ���
		if (apple_device->athostMessage.ReadyForSync)
		{
			logger.log("udid:%s,ReadyForSync message read success.", udid);
			// RemoteGetrs�ļ�
			string rs, rs_sig;
			client->RemoteGetRs(apple_device, grappa_session_id, rs, rs_sig);  // ���ֻ���Ϣ����remoteǩ��rs�ļ�
			apple_device->WriteAfsyncRs(rs, rs_sig);
			apple_device->SendMetadataSyncFinished(SYNC_FINISHED_2_KEYBAG); //������ͬ������
		}
		else {
			logger.log("udid:%s,ReadyForSync message read failed.", udid);
			ret = false;
		}
	}
	apple_device->CloseIOSFileSystem();
	if (!fut.get()) //��ȡ��ȡ�߳̽��
	{
		logger.log("udid:%s,AuthorizeDevice ReceiveMessage failed.", udid);
		ret = false;
	}
	apple_device->ATHostDisconnect();
	logger.log("udid:%s,ATHostDisconnect success.", udid);
	return ret;
}


bool InstallApplication(const char* udid, const char* ipaPath) {
	int i = 0;
	for (;;) {
		this_thread::sleep_for(chrono::milliseconds(2));
		if (gudid[udid] != nullptr) {
			break;
		}
		if (i++ >= 30) {
			logger.log("�豸û�в��룬��ʼ��ʧ�ܡ�");
			return false;
		}
	}
	auto apple_device = gudid.at(udid);
	if (!apple_device->DoPair()) {
		logger.log("����ʧ�ܻ�û��ͨ����");
		return false;
	};
	auto retInstall = apple_device->InstallApplication(ipaPath);
	if(iOSDevice::InstallApplicationCallback)
		iOSDevice::InstallApplicationCallback(retInstall ? "success" : "fail", 100);
	return retInstall;
}


//ע����Ȩ�ص���������Ȩ��������Ҫ�����Ϣ����Ȩ���ͨ���ص�����֪ͨ
void RegisterAuthorizeCallback(AuthorizeDeviceCallbackFunc callback) {
	iOSDevice::DoPairCallback = callback;
}

//ע�ᰲװ�ص���������Ȩ��������Ҫ�����Ϣ����Ȩ���ͨ���ص�����֪ͨ
void RegisterInstallCallback(InstallApplicationFunc callback) {
	iOSDevice::InstallApplicationCallback = callback;
}

void RegisterConnectCallback(ConnectCallbackFunc callback)
{
	ConnectCallback = callback;
}

void RegisterDisconnectCallback(DisconnectCallbackFunc callback)
{
	DisconnectCallback = callback;
}
