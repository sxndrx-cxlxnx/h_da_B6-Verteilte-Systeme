#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <unistd.h> 
#include <cstring> 
#include <netinet/in.h> 
#include <sys/socket.h>
#include <filesystem>
#include <iostream>

#include "../RPCWettanbieter/gen-cpp/Wettanbieter.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace Wettanbietercpp;
using namespace std;

#define ADRESSE     "localhost"
#define HTTP_PORT   8080
#define RPC_PORT    9090

#define ENDING_TEAM_A   1
#define ENDING_TEAM_B   2
#define ENDING_EVEN     3

string gameEnding(int ending)
{
    if (ending == ENDING_TEAM_A)
    {
        return "Team A";
    }
    else if (ending == ENDING_TEAM_B)
    {
        return "Team B";
    }
    else if (ending == ENDING_EVEN)
    {
        return "Even";
    }
    else
    {
        return "not decided";
    }
}

/*
void str_echo(int socketForClient){

    cout << "in str_echo"<< endl;
    char buffer[1024] = { "hello Client" }; 
    //recv(socketForClient, buffer, sizeof(buffer), 0); 
    cout << "Message from client: " << buffer << endl;
    send(socketForClient, buffer, strlen(buffer),0);

}
*/

struct HTTPrequest{

    bool valid = false;
    bool faviconReq = false;

    bool get = false;
    bool post = false;

    string requestURI = "";
    string httpVersion = "";
    string host = "";
    string userAgent = "";
    string accept = "";
    string acceptLang = "";
    string acceptEnc = "";
    string referer = "";                    

    int dnt = -1;
    int secGPC = -1;

    string connection = "";
    string upgradeInsecReq = "";            
    string ifModSince = "";                 
    string ifNoneMatch = "";                                 
    string cacheControl = "";               
    string data = "";

};

HTTPrequest validateHTTP(string message){
    HTTPrequest element;

    string GET = "GET /";
    string POST = "POST /";
    string HTTPv1_0 = "HTTP/1.0";
    string HTTPv1_1 = "HTTP/1.1";
    string HTTPv1_2 = "HTTP/1.2";
    string HTTPv2_0 = "HTTP/2.0";
    string HOST = "Host: ";
    string address = "localhost:8080";
    string USER_AGENT = "User-Agent: ";
    string ACCEPT = "Accept: ";
    string TEXT_HTML = "text/html";
    string IMAGE_AVIF = "image/avif";
    string FAVICON = "favicon.ico";
    string ACCEPT_LANGUAGE = "Accept-Language: ";
    string ACCEPT_ENCODING = "Accept-Encoding: ";
    string DNT = "DNT: ";
    string SEC_GPT = "Sec-GPC: ";
    string CONNECTION = "Connection: ";
    string KEEP_ALIVE = "keep-alive";
    string REFERER = "Referer: ";
    string REFERER_PREFIX = "http://localhost:8080/";

    std::stringstream sMessage;
    sMessage << message;

    string line = "";
    getline(sMessage, line);

    element.get = (line.find(GET,0) == 0);                              //1 line
    element.post = (line.find(POST,0) == 0);
    
    if( !(element.get | element.post)){
        return element;
    } else{
        if(element.get){
            line.erase(0, GET.size());
            cout << "*GET" <<  endl;
        } else{
            line.erase(0, POST.size());
            cout << "*POST" <<  endl;
        }
        
        element.requestURI = line.substr(0,line.find(" "));
        cout << "*URI: " << element.requestURI << endl;
        line.erase(0, element.requestURI.size());
        //cout << "line: " << line << endl;

        if(line.find(HTTPv1_0, 0) != -1){
            element.httpVersion = HTTPv1_0;
        } else if(line.find(HTTPv1_1, 0) != -1){
            element.httpVersion = HTTPv1_1;
        } else if(line.find(HTTPv1_2, 0) != -1){
            element.httpVersion = HTTPv1_2;
        }  else if(line.find(HTTPv2_0, 0) != -1){
            element.httpVersion = HTTPv2_0;
        } else {
            return element;
        }
        cout << "*HTTP Version: " << element.httpVersion << endl;
    }

    getline(sMessage, line);                                           
    cout << "line: " << line << endl;
    while( !(line.empty() || line == "\r")){ 

        //cout << "line: " << line << endl;

        if((line.find(HOST,0) == 0)){
            line.erase(0, HOST.size());
            if(line.find(address, 0) != -1){
                element.host = line;
                cout << "*Host: " << element.host << endl;
                getline(sMessage, line);
            } else{
                cout << "*Host: " << element.host << endl;
                return element;            
            }
        } else if((line.find(USER_AGENT,0) == 0)){
            line.erase(0, USER_AGENT.size());        
            element.userAgent = line; 
            cout << "*User-Agent: " << element.userAgent << endl;

        } else if((line.find(ACCEPT,0) == 0)){                                      
            line.erase(0, ACCEPT.size());
            if(line.find(TEXT_HTML, 0) != -1 | line.find(IMAGE_AVIF, 0) != -1){                
                if(line.find(IMAGE_AVIF, 0) != -1 && element.requestURI == FAVICON){
                    element.faviconReq = true;
                }                
                element.accept = line;
                cout << "*Accept: " << element.accept << endl;
            } else{
                return element;
            }
        } else if((line.find(ACCEPT_LANGUAGE,0) == 0)){                                      
            line.erase(0, ACCEPT_LANGUAGE.size());
            element.acceptLang = line;
            cout << "*Accept-Language: " << element.acceptLang << endl;

        } else if((line.find(ACCEPT_ENCODING,0) == 0)){                                       
            line.erase(0, ACCEPT_ENCODING.size());
            element.acceptEnc = line;
            cout << "*Accept-Encoding: " << element.acceptEnc << endl;

        } else if((line.find(DNT,0) == 0)){                                                    
            line.erase(0, DNT.size());
            element.dnt = stoi(line);
            cout << "*DNT: " << element.dnt << endl;

        } else if((line.find(SEC_GPT,0) == 0)){                                                    
            line.erase(0, SEC_GPT.size());
            element.secGPC = stoi(line);
            cout << "*Sec-GPT: " << element.secGPC << endl; 

        } else if((line.find(CONNECTION,0) == 0)){                                                    
            line.erase(0, CONNECTION.size());
            if(line.find(KEEP_ALIVE, 0) != -1){
                element.connection = line;
                cout << "*Connection: " << element.connection << endl;
            } else{
                return element;
            }       
        } else if((line.find(REFERER,0) == 0)){                                                    
            line.erase(0, REFERER.size());
            if(line.find(REFERER_PREFIX, 0) != -1){
                line.erase(0, REFERER_PREFIX.size());
                element.referer = line;
                cout << "*Referer: " << element.referer << endl;
            }     
        }

        getline(sMessage, line);               
    }
    //cout << "line: " << line << endl;
    //cout << "line: " << line.empty() << endl;

    while (getline(sMessage, line)){
        element.data = element.data + line;
        //cout << "line: " << line << line.length() << endl;
    } 
    cout << "*Data: " << element.data << endl << endl;

    element.valid = true;
    return element;
}

