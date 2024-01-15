#pragma once
#include "iTunesApi/simpleApi.h"
#include<string>
#include "aid2.h"
namespace aid2 {
	using namespace std;

	//��ȡudid
	string getUdid(AMDeviceRef deviceHandle);

	class iOSDeviceInfo
	{
	public:
		iOSDeviceInfo(AMDeviceRef deviceHandle);
		~iOSDeviceInfo();

		string FairPlayCertificate();
		uint64_t FairPlayDeviceType();
		string DeviceName();   // �豸����
		string ProductType();		// ��Ʒ���ͣ��ͻ���һһ��Ӧ
		string DeviceEnclosureColor(); // �����ɫ
		string MarketingName();			//Ӫ�����ͣ����iPhone 12��
		uint64_t TotalDiskCapacity();		//�ֻ����������128GB
		bool DoPair();  //ִ����Թ���

		// ����ִ�����ʱ�ص���Ϣ��ͨ����������ʹ���߰��� ���κͲ�������Ϣ
		static AuthorizeDeviceCallbackFunc DoPairCallback;

	private:
		AMDeviceRef m_deviceHandle = nullptr;

		// �������ֻ���ȡ������Ϣ
		string m_udid;  //���udid 
		string m_FairPlayCertificate;   // domain: com.apple.mobile.iTunes key: FairPlayCertificate   ���bytes
		uint64_t m_FairPlayDeviceType = 0;   //domain: com.apple.mobile.iTunes  key: FairPlayDeviceType+KeyTypeSupportVersion
		string m_deviceName;
		string m_productType;
		string m_deviceEnclosureColor;
		string m_marketingName;
		uint64_t m_totalDiskCapacity;

		void initializeDevice(AMDeviceRef deviceHandle);
		// ����session��Ϣ
		void initializeDeviceEx(AMDeviceRef deviceHandle);
	};
}

