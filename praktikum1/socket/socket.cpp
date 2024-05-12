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

    std::stringstream sMessage;
    sMessage << message;

    string line = "";
    getline(sMessage, line);

    element.get = (line.find(GET,0) == 0);                              //1 line
    element.post = (line.find(POST,0) == 0);
    
    if( !(element.get | element.post)){
        return element;
    }else{
        if(element.get){
            line.erase(0, GET.size());
            cout << "*GET" <<  endl;
        }else{
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
            }else{
                cout << "*Host: " << element.host << endl;
                return element;            
            }
        }else if((line.find(USER_AGENT,0) == 0)){
            line.erase(0, USER_AGENT.size());        
            element.userAgent = line; 
            cout << "*User-Agent: " << element.userAgent << endl;

        }else if((line.find(ACCEPT,0) == 0)){                                      
            line.erase(0, ACCEPT.size());
            if(line.find(TEXT_HTML, 0) != -1 | line.find(IMAGE_AVIF, 0) != -1){                
                if(line.find(IMAGE_AVIF, 0) != -1 && element.requestURI == FAVICON){
                    element.faviconReq = true;
                }                
                element.accept = line;
                cout << "*Accept: " << element.accept << endl;
            }else{
                return element;
            }
        }else if((line.find(ACCEPT_LANGUAGE,0) == 0)){                                      
            line.erase(0, ACCEPT_LANGUAGE.size());
            element.acceptLang = line;
            cout << "*Accept-Language: " << element.acceptLang << endl;

        }else if((line.find(ACCEPT_ENCODING,0) == 0)){                                       
            line.erase(0, ACCEPT_ENCODING.size());
            element.acceptEnc = line;
            cout << "*Accept-Encoding: " << element.acceptEnc << endl;

        }else if((line.find(DNT,0) == 0)){                                                    
            line.erase(0, DNT.size());
            element.dnt = stoi(line);
            cout << "*DNT: " << element.dnt << endl;

        }else if((line.find(SEC_GPT,0) == 0)){                                                    
            line.erase(0, SEC_GPT.size());
            element.secGPC = stoi(line);
            cout << "*Sec-GPT: " << element.secGPC << endl; 

        }else if((line.find(CONNECTION,0) == 0)){                                                    
            line.erase(0, CONNECTION.size());
            if(line.find(KEEP_ALIVE, 0) != -1){
                element.connection = line;
                cout << "*Connection: " << element.connection << endl;
            }else{
                return element;
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

    while(true){
        cout << "in while-loop"<< endl;

        int newClientSocket = accept(socketServer, nullptr, nullptr);
        pid_t childPID = 0;

        if(newClientSocket < 0){
            cout << "server: error while accepting client" << endl;
        }

        if((childPID = fork()) < 0){
            cout << "server: error while fork" << endl;
        }else if (childPID == 0){
            cout << "forked"<< endl;

            close(socketServer);

            char buffer[1024] = { "" }; 
            recv(newClientSocket, buffer, sizeof(buffer), 0);

            bool errorPage = false;

            string sBuffer(buffer); 
            HTTPrequest request = validateHTTP(sBuffer);
            if(request.valid){
                cout << "message from client: " << endl << buffer << endl;
            }else{
                cout << "invalid message from client" << endl << buffer << endl;
                errorPage = true;                
            }

            if(request.get){

                if (request.faviconReq){
                    string filename = "favicon.ico";
                    ifstream file(filename, ios::binary);
                    if(!file.good()){
                        cout << "file cannot be opened"<< endl;
                        errorPage = true;
                    }else{
                        int buffersize = 10000;
                        char* const buf = new char[buffersize];
                        char buffer[1024] = { "HTTP/1.1 200 Ok\nContent-type: image/avif\n\n\r\n\r\n" };
                        send(newClientSocket, buffer, strlen(buffer),0);
                        int pos{0};
                        while(file.get(buffer[pos++]));     

                        send(newClientSocket, buffer, strlen(buffer),0);
                        
                        delete[] buf;
                    }
                }else{
                    string filename = request.requestURI+".html";
                    ifstream file(filename, ios::binary);
                    if(!file.good()){
                        cout << "file cannot be opened"<< endl;
                        errorPage = true;
                    }else{
                        int buffersize = 10000;
                        char* const buf = new char[buffersize];
                        char buffer[1024] = { "HTTP/1.1 200 Ok\nContent-type: text/html\nAcceptLanguage: de,en-US;q=0.7,en|q=0.3\n\r\n\r\n" };
                        send(newClientSocket, buffer, strlen(buffer),0);
                        int pos{0};
                        while(file.get(buffer[pos++]));    

                        send(newClientSocket, buffer, strlen(buffer),0);                    
                        delete[] buf;
                    } 
                }

            }else if (request.post){
                int length = request.data.length(); 
                char* buffer = new char[length + 1]; 
                strcpy(buffer, request.data.c_str());  
                cout << "Message from client: " << buffer << endl;
                send(newClientSocket, buffer, strlen(buffer),0); 

            }
            if (errorPage){
                string filename = "error.html";
                ifstream file(filename, ios::binary);
                if(!file.good()){
                    cout << "file cannot be opened (error page)"<< endl;
                }else{
                    int buffersize = 10000;
                    char* const buf = new char[buffersize];
                    char buffer[1024] = { "HTTP/1.1 200 Ok\nContent-type: text/html\n\n\r\n\r\n" };
                    send(newClientSocket, buffer, strlen(buffer),0);
                    int pos{0};
                    while(file.get(buffer[pos++]));  

                    send(newClientSocket, buffer, strlen(buffer),0);
                    
                    delete[] buf;
                }
            }       
            exit(0);
        }   
        cout << "close newClientSocket"<< endl;
        close(newClientSocket); 
    }
}

