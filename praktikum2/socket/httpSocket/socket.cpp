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
    string ending;
    Wettoption wettoption;    // nameAnbieter, wett-id, winachance, winbchance, even
};

vector <Wettanbieter> alleWettanbieter;

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

void getAlleWettanbieter()   // initial
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
        // store it in a string variable, 'word' 
        getline(sLine, csv_name, ',');
        getline(sLine, csv_adresse, ',');
        getline(sLine, csv_port, '\n');

        Wettanbieter thisAnbieter;
        thisAnbieter.name = csv_name;
        thisAnbieter.adresse = csv_adresse;
        thisAnbieter.port = stoi(csv_port);

        alleWettanbieter.push_back(thisAnbieter);
    } 
    wettanbieterCSV.close();
}

vector<Wettoption> getAllBetsFromWettanbieter(string anbieterName, string adresse, int port) // rpc call
{
    // rpc call
    std::shared_ptr<TTransport> socket(new TSocket(adresse, port));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    string rpc;
    vector<Wettoption> allBets;

    try
    {
        transport->open();
        client.getMyBets(rpc, anbieterName);     // return string - \n fehlt
        transport->close();
    
    } catch (TException& tx) {
        cout << "ERROR: " << tx.what() << endl;
    }
    cout << "RPC: " << anbieterName << " " << rpc << endl;
    // Output
    // RPC: Fred_und_George 2#Fred_und_George:2:Ravenclaw:Slytherin:1.0:2.0:3.5;
    //      Fred_und_George:2:Gryffindor:Hufflepuff:1.5:2.5:3.5;

    string readAnbieterName, wettID, teamA, teamB, quoteA, quoteB, quoteEven;
    string line = "";
    stringstream sRPC;
    sRPC << rpc; 
    string sLoopCtr = "";
    int loopCtr = 0;

    getline(sRPC, sLoopCtr, '#');
    loopCtr = stoi(sLoopCtr);

    // read an every entire row and 
    // store it in a string variable 'line' 
    //cout << "getAllBetsFromWettanbieter: " << loopCtr << endl;
    for(int i = 0; i < loopCtr; i++)
    {
        //getline(sRPC, line, ';');
        // used for breaking words 
  
        // read every column data of a row and 
        // store it in a string variable, 'word' 
        getline(sRPC, readAnbieterName, ':');
        getline(sRPC, wettID, ':');
        getline(sRPC, teamA, ':');
        getline(sRPC, teamB, ':');
        getline(sRPC, quoteA, ':');
        getline(sRPC, quoteB, ':');
        getline(sRPC, quoteEven, ';');

        Wettoption* thisOption = new Wettoption;
        thisOption->id = stoi(wettID);
        thisOption->anbieterName = readAnbieterName;
        thisOption->playerA = teamA;
        thisOption->playerB = teamB;
        thisOption->quoteA = stod(quoteA);
        thisOption->quoteB = stod(quoteB);
        thisOption->quoteEven = stod(quoteEven);
        
        /*
        cout << thisOption->id << " " << anbieterName  << " "
             << thisOption->playerA  << " " << thisOption->playerB  << endl;
        cout << "Quote A: " << thisOption->quoteA << endl;
        */
        allBets.push_back(*thisOption);
        delete thisOption;
    }

    return allBets;
}

vector<Wettoption> getAllBets()  // Schnittstelle, die der Client aufruft
{
    vector<Wettoption> alleWetten;
    vector<Wettoption> alleAktuellenWetten;
    string sAlleWetten = "";
        
    //cout << " alleWettanbieter.size(): " << alleWettanbieter.size() << endl;

    // nimm alle Buchhalter
    for(int i = 0; i < alleWettanbieter.size(); i++)
    {
        // stelle für jeden Buchhalter eine Anfrage
        alleAktuellenWetten = getAllBetsFromWettanbieter(alleWettanbieter[i].name, alleWettanbieter[i].adresse, alleWettanbieter[i].port);
        //sAlleWetten += getAllBetsFromWettanbieter(alleWettanbieter[i].name, alleWettanbieter[i].adresse, alleWettanbieter[i].port);
        
        //cout << i << alleWettanbieter[i].name << " " << alleWettanbieter[i].adresse << " " << alleWettanbieter[i].port << endl;

        for(int j = 0; j < alleAktuellenWetten.size(); j++)
        {
            alleWetten.push_back(alleAktuellenWetten[j]);
        }
    
    }

    return alleWetten;
}

