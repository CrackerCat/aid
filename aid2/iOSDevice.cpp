#include "iOSDevice.h"
#include "Logger.h"

namespace aid2{
	kAMDError iOSDevice::Connect()
	{
		kAMDError kAMDRet = kAMDError::kAMDOk;
		kAMDRet = (kAMDError)AMDeviceConnect(m_deviceHandle);
		if (kAMDRet ) {
			logger.log("AMDeviceConnect failed.deviceHandle:%p", m_deviceHandle);
		}
		else
		{
			m_connect = true;
		}
		return kAMDRet;
	}

	kAMDError iOSDevice::Disconnect()
	{
		m_connect = false;
		return (kAMDError)AMDeviceDisconnect(m_deviceHandle);
	}

	ConnectMode iOSDevice::GetInterfaceType()
	{
		return (ConnectMode)AMDeviceGetInterfaceType(m_deviceHandle);
	}

	bool iOSDevice::GenDeviceInfo()
	{
		CFIndex len = 0;
		CFStringRef sKey = CFStringCreateWithCString(NULL, "UniqueDeviceID", kCFStringEncodingUTF8);
		CFStringRef sValue = AMDeviceCopyValue(m_deviceHandle, NULL, sKey);
		CFRelease(sKey);
		if (sValue) {
			len = CFStringGetLength(sValue);
			m_udid.resize(len);
			CFStringGetCString(sValue, (char*)m_udid.c_str(), len + 1, kCFStringEncodingUTF8);
			CFRelease(sValue);
		}
		sKey = CFStringCreateWithCString(NULL, "DeviceName", kCFStringEncodingUTF8);
		sValue = AMDeviceCopyValue(m_deviceHandle, NULL, sKey);
		CFRelease(sKey);
		if (sValue) {
			len = CFStringGetLength(sValue);
			m_deviceName.resize(len);
			CFStringGetCString(sValue, (char*)m_deviceName.c_str(), len + 1, kCFStringEncodingUTF8);
			CFRelease(sValue);
		}
		sKey = CFStringCreateWithCString(NULL, "ProductType", kCFStringEncodingUTF8);
		sValue = AMDeviceCopyValue(m_deviceHandle, NULL, sKey);
		CFRelease(sKey);
		if (sValue) {
			len = CFStringGetLength(sValue);
			m_productType.resize(len);
			CFStringGetCString(sValue, (char*)m_productType.c_str(), len + 1, kCFStringEncodingUTF8);
			CFRelease(sValue);
		}
		return true;
	}

	kAMDError iOSDevice::StartSession()
	{
		return (kAMDError)AMDeviceStartSession(m_deviceHandle);
	}

	kAMDError iOSDevice::StopSession()
	{
		return (kAMDError)AMDeviceStopSession(m_deviceHandle);
	}

	bool iOSDevice::GenFairDeviceInfo()
	{
		CFStringRef sDomain = CFStringCreateWithCString(NULL, "com.apple.mobile.iTunes", kCFStringEncodingUTF8);
		CFStringRef sKey = CFStringCreateWithCString(NULL, "FairPlayCertificate", kCFStringEncodingUTF8);
		CFStringRef sValue = AMDeviceCopyValue(m_deviceHandle, sDomain, sKey);
		CFRelease(sKey);
		CFIndex len = CFDataGetLength(sValue);
		m_fair_play_certificate.resize(len);
		CFDataGetBytes(sValue, CFRangeMake(0, len), (unsigned char *)m_fair_play_certificate.data());
		CFRelease(sValue);
		
		sKey = CFStringCreateWithCString(NULL, "FairPlayDeviceType", kCFStringEncodingUTF8);
		sValue = nullptr;
		sValue = AMDeviceCopyValue(m_deviceHandle, sDomain, sKey);
		CFRelease(sKey);
		CFNumberGetValue(sValue, kCFNumberSInt32Type, &m_fair_play_device_type);
		CFRelease(sValue);
		sKey = CFStringCreateWithCString(NULL, "KeyTypeSupportVersion", kCFStringEncodingUTF8);
		sValue = AMDeviceCopyValue(m_deviceHandle, sDomain, sKey);
		CFRelease(sKey);
		CFNumberGetValue(sValue, kCFNumberSInt32Type, (void *)(((__int64)&m_fair_play_device_type) + 4));
		CFRelease(sValue);
		CFRelease(sDomain);
		return true;
	}

