#include <string>
#include <grpcpp/grpcpp.h>
#include "ProtoBuf/GenerateRS/GenerateRS.grpc.pb.h"
#include "ProtoBuf/GenerateRS/GenerateRS.pb.h"
#include "iTunesApi/simpleApi.h"

namespace aid2 {
	using namespace std;
	using grpc::Channel;
	using AppleRemoteAuth::aid;

	class aidClient {
	public:
		aidClient(std::shared_ptr<Channel> channel, AMDeviceRef deviceHandle);

		/**
		 * 生成Grappa
		 *
		 * @param grappa 输出参数，grappa参数
		 * @return 读取到为true,否则为flase
		 */
		bool GenerateGrappa(string& grappa);

		/**
		 * 生成Grappa afsync.rs和afsync.rs.sig 文件信息
		 *
		 * @param grappa 输入参数，grappa参数
		 * @return 成功为true,否则为flase
		 */
		bool GenerateRs(const string& grappa);

		/**
		 * 生成新的aidClient实例，不使用需要使用delete 删除。
		 *
		 * @param deviceHandle 输入参数，deviceHandle参数
		 * @return aidClient实例指针
		 */
		static aidClient* NewInstance(AMDeviceRef deviceHandle);
	private:
		std::unique_ptr<aid::Stub> stub_;
		string m_udid;
		AMDeviceRef m_deviceHandle;
		unsigned long m_grappa_session_id;
	};
}