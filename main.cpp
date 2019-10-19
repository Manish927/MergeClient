#include "FMergerClient.h"

int main(int argc, char* argv[])
{
    //GOOGE_PROTOBUF_VERIFY_VERSION;

        std::string remote(argv[1]);
        std::string file(argv[2]);

        if (remote.empty())
        {
            std::cout << "Usage: FMergerClient <ipaddress:port> <filename>" << std::endl;
            return -1;
        }

        Client client(remote, file);

        if (client.connectPeer())
        {
            client.send();
        }

    return 0;
}