	bool iOSDevice::ATHostConnection()
	{
		CFStringRef sUDID = CFStringCreateWithCString(NULL, m_udid.c_str(), kCFStringEncodingUTF8);
		CFStringRef sLibrary = CFStringCreateWithCString(NULL, m_LibraryID.c_str(), kCFStringEncodingUTF8);

		m_ath = ATHostConnectionCreateWithLibrary(sLibrary, sUDID, 0);
		if (m_ath == NULL) {
			CFRelease(sUDID);
			CFRelease(sLibrary);
			logger.log("ATHostConnectionCreateWithLibrary failed.");
			return false;
		}
		CFRelease(sUDID);
		CFRelease(sLibrary);

		//����HostInfo
		ATHostConnectionSendPowerAssertion(m_ath,kCFBooleanTrue);
		CFMutableDictionaryRef dictHostInfo = CFDictionaryCreateMutable(NULL, 0, kCFTypeDictionaryKeyCallBacks, kCFTypeDictionaryValueCallBacks);

		CFStringRef sKey = CFStringCreateWithCString(NULL, "Version", kCFStringEncodingUTF8);
		CFStringRef sValue = CFStringCreateWithCString(NULL, m_Version.c_str(), kCFStringEncodingUTF8);
		CFDictionaryAddValue(dictHostInfo, sKey, sValue);
		CFRelease(sKey);
		CFRelease(sValue);

		sKey = CFStringCreateWithCString(NULL, "SyncHostName", kCFStringEncodingUTF8);
		sValue = CFStringCreateWithCString(NULL, m_ComputerName.c_str(), kCFStringEncodingUTF8);
		CFDictionaryAddValue(dictHostInfo, sKey, sValue);
		CFRelease(sKey);
		CFRelease(sValue);

		sKey = CFStringCreateWithCString(NULL, "LibraryID", kCFStringEncodingUTF8);
		sValue = CFStringCreateWithCString(NULL, m_LibraryID.c_str(), kCFStringEncodingUTF8);
		CFDictionaryAddValue(dictHostInfo, sKey, sValue);
		CFRelease(sKey);
		CFRelease(sValue);

		sKey = CFStringCreateWithCString(NULL, "SyncedDataclasses", kCFStringEncodingUTF8);
		CFMutableArrayRef arrSyncedDataclasses = CFArrayCreateMutable(NULL, 0, kCFTypeArrayCallBacks);
		CFDictionaryAddValue(dictHostInfo, sKey, arrSyncedDataclasses);
		CFRelease(sKey);
		CFRelease(arrSyncedDataclasses);

		ATHostConnectionSendPowerAssertion(m_ath, kCFBooleanFalse);
		CFRelease(dictHostInfo);

		return true;
	}

	void iOSDevice::ATHostDisconnect()
	{
		ATHostConnectionDestroy(m_ath);
	}

