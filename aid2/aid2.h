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
//��Ȩ�ص����������ṹ��
struct DeviceParameter {
	const char * udid;
	const char * DeviceName;
	const char* ProductType;
	AuthorizeReturnStatus ReturnFlag;
} ;
typedef void (*AuthorizeDeviceCallbackFunc)(DeviceParameter r1);//��Ȩ�ص���������

/*******************************************************
���̷߳�ʽ�������������������߳�
������autoAuthorize �Ƿ��Զ���Ȩ��true Ĭ��ֵ��Ϊ�Զ���Ȩ��flase Ϊ���Զ�
����ֵ���ɹ�Ϊtrue
**************************************************/
extern "C" AID2_API bool StartListen(bool autoAuthorize=true);
/*******************************************************
ֹͣ�������رռ����߳�
����ֵ���ɹ�Ϊtrue
**************************************************/
extern "C" AID2_API bool StopListen();
/*******************************************************
����udid��Ȩ
������udid
����ֵ���ɹ�Ϊtrue
**************************************************/
extern "C" AID2_API bool AuthorizeDevice(const char * udid);

/*******************************************************
ע��ص���������Ȩ��������Ҫ�����Ϣ����Ȩ���ͨ���ص�����֪ͨ
������callback �ص�����ָ��
**************************************************/
extern "C" AID2_API void RegisterCallback(AuthorizeDeviceCallbackFunc callback);

#endif