struct Wettoption{
    int id;
    string anbieterName;
    string playerA;
    string playerB;
    double quoteA;
    double quoteB;
    double quoteEven;
};

struct Wettanbieter{
    string name;
    string adresse;
    int port;
};

struct PlatzierteWette{
    string namebettor;
    double moneyset;
    int ending;
    Wettoption wettoption;    // nameAnbieter, wett-id, winachance, winbchance, even
};

vector <Wettanbieter> alleWettanbieter;

Wettoption newWettoption(int pid, string panbieterName, string pplayerA, string pplayerB, double pquoteA, double pquoteB, double pquoteEven)
{
    Wettoption wettoption;

    wettoption.id = pid;
    wettoption.anbieterName = panbieterName;
    wettoption.playerA = pplayerA;
    wettoption.playerB = pplayerB;
    wettoption.quoteA = pquoteA;
    wettoption.quoteB = pquoteB;
    wettoption.quoteEven = pquoteEven;

    return wettoption;
}

Wettanbieter newWettanbieter(string pname, string padresse, int pport)
{
    Wettanbieter anbieter;

    anbieter.name = pname;
    anbieter.adresse = padresse;
    anbieter.port = pport;

    return anbieter;
}

PlatzierteWette newPlatzierteWette(string pnamebettor, double pmoneyset, int pending, Wettoption pwettoption)
{
    PlatzierteWette wette;

    wette.namebettor = pnamebettor;
    wette.moneyset = pmoneyset;
    wette.ending = pending;
    wette.wettoption = pwettoption;

    return wette;
}

void getAllWettanbieter()   // initial
{
    // File pointer 
    ifstream wettanbieterCSV; 
  
    // Open an existing file 
    wettanbieterCSV.open("wettanbieter.csv", ios::in);

    // Read the Data from the file 
    string line;
    string csv_name, csv_adresse, csv_port;

    while(!wettanbieterCSV.eof())
    {
        // read an entire row and 
        // store it in a string variable 'line' 
        getline(wettanbieterCSV, line);
  
        // used for breaking words 
        stringstream sLine(line); 
  
        // read every column data of a row and 
        // store it in a string variable 
        getline(sLine, csv_name, ',');
        getline(sLine, csv_adresse, ',');
        getline(sLine, csv_port, '\n');

        alleWettanbieter.push_back( newWettanbieter(csv_name, csv_adresse, stoi(csv_port)) );
    } 

    wettanbieterCSV.close();
    cout << "*got all Wettanbieter" << endl;
}

void sendResponse(char* const fileBuffer, string httpCode, int newClientSocket)
{
    string header = httpCode + "\nContent-type: text/html\nContent-length: ";

    int content_length = strlen(fileBuffer);
    string length = to_string(content_length);
    
    header = header + length + "\n\r\n\r\n";

    cout << "**header: " << header << endl;

    char headerBuffer[3000];
    strcpy(headerBuffer, header.c_str());
    send(newClientSocket, headerBuffer, strlen(headerBuffer),0);

    send(newClientSocket, fileBuffer, strlen(fileBuffer),0);                         

    delete[] fileBuffer;
}

