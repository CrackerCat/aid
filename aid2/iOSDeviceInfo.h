#pragma once
#include "iTunesApi/simpleApi.h"
#include<string>
#include "aid2.h"
namespace aid2 {
	using namespace std;

	//获取udid
	string getUdid(AMDeviceRef deviceHandle);

	class iOSDeviceInfo
	{
	public:
		iOSDeviceInfo(AMDeviceRef deviceHandle);
		~iOSDeviceInfo();

		string FairPlayCertificate();
		uint64_t FairPlayDeviceType();
		string DeviceName();   // 设备名称
		string ProductType();		// 产品类型，和机型一一对应
		string DeviceEnclosureColor(); // 外壳颜色
		string MarketingName();			//营销机型，如果iPhone 12等
		uint64_t TotalDiskCapacity();		//手机容量，如果128GB
		bool DoPair();  //执行配对工作

		// 定义执行配对时回调信息，通过参数提醒使用者按下 信任和不信任信息
		static AuthorizeDeviceCallbackFunc DoPairCallback;

	private:
		AMDeviceRef m_deviceHandle = nullptr;

		// 以下是手机获取到的信息
		string m_udid;  //存的udid 
		string m_FairPlayCertificate;   // domain: com.apple.mobile.iTunes key: FairPlayCertificate   存的bytes
		uint64_t m_FairPlayDeviceType = 0;   //domain: com.apple.mobile.iTunes  key: FairPlayDeviceType+KeyTypeSupportVersion
		string m_deviceName;
		string m_productType;
		string m_deviceEnclosureColor;
		string m_marketingName;
		uint64_t m_totalDiskCapacity;

		void initializeDevice(AMDeviceRef deviceHandle);
		// 带有session信息
		void initializeDeviceEx(AMDeviceRef deviceHandle);
	};
}