	void iOSDevice::SendSyncRequest(unsigned int flags, string grappa)
	{
		CFMutableArrayRef arrDataclasses = CFArrayCreateMutable(NULL, 0, kCFTypeArrayCallBacks);

		if ((flags & SYNC_RINGTONE) != 0)
		{
			CFStringRef sRingtong = CFStringCreateWithCString(NULL, "Ringtong", kCFStringEncodingUTF8);
			CFArrayAppendValue(arrDataclasses, sRingtong);
			CFRelease(sRingtong);
		}
		if ((flags & SYNC_MEDIA) != 0)
		{
			CFStringRef sMedia = CFStringCreateWithCString(NULL, "Media", kCFStringEncodingUTF8);
			CFArrayAppendValue(arrDataclasses, sMedia);
			CFRelease(sMedia);
		}
		if ((flags & SYNC_VOICEMEMO) != 0)
		{
			CFStringRef sVoiceMemo = CFStringCreateWithCString(NULL, "VoiceMemo", kCFStringEncodingUTF8);
			CFArrayAppendValue(arrDataclasses, sVoiceMemo);
			CFRelease(sVoiceMemo);
		}
		if ((flags & SYNC_BOOK) != 0)
		{
			CFStringRef sBook = CFStringCreateWithCString(NULL, "Book", kCFStringEncodingUTF8);
			CFArrayAppendValue(arrDataclasses, sBook);
			CFRelease(sBook);
		}
		if ((flags & SYNC_PHOTO) != 0)
		{
			CFStringRef sPhoto = CFStringCreateWithCString(NULL, "Photo", kCFStringEncodingUTF8);
			CFArrayAppendValue(arrDataclasses, sPhoto);
			CFRelease(sPhoto);
		}
		if ((flags & SYNC_DATA) != 0)
		{
			CFStringRef sData = CFStringCreateWithCString(NULL, "Data", kCFStringEncodingUTF8);
			CFArrayAppendValue(arrDataclasses, sData);
			CFRelease(sData);
		}
		if ((flags & SYNC_KEYBAG) != 0)
		{
			CFStringRef sKeyBag = CFStringCreateWithCString(NULL, "Keybag", kCFStringEncodingUTF8);
			CFArrayAppendValue(arrDataclasses, sKeyBag);
			CFRelease(sKeyBag);
		}
		if ((flags & SYNC_APPLICATION) != 0)
		{
			CFStringRef sApplication = CFStringCreateWithCString(NULL, "Application", kCFStringEncodingUTF8);
			CFArrayAppendValue(arrDataclasses, sApplication);
			CFRelease(sApplication);
		}

		CFMutableDictionaryRef dictDataclassAnchors = CFDictionaryCreateMutable(NULL, 0, kCFTypeDictionaryKeyCallBacks, kCFTypeDictionaryValueCallBacks);

		if ((flags & SYNC_RINGTONG_DICT) != 0)
		{
			CFStringRef sKey = CFStringCreateWithCString(NULL, "Ringtong", kCFStringEncodingUTF8);
			CFStringRef sValue = CFStringCreateWithCString(NULL, "1", kCFStringEncodingUTF8);

			CFDictionaryAddValue(dictDataclassAnchors, sKey, sValue);

			CFRelease(sKey);
			CFRelease(sValue);
		}
		if ((flags & SYNC_MEDIA_DICT) != 0)
		{
			CFStringRef sKey = CFStringCreateWithCString(NULL, "Media", kCFStringEncodingUTF8);
			CFStringRef sValue = CFStringCreateWithCString(NULL, "1", kCFStringEncodingUTF8);

			CFDictionaryAddValue(dictDataclassAnchors, sKey, sValue);

			CFRelease(sKey);
			CFRelease(sValue);
		}
		if ((flags & SYNC_VOICEMEMO_DICT) != 0)
		{
			CFStringRef sKey = CFStringCreateWithCString(NULL, "VoiceMemo", kCFStringEncodingUTF8);
			CFStringRef sValue = CFStringCreateWithCString(NULL, "1", kCFStringEncodingUTF8);

			CFDictionaryAddValue(dictDataclassAnchors, sKey, sValue);

			CFRelease(sKey);
			CFRelease(sValue);
		}

		CFMutableArrayRef arrSyncedDataclasses = CFArrayCreateMutable(NULL, 0, kCFTypeArrayCallBacks);

		if ((flags & SYNC_RINGTONG_ARR2) != 0)
		{
			CFStringRef sValue = CFStringCreateWithCString(NULL, "Ringtong", kCFStringEncodingUTF8);
			CFArrayAppendValue(arrSyncedDataclasses, sValue);
			CFRelease(sValue);
		}
		if ((flags & SYNC_MEDIA_ARR2) != 0)
		{
			CFStringRef sValue = CFStringCreateWithCString(NULL, "Media", kCFStringEncodingUTF8);
			CFArrayAppendValue(arrSyncedDataclasses, sValue);
			CFRelease(sValue);
		}
		if ((flags & SYNC_VOICEMEMO_ARR2) != 0)
		{
			CFStringRef sValue = CFStringCreateWithCString(NULL, "VoiceMemo", kCFStringEncodingUTF8);
			CFArrayAppendValue(arrSyncedDataclasses, sValue);
			CFRelease(sValue);
		}
		if ((flags & SYNC_BOOK_ARR2) != 0)
		{
			CFStringRef sValue = CFStringCreateWithCString(NULL, "Book", kCFStringEncodingUTF8);
			CFArrayAppendValue(arrSyncedDataclasses, sValue);
			CFRelease(sValue);
		}
		if ((flags & SYNC_PHOTO_ARR2) != 0)
		{
			CFStringRef sValue = CFStringCreateWithCString(NULL, "Photo", kCFStringEncodingUTF8);
			CFArrayAppendValue(arrSyncedDataclasses, sValue);
			CFRelease(sValue);
		}
		if ((flags & SYNC_DATA_ARR2) != 0)
		{
			CFStringRef sValue = CFStringCreateWithCString(NULL, "Data", kCFStringEncodingUTF8);
			CFArrayAppendValue(arrSyncedDataclasses, sValue);
			CFRelease(sValue);
		}
		if ((flags & SYNC_APPLICATION_ARR2) != 0)
		{
			CFStringRef sValue = CFStringCreateWithCString(NULL, "Application", kCFStringEncodingUTF8);
			CFArrayAppendValue(arrSyncedDataclasses, sValue);
			CFRelease(sValue);
		}

		CFMutableDictionaryRef dictHostInfo = CFDictionaryCreateMutable(NULL, 0, kCFTypeDictionaryKeyCallBacks, kCFTypeDictionaryValueCallBacks);

		CFStringRef sKey = CFStringCreateWithCString(NULL, "Version", kCFStringEncodingUTF8);
		CFStringRef sValue = CFStringCreateWithCString(NULL, m_Version.c_str(), kCFStringEncodingUTF8);
		CFDictionaryAddValue(dictHostInfo, sKey, sValue);
		CFRelease(sKey);
		CFRelease(sValue);

		sKey = CFStringCreateWithCString(NULL, "SyncedDataclasses", kCFStringEncodingUTF8);
		CFDictionaryAddValue(dictHostInfo, sKey, arrSyncedDataclasses);
		CFRelease(sKey);

		sKey = CFStringCreateWithCString(NULL, "SyncHostName", kCFStringEncodingUTF8);
		sValue = CFStringCreateWithCString(NULL, m_ComputerName.c_str(), kCFStringEncodingUTF8);
		CFDictionaryAddValue(dictHostInfo, sKey, sValue);
		CFRelease(sKey);
		CFRelease(sValue);

		sKey = CFStringCreateWithCString(NULL, "LibraryID", kCFStringEncodingUTF8);
		sValue = CFStringCreateWithCString(NULL, m_LibraryID.c_str(), kCFStringEncodingUTF8);
		CFDictionaryAddValue(dictHostInfo, sKey, sValue);
		CFRelease(sKey);
		CFRelease(sValue);

		sKey = CFStringCreateWithCString(NULL, "Grappa", kCFStringEncodingUTF8);
		CFDataRef sValue_grappa = CFDataCreate(NULL, (unsigned char*)grappa.data(), grappa.length());
		CFDictionaryAddValue(dictHostInfo, sKey, sValue_grappa);
		CFRelease(sKey);
		CFRelease(sValue_grappa);


		CFMutableDictionaryRef dictRsp = CFDictionaryCreateMutable(NULL, 0, kCFTypeDictionaryKeyCallBacks, kCFTypeDictionaryValueCallBacks);

		sKey = CFStringCreateWithCString(NULL, "Dataclasses", kCFStringEncodingUTF8);
		CFDictionaryAddValue(dictRsp, sKey, arrDataclasses);
		CFRelease(sKey);

		sKey = CFStringCreateWithCString(NULL, "DataclassAnchors", kCFStringEncodingUTF8);
		CFDictionaryAddValue(dictRsp, sKey, dictDataclassAnchors);
		CFRelease(sKey);

		sKey = CFStringCreateWithCString(NULL, "HostInfo", kCFStringEncodingUTF8);
		CFDictionaryAddValue(dictRsp, sKey, dictHostInfo);
		CFRelease(sKey);

		sKey = CFStringCreateWithCString(NULL, "RequestingSync", kCFStringEncodingUTF8);
		auto rspMsg = ATCFMessageCreate(ATHostConnectionGetCurrentSessionNumber(m_ath), sKey, dictRsp);
		CFRelease(sKey);

		ATHostConnectionSendMessage(m_ath, rspMsg);

		CFRelease(arrDataclasses);
		CFRelease(arrSyncedDataclasses);
		CFRelease(dictDataclassAnchors);
		CFRelease(dictHostInfo);
		CFRelease(dictRsp);
		CFRelease(rspMsg);
		return;
	}

