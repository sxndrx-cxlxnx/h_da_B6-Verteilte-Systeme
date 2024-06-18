#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <unistd.h> 
#include <netinet/in.h> 
#include <sys/socket.h>
#include <filesystem>

#include "../RPCWettanbieter/gen-cpp/Wettanbieter.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include "defines.h"

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

vector <Wettanbieter> alleWettanbieter;

Spiel newSpiel(int id, string teamA, string teamB, int status, int ending, int standA, int standB)
{
    Spiel spiel;

    spiel.id = id;
    spiel.teamA = teamA;
    spiel.teamB = teamB;
    spiel.status = status;
    spiel.ending = ending;
    spiel.standA = standA;
    spiel.standB = standB;

    return spiel;
};

Wettoption newWettoption(int pid, string panbieterName, Spiel pspiel, double pquoteA, double pquoteB, double pquoteEven)
{
    Wettoption wettoption;

    wettoption.id = pid;
    wettoption.anbieterName = panbieterName;
    wettoption.spiel = pspiel;
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

Earnings newEarnings(double pgewinn, double pverlust, double pgesamt)
{
    Earnings earnings;

    earnings.gewinn = pgewinn;
    earnings.verlust = pverlust;
    earnings.gesamt = pgesamt;

    return earnings;
}

void getAllWettanbieter()   // initial
{
    // File pointer 
    ifstream wettanbieterCSV; 
  
    // Open an existing file 
    wettanbieterCSV.open("../RPCWettanbieter/server/wettanbieter.csv", ios::in);

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
void placeNewBet(string nameanbieter, int betid, string namebettor, double moneyset, int prediction, double winachance, double winbchance, double even)
{
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    try
    {
        transport->open();
        client.placeNewBet(nameanbieter, betid, namebettor, moneyset, prediction, winachance, winbchance, even);
        transport->close();

    } catch (TException& tx) {
        cout << "ERROR in placeNewBet(): " << tx.what() << endl;
    }

    cout << "*new Bet placed" << endl;
}

void addBetOption(string nameanbieter, int wettID, int gameID, double winachance, double winbchance, double even)
{
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    try
    {
        transport->open();
        client.addBet(nameanbieter, wettID, gameID, winachance, winbchance, even);
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

Spiel readSpiel(stringstream* sRPC, int status)
{
    cout << "readSpiel" << endl;
    string spielID, teamA, teamB, ending, standA, standB, nulln;
    int iEnding;

    if(status == STATUS_OPEN)
    {
        // open:7:Hufflepuff:Gryffindor;
        getline(*sRPC, spielID, ':');
        getline(*sRPC, teamA, ':');
        getline(*sRPC, teamB, ';');
        getline(*sRPC, nulln, '\n');
        iEnding = ENDING_OPEN;
        standA = "0";
        standB = "0";
    }
    else if (status == STATUS_CLOSED)
    {
        // closed:1:Chudley_Cannons:Holyhead_Harpies:1:200:70;
        getline(*sRPC, spielID, ':');
        getline(*sRPC, teamA, ':');
        getline(*sRPC, teamB, ':');
        getline(*sRPC, ending, ':');
        getline(*sRPC, standA, ':');
        getline(*sRPC, standB, ';');
        getline(*sRPC, nulln, '\n');
        iEnding = stoi(ending);
    }
    else
    {
        spielID = "-1";
        teamA = "invalid Team A";
        teamB = "invalid Team B";
        iEnding = -1;
        standA = "-1";
        standB = "-1";
    }
    cout << "* read Spiel:" << spielID << ":" << teamA << ":" << teamB << ":" << gameStatus(status) << ":" << gameEnding(iEnding) << ":" << standA << ":" << standB << endl << flush;
    
    return newSpiel(stoi(spielID), teamA, teamB, status, iEnding, stoi(standA), stoi(standB));
}

Spiel getSpecificSpiel(int id)
{
    //cout << "entry: get specific Spiel" << endl;
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    string rpc, status;

    try
    {
        transport->open();
        client.getSpecificGame(rpc, id);     // return string - \n fehlt
        transport->close();
    
    } catch (TException& tx) {
        cout << "ERROR in getAllBetOptions(): " << tx.what() << endl;
    }
    //cout << "RPC: " << rpc << "<<" << endl;
    // Output
    // RPC: open:4:Ravenclaw:Slytherin;
    // RPC: closed:1:Chudley_Cannons:Holyhead_Harpies:1:200:70;

    stringstream sRPC;
    sRPC << rpc;
    int iStatus;
    
    getline(sRPC, status, ':');
    cout << "Status Spiel: " << status << endl;

    if(strcmp(status.c_str(), "open") == 0){
        iStatus = STATUS_OPEN;
    } else if (strcmp(status.c_str(), "closed") == 0){
        iStatus = STATUS_CLOSED;
    } else {
        iStatus = -1;
    }

    Spiel spefificGame = readSpiel(&sRPC, iStatus);

    cout << "*got specific Spiel" << endl;

    return spefificGame;
}

Wettoption readWettoption (stringstream* sRPC) //
{
    string readAnbieterName, wettID, spielID, quoteA, quoteB, quoteEven, nulln;

    //cout << "Entry Read Wettoption" << endl << flush;

    // read every column data of a row and 
    // store it in a string variable
    getline(*sRPC, readAnbieterName, ':');
    getline(*sRPC, wettID, ':');
    getline(*sRPC, spielID, ':');
    getline(*sRPC, quoteA, ':');
    getline(*sRPC, quoteB, ':');
    getline(*sRPC, quoteEven, ';');
    getline(*sRPC, nulln, '\n');

    //cout << "*Read Wettoption:" << readAnbieterName << ":" << wettID << ":" << spielID << ":" << quoteA << ":" << quoteB << ":" << quoteEven << endl << flush;

    return newWettoption(stoi(wettID), readAnbieterName, getSpecificSpiel(stoi(spielID)), stod(quoteA), stod(quoteB), stod(quoteEven));
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
    // RPC: Fred_und_George:2:4:1.0:2.0:3.5;

    stringstream sRPC;
    sRPC << rpc;

    Wettoption specificBetOption = readWettoption(&sRPC);

    cout << "*got specific BetOption" << endl;

    return specificBetOption;
}

PlatzierteWette readPlatzierteWette(stringstream* sRPC)
{
    //cout << "Entry: readPlatzierteWette" << endl;
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

    //cout << "*Read Platzierte Wette: " << readAnbieterName << ":" << wettID << ":" << namebettor << ":" << moneyset << ":" << ending << ":" << quoteA << ":" << quoteB << ":" << quoteEven << endl << flush;

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
    // RPC: Fred_und_George 2#Fred_und_George:2:4:1.0:2.0:3.5;
    //      Fred_und_George:2:5:1.5:2.5:3.5;

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
    // RPC: 2#Fred_und_George:2:4:1.0:2.0:3.5;
    //      Fred_und_George:2:5:1.5:2.5:3.5;

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

vector<Spiel> getAllOpenGames()
{
    //cout << "entry: get specific Spiel" << endl;
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    vector <Spiel> openGames;
    string rpc;

    try
    {
        transport->open();
        client.getAllOpenGames(rpc);     // return string - \n fehlt
        transport->close();
    
    } catch (TException& tx) {
        cout << "ERROR in getAllBetOptions(): " << tx.what() << endl;
    }
    //cout << "RPC: " << rpc << "<<" << endl;
    // Output
    // RPC: 4:Ravenclaw:Slytherin;

    stringstream sRPC;
    sRPC << rpc;
    string sLoopCtr = "";
    int loopCtr = 0;

    getline(sRPC, sLoopCtr, '#');
    loopCtr = stoi(sLoopCtr);

    for(int i = 0; i < loopCtr; i++)
    {
        openGames.push_back(readSpiel(&sRPC, STATUS_OPEN));
    }

    cout << "*got all open Spiele" << endl;

    return openGames;
}

vector<Spiel> getAllClosedGames()
{
    //cout << "entry: get specific Spiel" << endl;
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    vector <Spiel> closedGames;
    string rpc;

    try
    {
        transport->open();
        client.getAllClosedGames(rpc);     // return string - \n fehlt
        transport->close();
    
    } catch (TException& tx) {
        cout << "ERROR in getAllBetOptions(): " << tx.what() << endl;
    }
    //cout << "RPC: " << rpc << "<<" << endl;
    // Output
    // RPC: 1:Chudley_Cannons:Holyhead_Harpies:1:200:70;

    stringstream sRPC;
    sRPC << rpc;
    string sLoopCtr = "";
    int loopCtr = 0;

    getline(sRPC, sLoopCtr, '#');
    loopCtr = stoi(sLoopCtr);

    for(int i = 0; i < loopCtr; i++)
    {
        closedGames.push_back(readSpiel(&sRPC, STATUS_CLOSED));
    }

    cout << "*got all closed Spiele" << endl;

    return closedGames;
}

Earnings getEarningsOfBettor(string nameBettor)
{
    cout << "Entry getEarningsOfBettor " + nameBettor << endl;
    vector<double> alleEinnahmen;
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    double einnahme;
    vector<PlatzierteWette> alleMeineWetten = getAllMyPlacedBets(nameBettor);

    for(int i = 0; i < alleMeineWetten.size(); i++)
    {
        try
        {
            transport->open();
            einnahme = client.getEarningBettor(alleMeineWetten[i].wettoption.anbieterName, alleMeineWetten[i].wettoption.id, nameBettor);
            transport->close();
        
        } catch (TException& tx) {
            cout << "ERROR in getAllBetOptions(): " << tx.what() << endl;
        }
        alleEinnahmen.push_back(einnahme);
    }

    double gewinn = 0;
    double verlust = 0;
    double gesamt = 0;

    for(int i = 0; i < alleEinnahmen.size(); i++)
    {
        if (alleEinnahmen[i] < 0){
            verlust += alleEinnahmen[i];
        } else {
            gewinn += alleEinnahmen[i];
        }
        gesamt += alleEinnahmen[i];
    }

    cout << "*got Earnings Of Bettor " + nameBettor << endl;

    return newEarnings(gewinn, verlust, gesamt);
}

// ******************************
// RPC - Update
// ******************************
void changeBetOption(string nameanbieter, int wettID, int gameID, double winachance, double winbchance, double even)
{
    std::shared_ptr<TTransport> socket(new TSocket(ADRESSE, RPC_PORT));
    std::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    std::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    WettanbieterClient client(protocol);

    try
    {
        transport->open();
        client.changeBet(nameanbieter, wettID, gameID, winachance, winbchance, even);
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

    //getAllWettanbieter();

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
                            newHTML_getSelection += "<option value = \"" + alleWetten[i].anbieterName + "AA" + to_string(alleWetten[i].id) + "AA" + alleWetten[i].spiel.teamA + "AA" + alleWetten[i].spiel.teamB + "AA" 
                                        + quoteA.substr(0, quoteA.find('.')+3) + "AA" + quoteB.substr(0, quoteA.find('.')+3) + "AA" + quoteEven.substr(0, quoteA.find('.')+3) + "\">"
                                        + alleWetten[i].spiel.teamA + " (" + quoteA.substr(0, quoteA.find('.')+3) + ") vs. "
                                        + alleWetten[i].spiel.teamB + " (" + quoteB.substr(0, quoteB.find('.')+3) + ") - "
                                        + "Even (" + quoteEven.substr(0, quoteEven.find('.')+3) + ") : [" + alleWetten[i].anbieterName + "]</option>";
                        }
                        replace(sEditFileBuffer, htmlVariable, newHTML_getSelection);
/*
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
                            if(alleMeineWetten[i].wettoption.spiel.ending == STATUS_OPEN)
                            {
                                //cout << "wettender.html: getCurrentBets >> " << alleWetten[i].anbieterName << endl;
                                newHTMLcurrent += "<li>" + alleMeineWetten[i].wettoption.spiel.teamA + " (" + quoteA.substr(0, quoteA.find('.')+3) + ") vs. "
                                                + alleMeineWetten[i].wettoption.spiel.teamB + " (" + quoteB.substr(0, quoteB.find('.')+3) + ") - "
                                                + "Even (" + quoteEven.substr(0, quoteEven.find('.')+3) + ") : [" + alleWetten[i].anbieterName + "]<br>"
                                                + moneyset.substr(0, moneyset.find('.')+3) + "€ auf " + gameEnding(alleMeineWetten[i].ending) + "</li>";
                            }
                            else if (alleMeineWetten[i].wettoption.spiel.ending == STATUS_CLOSED)
                            {
                                //cout << "wettender.html: getCurrentBets >> " << alleWetten[i].anbieterName << endl;
                                newHTMLprevious += "<li>" + alleMeineWetten[i].wettoption.spiel.teamA + " (" + quoteA.substr(0, quoteA.find('.')+3) + ") vs. "
                                                + alleMeineWetten[i].wettoption.spiel.teamB + " (" + quoteB.substr(0, quoteB.find('.')+3) + ") - "
                                                + "Even (" + quoteEven.substr(0, quoteEven.find('.')+3) + ") : [" + alleWetten[i].anbieterName + "]<br>"
                                                + moneyset.substr(0, moneyset.find('.')+3) + "€ auf " + gameEnding(alleMeineWetten[i].ending) + "</li>";
                            }
                        }
                        replace(sEditFileBuffer, htmlVariableCurrentBets, newHTMLcurrent);
                        replace(sEditFileBuffer, htmlVariablePreviousBets, newHTMLcurrent);
*/
                    }
                    else if ((filename.find("wettanbieter.html")) != string::npos)
                    {
                        string htmlVariable = "{booker_get_bets_of_this_bookmaker}";
                        string newHTML = "";
                        string thisAnbieter = "Fred_und_George";        // S-TODO: dynamisch
                        //vector<Wettoption> alleWAWetten = getAllBetOptionsFromWettanbieter(thisAnbieter);       // S-ToDo - statischen Anbieter in dynamisch ändern
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

                            newHTML += "<li> <b>" + alleWAWetten[ctr_wette].spiel.teamA + "</b> (" + quoteA.substr(0, quoteA.find('.')+3) + ") "
                                    + "vs. <b>" + alleWAWetten[ctr_wette].spiel.teamB + "</b> (" + quoteB.substr(0, quoteB.find('.')+3) + ") - "
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
                    else if((filename.find("news.html")) != string::npos)
                    {
                        string htmlVariable;

                        /*
                        // <li>motag: Dobby hat einen Klatscher gegen Harry verhext</li>
                        htmlVariable = "{news_ticker}";
                        string newHTML_ticker = "";
                        vector<News> alleNews = getAllNews();

                        for(int i = 0; i < alleNews.size(); i++)
                        {
                            newHTML_ticker += "<li>" + "message" + "</li>";
                        }
                        replace(sEditFileBuffer, htmlVariable, newHTML_ticker);
                        */

                        // <li>Chudley Cannons vs. Holyhead Harpines</li>
                        htmlVariable = "{news_open_games}";
                        string newHTML_open = "";
                        vector<Spiel> alleOffenenSpiele = getAllOpenGames();

                        for(int i = 0; i < alleOffenenSpiele.size(); i++)
                        {
                            newHTML_open += "<li>" + alleOffenenSpiele[i].teamA + " vs. " + alleOffenenSpiele[i].teamB + "</li>";
                        }
                        replace(sEditFileBuffer, htmlVariable, newHTML_open);

                        // <li>Ravenclaw vs. Hufflepuff <br> 200 : 90</li>
                        htmlVariable = "{news_closed_games}";
                        string newHTML_closed = "";
                        vector<Spiel> alleBeendetenSpiele = getAllClosedGames();

                        for(int i = 0; i < alleBeendetenSpiele.size(); i++)
                        {
                            newHTML_closed += "<li><b>" + alleBeendetenSpiele[i].teamA + "</b> vs. <b>" + alleBeendetenSpiele[i].teamB + "</b><br>"
                                            + to_string(alleBeendetenSpiele[i].standA) + " : " + to_string(alleBeendetenSpiele[i].standB) + "</li>";
                        }
                        replace(sEditFileBuffer, htmlVariable, newHTML_closed);
                    }

                    char* updatedFileBuffer = new char[sEditFileBuffer.length() + 1];
                    strcpy(updatedFileBuffer, sEditFileBuffer.c_str());
                    delete[] originFileBuffer;

                    sendResponse(updatedFileBuffer, "HTTP/1.1 200 Ok", newClientSocket);
                }

            } else if (request.post){
                bool sendEcho = true;
                cout << "* request.referer: " << request.referer; 
                cout << "!"<< endl;

                string dataToSend = "";

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
                    string wettid = "";
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
                        wettid = post.substr(0,pos);
                        post.erase(0, wettid.size());
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
                        } else if (auswahl.find(TEAM_B,0) != -1){
                            ergebnis = 2;
                        } else{
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

                        placeNewBet(anbieter, stoi(wettid), name, stod(einsatz), ergebnis, stod(aQuote), stod(bQuote), stod(evenQuote));
                    }
                    //Wettender Beispiel POST-String: GetValuesOfName=emmy
                    string GETVALUES = "GetValuesOfName=";
                    vector<Wettoption> alleWetten = getAllBetOptions();

                    if((post.find(GETVALUES,0) == 0)){
                        sendEcho = false;
                        post.erase(0, GETVALUES.size());
                        name = post;
                        cout << "*post: " << post << endl;

                        dataToSend += "Ihr Name: " + name;

                        // <li>Chudley Cannons (2.0) vs. Holyhead Harpines (1.5) - Even (4.0) : [Anbieter]<br> 15 auf Team B</li>
                        string newCurrentWetten = "";
                        string newPreviousWetten = "";
                        vector<PlatzierteWette> alleMeineWetten = getAllMyPlacedBets(name);

                        for(int i = 0; i < alleMeineWetten.size(); i++)
                        {
                            string quoteA = to_string(alleMeineWetten[i].wettoption.quoteA);
                            string quoteB = to_string(alleMeineWetten[i].wettoption.quoteB);
                            string quoteEven = to_string(alleMeineWetten[i].wettoption.quoteEven);
                            string moneyset = to_string(alleMeineWetten[i].moneyset);
                            string newString = "";

                            if(alleMeineWetten[i].wettoption.spiel.status == STATUS_OPEN)
                            {
                                //cout << "wettender.html: getCurrentBets >> " << alleWetten[i].anbieterName << endl;
                                newString       = "- " + alleMeineWetten[i].wettoption.spiel.teamA + " (" + quoteA.substr(0, quoteA.find('.')+3) + ") vs. "
                                                + alleMeineWetten[i].wettoption.spiel.teamB + " (" + quoteB.substr(0, quoteB.find('.')+3) + ") - "
                                                + "Even (" + quoteEven.substr(0, quoteEven.find('.')+3) + ") : [" + alleWetten[i].anbieterName + "]\n   "
                                                + moneyset.substr(0, moneyset.find('.')+3) + " Euro auf " + gameEnding(alleMeineWetten[i].ending) + "\n";
                                newCurrentWetten += newString;
                            }
                            else if (alleMeineWetten[i].wettoption.spiel.status == STATUS_CLOSED)
                            {
                                //cout << "wettender.html: getCurrentBets >> " << alleWetten[i].anbieterName << endl;
                                newString       = "- " + alleMeineWetten[i].wettoption.spiel.teamA + " (" + quoteA.substr(0, quoteA.find('.')+3) + ") vs. "
                                                + alleMeineWetten[i].wettoption.spiel.teamB + " (" + quoteB.substr(0, quoteB.find('.')+3) + ") - "
                                                + "Even (" + quoteEven.substr(0, quoteEven.find('.')+3) + ") : [" + alleWetten[i].anbieterName + "]\n   "
                                                + moneyset.substr(0, moneyset.find('.')+3) + " Euro auf " + gameEnding(alleMeineWetten[i].ending) + "\n";
                                newPreviousWetten += newString;
                            }
                        }

                        dataToSend += "\n\n";
                        dataToSend += "Aktuelle Wetten:\n";
                        dataToSend += newCurrentWetten;
                        dataToSend += "\n\n";
                        dataToSend += "Abgeschlossene Wetten:\n";
                        dataToSend += newPreviousWetten;

                        double winMoney = 0;
                        double lossMoney = 0;
                        double wholeMoney = 0;

                        // TODO: hier weiter
                        Earnings earnings = getEarningsOfBettor(name);
                        winMoney = earnings.gewinn;
                        lossMoney = earnings.verlust;
                        wholeMoney = earnings.gesamt;

                        string win = to_string(winMoney);
                        string loss = to_string(lossMoney);
                        string whole = to_string(wholeMoney);
                        
                        dataToSend += "\n\n";
                        dataToSend += "Gesamtuebersicht:\n";
                        dataToSend += "Gewinn:  " + win.substr(0, win.find('.')+3) + "Euro\n";
                        dataToSend += "Verlust: " + loss.substr(0, loss.find('.')+3) + "Euro\n";
                        dataToSend += "Gesamt:  " + whole.substr(0, whole.find('.')+3) + "Euro";
                    }
                }
                else if(request.referer.find("wettanbieter",0) != -1)
                {
                //idbet=5&idgame=3&AGewinnquote=1.5&&BGewinnquote=1.5&even=1.5&nameanbieter=baum  
                cout << "POST bei wettanbieter" << endl;  
                    
                    string GEWINNQUOTEA = "AGewinnquote=";
                    string GEWINNQUOTEB = "BGewinnquote=";
                    string EVENQUOTE = "even=";
                    string IDBET = "idbet=";
                    string IDGAME = "idgame=";
                    string IDBET_DELETE = "idbetdel=";
                    string AMPERSAND = "&";
                    string NAME = "nameanbieter=";
                    string NAME_DELETE = "nameanbieterDel=";
                    string CARRIAGERETURN = "%0D%0A";

                    string anbieter = "";
                    string wettid = "";
                    string gameid = "";
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

                        wettid = post.substr(0,pos);
                        post.erase(0, wettid.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;
                        
                        post.erase(0, IDGAME.size());
                        pos = post.find(AMPERSAND,0);
                        gameid= post.substr(0,pos);
                        post.erase(0, gameid.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;
                        
                        post.erase(0, GEWINNQUOTEA.size());
                        pos = post.find(AMPERSAND,0);
                        aQuote = post.substr(0,pos);
                        post.erase(0, aQuote.size());
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

                        cout << "* anbieter und id: " << anbieter << ":" << stoi(wettid) << endl;


                        if(betExists(anbieter, stoi(wettid)))
                        {
                            changeBetOption(anbieter, stoi(wettid), stoi(gameid), stod(aQuote), stod(bQuote), stod(evenQuote));
                        }
                        else
                        {
                            addBetOption(anbieter, stoi(wettid), stoi(gameid), stod(aQuote), stod(bQuote), stod(evenQuote));
                        }
                        
                    }
                    
                    else if ((post.find(IDBET_DELETE,0) == 0))
                    {                        
                        //idbetdel=3&nameanbieterDel=Hogwarts

                        post.erase(0, IDBET_DELETE.size());                        
                        cout << "*post: " << post << endl;

                        if((post.find(CARRIAGERETURN,0) == 0))
                        {
                            cout << "found Carriagereturn" << endl;
                            post.erase(0, CARRIAGERETURN.size());
                            cout << "*post: " << post << endl;
                        }

                        int pos = post.find(AMPERSAND,0);

                        wettid = post.substr(0,pos);
                        post.erase(0, wettid.size());
                        post.erase(0, AMPERSAND.size());
                        cout << "*post: " << post << endl;
                        
                        post.erase(0, NAME_DELETE.size());
                        anbieter = post;
                        cout << "*post: " << post << endl;

                        if(betExists(anbieter, stoi(wettid)))
                        {                        
                            deleteBetOption(anbieter, stoi(wettid));
                        }
                        else
                        {
                            cout << "Bet does not exist" << endl;
                        }
                    }
                }

                if (sendEcho)
                {
                    int length = request.data.length(); 
                    char* bufferPostHeader = new char[length + 1]; 
                    strcpy(bufferPostHeader, request.data.c_str());
                    cout << "Message from client: " << bufferPostHeader << endl;
                    send(newClientSocket, bufferPostHeader, strlen(bufferPostHeader),0);
                }
                else
                {
                    int length = dataToSend.length();
                    char* bufferPostHeader = new char[length + 1];
                    strcpy(bufferPostHeader, dataToSend.c_str());
                    cout << "Message from client: " << bufferPostHeader << endl;
                    send(newClientSocket, bufferPostHeader, strlen(bufferPostHeader),0);
                }
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