bool replace(std::string& str, const std::string& from, const std::string& to)
{
    //c_str(str);
    // https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
    {
        return false;
    }
    str.replace(start_pos, from.length(), to);

    return true;
}

// ******************************
// RPC - Create
// ******************************

void placeNewBet(string nameanbieter, int betid, string namebettor, double moneyset, int ending, double winachance, double winbchance, double even)
{
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    try
    {
        transport->open();
        client.placeNewBet(nameanbieter, betid, namebettor, moneyset, ending, winachance, winbchance, even);
        transport->close();

    } catch (TException& tx) {
        cout << "ERROR in placeNewBet(): " << tx.what() << endl;
    }

    cout << "*new Bet placed" << endl;
}

void addBetOption(string nameanbieter, int id, string playera, string playerb, double winachance, double winbchance, double even)
{
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    try
    {
        transport->open();
        client.addBet(nameanbieter, id, playera, playerb, winachance, winbchance, even);
        transport->close();

    } catch (TException& tx) {
        cout << "ERROR in addBetOption(): " << tx.what() << endl;
    }

    cout << "*added BetOption" << endl;
}

// ******************************
// RPC - Read
// ******************************
bool betExists(string nameanbieter, int id)
{
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    bool response = false;

    try
    {
        transport->open();
        response = client.betExists(nameanbieter, id);
        transport->close();

    } catch (TException& tx) {
        cout << "ERROR in betExists(): " << tx.what() << endl;
    }
    cout << "*betExists" << endl;

    return response;
}

Wettoption readWettoption (stringstream* sRPC)
{
    string readAnbieterName, wettID, teamA, teamB, quoteA, quoteB, quoteEven, nulln;

    //cout << "Read line" << endl << flush;

    // read every column data of a row and 
    // store it in a string variable
    getline(*sRPC, readAnbieterName, ':');
    getline(*sRPC, wettID, ':');
    getline(*sRPC, teamA, ':');
    getline(*sRPC, teamB, ':');
    getline(*sRPC, quoteA, ':');
    getline(*sRPC, quoteB, ':');
    getline(*sRPC, quoteEven, ';');
    getline(*sRPC, nulln, '\n');

    //cout << readAnbieterName << wettID << teamA << teamB << quoteA << quoteB << quoteEven << endl << flush;

    return newWettoption(stoi(wettID), readAnbieterName, teamA, teamB, stod(quoteA), stod(quoteB), stod(quoteEven));
}

Wettoption getSpecificBetOption(string nameAnbieter, int id)
{
    cout << "get specific BetOption" << endl;
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    string rpc;

    try
    {
        transport->open();
        client.getSpecificBetOption(rpc, nameAnbieter, id);     // return string - \n fehlt
        transport->close();
    
    } catch (TException& tx) {
        cout << "ERROR in getAllBetOptions(): " << tx.what() << endl;
    }
    cout << "RPC: " << rpc << endl;
    // Output
    // RPC: Fred_und_George:2:Ravenclaw:Slytherin:1.0:2.0:3.5;

    stringstream sRPC;
    sRPC << rpc;

    Wettoption specificBetOption = readWettoption(&sRPC);

    cout << "*got specific BetOption" << endl;

    return specificBetOption;
}

PlatzierteWette readPlatzierteWette(stringstream* sRPC)
{
    string readAnbieterName, wettID, namebettor, moneyset, ending, quoteA, quoteB, quoteEven, nulln;

    // read every column data of a row and 
    // store it in a string variable
    getline(*sRPC, readAnbieterName, ':');
    getline(*sRPC, wettID, ':');
    getline(*sRPC, namebettor, ':');
    getline(*sRPC, moneyset, ':');
    getline(*sRPC, ending, ':');
    getline(*sRPC, quoteA, ':');
    getline(*sRPC, quoteB, ':');
    getline(*sRPC, quoteEven, ';');
    getline(*sRPC, nulln, '\n');

    //cout << readAnbieterName << wettID << namebettor << moneyset << ending << quoteA << quoteB << quoteEven << endl << flush;

    return newPlatzierteWette(namebettor, stod(moneyset), stoi(ending), getSpecificBetOption(readAnbieterName, stoi(wettID)) );
}