	void iOSDevice::SendMetadataSyncFinished(unsigned int flags)
	{
		CFMutableDictionaryRef dictDataclassAnchors = CFDictionaryCreateMutable(NULL, 0, kCFTypeDictionaryKeyCallBacks, kCFTypeDictionaryValueCallBacks);

		if ((flags & SYNC_FINISHED_BOOK) != 0)
		{
			CFStringRef sKey = CFStringCreateWithCString(NULL, "Book", kCFStringEncodingUTF8);
			CFStringRef sValue = CFStringCreateWithCString(NULL, "1", kCFStringEncodingUTF8);
			CFDictionaryAddValue(dictDataclassAnchors, sKey, sValue);
			CFRelease(sKey);
			CFRelease(sValue);
		}

		if ((flags & SYNC_FINISHED_DATA) != 0)
		{
			CFStringRef sKey = CFStringCreateWithCString(NULL, "Data", kCFStringEncodingUTF8);
			CFStringRef sValue = CFStringCreateWithCString(NULL, "1", kCFStringEncodingUTF8);
			CFDictionaryAddValue(dictDataclassAnchors, sKey, sValue);
			CFRelease(sKey);
			CFRelease(sValue);
		}

		if ((flags & SYNC_FINISHED_RINGTONE) != 0)
		{
			CFStringRef sKey = CFStringCreateWithCString(NULL, "Ringtone", kCFStringEncodingUTF8);
			CFStringRef sValue = CFStringCreateWithCString(NULL, "1", kCFStringEncodingUTF8);
			CFDictionaryAddValue(dictDataclassAnchors, sKey, sValue);
			CFRelease(sKey);
			CFRelease(sValue);
		}

		if ((flags & SYNC_FINISHED_MEDIA) != 0)
		{
			CFStringRef sKey = CFStringCreateWithCString(NULL, "Media", kCFStringEncodingUTF8);
			CFStringRef sValue = CFStringCreateWithCString(NULL, "1", kCFStringEncodingUTF8);
			CFDictionaryAddValue(dictDataclassAnchors, sKey, sValue);
			CFRelease(sKey);
			CFRelease(sValue);
		}

		if ((flags & SYNC_FINISHED_VOICEMEMO) != 0)
		{
			CFStringRef sKey = CFStringCreateWithCString(NULL, "VoiceMemo", kCFStringEncodingUTF8);
			CFStringRef sValue = CFStringCreateWithCString(NULL, "1", kCFStringEncodingUTF8);
			CFDictionaryAddValue(dictDataclassAnchors, sKey, sValue);
			CFRelease(sKey);
			CFRelease(sValue);
		}

		CFMutableDictionaryRef dictSyncTypes = CFDictionaryCreateMutable(NULL, 0, kCFTypeDictionaryKeyCallBacks, kCFTypeDictionaryValueCallBacks);

		if ((flags & SYNC_FINISHED_2_KEYBAG) != 0)
		{
			CFStringRef sKey = CFStringCreateWithCString(NULL, "Keybag", kCFStringEncodingUTF8);
			int value = 1;
			CFNumberRef nValue = CFNumberCreate(NULL, kCFNumberSInt32Type, &value);
			CFDictionaryAddValue(dictSyncTypes, sKey, nValue);
			CFRelease(sKey);
		}

		if ((flags & SYNC_FINISHED_2_APPLICATION) != 0)
		{
			CFStringRef sKey = CFStringCreateWithCString(NULL, "Application", kCFStringEncodingUTF8);
			int value = 1;
			CFNumberRef nValue = CFNumberCreate(NULL, kCFNumberSInt32Type, &value);
			CFDictionaryAddValue(dictSyncTypes, sKey, nValue);
			CFRelease(sKey);
		}

		if ((flags & SYNC_FINISHED_2_VOICEMEMO) != 0)
		{
			CFStringRef sKey = CFStringCreateWithCString(NULL, "VoiceMemo", kCFStringEncodingUTF8);
			int value = 1;
			CFNumberRef nValue = CFNumberCreate(NULL, kCFNumberSInt32Type, &value);
			CFDictionaryAddValue(dictSyncTypes, sKey, nValue);
			CFRelease(sKey);
		}

		if ((flags & SYNC_FINISHED_2_RINGTONE) != 0)
		{
			CFStringRef sKey = CFStringCreateWithCString(NULL, "Ringtone", kCFStringEncodingUTF8);
			int value = 1;
			CFNumberRef nValue = CFNumberCreate(NULL, kCFNumberSInt32Type, &value);
			CFDictionaryAddValue(dictSyncTypes, sKey, nValue);
			CFRelease(sKey);
		}

		if ((flags & SYNC_FINISHED_2_MEDIA) != 0)
		{
			CFStringRef sKey = CFStringCreateWithCString(NULL, "Media", kCFStringEncodingUTF8);
			int value = 1;
			CFNumberRef nValue = CFNumberCreate(NULL, kCFNumberSInt32Type, &value);
			CFDictionaryAddValue(dictSyncTypes, sKey, nValue);
			CFRelease(sKey);
		}

		if ((flags & SYNC_FINISHED_2_BOOK) != 0)
		{
			CFStringRef sKey = CFStringCreateWithCString(NULL, "Book", kCFStringEncodingUTF8);
			int value = 1;
			CFNumberRef nValue = CFNumberCreate(NULL, kCFNumberSInt32Type, &value);
			CFDictionaryAddValue(dictSyncTypes, sKey, nValue);
			CFRelease(sKey);
		}

		if ((flags & SYNC_FINISHED_2_DATA) != 0)
		{
			CFStringRef sKey = CFStringCreateWithCString(NULL, "Data", kCFStringEncodingUTF8);
			int value = 1;
			CFNumberRef nValue = CFNumberCreate(NULL, kCFNumberSInt32Type, &value);
			CFDictionaryAddValue(dictSyncTypes, sKey, nValue);
			CFRelease(sKey);
		}

		ATHostConnectionSendMetadataSyncFinished(m_ath, dictSyncTypes, dictDataclassAnchors);

		CFRelease(dictDataclassAnchors);
		CFRelease(dictSyncTypes);
		return;
	}

