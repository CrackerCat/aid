#include<fstream>
#include "Logger.h"
#include "GenerateRS_client.h"
#include "Afsync.h"
#include "iOSDeviceInfo.h"


namespace aid2 {
    using grpc::ClientContext;
    using grpc::Status;
    using AppleRemoteAuth::RemoteDeviceInfo;
    using AppleRemoteAuth::rsdata;
    using AppleRemoteAuth::Grappa;
    using AppleRemoteAuth::rqGeneGrappa;

    const char rootcert_path[] = "certificate/ca.pem";
    const char clientcert_path[] = "certificate/client.pem";
    const char clientkey_path[] = "certificate/client.key";

    static string get_file_contents(const char* fpath)
    {
        std::ifstream finstream(fpath);
        std::string contents;
        contents.assign((std::istreambuf_iterator<char>(finstream)),
            std::istreambuf_iterator<char>());
        finstream.close();
        return contents;
    }

    aidClient::aidClient(std::shared_ptr<Channel> channel, AMDeviceRef deviceHandle)
        : stub_(aid::NewStub(channel))
    {
        this->m_deviceHandle = deviceHandle;
        this->m_udid = getUdid(m_deviceHandle);
    }

     bool aidClient::GenerateGrappa(string& grappa)
    {
        rqGeneGrappa request;
        request.set_udid(m_udid.c_str(), m_udid.size());
        Grappa reply;
        ClientContext context;
        bool ret = false;
        // The actual RPC.
        Status status = stub_->GenerateGrappa(&context, request, &reply);

        if (status.ok()) {
            if (reply.ret()) {
                grappa = reply.grappadata();
                m_grappa_session_id = reply.grappa_session_id();
                ret = true;
            }
            else {
                logger.log("CreateHostGrappa error");
            }
        }
        else {
            logger.log("grpc调用错误：错误代码%d，错误信息%s", status.error_code(), status.error_message().c_str());
        }
        return ret;
    }

    bool aidClient::GenerateRs(const string& grappa)
    {
        //读取rq和rq_sid文件   
        try
        {
            iOSDeviceInfo appleInfo(m_deviceHandle);
            appleInfo.DoPair();
            Afsync afsync(m_deviceHandle);
            string rq_data = afsync.ReadRq();
            string rq_sig_data = afsync.ReadRqSig();
            
            bool ret = false;

            // 通过远程服务器来生成afsync.rs和afsync.rs.sig 文件内容
            RemoteDeviceInfo request;
            request.set_rq_data(rq_data.data(), rq_data.size());
            request.set_rq_sig_data(rq_sig_data.data(), rq_sig_data.size());
            request.set_grappa_session_id(m_grappa_session_id);
            request.set_fair_play_certificate(appleInfo.FairPlayCertificate().data(), appleInfo.FairPlayCertificate().size());
            request.set_fair_device_type(appleInfo.FairPlayDeviceType());
            request.set_fair_play_guid(m_udid.c_str(), m_udid.size());
            request.set_grappa(grappa.data(), grappa.size());;
            // Container for the data we expect from the server.
            rsdata reply;

            // Context for the client. It could be used to convey extra information to
            // the server and/or tweak certain RPC behaviors.
            ClientContext context;

            // The actual RPC.
            Status status = stub_->GenerateRS(&context, request, &reply);
            // 如果获取到了afsync.rs和afsync.rs.sig 文件，就写入到手机/AirFair/sync/目录下面
            if (status.ok()) {
                if (reply.ret()) {
                    afsync.WriteRs(reply.rs_data());
                    afsync.WriteRsSig(reply.rs_sig_data());
                    ret = true;
                }
                else
                {
                    logger.log("udid:%s genreate rs failed!", m_udid.c_str());
                }
            }
            else {
                logger.log("grpc调用错误：错误代码%d，错误信息%s", status.error_code(), status.error_message().c_str());
            }
            return ret;
        }
        catch (const char* e)
        {
            logger.log(e);
            return false;
        }
    }

    aidClient* aidClient::NewInstance(AMDeviceRef deviceHandle)
    {
        {
            auto rootcert = get_file_contents(rootcert_path);
            auto clientkey = get_file_contents(clientkey_path);
            auto clientcert = get_file_contents(clientcert_path);
            grpc::SslCredentialsOptions ssl_opts;
            ssl_opts.pem_root_certs = rootcert;
            ssl_opts.pem_private_key = clientkey;
            ssl_opts.pem_cert_chain = clientcert;
            std::shared_ptr<grpc::ChannelCredentials> creds = grpc::SslCredentials(ssl_opts);
            return new aidClient(grpc::CreateChannel("aid.aidserv.cn:50051", creds), deviceHandle);
        }
    }
}