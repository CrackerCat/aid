#include "passport/itunes_native_interface.h"
#include <ImageHlp.h>
#pragma comment(lib,"Imagehlp.lib")
#include "passport/itunes_internal_interface.h"
#include "base/string/string_conv.h"
#include "ABI/ios_authorize/apple_import.h"
#pragma comment(lib,"Version.lib")

namespace passport{
	namespace internal{
		void AddEnvironmentVariable(const std::wstring& path){
			wchar_t env_path[4096] = {0};
			GetEnvironmentVariableW(L"PATH",env_path,4095);
			std::wstring new_env_path = (std::wstring(env_path) + std::wstring(L";")) + path;
			size_t posiltion = new_env_path.find_last_of('\\');
			if(posiltion!=std::wstring::npos){
				new_env_path[posiltion] = 0;
			}
			SetEnvironmentVariableW(L"PATH",new_env_path.c_str());
		}
		std::wstring GetSoftwareReleaseVersion(const wchar_t* full_path){
			VS_FIXEDFILEINFO *pVerInfo = NULL;
			DWORD dwTemp, dwSize;
			BYTE *pData = NULL;
			UINT uLen;
			dwSize = GetFileVersionInfoSizeW(full_path, &dwTemp);
			if(dwSize == 0){
				return L"";
			}
			pData = new BYTE[dwSize+1];
			if(pData == NULL){
				return L"";
			}
			if(!GetFileVersionInfoW(full_path, 0, dwSize, pData)){
				delete[] pData;
				return L"";
			}
			if(!VerQueryValueW(pData,L"\\",(void **)&pVerInfo,&uLen)){
				delete[] pData;
				return L"";
			}
			DWORD verMS = pVerInfo->dwFileVersionMS;
			DWORD verLS = pVerInfo->dwFileVersionLS;
			DWORD major = HIWORD(verMS);
			DWORD minor = LOWORD(verMS);
			DWORD build = HIWORD(verLS);
			DWORD revision = LOWORD(verLS);
			delete[] pData;
			wchar_t version[1024] = {0};
			_snwprintf(version,1024,L"%d.%d.%d.%d",major,minor,build,revision);
			return version;
		}
		std::wstring GetAppleMobileDeviceSupportDll(const std::wstring dll_name){
			void* hSetting = NULL;
			unsigned long length = 0;
			wchar_t* path = new wchar_t[MAX_PATH*sizeof(wchar_t)];
			if(path==NULL){
				return L"";
			}
			if(::RegCreateKeyW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Apple Inc.\\Apple Mobile Device Support\\Shared",reinterpret_cast<PHKEY>(&hSetting))!=ERROR_SUCCESS){
				return L"";
			}
			if(::RegQueryValueExW(reinterpret_cast<HKEY>(hSetting),dll_name.c_str(), NULL, NULL, NULL, &length)!=ERROR_SUCCESS){
				return L"";
			}
			::RegQueryValueExW(reinterpret_cast<HKEY>(hSetting),dll_name.c_str(), NULL, NULL, (LPBYTE)path,&length);
			::RegCloseKey(reinterpret_cast<HKEY>(hSetting));
			std::wstring shared_dll(path);
			delete[] path;
			return shared_dll;
		}
		std::wstring GetAppleApplicationSupportDll(const std::wstring dll_name){
			void* hSetting = NULL;
			unsigned long length = 0;
			wchar_t* pCoreFoundationPath = new wchar_t[MAX_PATH*sizeof(wchar_t)];
			if(pCoreFoundationPath==NULL){
				return L"";
			}
			if(::RegCreateKeyW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Apple Inc.\\Apple Application Support",reinterpret_cast<PHKEY>(&hSetting))!=ERROR_SUCCESS){
				return L"";
			}
			if(::RegQueryValueExW(reinterpret_cast<HKEY>(hSetting),L"InstallDir", NULL, NULL, NULL, &length)!=ERROR_SUCCESS){
				return L"";
			}
			::RegQueryValueExW(reinterpret_cast<HKEY>(hSetting),L"InstallDir", NULL, NULL, (LPBYTE)pCoreFoundationPath, &length);
			::RegCloseKey(reinterpret_cast<HKEY>(hSetting));
			if(pCoreFoundationPath[wcslen(pCoreFoundationPath)-1] != '\\'){
				wcscat(pCoreFoundationPath,L"\\");
			}
			if(dll_name.length()>0){
				wcscat(pCoreFoundationPath,dll_name.c_str());
			}
			std::wstring support_dll(pCoreFoundationPath);
			delete[] pCoreFoundationPath;
			return support_dll;
		}
		std::wstring GetDirectory(){
			wchar_t buffer[MAX_PATH] = {0};
			wchar_t drive[_MAX_DRIVE] = {0};
			wchar_t dir[_MAX_DIR] = {0};
			wchar_t fname[_MAX_FNAME] = {0};
			wchar_t ext[_MAX_EXT] = {0};
			GetModuleFileNameW(NULL,buffer,MAX_PATH);
			_wsplitpath_s(buffer,drive,_MAX_DRIVE,dir,_MAX_DIR,fname,_MAX_FNAME,ext,_MAX_EXT);
			return (std::wstring(std::wstring(drive)+std::wstring(dir)));
		}
		std::wstring GetITunesInstallDll(const std::wstring dll_name){
			return GetDirectory() + std::wstring(L"iTunes\\") + dll_name;
		}
	}
	iTunesNativeInterface* iTunesNativeInterface::GetInstance(){
		static iTunesNativeInterface* info;
		if(!info){
			iTunesNativeInterface* new_info = new iTunesNativeInterface();
			if(InterlockedCompareExchangePointer(reinterpret_cast<PVOID*>(&info),new_info,NULL)){
				delete new_info;
			}
			const std::wstring corefp_dll = internal::GetITunesInstallDll(L"CoreFP.dll");
			if (PathFileExistsW(corefp_dll.c_str()))
				new_info->HKLMCustomizeModule(CustomizeModule::kCoreFP, corefp_dll.c_str());
			
		}
		return info;
	}
	iTunesNativeInterface::iTunesNativeInterface(void){
		internal::AddEnvironmentVariable(internal::GetITunesInstallDll(L""));
	}
	iTunesNativeInterface::~iTunesNativeInterface(void){

	}
	bool iTunesNativeInterface::HKLMCustomizeModule(const CustomizeModule& customize_module, const wchar_t* module_name){
		std::wstring sub_key = L"";
		std::wstring sub_key_name = L"";
		std::wstring sub_key_name2 = L"";
		if (customize_module == CustomizeModule::kCoreFP){
			sub_key = L"SOFTWARE\\Apple Inc.\\CoreFP";
			sub_key_name = L"Libi4CFPath";
			sub_key_name2 = L"LibiiiiPath";
		}
		else if (customize_module == CustomizeModule::kiTunesMobileDeviceDLL){
			sub_key = L"SOFTWARE\\Apple Inc.\\Apple Mobile Device Support\\Shared";
			sub_key_name = L"iTunesMobileDeviceDLL";
		}
		else{
			return true;
		}
		HKEY h_setting = NULL;
		bool is_success = true;
		if (::RegCreateKeyW(HKEY_LOCAL_MACHINE, sub_key.c_str(), reinterpret_cast<PHKEY>(&h_setting)) != ERROR_SUCCESS)
			return is_success;
		if (::RegSetValueExW(h_setting, 
			sub_key_name.c_str(),
			0, 
			REG_SZ, 
			(const BYTE*)module_name, 
			wcslen(module_name)*sizeof(wchar_t)) == ERROR_SUCCESS)
			is_success = false;
		if (customize_module == CustomizeModule::kCoreFP) {
			if (::RegSetValueExW(h_setting,
				sub_key_name2.c_str(),
				0,
				REG_SZ,
				(const BYTE*)module_name,
				wcslen(module_name) * sizeof(wchar_t)) == ERROR_SUCCESS)
				is_success = false;
		}
		RegCloseKey(h_setting);
		return is_success;
	}