Wettoption getSpecificBetOption(string nameAnbieter, int id, string adresse, int port)
{
    // rpc call
    std::shared_ptr<TTransport> socket(new TSocket(adresse, port));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    string rpc;

    try
    {
        transport->open();
        client.getSpecificBet(rpc, nameAnbieter, id);     // return string - \n fehlt
        transport->close();
    
    } catch (TException& tx) {
        cout << "ERROR: " << tx.what() << endl;
    }
    cout << "getSpecificBetOption - RPC: " << nameAnbieter << ": " << rpc << endl;
    // Output
    // RPC: Fred_und_George:2:Ravenclaw:Slytherin:1.0:2.0:3.5;

    string readAnbieterName, wettID, teamA, teamB, quoteA, quoteB, quoteEven;
    string line = "";
    stringstream sRPC;
    sRPC << rpc; 

    // read every column data of a row and 
    // store it in a string variable, 'word' 
    getline(sRPC, readAnbieterName, ':');
    getline(sRPC, wettID, ':');
    getline(sRPC, teamA, ':');
    getline(sRPC, teamB, ':');
    getline(sRPC, quoteA, ':');
    getline(sRPC, quoteB, ':');
    getline(sRPC, quoteEven, ';');

    Wettoption thisBet;
    thisBet.id = stoi(wettID);
    thisBet.anbieterName = readAnbieterName;
    thisBet.playerA = teamA;
    thisBet.playerB = teamB;
    thisBet.quoteA = stod(quoteA);
    thisBet.quoteB = stod(quoteB);
    thisBet.quoteEven = stod(quoteEven);
    
    cout << "getSpecificBetOption:" << thisBet.id << " " << thisBet.anbieterName << " " << thisBet.playerA << " " << thisBet.playerB << endl;

    return thisBet;
}

vector<PlatzierteWette> getAllMyBetsOfBookmaker(string myName, int betid, string nameAnbieter, string adresse, int port)
{
    cout << "getAllMyBetsOfBookmaker entered" << endl;
    // rpc call
    std::shared_ptr<TTransport> socket(new TSocket(adresse, port));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    string rpc;
    vector<PlatzierteWette> allMyBets;

    try
    {
        transport->open();
        client.getAllPlacedBetsToAnBetOffer(rpc, nameAnbieter, betid);     // return string - \n fehlt
        transport->close();
    
    } catch (TException& tx) {
        cout << "ERROR: " << tx.what() << endl;
    }
    cout << "getAllMyBetsOfBookmaker - RPC: " << rpc << endl;
    // Output
    // RPC: nr#ich:3:huhu:30.0:1:3.0:2.0:2.0
    //nameanbieter, betid, namebettor, moneyset, ending, winachance, winbchance, even
    string readAnbieterName, wettID, namebettor, moneyset, ending, quoteA, quoteB, quoteEven;
    string line = "";
    stringstream sRPC;
    sRPC << rpc; 
    string sLoopCtr = "";
    int loopCtr = 0;

    getline(sRPC, sLoopCtr, '#');
    loopCtr = stoi(sLoopCtr);
    // TODO Hier liegt der Fehler. Er kommt nicht weiter weil sLoopCtr im sRPC nicht gefunden wrd (Was heißt #?).

    // for each row
    cout << "getAllMyBetsOfBookmaker: " << loopCtr << endl;
    for(int i = 0; i < loopCtr; i++)
    {
        cout << "getAllMyBetsOfBookmaker - For Loop" << endl;
        // read every column data of a row and 
        // store it in a string variable
        getline(sRPC, readAnbieterName, ':');
        getline(sRPC, wettID, ':');
        getline(sRPC, namebettor, ':');
        getline(sRPC, moneyset, ':');
        getline(sRPC, ending, ':');
        getline(sRPC, quoteA, ':');
        getline(sRPC, quoteB, ':');
        getline(sRPC, quoteEven, ';');

        if(strcmp(namebettor.c_str(), myName.c_str()) != 0)
        {
            continue;
        }
        // 2. get
        /*
        Wettoption* thisOption = new Wettoption;
        thisOption->id = stoi(wettID);
        thisOption->anbieterName = readAnbieterName;
        thisOption->playerA = teamA;
        thisOption->playerB = teamB;
        thisOption->quoteA = stod(quoteA);
        thisOption->quoteB = stod(quoteB);
        thisOption->quoteEven = stod(quoteEven);*/

        PlatzierteWette* thisWette = new PlatzierteWette;
        thisWette->namebettor = namebettor;
        thisWette->moneyset = stod(moneyset);
        thisWette->ending = ending;
        int iWettID = stoi(wettID);

        Wettoption thisOption = getSpecificBetOption(readAnbieterName, iWettID, adresse, port);
        thisWette->wettoption = thisOption;
        
        cout << "getAllMyBetsOfBookmaker" << thisWette->wettoption.id << " " << thisWette->namebettor << " " << thisWette->moneyset << " " << thisWette->ending << endl;

        allMyBets.push_back(*thisWette);
        delete thisWette;
    }

    return allMyBets;
}