vector<Wettoption> getAllBetOptionsFromWettanbieter(string anbieterName)
{
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    string rpc;
    vector<Wettoption> allBets;

    try
    {
        transport->open();
        client.getMyBetOptions(rpc, anbieterName);     // return string - \n fehlt
        transport->close();
    
    } catch (TException& tx) {
        cout << "ERROR in getAllBetOptionsFromWettanbieter(): " << tx.what() << endl;
    }
    //cout << "RPC: " << anbieterName << " " << rpc << endl;
    // Output
    // RPC: Fred_und_George 2#Fred_und_George:2:Ravenclaw:Slytherin:1.0:2.0:3.5;
    //      Fred_und_George:2:Gryffindor:Hufflepuff:1.5:2.5:3.5;

    stringstream sRPC;
    sRPC << rpc; 
    string sLoopCtr = "";
    int loopCtr = 0;

    getline(sRPC, sLoopCtr, '#');
    loopCtr = stoi(sLoopCtr);

    //cout << "LoopCtr getAllBetOptionsFromWettanbieter(): " << loopCtr << endl;
    for(int i = 0; i < loopCtr; i++)
    {  
        allBets.push_back( readWettoption(&sRPC) );
    }

    cout << "*got all BetOptions from Wettanbieter" << endl;

    return allBets;
}

vector<Wettoption> getAllBetOptions()
{
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    string rpc;
    vector<Wettoption> allBetOptions;

    try
    {
        transport->open();
        client.getAllBetOptions(rpc);     // return string - \n fehlt
        transport->close();
    
    } catch (TException& tx) {
        cout << "ERROR in getAllBetOptions(): " << tx.what() << endl;
    }
    //cout << "RPC: " << rpc << endl;
    // Output
    // RPC: 2#Fred_und_George:2:Ravenclaw:Slytherin:1.0:2.0:3.5;
    //      Fred_und_George:2:Gryffindor:Hufflepuff:1.5:2.5:3.5;

    stringstream sRPC;
    sRPC << rpc; 
    string sLoopCtr = "";
    int loopCtr = 0;

    getline(sRPC, sLoopCtr, '#');
    loopCtr = stoi(sLoopCtr);

    for(int i = 0; i < loopCtr; i++)
    {
        allBetOptions.push_back( readWettoption(&sRPC) );
    }
    cout << "*got all BetOptions" << endl;

    return allBetOptions;
}

vector<PlatzierteWette> getAllPlacedBets()
{
    // rpc call
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    string rpc;
    vector<PlatzierteWette> allPlacedBets;

    try
    {
        transport->open();
        client.getAllPlacedBets(rpc);     // return string - \n fehlt
        transport->close();
    
    } catch (TException& tx) {
        cout << "ERROR in getAllPlacedBets(): " << tx.what() << endl;
    }
    //cout << "getAllPlacedBets - RPC: " << rpc << endl;
    // Output
    // RPC: nr#ich:3:huhu:30.0:1:3.0:2.0:2.0

    stringstream sRPC;
    sRPC << rpc; 
    string sLoopCtr = "";
    int loopCtr = 0;

    getline(sRPC, sLoopCtr, '#');
    loopCtr = stoi(sLoopCtr);

    for(int i = 0; i < loopCtr; i++)
    {
        allPlacedBets.push_back( readPlatzierteWette(&sRPC) );
    }
    cout << "*got all PlacedBets" << endl;

    return allPlacedBets;
}

vector<PlatzierteWette> getAllMyPlacedBets(string myName)
{
    vector<PlatzierteWette> allPlacedBets;
    vector<PlatzierteWette> myPlacedBets;

    allPlacedBets = getAllPlacedBets();

    for (int i = 0; i < allPlacedBets.size(); i++)
    {
        if (strcmp(allPlacedBets[i].namebettor.c_str(), myName.c_str()) == 0)
        {
            myPlacedBets.push_back(allPlacedBets[i]);
        }
    }
    cout << "*got all my PlacedBets" << endl;

    return myPlacedBets;
}

vector<PlatzierteWette> getAllPlacedBetsToABetOption(string nameAnbieter, int id)
{
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    string rpc;
    vector<PlatzierteWette> allPlacedBets;

    try
    {
        transport->open();
        client.getAllPlacedBetsToABetOption(rpc, nameAnbieter, id);     // return string - \n fehlt
        transport->close();
    
    } catch (TException& tx) {
        cout << "ERROR in getAllPlacedBetsToABetOption(): " << tx.what() << endl;
    }
    //cout << "getAllPlacedBetsToABetOption - RPC: " << rpc << endl;
    // Output
    // RPC: nr#ich:3:huhu:30.0:1:3.0:2.0:2.0

    stringstream sRPC;
    sRPC << rpc; 
    string sLoopCtr = "";
    int loopCtr = 0;

    getline(sRPC, sLoopCtr, '#');
    loopCtr = stoi(sLoopCtr);

    for(int i = 0; i < loopCtr; i++)
    {
        allPlacedBets.push_back( readPlatzierteWette(&sRPC));
    }
    cout << "*got all PlacedBets to a BetOption" << endl;

    return allPlacedBets;
}

// ******************************
// RPC - Update
// ******************************
void changeBetOption(string nameanbieter, int id, string playera, string playerb, double winachance, double winbchance, double even)
{
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    try
    {
        transport->open();
        client.changeBet(nameanbieter, id, playera, playerb, winachance, winbchance, even);
        transport->close();
    } catch (TException& tx) {
        cout << "ERROR in changeBetOption(): " << tx.what() << endl;
    }

    cout << "*Updated BetOption" << endl;
}

