#pragma once

#define SYNC_RINGTONE 0x100
#define SYNC_MEDIA 0x200
#define SYNC_VOICEMEMO 0x400
#define SYNC_BOOK 0x800
#define SYNC_PHOTO 0x1000
#define SYNC_DATA 0x2000
#define SYNC_KEYBAG 0x4000
#define SYNC_APPLICATION 0x8000

#define SYNC_RINGTONG_DICT 0x1
#define SYNC_MEDIA_DICT 0x2
#define SYNC_VOICEMEMO_DICT 0x4

#define SYNC_RINGTONG_ARR2 0x10000
#define SYNC_MEDIA_ARR2 0x20000
#define SYNC_VOICEMEMO_ARR2 0x40000
#define SYNC_BOOK_ARR2 0x80000
#define SYNC_PHOTO_ARR2 0x100000
#define SYNC_DATA_ARR2 0x200000
#define SYNC_APPLICATION_ARR2 0x800000

//////////////////////////////////////////////

#define SYNC_FINISHED_RINGTONE 0x01
#define SYNC_FINISHED_MEDIA 0x02
#define SYNC_FINISHED_VOICEMEMO 0x04
#define SYNC_FINISHED_BOOK 0x08
#define SYNC_FINISHED_DATA 0x20

#define SYNC_FINISHED_2_RINGTONE 0x1000000
#define SYNC_FINISHED_2_MEDIA 0x2000000
#define SYNC_FINISHED_2_VOICEMEMO 0x4000000
#define SYNC_FINISHED_2_BOOK 0x8000000
#define SYNC_FINISHED_2_PHOTO 0x10000000
#define SYNC_FINISHED_2_DATA 0x20000000
#define SYNC_FINISHED_2_KEYBAG 0x40000000
#define SYNC_FINISHED_2_APPLICATION 0x80000000

#include "iTunesApi/simpleApi.h"
#include <string>
#include <future>
namespace aid2{

	#ifndef AID2_H
	enum AuthorizeReturnStatus
	{
		AuthorizeSuccess = 0,   //��Ȩ�ɹ�
		AuthorizeDopairingLocking = 1,  //ִ�������������ִ�ж�����⿪����
		AuthorizeFailed = 2,  //��Ȩʧ��
		AuthorizeDopairingTrust = 3,  //ִ���������Ҫ�����Σ�ִ�ж����밴����
		AuthorizeDopairingNotTrust = 4  //ִ�������ʹ���߰��²�����
	};
	//�ص����������ṹ��
	struct DeviceParameter {
		const char* udid;
		const char* DeviceName;
		const char* ProductType;
		AuthorizeReturnStatus ReturnFlag;
	};
	typedef void (*AuthorizeDeviceCallbackFunc)(DeviceParameter r1);//�ص���������
	#endif

	using namespace std;
	class iOSDevice
	{
		public:
			struct ATHostMessage {
				bool SyncAllowed = false;
				bool ReadyForSync = false;
				bool SyncFinished = false;
				bool SyncFailed = false;
				bool Ping = false;
				string grappa;
			};

			ATHostMessage athostMessage;
			iOSDevice(AMDeviceRef deviceHandle) {m_deviceHandle = deviceHandle;};
			~iOSDevice() { m_deviceHandle = nullptr; };
			AMDeviceRef DeviceRef(){ return m_deviceHandle;	};
			string udid(){return m_udid;};
			string FairPlayCertificate(){return m_fair_play_certificate;};
			__int64 FairPlayDeviceType() { return m_fair_play_device_type; };
			string DeviceName() { return m_deviceName; };
			string ProductType() { return m_productType; };
			kAMDError Connect();
			kAMDError Disconnect();
			ConnectMode GetInterfaceType();
			bool GenDeviceInfo();  //�����豸������Ϣ
			bool GenFairDeviceInfo();  // ��ȡ��Ҫ��Ȩ�Ļ�����Ϣ,��ҪStartSession()����֮��ſ��Ե���
			bool ATHostConnection();  // �ɹ�����true
			void ATHostDisconnect();
			bool OpenIOSFileSystem();
			void CloseIOSFileSystem() { AFCConnectionClose(m_afc); };
			int DeleteAfsyncRq() { return AFCRemovePath(m_afc, m_afsync_rq_path.c_str()); };
			void SendSyncRequest(unsigned int flags, string grappa);
			void SendMetadataSyncFinished(unsigned int flags);
			bool ReadAfsyncRq(string &afsync_rq, string &afsync_rq_sig);
			bool WriteAfsyncRs(const string afsync_rs, const string afsync_rs_sig);
			bool ReceiveMessage(int timeout);  //��ȡ��Ϣ����,timeout����Ϊ��ʱʱ�䣬��λΪ���룬��ָ����δִ��������߳�
			bool DoPair();  //�����
			kAMDError StartSession();
			kAMDError StopSession();
			size_t AFCFileSize(string path);
			// ����ִ�����ʱ�ص���Ϣ��ͨ����������ʹ���߰��� ���κͲ�������Ϣ
			AuthorizeDeviceCallbackFunc DoPairCallback;  
	private:
		const string m_afsync_rq_path = "/AirFair/sync/afsync.rq";
		const string m_afsync_rq_sig_path = "/AirFair/sync/afsync.rq.sig";
		const string m_filename_afsync_rs = "/AirFair/sync/afsync.rs";
		const string m_filename_afsync_rs_sig = "/AirFair/sync/afsync.rs.sig";

		AMDeviceRef m_deviceHandle=nullptr;
		ATHRef m_ath = nullptr;
		AFCRef m_afc = nullptr;
		bool m_connect = false;
		//��������Ҫͬ����Ϣ
		string m_LibraryID =  "5AC547BA5322B210" ;
		string m_ComputerName = "pc";
		string m_Version = "12.6.0.100";

		// �������ֻ���ȡ������Ϣ
		string m_udid;  //���string 
		string m_fair_play_certificate;   // domain: com.apple.mobile.iTunes key: FairPlayCertificate   ���bytes
		__int64 m_fair_play_device_type=0;   //domain: com.apple.mobile.iTunes  key: FairPlayDeviceType+KeyTypeSupportVersion
		string m_deviceName;
		string m_productType;
	};

}