	bool iOSDevice::OpenIOSFileSystem()
	{
		void* serviceHandle;
		CFStringRef sServiceName = CFStringCreateWithCString(NULL, "com.apple.afc", kCFStringEncodingUTF8);
		if (AMDeviceStartService(m_deviceHandle, sServiceName, &serviceHandle, NULL) != 0) {
			return false;
		}
		CFRelease(sServiceName);
		if (AFCConnectionOpen(serviceHandle, 0, &m_afc) != 0) {
			return false;
		}
		return true;
	}

	size_t iOSDevice::AFCFileSize(string path)
	{
		size_t rqSize = 0;
		afc_dictionary* fileinfo=nullptr;
		if (AFCFileInfoOpen(m_afc, path.c_str(), &fileinfo) != 0) {
			return 0;
		}
		char* pKey, * pValue;
		AFCKeyValueRead(fileinfo, &pKey, &pValue);
		while (pKey || pValue) {
			if (pKey == NULL || pValue == NULL)
				break;
			if (!_stricmp(pKey, "st_size")) {
				rqSize = atoi(pValue);
				break;
			}
			AFCKeyValueRead(fileinfo, &pKey, &pValue);
		}
		AFCKeyValueClose(fileinfo);
		return rqSize;
	}


	bool iOSDevice::ReadAfsyncRq(string& afsync_rq, string& afsync_rq_sig)
	{
		//��ȡ	afsync.rq 	
		size_t  rq_size = AFCFileSize(m_afsync_rq_path);
		afsync_rq.resize(rq_size);
		AFCFileRef fileAfsyncRq = NULL;
		unsigned int ret = AFCFileRefOpen(m_afc, m_afsync_rq_path.c_str(), AFC_FILEMODE_READ, &fileAfsyncRq);
		if (ret != 0) {
			return false;
		}
		//size_t len = rq_size;
		ret = AFCFileRefRead(m_afc, fileAfsyncRq, (void *)afsync_rq.data(), &rq_size);
		if (ret != 0) {
			return false;
		}
		AFCFileRefClose(m_afc, fileAfsyncRq);

		// ��ȡ afsync.rq.sig 
		size_t rq_sig_size = 21; 
		afsync_rq_sig.resize(rq_sig_size);
		AFCFileRef fileAfsyncRqSig = NULL;
		ret = AFCFileRefOpen(m_afc, m_afsync_rq_sig_path.c_str(), AFC_FILEMODE_READ, &fileAfsyncRqSig);
		if (ret != 0) {
			return false;
		}
		//size_t sig_len = rq_sig_size;
		ret = AFCFileRefRead(m_afc, fileAfsyncRqSig, (void*)afsync_rq_sig.data(), &rq_sig_size);
		if (ret != 0) {
			return false;
		}
		AFCFileRefClose(m_afc, fileAfsyncRqSig);
		return true;
	}