// ******************************
// RPC - Delete
// ******************************
void deleteBetOption(string nameanbieter, int id)             // keine Sicherheit, ob noch platzierte Wetten existieren :(
{
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    try
    {
        transport->open();
        client.deleteBet(nameanbieter, id);
        transport->close();

    } catch (TException& tx) {
        cout << "ERROR in deleteBetOption(): " << tx.what() << endl;
    }

    cout << "*deleted BetOption" << endl;
}

int main() {

    //basiert auf: https://www.geeksforgeeks.org/socket-programming-in-cpp/
    auto socketServer = socket(AF_INET, SOCK_STREAM, 0);                   //AF_INET = IPv4 protocol family, SOCK_STREAM = socket is a stream (TCP)       
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(HTTP_PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    cout << "bind Socket"<< endl;
    bind(socketServer, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    cout << "listen"<< endl;
    listen(socketServer, 5);

    getAllWettanbieter();

    while(true){
        cout << "in while-loop"<< endl;

        int newClientSocket = accept(socketServer, nullptr, nullptr);
        pid_t childPID = 0;

        if(newClientSocket < 0){
            cout << "server: error while accepting client" << endl;
        }

        if((childPID = fork()) < 0){
            cout << "server: error while fork" << endl;
        } else if (childPID == 0){
            cout << "forked"<< endl;

            close(socketServer);

            char clientReqBuffer[1024] = { "" }; 
            recv(newClientSocket, clientReqBuffer, sizeof(clientReqBuffer), 0);

            bool errorPage = false;

            string sBuffer(clientReqBuffer); 
            HTTPrequest request = validateHTTP(sBuffer);
            if(request.valid){
                cout << "message from client: " << endl << clientReqBuffer << endl;
            } else{
                cout << "invalid message from client" << endl << clientReqBuffer << endl;
                errorPage = true;                
            }

            if(request.get){
                string filename;
                if (request.faviconReq){
                    filename = "favicon.ico";
                } else if (request.requestURI == ""){
                    filename = "index.html";
                } else {
                    filename = request.requestURI+".html";
                }

                ifstream file(filename, ios::binary);
                if(!file.good()){
                    cout << "file cannot be opened"<< endl;
                    errorPage = true;
                } else {
                    std::filesystem::path p(filename);
                    auto buffersize = std::filesystem::file_size(p); 
                    char* const originFileBuffer = new char[buffersize];
                    int pos{0};
                    while(file.get(originFileBuffer[pos++]));

                    // Variablen in html ersetzen
                    string sEditFileBuffer(originFileBuffer);

                    if((filename.find("wettender.html")) != string::npos)
                    {
                        // <option value = "Gryffindor vs. Slytherin">Gryffindor (3.5) vs. Slytherin (1.3) - Even (4.0)</option>
                        string htmlVariable = "{bettor_post_get_Selection}";
                        string newHTML_getSelection = "";
                        vector<Wettoption> alleWetten = getAllBetOptions();

                        for(int i = 0; i < alleWetten.size(); i++)
                        {
                            string quoteA = to_string(alleWetten[i].quoteA);
                            string quoteB = to_string(alleWetten[i].quoteB);
                            string quoteEven = to_string(alleWetten[i].quoteEven);

                            //cout << "wettender.html alleWettoptionen: " << alleWetten[i].anbieterName << endl;
                            newHTML_getSelection += "<option value = \"" + alleWetten[i].anbieterName + "AA" + to_string(alleWetten[i].id) + "AA" + alleWetten[i].playerA + "AA" + alleWetten[i].playerB + "AA" 
                                        + quoteA.substr(0, quoteA.find('.')+3) + "AA" + quoteB.substr(0, quoteA.find('.')+3) + "AA" + quoteEven.substr(0, quoteA.find('.')+3) + "\">"
                                        + alleWetten[i].playerA + " (" + quoteA.substr(0, quoteA.find('.')+3) + ") vs. "
                                        + alleWetten[i].playerB + " (" + quoteB.substr(0, quoteB.find('.')+3) + ") - "
                                        + "Even (" + quoteEven.substr(0, quoteEven.find('.')+3) + ") : [" + alleWetten[i].anbieterName + "]</option>";
                        }
                        replace(sEditFileBuffer, htmlVariable, newHTML_getSelection);

                        // <li>Chudley Cannons (2.0) vs. Holyhead Harpines (1.5) - Even (4.0) : [Anbieter]<br> 15 auf Team B</li>
                        string htmlVariableCurrentBets = "{bettor_get_my_current_bets}";
                        string htmlVariablePreviousBets = "{bettor_get_my_previous_bets}";
                        string newHTMLcurrent = "";
                        string newHTMLprevious = "";
                        vector<PlatzierteWette> alleMeineWetten = getAllMyPlacedBets("emmy");

                        for(int i = 0; i < alleMeineWetten.size(); i++)
                        {
                            string quoteA = to_string(alleMeineWetten[i].wettoption.quoteA);
                            string quoteB = to_string(alleMeineWetten[i].wettoption.quoteB);
                            string quoteEven = to_string(alleMeineWetten[i].wettoption.quoteEven);
                            string moneyset = to_string(alleMeineWetten[i].moneyset);

                            // if (game is open) -> current_bets

                            //cout << "wettender.html: getCurrentBets >> " << alleWetten[i].anbieterName << endl;
                            newHTMLcurrent += "<li>" + alleMeineWetten[i].wettoption.playerA + " (" + quoteA.substr(0, quoteA.find('.')+3) + ") vs. "
                                            + alleMeineWetten[i].wettoption.playerB + " (" + quoteB.substr(0, quoteB.find('.')+3) + ") - "
                                            + "Even (" + quoteEven.substr(0, quoteEven.find('.')+3) + ") : [" + alleWetten[i].anbieterName + "]<br>"
                                            + moneyset.substr(0, moneyset.find('.')+3) + "€ auf " + gameEnding(alleMeineWetten[i].ending) + "</li>";

                            // else (game is closed)-> previous bets
                            /*
                            newHTMLprevious += "<li>" + alleMeineWetten[i].wettoption.playerA + " (" + quoteA.substr(0, quoteA.find('.')+3) + ") vs. "
                                            + alleMeineWetten[i].wettoption.playerB + " (" + quoteB.substr(0, quoteB.find('.')+3) + ") - "
                                            + "Even (" + quoteEven.substr(0, quoteEven.find('.')+3) + ") : [" + alleWetten[i].anbieterName + "]<br>"
                                            + moneyset.substr(0, moneyset.find('.')+3) + "€ auf " + gameEnding(alleMeineWetten[i].ending) + "</li>";
                            */
                        }
                        replace(sEditFileBuffer, htmlVariableCurrentBets, newHTMLcurrent);
                        replace(sEditFileBuffer, htmlVariablePreviousBets, newHTMLcurrent);

                    }
                    else if ((filename.find("wettanbieter.html")) != string::npos)
                    {
                        string htmlVariable = "{booker_get_bets_of_this_bookmaker}";
                        string newHTML = "";
                        string thisAnbieter = "Fred_und_George";        // TODO: dynamisch
                        //vector<Wettoption> alleWAWetten = getAllBetOptionsFromWettanbieter(thisAnbieter);       // ToDo - statischen Anbieter in dynamisch ändern
                        vector<Wettoption> alleWAWetten = getAllBetOptions();
                        double sumA = 0;
                        double sumB = 0;
                        double sumEven = 0;
                        int anzahlPlatzierteWetten;

                        for(int ctr_wette = 0; ctr_wette < alleWAWetten.size(); ctr_wette++)
                        {
                            sumA = 0;
                            sumB = 0;
                            sumEven = 0;

                            //vector<PlatzierteWette> thisOptionsPlaced = getAllPlacedBetsToABetOption(thisAnbieter, alleWAWetten[ctr_wette].id);
                            vector<PlatzierteWette> thisOptionsPlaced = getAllPlacedBetsToABetOption(alleWAWetten[ctr_wette].anbieterName, alleWAWetten[ctr_wette].id);

                            for(int ctr_placed = 0; ctr_placed < thisOptionsPlaced.size(); ctr_placed++)
                            {
                                if(thisOptionsPlaced[ctr_placed].ending == ENDING_TEAM_A)
                                {
                                    sumA += thisOptionsPlaced[ctr_placed].moneyset;
                                }
                                else if(thisOptionsPlaced[ctr_placed].ending == ENDING_TEAM_B)
                                {
                                    sumB += thisOptionsPlaced[ctr_placed].moneyset;
                                }
                                else if(thisOptionsPlaced[ctr_placed].ending == ENDING_EVEN)
                                {
                                    sumEven += thisOptionsPlaced[ctr_placed].moneyset;
                                }
                            }

                            string quoteA = to_string(alleWAWetten[ctr_wette].quoteA);
                            string quoteB = to_string(alleWAWetten[ctr_wette].quoteB);
                            string quoteEven = to_string(alleWAWetten[ctr_wette].quoteEven);
                            string ssumA = to_string(sumA).substr(0, to_string(sumA).find('.')+3);
                            string ssumB = to_string(sumB).substr(0, to_string(sumB).find('.')+3);
                            string ssumEven = to_string(sumEven).substr(0, to_string(sumEven).find('.')+3);
                            anzahlPlatzierteWetten = thisOptionsPlaced.size();

                            newHTML += "<li> <b>" + alleWAWetten[ctr_wette].playerA + "</b> (" + quoteA.substr(0, quoteA.find('.')+3) + ") "
                                    + "vs. <b>" + alleWAWetten[ctr_wette].playerB + "</b> (" + quoteB.substr(0, quoteB.find('.')+3) + ") - "
                                    + "Even (" + quoteEven.substr(0, quoteEven.find('.')+3) + ") : "
                                    + "[" + alleWAWetten[ctr_wette].anbieterName + "]<br>"
                                    + to_string(anzahlPlatzierteWetten) + " platzierte Wetten: " + ssumA + "€ vs. " + ssumB + "€ - " + ssumEven + "€</li>";
                        }
                        // <span class="rot">rote Schrift</span>
                        /* HTML im Head-Bereich:
                            <syle type="text/css">
                            .rot {color: #f00;}
                            </style>
                        */
                        replace(sEditFileBuffer, htmlVariable, newHTML);
                    }

                    char* updatedFileBuffer = new char[sEditFileBuffer.length() + 1];
                    strcpy(updatedFileBuffer, sEditFileBuffer.c_str());
                    delete[] originFileBuffer;

                    sendResponse(updatedFileBuffer, "HTTP/1.1 200 Ok", newClientSocket);
                }

            } else if (request.post){
                cout << "* request.referer: " << request.referer; 
                cout << "!"<< endl;
                if(request.referer.find("wettender",0) != -1){
                    cout << "POST bei wettender" << endl;
                    //Wettender Beispiel POST-String: Spiel=Fred_und_GeorgeAA2AARavenclawAASlytherinAA1.00AA2.00AA3.50&gewaehlteMannschaft=Team+A&Wetteinsatz=10&Name=emmy
                    string SPIEL = "Spiel=";
                    string TRENNUNG = "AA";
                    string AUSGEWAEHLT = "gewaehlteMannschaft=";
                    string TEAM_A = "Team+A";
                    string TEAM_B = "Team+B";
                    string EVEN = "Even";
                    string AMPERSAND = "&";
                    string WETTEINSATZ = "Wetteinsatz=";
                    string NAME = "Name=";
                    string CARRIAGERETURN = "%0D%0A";

                    string anbieter = "";
                    string spielid = "";
                    string playerA = "";
                    string playerB = "";
                    string aQuote = "";
                    string bQuote = "";
                    string evenQuote = "";
                    int ergebnis = -1;
                    string name = "";
                    string einsatz = "";

                    string post = request.data;

                    cout << "*post: " << post << endl;

                    if((post.find(SPIEL,0) == 0)){
                        post.erase(0, SPIEL.size());                        
                        cout << "*post: " << post << endl;

                        if((post.find(CARRIAGERETURN,0) == 0)){
                            cout << "found Carriagereturn" << endl;
                            post.erase(0, CARRIAGERETURN.size());
                            cout << "*post: " << post << endl;
                        }
                        int pos = post.find(TRENNUNG,0);

                        anbieter = post.substr(0,pos);
                        post.erase(0, anbieter.size());
                        post.erase(0, TRENNUNG.size());
                        cout << "*post: " << post << endl;

                        pos = post.find(TRENNUNG,0);
                        spielid = post.substr(0,pos);
                        post.erase(0, spielid.size());
                        post.erase(0, TRENNUNG.size());
                        cout << "*post: " << post << endl;

                        pos = post.find(TRENNUNG,0);
                        playerA = post.substr(0,pos);
                        post.erase(0, playerA.size());
                        post.erase(0, TRENNUNG.size());
                        cout << "*post: " << post << endl;

                        pos = post.find(TRENNUNG,0);
                        playerB = post.substr(0,pos);
                        post.erase(0, playerB.size());
                        post.erase(0, TRENNUNG.size());
                        cout << "*post: " << post << endl;

                        pos = post.find(TRENNUNG,0);
                        aQuote = post.substr(0,pos);
                        post.erase(0, aQuote.size());
                        post.erase(0, TRENNUNG.size());
                        cout << "*post: " << post << endl;

                        pos = post.find(TRENNUNG,0);
                        bQuote = post.substr(0,pos);
                        post.erase(0, bQuote.size());
                        post.erase(0, TRENNUNG.size());
                        cout << "*post: " << post << endl;

                        pos = post.find(AMPERSAND,0);
                        evenQuote = post.substr(0,pos);
                        post.erase(0, evenQuote.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;

                        post.erase(0, AUSGEWAEHLT.size());
                        cout << "*post: " << post << endl;

                        pos = post.find(AMPERSAND,0);
                        string auswahl = post.substr(0,pos);
                        post.erase(0, auswahl.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;

                        if(auswahl.find(TEAM_A,0) != -1){
                            ergebnis = 1;
                        } else if (auswahl.find(TEAM_B,0)){
                            ergebnis = 2;
                        }else{
                            ergebnis = 3;
                        }

                        post.erase(0, WETTEINSATZ.size());
                        pos = post.find(AMPERSAND,0);
                        einsatz = post.substr(0,pos);
                        post.erase(0, einsatz.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;

                        post.erase(0, NAME.size());
                        name = post;
                        cout << "*post: " << post << endl;

                        placeNewBet(anbieter, stoi(spielid), name, stod(einsatz), ergebnis, stod(aQuote), stod(bQuote), stod(evenQuote));
                    }
                }
                else if(request.referer.find("wettanbieter",0) != -1)
                {
                //idbet=5&teama=a&AGewinnquote=1.5&teamb=b&BGewinnquote=1.5&even=1.5&nameanbieter=baum  
                cout << "POST bei wettanbieter" << endl;
                    
                    string GEWINNQUOTEA = "AGewinnquote=";
                    string GEWINNQUOTEB = "BGewinnquote=";
                    string EVENQUOTE = "even=";
                    string TEAM_A = "teama=";
                    string TEAM_B = "teamb=";
                    string IDBET = "idbet=";
                    string IDBET_DELETE = "idbetdel=";
                    string AMPERSAND = "&";
                    string NAME = "nameanbieter=";
                    string NAME_DELETE = "nameanbieteDel=";
                    string CARRIAGERETURN = "%0D%0A";

                    string anbieter = "";
                    string spielid = "";
                    string playerA = "";
                    string playerB = "";
                    string aQuote = "";
                    string bQuote = "";
                    string evenQuote = "";

                    string post = request.data;

                    cout << "*post: " << post << endl;

                    if((post.find(IDBET,0) == 0)){
                        post.erase(0, IDBET.size());                        
                        cout << "*post: " << post << endl;

                        if((post.find(CARRIAGERETURN,0) == 0)){
                            cout << "found Carriagereturn" << endl;
                            post.erase(0, CARRIAGERETURN.size());
                            cout << "*post: " << post << endl;
                        }

                        int pos = post.find(AMPERSAND,0);

                        spielid = post.substr(0,pos);
                        post.erase(0, spielid.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;
                        
                        post.erase(0, TEAM_A.size());
                        pos = post.find(AMPERSAND,0);
                        playerA = post.substr(0,pos);
                        post.erase(0, playerA.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;
                        
                        post.erase(0, GEWINNQUOTEA.size());
                        pos = post.find(AMPERSAND,0);
                        aQuote = post.substr(0,pos);
                        post.erase(0, aQuote.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;

                        post.erase(0, TEAM_B.size());
                        pos = post.find(AMPERSAND,0);
                        playerB = post.substr(0,pos);
                        post.erase(0, playerB.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;                       

                        post.erase(0, GEWINNQUOTEB.size());
                        pos = post.find(AMPERSAND,0);
                        bQuote = post.substr(0,pos);
                        post.erase(0, bQuote.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post vor even: " << post << endl;

                        post.erase(0, EVENQUOTE.size());
                        cout << "*post: " << post << endl;
                        pos = post.find(AMPERSAND,0);
                        evenQuote = post.substr(0,pos);
                        post.erase(0, evenQuote.size());
                        cout << "*post: " << post << endl;
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;

                        post.erase(0, NAME.size());
                        anbieter = post;
                        cout << "*post: " << post << endl;

                        cout << "* anbieter und id: " << anbieter << ":" << stoi(spielid) << endl;

                        if(betExists(anbieter, stoi(spielid)))
                        {
                            changeBetOption(anbieter, stoi(spielid),  playerA,  playerB, stod(aQuote), stod(bQuote), stod(evenQuote));
                        }
                        else
                        {
                            addBetOption(anbieter, stoi(spielid),  playerA,  playerB, stod(aQuote), stod(bQuote), stod(evenQuote));
                        }
                    }
                    else if ((post.find(IDBET_DELETE,0) == 0))
                    {                        
                        //idbetdel=3&nameanbieteDel=Hogwarts

                        post.erase(0, IDBET_DELETE.size());                        
                        cout << "*post: " << post << endl;

                        if((post.find(CARRIAGERETURN,0) == 0))
                        {
                            cout << "found Carriagereturn" << endl;
                            post.erase(0, CARRIAGERETURN.size());
                            cout << "*post: " << post << endl;
                        }

                        int pos = post.find(AMPERSAND,0);

                        spielid = post.substr(0,pos);
                        post.erase(0, spielid.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;
                        
                        post.erase(0, NAME_DELETE.size());
                        anbieter = post;
                        cout << "*post: " << post << endl;

                        if(betExists(anbieter, stoi(spielid)))
                        {                        
                            deleteBetOption(anbieter, stoi(spielid));
                        }
                        else
                        {
                            cout << "Bet does not exist" << endl;
                        }
                    }
                }
                int length = request.data.length(); 
                char* bufferPostHeader = new char[length + 1]; 
                strcpy(bufferPostHeader, request.data.c_str());  
                cout << "Message from client: " << bufferPostHeader << endl;
                send(newClientSocket, bufferPostHeader, strlen(bufferPostHeader),0);
            }

            if (errorPage)
            {
                string filename = "error.html";
                ifstream file(filename, ios::binary);
                if(!file.good())
                {
                    cout << "file cannot be opened (error page)"<< endl;
                }
                else
                {
                    std::filesystem::path p(filename);
                    auto buffersize = std::filesystem::file_size(p); 
                    char* const fileBuffer = new char[buffersize];
                    int pos{0};

                    while(file.get(fileBuffer[pos++]));

                    sendResponse(fileBuffer, "HTTP/1.1 404 Not Found", newClientSocket);
                }
            }       
            exit(0);
        }

        cout << "close newClientSocket"<< endl;
        close(newClientSocket); 
    }
}
