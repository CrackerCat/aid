#include "iOSDeviceInfo.h"
#include "Logger.h"
namespace aid2 {
	AuthorizeDeviceCallbackFunc iOSDeviceInfo::DoPairCallback = nullptr;

	//获取udid
	string getUdid(AMDeviceRef deviceHandle)
	{
		string udid;
		CFStringRef found_device_id = AMDeviceCopyDeviceIdentifier(deviceHandle);
		auto len = CFStringGetLength(found_device_id);
		udid.resize(len);
		CFStringGetCString(found_device_id, (char*)udid.c_str(), len + 1, kCFStringEncodingUTF8);
		CFRelease(found_device_id);
		return udid;
	}

	iOSDeviceInfo::iOSDeviceInfo(AMDeviceRef deviceHandle)
	{
		m_deviceHandle = deviceHandle;
		AMDeviceConnect(m_deviceHandle);
		initializeDevice(deviceHandle);
	}

	iOSDeviceInfo::~iOSDeviceInfo()
	{
		AMDeviceDisconnect(m_deviceHandle);
	}

	string iOSDeviceInfo::FairPlayCertificate()
	{
		return m_FairPlayCertificate;
	}

	uint64_t iOSDeviceInfo::FairPlayDeviceType()
	{
		return m_FairPlayDeviceType;
	}

	string iOSDeviceInfo::DeviceName()
	{
		return m_deviceName;
	}

	string iOSDeviceInfo::ProductType()
	{
		return m_productType;
	}

	string iOSDeviceInfo::DeviceEnclosureColor()
	{
		return m_deviceEnclosureColor;
	}

	string iOSDeviceInfo::MarketingName()
	{
		return m_marketingName;
	}

	uint64_t iOSDeviceInfo::TotalDiskCapacity()
	{
		return m_totalDiskCapacity;
	}

	void iOSDeviceInfo::initializeDevice(AMDeviceRef deviceHandle)
	{
		CFStringRef found_device_id = AMDeviceCopyDeviceIdentifier(deviceHandle);
		auto ilen = CFStringGetLength(found_device_id);
		m_udid.resize(ilen);
		CFStringGetCString(found_device_id, (char*)m_udid.c_str(), ilen + 1, kCFStringEncodingUTF8);
		CFRelease(found_device_id);

		//AMDeviceConnect(deviceHandle);
		//DeviceName
		CFStringRef sKey = CFStringCreateWithCString(NULL, "DeviceName", kCFStringEncodingUTF8);
		CFStringRef sValue = AMDeviceCopyValue(deviceHandle, NULL, sKey);
		CFRelease(sKey);
		CFIndex len = CFStringGetLength(sValue);
		m_deviceName.resize(len);
		CFStringGetCString(sValue, (char*)m_deviceName.c_str(), len + 1, kCFStringEncodingUTF8);
		CFRelease(sValue);
		
		//ProductType
		sKey = CFStringCreateWithCString(NULL, "ProductType", kCFStringEncodingUTF8);
		sValue = AMDeviceCopyValue(deviceHandle, NULL, sKey);
		CFRelease(sKey);
		len = CFStringGetLength(sValue);
		m_productType.resize(len);
		CFStringGetCString(sValue, (char*)m_productType.c_str(), len + 1, kCFStringEncodingUTF8);
		CFRelease(sValue);
	
		//DeviceEnclosureColor
		sKey = CFStringCreateWithCString(NULL, "DeviceEnclosureColor", kCFStringEncodingUTF8);
		sValue = AMDeviceCopyValue(deviceHandle, NULL, sKey);
		CFRelease(sKey);
		len = CFStringGetLength(sValue);
		m_productType.resize(len);
		CFStringGetCString(sValue, (char*)m_deviceEnclosureColor.c_str(), len + 1, kCFStringEncodingUTF8);
		CFRelease(sValue);
		//MarketingName
		sKey = CFStringCreateWithCString(NULL, "MarketingName", kCFStringEncodingUTF8);
		sValue = AMDeviceCopyValue(deviceHandle, NULL, sKey);
		CFRelease(sKey);
		len = CFStringGetLength(sValue);
		m_productType.resize(len);
		CFStringGetCString(sValue, (char*)m_marketingName.c_str(), len + 1, kCFStringEncodingUTF8);
		CFRelease(sValue);
		
		
	}