	bool iOSDevice::WriteAfsyncRs(const string afsync_rs, const string afsync_rs_sig)
	{
		AFCFileRef fileAfsyncRs = NULL;
		// дafsync.rs�ļ�
		if (AFCFileRefOpen(m_afc, m_filename_afsync_rs.c_str(), AFC_FILEMODE_WRITE, &fileAfsyncRs) != 0) {
			return false;
		}
		unsigned long ret = AFCFileRefWrite(m_afc, fileAfsyncRs, (void *)afsync_rs.data(), afsync_rs.size());
		if (ret != 0) {
			return false;
		}
		AFCFileRefClose(m_afc, fileAfsyncRs);
		// дafsync.rs.sig�ļ�
		AFCFileRef fileAfsyncRsSig = NULL;
		ret = AFCFileRefOpen(m_afc, m_filename_afsync_rs_sig.c_str(), AFC_FILEMODE_WRITE, &fileAfsyncRsSig);
		if (ret != 0) {
			return false;
		}
		ret = AFCFileRefWrite(m_afc, fileAfsyncRsSig, (void*)afsync_rs_sig.data(), afsync_rs_sig.size());
		if (ret != 0) {
			return false;
		}
		AFCFileRefClose(m_afc, fileAfsyncRsSig);
		return true;
	}
	// ��ȡ��Ϣ����,��ȡ�ɹ���Ϣ��д��athostMessage�ṹ����
	// int timeout  ��ʱ����λΪ����
	bool iOSDevice::ReceiveMessage(int timeout)
	{
		ATHostConnectionRetain(m_ath);
		auto startTime = chrono::high_resolution_clock::now();
		chrono::milliseconds durationMs;
		do {
			CFDictionaryRef dict = ATHostConnectionReadMessage(m_ath);
			//��ȡ������
			CFStringRef sKey = CFStringCreateWithCString(NULL, "Command", kCFStringEncodingUTF8);
			CFStringRef sValue = CFDictionaryGetValue(dict, sKey);
			CFIndex cmd_len = CFStringGetLength(sValue);
			string sCommand(cmd_len, '\0');
			CFStringGetCString(sValue, (char*)sCommand.data(), cmd_len + 1, kCFStringEncodingUTF8);
			CFRelease(sKey);
			CFRelease(sValue);
			
			logger.log("udid:%s,ReceiveMessage:%s",m_udid.c_str(),sCommand.c_str());

			if ("SyncAllowed" == sCommand) {
				athostMessage.SyncAllowed = true;
			}
			if ("ReadyForSync" == sCommand) {
				athostMessage.ReadyForSync = true;
				CFStringRef sKey = CFStringCreateWithCString(NULL, "Params", kCFStringEncodingUTF8);
				CFDictionaryRef DictParams = CFDictionaryGetValue(dict, sKey);
				CFRelease(sKey);
				sKey = CFStringCreateWithCString(NULL, "DeviceInfo", kCFStringEncodingUTF8);
				CFDictionaryRef DictDeviceInfo = CFDictionaryGetValue(DictParams, sKey);
				CFRelease(sKey);
				sKey = CFStringCreateWithCString(NULL, "Grappa", kCFStringEncodingUTF8);
				CFDataRef DataGrappa = CFDictionaryGetValue(DictDeviceInfo, sKey);
				CFRelease(sKey);
				auto grappa_len = CFDataGetLength(DataGrappa);
				athostMessage.grappa.resize(grappa_len);
				CFDataGetBytes(DataGrappa, CFRangeMake(0, grappa_len), (unsigned char*)athostMessage.grappa.data());
				// mac win32 ����μ����ܲ�ͨ
				//CFRelease(DataGrappa);
				//CFRelease(DictDeviceInfo);
				//CFRelease(DictParams); 
			}

			if ("Ping" == sCommand) {
				athostMessage.Ping = true;
				ATHostConnectionSendPing(m_ath);
			}
			if ("SyncFinished" == sCommand) {
				athostMessage.SyncFinished = true;
				break;
			}
			if ("SyncFailed" == sCommand) {
				athostMessage.SyncFailed = true;
				break;
			}
			durationMs = chrono::duration_cast<std::chrono::milliseconds>(chrono::high_resolution_clock::now() - startTime);
		} while (durationMs.count()< timeout && m_connect);
		ATHostConnectionRelease(m_ath);
		if (durationMs.count() >= timeout || !m_connect)
		{
			logger.log("udid:%s,ReceiveMessage terminate.",m_udid.c_str());
			return false;
		}
		logger.log("udid:%s,ReceiveMessage end.", m_udid.c_str());
		return true;
	}

