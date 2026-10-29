//programming assignment two
//Elektra Cavazos PSID:1877122
//Using code snippets from Dr. Rincon's client.cpp file as a reference.


#include <unistd.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <pthread.h>
#include <cstring>
#include <vector>
#include <set>

struct freqData{
    char symbol;
    std::string message;
    sockaddr_in serv_addr;
    int frequency;
    int binary[16];
    int hexa[4];
    int tenLittle;
    int twoLittle[16];
    int sixteenLittle[4];
    
};

//uniqueness function to be used in main
std::vector<char> uniqueOnes(const std::string& str){
    std::set<char> num;    //store uniques                            
    std::vector<char> unique_vector;

    for(int i=0; i<str.size(); i++){ //pushing it back in the order it appeared in the OG message 
        if(num.insert(str[i]).second){
            unique_vector.push_back(str[i]);
        }
    }
    return unique_vector;
};
// straggler >:(
char hextoChar(int digit){ 
    if(digit < 10){
        return '0' + digit;        
    } else {
        return 'A' + (digit - 10); 
    }
}

//need this to open connections to the server for each thread
void* threadPeasant( void* whatever){
    freqData* data = static_cast<freqData*>(whatever);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // OPENING THE PORTAL
   
    if(connect(sockfd,(struct sockaddr *)&data->serv_addr,sizeof(data->serv_addr)) < 0){
        std::cerr << "ERROR connecting" << std::endl;
        exit(0);
    }

    int messageSize = data->message.size();

    //i need to write the unique Character and the message to the server
    write(sockfd,&data->symbol,sizeof(char));
    write(sockfd,&messageSize,sizeof(int)); // just to keep them SEPARATE on input
    write(sockfd,&data->message[0],data->message.size()); 
    //double checking what im writing BEFORE entering the portal 
    // std::cout << "We're going in boys--------------" << std::endl;
    // std::cout << "Symbol: " << data->symbol << std::endl;
    // std::cout << "Message size: " << messageSize << std::endl;
    // std::cout << "Message: " << data->message << std::endl;
       
    //reading the return values from the server
    read(sockfd,&data->frequency,sizeof(int));
    read(sockfd,data->binary,sizeof(int)*16);
    read(sockfd,data->hexa,sizeof(int)*4);
    read(sockfd,&data->tenLittle,sizeof(int));
    read(sockfd,data->twoLittle,sizeof(int)*16);
    read(sockfd,data->sixteenLittle,sizeof(int)*4);
    
    close(sockfd);
    return nullptr;
}
int main(int argc, char* argv[]){
    //I want to send the server my unique symbols and the message for processing
     //has to have all the arguments i need
    if(argc != 3){
        std::cerr << "Usage: " << argv[0] << " <hostname> <port>" << std::endl;
        exit(1);
     }

    //need to read message from a file
    std::string input;
    std::cin >> input;

    if(input.empty()){
        std::cerr << "ERROR: Input message is empty" << std::endl;
        exit(1);
    }

    //get the server information before i call threads
    struct hostent* server = gethostbyname(argv[1]);
    if (server == NULL) { std::cerr << "Where is your IP address? Hand it over " << std::endl; exit(0); }

    sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(atoi(argv[2]));


    //get unique symbols in original order 
    std::vector<freqData> v_Frequency; 
    std::vector<char> uniques = uniqueOnes(input);
    int size = uniques.size();

    //now i want to upload each unique character & message to a struct vector ANDDD ALSOOO server addr
    for(int i = 0 ; i < uniques.size() ; i++){
        v_Frequency.push_back({uniques[i], input, serv_addr});
    }

    std::vector<pthread_t> allThreads(uniques.size()); //intializing as many threads as unique char's i have.

    //see if the message made it into my v_Frequency vector
    // for(int i = 0; i < v_Frequency.size(); i++){
    //     std::cout << "Symbol: " << v_Frequency[i].symbol << ", Message: " << v_Frequency[i].message << std::endl;
    // }

    //start creation of threads for each struct
    for(int i = 0; i < uniques.size() ; i++){
        pthread_create(&allThreads[i], NULL, threadPeasant, static_cast<void*>(&v_Frequency[i])); // ok i got my pthread vector, the function i want it to start, and where I want it to be reading from for the funciton.
    }
    

    //bring it back now yall
    for(int i = 0; i < uniques.size(); i ++){
        pthread_join(allThreads[i], NULL);
    }

    for(int i = 0 ; i < uniques.size(); i++){
        std::cout << "Symbol: " << v_Frequency[i].symbol << std::endl;
        std::cout << "Frequency (raw) value: " << v_Frequency[i].frequency << std::endl ;

        std::cout << "Frequency base 2 (raw) value: " ;
        for(int x = 0; x < 16; x++){
             std::cout << v_Frequency[i].binary[x];
        }
        std::cout << std::endl ;

        std::cout << "Frequency base 16 (raw) value: " ;
        for(int x = 0; x < 4; x++){
        std::cout << hextoChar(v_Frequency[i].hexa[x]);   // adding the hextoChar function
        }

        std::cout << std::endl ;

        std::cout << "Frequency Little Endian base 10 value: " << v_Frequency[i].tenLittle << std::endl;

        std::cout << "Frequency Little Endian base 2 value: " ;
        for(int x = 0; x < 16; x++){
             std::cout << v_Frequency[i].twoLittle[x];
        }
        std::cout << std::endl ;

        std::cout << "Frequency Little Endian base 16 value: " ;
        for(int x = 0; x < 4; x++){
        std::cout << hextoChar(v_Frequency[i].sixteenLittle[x]);   // adding the hextoChar function
        }
        std::cout << std::endl ;


        std::cout << "Frequency Big Endian base 10 value: " << v_Frequency[i].frequency << std::endl ;

        std::cout << "Frequency Big Endian base 2 value: " ;
        for(int x = 0; x < 16; x++){
             std::cout << v_Frequency[i].binary[x];
        }
        std::cout << std::endl ;

        std::cout << "Frequency Big Endian base 16 value: " ;
        for(int x = 0; x < 4; x++){
        std::cout << hextoChar(v_Frequency[i].hexa[x]);   // adding the hextoChar function
        }
        std::cout << std::endl ;
        std::cout << std::endl ;
    }
   
    return 0;
};