vector<PlatzierteWette> getAllMyBets(string myName)  // Schnittstelle, die der Client aufruft
{
    vector<Wettoption> alleAktuellenWetten;
    vector<PlatzierteWette> meineAktuellenWetten;
    vector<PlatzierteWette> alleMeineWetten;
    string sAlleMeineWetten = "";
        
    //cout << " alleWettanbieter.size(): " << alleWettanbieter.size() << endl;

    // nimm alle Buchhalter
    for(int wa = 0; wa < alleWettanbieter.size(); wa++)
    {
        cout << "GetAllMyBets - for1 - Anbieter " << wa << " " << alleWettanbieter[wa].name << endl; // **************************************************
        // stelle für jeden Buchhalter eine Anfrage
        alleAktuellenWetten = getAllBetsFromWettanbieter(alleWettanbieter[wa].name, alleWettanbieter[wa].adresse, alleWettanbieter[wa].port);
        //sAlleMeineWetten += getAllBetsFromWettanbieter(alleWettanbieter[wa].name, alleWettanbieter[wa].adresse, alleWettanbieter[wa].port);
        
        //cout << wa << alleWettanbieter[wa].name << " " << alleWettanbieter[wa].adresse << " " << alleWettanbieter[wa].port << endl;

        cout << "alleAktuellenWetten.size(): " << alleAktuellenWetten.size() << endl; // **************************************************
        if(alleAktuellenWetten.size() == 0)
        {
            continue;
        }
        for(int w = 0; w < alleAktuellenWetten.size(); w++)
        {
            cout << "GetAllMyBets - for2 - WettoptionID " << w << " " << alleAktuellenWetten[w].id << endl; // **************************************************
            //cout << "Test" << endl;
            
            meineAktuellenWetten = getAllMyBetsOfBookmaker(myName, alleAktuellenWetten[w].id, alleWettanbieter[wa].name, alleWettanbieter[wa].adresse, alleWettanbieter[wa].port);

            cout << " " << alleAktuellenWetten[w].id;
            cout << " " << alleWettanbieter[wa].name;
            cout << " " << alleWettanbieter[wa].adresse;
            cout << " " << alleWettanbieter[wa].port << endl;

            cout << "meineAktuellenWetten.size(): " << meineAktuellenWetten.size() << endl; // **************************************************
            if(meineAktuellenWetten.size() == 0)
            {
                continue;
            }
            for(int m = 0; m < meineAktuellenWetten.size(); m++)
            {
                cout << "GetAllMyBets - for3 - meineAktuellenWetten-ID " << m << " " << meineAktuellenWetten[m].wettoption.id << endl; // **************************************************
                alleMeineWetten.push_back(meineAktuellenWetten[m]);
            }
        }
    }

    return alleMeineWetten;
}