	bool iTunesNativeInterface::IsMachineAmd64(const wchar_t* file,const wchar_t* dir){
		bool result = false;
		PLOADED_IMAGE load_image = ImageLoad(ABI::base::UnicodeToAscii(file).c_str(),ABI::base::UnicodeToAscii(dir).c_str());
		if(load_image->FileHeader->FileHeader.Machine==IMAGE_FILE_MACHINE_AMD64){
			result = true;
		}
		else if(load_image->FileHeader->FileHeader.Machine==IMAGE_FILE_MACHINE_I386){
			result = false;
		}
		ImageUnload(load_image);
		return result;
	}
	bool iTunesNativeInterface::iTunesDllVersion(const wchar_t* version){
		static wchar_t g_version[1024] = {0};
		if(!g_version[0]){
			const std::wstring itunes_dll = internal::GetITunesInstallDll(L"iTunes.dll");
			wcscpy_s(g_version,1023,internal::GetSoftwareReleaseVersion(itunes_dll.c_str()).c_str());
		}
		return (std::wstring(g_version)==version);
	}
	bool iTunesNativeInterface::AirTrafficHostDllVersion(const wchar_t* version){
		static wchar_t g_version_a[1024] = {0};
		if(!g_version_a[0]){
			const std::wstring air_traffic_host_dll = internal::GetITunesInstallDll(L"AirTrafficHost.dll");
			wcscpy_s(g_version_a,1023,internal::GetSoftwareReleaseVersion(air_traffic_host_dll.c_str()).c_str());
		}
		return (std::wstring(g_version_a)==version);
	}
}