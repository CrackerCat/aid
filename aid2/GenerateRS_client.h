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
		 * ����Grappa
		 *
		 * @param grappa ���������grappa����
		 * @return ��ȡ��Ϊtrue,����Ϊflase
		 */
		bool GenerateGrappa(string& grappa);

		/**
		 * ����Grappa afsync.rs��afsync.rs.sig �ļ���Ϣ
		 *
		 * @param grappa ���������grappa����
		 * @return �ɹ�Ϊtrue,����Ϊflase
		 */
		bool GenerateRs(const string& grappa);

		/**
		 * �����µ�aidClientʵ������ʹ����Ҫʹ��delete ɾ����
		 *
		 * @param deviceHandle ���������deviceHandle����
		 * @return aidClientʵ��ָ��
		 */
		static aidClient* NewInstance(AMDeviceRef deviceHandle);
	private:
		std::unique_ptr<aid::Stub> stub_;
		string m_udid;
		AMDeviceRef m_deviceHandle;
		unsigned long m_grappa_session_id;
	};
}