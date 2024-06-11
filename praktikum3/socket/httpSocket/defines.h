#include <string.h>
#include <string>
#include <iostream>
#include <cstring>
#include <sstream>

#define ADRESSE     "localhost"
#define HTTP_PORT   8080
#define RPC_PORT    9090

#define ENDING_OPEN     0
#define ENDING_TEAM_A   1
#define ENDING_TEAM_B   2
#define ENDING_EVEN     3

#define STATUS_OPEN     1
#define STATUS_CLOSED   2

using namespace std;

std::string gameEnding(int ending)
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
        return "Open";
    }
}

std::string gameStatus(int status)
{
    if (status == STATUS_OPEN)
    {
        return "Offen";
    }
    else if (status == STATUS_CLOSED)
    {
        return "Beendet";
    }
    else
    {
        return "Status undefiniert";
    }
}

struct Spiel{
    int id;
    std::string teamA;
    std::string teamB;
    int status;
    int ending = ENDING_OPEN;
    int standA = 0;
    int standB = 0;
};

struct Wettoption{
    int id;
    std::string anbieterName;
    Spiel spiel;
    double quoteA;
    double quoteB;
    double quoteEven;
};

struct Wettanbieter{
    std::string name;
    std::string adresse;
    int port;
};

struct PlatzierteWette{
    std::string namebettor;
    double moneyset;
    int ending;
    Wettoption wettoption;    // nameAnbieter, wett-id, winachance, winbchance, even
};

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