	void iOSDeviceInfo::initializeDeviceEx(AMDeviceRef deviceHandle) {

		//AMDeviceConnect(deviceHandle);
		AMDeviceStartSession(deviceHandle);
		//FairPlayCertificate
		CFStringRef sDomain = CFStringCreateWithCString(NULL, "com.apple.mobile.iTunes", kCFStringEncodingUTF8);
		CFStringRef sKey = CFStringCreateWithCString(NULL, "FairPlayCertificate", kCFStringEncodingUTF8);
		CFStringRef sValue = AMDeviceCopyValue(deviceHandle, sDomain, sKey);
		CFRelease(sKey);
		CFIndex len = CFDataGetLength(sValue);
		m_FairPlayCertificate.resize(len);
		CFDataGetBytes(sValue, CFRangeMake(0, len), (unsigned char*)m_FairPlayCertificate.data());
		CFRelease(sValue);
		CFRelease(sDomain);
		//FairPlayDeviceType
		sDomain = CFStringCreateWithCString(NULL, "com.apple.mobile.iTunes", kCFStringEncodingUTF8);
		sKey = CFStringCreateWithCString(NULL, "FairPlayDeviceType", kCFStringEncodingUTF8);
		sValue = AMDeviceCopyValue(deviceHandle, sDomain, sKey);
		CFRelease(sKey);
		CFNumberGetValue(sValue, kCFNumberSInt32Type, &m_FairPlayDeviceType);
		CFRelease(sValue);
		sKey = CFStringCreateWithCString(NULL, "KeyTypeSupportVersion", kCFStringEncodingUTF8);
		sValue = AMDeviceCopyValue(deviceHandle, sDomain, sKey);
		CFRelease(sKey);
		uint64_t SupportVersion = 0;
		CFNumberGetValue(sValue, kCFNumberSInt32Type, &SupportVersion);
		SupportVersion <<= 32;
		m_FairPlayDeviceType |= SupportVersion;
		CFRelease(sValue);
		CFRelease(sDomain);
		//TotalDiskCapacity
		sDomain = CFStringCreateWithCString(NULL, "com.apple.disk_usage", kCFStringEncodingUTF8);
		sKey = CFStringCreateWithCString(NULL, "TotalDiskCapacity", kCFStringEncodingUTF8);
		sValue = AMDeviceCopyValue(deviceHandle, sDomain, sKey);
		CFRelease(sKey);
		CFNumberGetValue(sValue, kCFNumberSInt64Type, &m_totalDiskCapacity);
		CFRelease(sValue);
		CFRelease(sDomain);
		AMDeviceStopSession(deviceHandle);
		//AMDeviceDisconnect(deviceHandle);
	
	}

	bool iOSDeviceInfo::DoPair() {
		
		int rc = 0;
		while (true) {
			AMDeviceIsPaired(m_deviceHandle);
			rc = AMDeviceValidatePairing(m_deviceHandle);
			if (rc == 0) {
				break;
			}
			// Do pairing
			CFDictionaryRef dictOptions = CFDictionaryCreateMutable(NULL, 0, kCFTypeDictionaryKeyCallBacks, kCFTypeDictionaryValueCallBacks);
			CFStringRef key = CFStringCreateWithCString(NULL, "ExtendedPairingErrors", kCFStringEncodingUTF8);
			CFDictionarySetValue(dictOptions, key, kCFBooleanTrue);
			rc = AMDevicePairWithOptions(m_deviceHandle, dictOptions);

			if (rc == 0xe800001a) {
				logger.log("udid:%s，请打开密码锁定，进入ios主界面。", m_udid.c_str());
				if (DoPairCallback) {
					DoPairCallback(m_udid.c_str(),  AuthorizeReturnStatus::AuthorizeDopairingLocking);
				}
			}
			else if (rc == 0xe8000096) {
				logger.log("udid:%s，请在设备端按下“信任”按钮。", m_udid.c_str());
				if (DoPairCallback) {
					DoPairCallback(m_udid.c_str(), AuthorizeReturnStatus::AuthorizeDopairingTrust);
				}
			}
			else if (rc == 0xe8000095) {
				logger.log("udid:%s，使用者按下了“不信任”按钮。", m_udid.c_str());
				if (DoPairCallback) {
					DoPairCallback(m_udid.c_str(), AuthorizeReturnStatus::AuthorizeDopairingNotTrust);
				}
				//AMDeviceDisconnect(m_deviceHandle);
				return false;
			}
			else if (rc == 0) {
				break;
			}
			this_thread::sleep_for(chrono::milliseconds(1000));
		}
		//AMDeviceDisconnect(m_deviceHandle);
		this->initializeDeviceEx(m_deviceHandle);
		return true;
	}
}