void setBettor(string nameanbieter, int betid, string namebettor, double moneyset, int ending, double winachance, double winbchance, double even){
    std::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    try
    {
        transport->open();

        client.setBettor(nameanbieter, betid, namebettor, moneyset, ending, winachance, winbchance, even);

        transport->close();

    } catch (TException& tx) {
    cout << "ERROR in setBettor(): " << tx.what() << endl;
    }
    cout << "added Bettor" << endl;

}

void addBet(string nameanbieter, int id, string playera, string playerb, double winachance, double winbchance, double even)
{
    std::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    try
    {
        transport->open();

        client.addBet(nameanbieter, id, playera, playerb, winachance, winbchance, even);

        transport->close();

    } catch (TException& tx) {
    cout << "ERROR in addBet(): " << tx.what() << endl;
    }
    cout << "added Bet" << endl;
}

int main() {

    //basiert auf: https://www.geeksforgeeks.org/socket-programming-in-cpp/
    auto socketServer = socket(AF_INET, SOCK_STREAM, 0);                   //AF_INET = IPv4 protocol family, SOCK_STREAM = socket is a stream (TCP)       
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    cout << "bind Socket"<< endl;
    bind(socketServer, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    cout << "listen"<< endl;
    listen(socketServer, 5);

    getAlleWettanbieter();

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

                    string sEditFileBuffer(originFileBuffer);

                    // Variablen in html ersetzen
                    if((filename.find("wettender.html")) != string::npos){                        
                        // <option value = "Gryffindor vs. Slytherin">Gryffindor (3.5) vs. Slytherin (1.3) - Even (4.0)</option>
                        string htmlVariableGetSelection = "{bettor_post_get_Selection}";
                        string newHTML = "";
                        vector<Wettoption> alleWetten = getAllBets();
                        /*
                        // TODO: entfernen - Test SpecificBetOption
                        Wettoption test = getSpecificBetOption("Fred_und_George", 3, "localhost", 9090); // string nameanbieter, int id, string adresse, int port)
                        cout << "Test: specificWettOption " << test.id << " " << test.anbieterName << " " << test.playerA << " " << test.playerB << " " << test.quoteA << endl;
                        */
                        for(int i = 0; i < alleWetten.size(); i++)
                        {
                            string quoteA = to_string(alleWetten[i].quoteA);
                            string quoteB = to_string(alleWetten[i].quoteB);
                            string quoteEven = to_string(alleWetten[i].quoteEven);
                            /*
                            newHTML += "<option value = \"" + alleWetten[i].playerA + " vs. " + alleWetten[i].playerB + "\">"
                                      + alleWetten[i].playerA + " (" + quoteA.substr(0, quoteA.find('.')+3) + ") vs. "
                                      + alleWetten[i].playerB + " (" + quoteB.substr(0, quoteB.find('.')+3) + ") - "
                                      + "Even (" + quoteEven.substr(0, quoteEven.find('.')+3) + ")</option>";
                            */

                            //cout << "wettender.html - getPostOptions >> " << alleWetten[i].anbieterName << endl;
                            newHTML += "<option value = \"" + alleWetten[i].anbieterName + "AA" + to_string(alleWetten[i].id) + "AA" + alleWetten[i].playerA + "AA" + alleWetten[i].playerB + "AA" 
                                        + quoteA.substr(0, quoteA.find('.')+3) + "AA" + quoteB.substr(0, quoteA.find('.')+3) + "AA" + quoteEven.substr(0, quoteA.find('.')+3) + "\">"
                                        + alleWetten[i].playerA + " (" + quoteA.substr(0, quoteA.find('.')+3) + ") vs. "
                                        + alleWetten[i].playerB + " (" + quoteB.substr(0, quoteB.find('.')+3) + ") - "
                                        + "Even (" + quoteEven.substr(0, quoteEven.find('.')+3) + ")</option>";
                            /*
                            newHTML += "<p> " + alleWetten[i].playerA + " vs. " + alleWetten[i].playerB + "\">"
                                      + alleWetten[i].playerA + "(" + quoteA.substr(0, quoteA.find('.')+3) + ") vs. "
                                      + alleWetten[i].playerB + "(" + quoteB.substr(0, quoteB.find('.')+3) + ") - "
                                      + "Even (" + quoteEven.substr(0, quoteEven.find('.')+3) + ")</p>";
                            */
                        }
                        replace(sEditFileBuffer, htmlVariableGetSelection, newHTML);
                        
                        // <li>Chudley Cannons (2.0) vs. Holyhead Harpines (1.5) - Even (4.0)<br> 15 auf Team B</li>
                        string htmlVariableCurrentBets = "{bettor_get_my_current_bets}";
                        string htmlVariablePreviousBets = "{bettor_get_my_previous_bets}";
                        vector<PlatzierteWette> alleMeineWetten = getAllMyBets("emmy");

                        for(int i = 0; i < alleMeineWetten.size(); i++)
                        {
                            string quoteA = to_string(alleMeineWetten[i].wettoption.quoteA);
                            string quoteB = to_string(alleMeineWetten[i].wettoption.quoteB);
                            string quoteEven = to_string(alleMeineWetten[i].wettoption.quoteEven);
                            string moneyset = to_string(alleMeineWetten[i].moneyset);

                            // if (game is open) -> current_bets

                            cout << "wettender.html: getCurrentBets >> " << alleWetten[i].anbieterName << endl;
                            newHTML += "<li>" + alleMeineWetten[i].wettoption.playerA + " (" + quoteA.substr(0, quoteA.find('.')+3) + ") vs. "
                                        + alleMeineWetten[i].wettoption.playerB + " (" + quoteB.substr(0, quoteB.find('.')+3) + ") - "
                                        + "Even (" + quoteEven.substr(0, quoteEven.find('.')+3) + ")<br>"
                                        + moneyset.substr(0, quoteEven.find('.')+3) + " auf Team " + alleMeineWetten[i].ending + "</li>";

                            // else (game is closed)-> previous bets
                            /*
                            newHTML += "<li>" + alleMeineWetten[i].wettoption.playerA + " (" + quoteA.substr(0, quoteA.find('.')+3) + ") vs. "
                                        + alleMeineWetten[i].wettoption.playerB + " (" + quoteB.substr(0, quoteB.find('.')+3) + ") - "
                                        + "Even (" + quoteEven.substr(0, quoteEven.find('.')+3) + ")<br>"
                                        + alleMeineWetten[i].moneyset + " auf Team " + alleMeineWetten[i].ending + "<br>"
                                        + "Ausgang: " + /*Spielausgang (braucht ne SpielID - steht in WettOption)*//* + "</li>";
                            */
                        }
                        replace(sEditFileBuffer, htmlVariableCurrentBets, newHTML);
                        replace(sEditFileBuffer, htmlVariablePreviousBets, newHTML);

                    } else if ((filename.find("wettanbieter.html")) != string::npos){
                        string htmlVariable = "{booker_get_bets_of_this_bookmaker}";
                        string newHTML = "";
                        /*vector<Wettoption> alleWAWetten = getAllBetsFromWettanbieter("Fred_und_George", "localhost", 9090);       // ToDo - statischen Anbieter in dynamisch ändern
                        int sumA = 0;
                        int sumB = 0;
                        int sumEven = 0;
                        for(int i = 0; i < alleWAWetten.size(); i++)
                        {
                            newHTML += "<li> " + alleWAWetten[i].playerA + " (" + to_string(alleWAWetten[i].quoteA) + ") "
                                    + "vs. " + alleWAWetten[i].playerB + " (" + to_string(alleWAWetten[i].quoteB) + ") - "
                                    + "Even (" + to_string(alleWAWetten[i].quoteEven) + ") <br>"
                                    + to_string(sumA) + "€ vs. " + to_string(sumB) + "€ - " + to_string(sumEven) + "€</li>";
                            //  TODO: 1) Liste aller eigenen platzierten Wetten - bettors.txt - über RPC-Call
                            // 2) alle Wetten zu diesem Wettangebot raussuchen
                            // 3) je nach "Ending" zu A/B/Even zuaddieren
                        //        40€ vs. 10€ - 0€</li>";

                        }*/
                        //newHTML = getAllBetsFromWettanbieter("Fred_und_George", "localhost", 9090);
                        replace(sEditFileBuffer, htmlVariable, newHTML);

                        htmlVariable = "{booker_update_get_bets_of_this_bookmaker}";
                        // <option value = "Ravenclaw vs. Hufflepuff"> Ravenclaw (1.5) vs. Hufflepuff (2.0)</option>
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

                        setBettor(anbieter, stoi(spielid), name, stod(einsatz), ergebnis, stod(aQuote), stod(bQuote), stod(evenQuote));

                    }                      
                    
                } else if (request.referer.find("wettanbieter",0) != -1)
                {
                    cout << "POST bei wettanbieter" << endl;
                    // Input+Team+A=x&A+Gewinnquote=1.5&Input+Team+B=y&B+Gewinnquote=1.5&Even+Gewinnquote=1.5&Name=hallo
                    string TEAM_A = "Input+Team+A";
                    string TEAM_B = "Input+Team+B";
                    string EVEN = "Even";
                    string QUOTE_A = "A+Gewinnquote=";
                    string QUOTE_B = "B+Gewinnquote=";
                    string QUOTE_EVEN = "Even+Gewinnquote=";
                    string AMPERSAND = "&";
                    string NAME = "Name=";
                    string CARRIAGERETURN = "%0D%0A";

                    /*
                    Input+Team+A=x
                    &A+Gewinnquote=1.5
                    &Input+Team+B=y
                    &B+Gewinnquote=1.5
                    &Even+Gewinnquote=1.5
                    &Name=hallo*/

                    string nameAnbieter = "";
                    string wettID = "";
                    string teamA = "";
                    string teamB = "";
                    string quoteA = "";
                    string quoteB = "";
                    string quoteEven = "";

                    string post = request.data;

                    cout << "*post: " << post << endl;

                    if((post.find(TEAM_A,0) == 0)){      // das habe ich nicht
                        post.erase(0, TEAM_A.size());                        
                        cout << "*post: " << post << endl;

                        if((post.find(CARRIAGERETURN,0) == 0)){
                            cout << "found Carriagereturn" << endl;
                            post.erase(0, CARRIAGERETURN.size());
                            cout << "*post: " << post << endl;
                        }

                        int pos = post.find(AMPERSAND,0);

                        teamA = post.substr(0,pos);
                        post.erase(0, teamA.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;

                        pos = post.find(AMPERSAND,0);
                        quoteA = post.substr(0,pos);
                        post.erase(0, quoteA.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;

                        pos = post.find(AMPERSAND,0);
                        teamB = post.substr(0,pos);
                        post.erase(0, teamB.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;

                        pos = post.find(AMPERSAND,0);
                        quoteB = post.substr(0,pos);
                        post.erase(0, quoteB.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;

                        pos = post.find(AMPERSAND,0);
                        quoteEven = post.substr(0,pos);
                        post.erase(0, quoteEven.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;

                        post.erase(0, NAME.size());
                        nameAnbieter = post;
                        cout << "*post: " << post << endl;

                        addBet(nameAnbieter, 42/*stoi(wettID)*/, teamA, teamB, stod(quoteA), stod(quoteB), stod(quoteEven));
                    }
                }
                int length = request.data.length(); 
                char* bufferPostHeader = new char[length + 1]; 
                strcpy(bufferPostHeader, request.data.c_str());  
                cout << "Message from client: " << bufferPostHeader << endl;
                send(newClientSocket, bufferPostHeader, strlen(bufferPostHeader),0);           

                
            }
            if (errorPage){
                string filename = "error.html";
                ifstream file(filename, ios::binary);
                if(!file.good()){
                    cout << "file cannot be opened (error page)"<< endl;
                } else{
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
