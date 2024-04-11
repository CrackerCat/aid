#ifndef AID2_H
#define AID2_H

#ifdef AID2_EXPORTS
#define AID2_API __declspec(dllexport)
#else
#define AID2_API __declspec(dllimport)
#endif

//��Ȩ����״̬��Ϣ
enum AuthorizeReturnStatus
{
	AuthorizeSuccess = 0,   //��Ȩ�ɹ�
	AuthorizeDopairingLocking = 1,  //ִ�������������ִ�ж�����⿪����
	AuthorizeFailed = 2,  //��Ȩʧ��
	AuthorizeDopairingTrust = 3,  //ִ���������Ҫ�����Σ�ִ�ж����밴����
	AuthorizeDopairingNotTrust = 4  //ִ�������ʹ���߰��²�����
};

typedef void (*AuthorizeDeviceCallbackFunc)(const char* udid, AuthorizeReturnStatus ReturnFlag);//��Ȩ�ص���������
typedef void (*InstallApplicationFunc)(const char* status, const int percent);//��װ�����ص�����
typedef void (*ConnectCallbackFunc)(const char* udid, const char* DeviceName, const char* ProductType, const char* DeviceEnclosureColor, const char* MarketingName);//���Ӻ����ص�����
typedef void (*DisconnectCallbackFunc)(const char* udid);//�Ͽ��豸���ӻص�����

/*******************************************************
���̷߳�ʽ�������������������߳�
������autoAuthorize �Ƿ��Զ���Ȩ��true Ĭ��ֵ��Ϊ�Զ���Ȩ��flase Ϊ���Զ�
      ipaPath  �Զ���װipa ����·���������ֵ����װ
����ֵ���ɹ�Ϊtrue
**************************************************/
extern "C" AID2_API bool StartListen(bool autoAuthorize=true,const char * ipaPath=nullptr);
/*******************************************************
ֹͣ�������رռ����߳�
����ֵ���ɹ�Ϊtrue
**************************************************/
extern "C" AID2_API bool StopListen();

/*******************************************************
����deviceHandle��Ȩ
������deviceHandle
����ֵ���ɹ�Ϊtrue
*******************************************************/
extern "C" AID2_API bool AuthorizeDeviceEx(void* deviceHandle);


/*******************************************************
����udid��Ȩ
������udid
����ֵ���ɹ�Ϊtrue
*******************************************************/
extern "C" AID2_API bool AuthorizeDevice(const char * udid);


/*******************************************************
����udid��װpath ��ipa ��
������deviceHandle
	  ipaPath ipa����·��
����ֵ���ɹ�Ϊtrue
*******************************************************/
extern "C" AID2_API bool InstallApplicationEx(void* deviceHandle, const char* ipaPath);


/*******************************************************
����udid��װpath ��ipa ��
������udid
      ipaPath ipa����·��
����ֵ���ɹ�Ϊtrue
*******************************************************/
extern "C" AID2_API bool InstallApplication(const char* udid,const char* ipaPath);

/*******************************************************
ע����Ȩ�ص���������Ȩ��������Ҫ�����Ϣ����Ȩ���ͨ���ص�����֪ͨ
������callback �ص�����ָ��
**************************************************/
extern "C" AID2_API void RegisterAuthorizeCallback(AuthorizeDeviceCallbackFunc callback);


/*******************************************************
ע�ᰲװ�ص���������Ȩ��������Ҫ�����Ϣ����Ȩ���ͨ���ص�����֪ͨ
������callback �ص�����ָ��
**************************************************/
extern "C" AID2_API void RegisterInstallCallback(InstallApplicationFunc callback);

/*******************************************************
�����豸����֪ͨ�Ļص�����
������callback �ص�����ָ��
**************************************************/
extern "C" AID2_API void RegisterConnectCallback(ConnectCallbackFunc callback);

/*******************************************************
�Ͽ��豸����֪ͨ�Ļص�����
������callback �ص�����ָ��
**************************************************/
extern "C" AID2_API void RegisterDisconnectCallback(DisconnectCallbackFunc callback);

#endif