	bool iOSDevice::DoPair() {
		int rc = 0;
 		while(m_connect){
 			AMDeviceIsPaired(m_deviceHandle);
 			rc = AMDeviceValidatePairing(m_deviceHandle);
 			if(rc == 0){
 				break;
 			}
 			// Do pairing
 			CFDictionaryRef dictOptions = CFDictionaryCreateMutable(NULL, 0, kCFTypeDictionaryKeyCallBacks, kCFTypeDictionaryValueCallBacks);
 			CFStringRef key = CFStringCreateWithCString(NULL, "ExtendedPairingErrors", kCFStringEncodingUTF8);
 			CFDictionarySetValue(dictOptions, key, kCFBooleanTrue);
 			rc = AMDevicePairWithOptions(m_deviceHandle, dictOptions);

 			if (rc == 0xe800001a){
				logger.log("udid:%s�������������������ios�����档",m_udid.c_str());
				DoPairCallback({ m_udid.c_str(),m_deviceName.c_str(),m_productType.c_str(), AuthorizeReturnStatus::AuthorizeDopairingLocking });

 			}
 			else if (rc == 0xe8000096){
				logger.log("udid:%s�������豸�˰��¡����Ρ���ť��", m_udid.c_str());
				DoPairCallback({ m_udid.c_str(),m_deviceName.c_str(),m_productType.c_str(), AuthorizeReturnStatus::AuthorizeDopairingTrust });
 			}
 			else if (rc == 0xe8000095){
				logger.log("udid:%s��ʹ���߰����ˡ������Ρ���ť��", m_udid.c_str());
				DoPairCallback({ m_udid.c_str(),m_deviceName.c_str(),m_productType.c_str(), AuthorizeReturnStatus::AuthorizeDopairingNotTrust });
 				return false;
 			}
 			else if (rc == 0){
 				break;
 			}
			this_thread::sleep_for(chrono::milliseconds(1000));
 		}
		if (m_connect) {
			return true;
		}
		else {
			return false;
		}
	}
}