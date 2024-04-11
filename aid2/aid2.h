#ifndef AID2_H
#define AID2_H

#ifdef AID2_EXPORTS
#define AID2_API __declspec(dllexport)
#else
#define AID2_API __declspec(dllimport)
#endif

//授权返回状态信息
enum AuthorizeReturnStatus
{
	AuthorizeSuccess = 0,   //授权成功
	AuthorizeDopairingLocking = 1,  //执行配对中锁屏，执行动作请解开锁屏
	AuthorizeFailed = 2,  //授权失败
	AuthorizeDopairingTrust = 3,  //执行配对中需要按信任，执行动作请按信任
	AuthorizeDopairingNotTrust = 4  //执行配对中使用者按下不信任
};

typedef void (*AuthorizeDeviceCallbackFunc)(const char* udid, AuthorizeReturnStatus ReturnFlag);//授权回调函数定义
typedef void (*InstallApplicationFunc)(const char* status, const int percent);//安装函数回调定义
typedef void (*ConnectCallbackFunc)(const char* udid, const char* DeviceName, const char* ProductType, const char* DeviceEnclosureColor, const char* MarketingName);//连接函数回调定义
typedef void (*DisconnectCallbackFunc)(const char* udid);//断开设备连接回调定义

/*******************************************************
以线程方式开户监听，不阻塞主线程
参数：autoAuthorize 是否自动授权，true 默认值，为自动授权，flase 为不自动
      ipaPath  自动安装ipa 包的路径，输入空值不安装
返回值：成功为true
**************************************************/
extern "C" AID2_API bool StartListen(bool autoAuthorize=true,const char * ipaPath=nullptr);
/*******************************************************
停止监听，关闭监听线程
返回值：成功为true
**************************************************/
extern "C" AID2_API bool StopListen();

/*******************************************************
根据deviceHandle授权
参数：deviceHandle
返回值：成功为true
*******************************************************/
extern "C" AID2_API bool AuthorizeDeviceEx(void* deviceHandle);


/*******************************************************
根据udid授权
参数：udid
返回值：成功为true
*******************************************************/
extern "C" AID2_API bool AuthorizeDevice(const char * udid);


/*******************************************************
根据udid安装path 的ipa 包
参数：deviceHandle
	  ipaPath ipa包的路径
返回值：成功为true
*******************************************************/
extern "C" AID2_API bool InstallApplicationEx(void* deviceHandle, const char* ipaPath);


/*******************************************************
根据udid安装path 的ipa 包
参数：udid
      ipaPath ipa包的路径
返回值：成功为true
*******************************************************/
extern "C" AID2_API bool InstallApplication(const char* udid,const char* ipaPath);

/*******************************************************
注册授权回调函数，授权过程中需要配对信息和授权结果通过回调函数通知
参数：callback 回调函数指针
**************************************************/
extern "C" AID2_API void RegisterAuthorizeCallback(AuthorizeDeviceCallbackFunc callback);


/*******************************************************
注册安装回调函数，授权过程中需要配对信息和授权结果通过回调函数通知
参数：callback 回调函数指针
**************************************************/
extern "C" AID2_API void RegisterInstallCallback(InstallApplicationFunc callback);

/*******************************************************
插入设备连接通知的回调函数
参数：callback 回调函数指针
**************************************************/
extern "C" AID2_API void RegisterConnectCallback(ConnectCallbackFunc callback);

/*******************************************************
断开设备连接通知的回调函数
参数：callback 回调函数指针
**************************************************/
extern "C" AID2_API void RegisterDisconnectCallback(DisconnectCallbackFunc callback);

#endif