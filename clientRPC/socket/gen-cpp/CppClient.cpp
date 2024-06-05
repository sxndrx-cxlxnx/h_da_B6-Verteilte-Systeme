#include <iostream>
#include <chrono>
#include "Wettanbieter.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace Wettanbietercpp;
using namespace std;
using namespace std::chrono;

int main(int argc, char **argv)
{

    // Source: https://www.auctoris.co.uk/2016/08/19/apache-thrift-tutorial-part-2/
    //         https://thrift.apache.org/tutorial/cpp.html

    std::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    //source: https://www.geeksforgeeks.org/measure-execution-time-function-cpp/

    auto start = high_resolution_clock::now();

    try
    {
        transport->open();

        client.addBet("cpp", 42, "A", "B", 1.0, 2.0, 3.0);

        transport->close();

    } catch (TException& tx) {
    cout << "ERROR: " << tx.what() << endl;
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);

    cout << "addBet: Dauer in Microsekunden: "<< duration.count() << endl;

    return 0;
}