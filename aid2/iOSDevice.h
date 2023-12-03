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
#include "aid2.h"
namespace aid2{
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
			iOSDevice(AMDeviceRef deviceHandle);
			~iOSDevice();
			AMDeviceRef DeviceRef(){ return m_deviceHandle;	};
			string udid();
			string FairPlayCertificate();
			__int64 FairPlayDeviceType();
			string DeviceName();
			string ProductType();
			kAMDError Connect();
			kAMDError Disconnect();
			ConnectMode GetInterfaceType();
			bool ATHostConnection();  // 成功返回true
			void ATHostDisconnect();
			bool OpenIOSFileSystem();
			void CloseIOSFileSystem() { AFCConnectionClose(m_afc); };
			int DeleteAfsyncRq() { return AFCRemovePath(m_afc, m_afsync_rq_path.c_str()); };
			void SendSyncRequest(unsigned int flags, string grappa);
			void SendMetadataSyncFinished(unsigned int flags);
			bool ReadAfsyncRq(string &afsync_rq, string &afsync_rq_sig);
			bool WriteAfsyncRs(const string afsync_rs, const string afsync_rs_sig);
			bool ReceiveMessage(int timeout);  //读取消息方法,timeout参数为超时时间，单位为毫秒，在指定内未执行完结束线程
			bool DoPair();  //做配对
			kAMDError StartSession();
			kAMDError StopSession();
			size_t AFCFileSize(string path);

			bool InstallApplication(const string path);

			// 定义执行配对时回调信息，通过参数提醒使用者按下 信任和不信任信息
			static AuthorizeDeviceCallbackFunc DoPairCallback;
			// 安装ipa回调函数指针
			static InstallApplicationFunc InstallApplicationCallback;

			

	private:
		const string m_afsync_rq_path = "/AirFair/sync/afsync.rq";
		const string m_afsync_rq_sig_path = "/AirFair/sync/afsync.rq.sig";
		const string m_filename_afsync_rs = "/AirFair/sync/afsync.rs";
		const string m_filename_afsync_rs_sig = "/AirFair/sync/afsync.rs.sig";

		AMDeviceRef m_deviceHandle=nullptr;
		ATHRef m_ath = nullptr;
		AFCRef m_afc = nullptr;
		bool m_connect = false;
		//以下是需要同步信息
		string m_LibraryID =  "5AC547BA5322B210" ;
		string m_ComputerName = "pc";
		string m_Version = "12.6.0.100";

		// 以下是手机获取到的信息
		string m_udid = "";  //存的udid 
		string m_fair_play_certificate;   // domain: com.apple.mobile.iTunes key: FairPlayCertificate   存的bytes
		uint64_t m_fair_play_device_type=0;   //domain: com.apple.mobile.iTunes  key: FairPlayDeviceType+KeyTypeSupportVersion
		string m_deviceName = "";
		string m_productType = "";
		// 安装应用回调处理函数
		static void install_callback(CFDictionaryRef dict, int arg);
		// copy ipa 文件到设备回调处理函数
		static void transfer_callback(CFDictionaryRef dict, int arg);
	};

}
