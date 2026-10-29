
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <set>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>


//struct to hold every symbol value's input and all its results :
struct freqData{
    char symbol;
    std::string message;
    int frequency;
    int binary[16] ; 
    int hexa[4] ; 
    int tenLittle;
    int twoLittle[16] ; 
    int sixteenLittle[4] ; 
};

//Using Dr.Rincon's client.cpp file as a reference point
//function to count frequency of each unique character in the input string
void* freqCalc(void* data){ // need to read in the symbol and the message from each thread struct
    std::string message = static_cast<freqData*>(data)->message; //static_cast turns my void into the struct i need
    char symbol = static_cast<freqData*>(data)->symbol;
    int count = 0;

    for(int i=0 ; i < message.size() ; i++){
        if(message[i] == symbol){
            count += 1;
        }
    }

    static_cast<freqData*>(data)->frequency = count;
    return nullptr;
    };

// function to change input to 16 bit binary 
void* binarytime(void* freqval){
        freqData* data = static_cast<freqData*>(freqval); // pointer to my struc 
        int incomingValue = data->frequency; // what im using for this interation
      
        if(incomingValue == 0){
            return nullptr;
        }

        //stepping through the loop backwards this time so i don't have to flip the array later
        for(int i = 15 ; i >= 0; i--){
            data->binary[i] = incomingValue % 2;
            incomingValue = incomingValue / 2;
        }
        return nullptr;
}

// function to change input to hexadecimal
void *hexatime(void* freqval){
        freqData* data = static_cast<freqData*>(freqval); // pointer to my struc 
        int incomingValue = data->frequency; // what im using for this interation

        if(incomingValue == 0){
            return nullptr;
        }

        //loop to convert the val to hexadecimal && also again stepping backwards
        for(int i = 3 ; i >= 0; i--){
            data->hexa[i] =  incomingValue % 16;
            incomingValue = incomingValue / 16;
        }

        //Assign my little endian 16 since its this but the bytes are switched around
        data->sixteenLittle[0] = data->hexa[2];
        data->sixteenLittle[1] = data->hexa[3];
        data->sixteenLittle[2] = data->hexa[0];
        data->sixteenLittle[3] = data->hexa[1];

        return nullptr;
}

// function to change input to little endian base 10
void *ten_littleendian(void *num){
    freqData* data = static_cast<freqData*>(num); // pointer to my struc 

    int exponent = 1;
    int sum = 0;

    //loop to multiply everything and push it onto tenLittle
    for(int i = 3; i >=0; i--){
        sum += data->sixteenLittle[i] * (exponent);
        exponent *= 16;
    }

    data->tenLittle = sum;
    return nullptr;
}

// function to change input to little endian base 2
void *two_littleendians(void *num){
    //basically read in my binary value and divide it into two halves then flip them so that it reads out in little endian 
    freqData* data = static_cast<freqData*>(num); // pointer to my struc
    std::vector<int> initial, secondHalf;

    for(int i = 0 ; i < 16 ; i++){
        if(i < 8){
            secondHalf.push_back(data->binary[i]);
        }
        else{
            initial.push_back(data->binary[i]);  
        }
    }

    //  CONCATENATE!!!
    initial.insert(initial.end(),secondHalf.begin(),secondHalf.end());

    for(int i = 0; i < 16; i++){
        data->twoLittle[i] = initial[i];
    }



    return nullptr;
}

void* TheThreadMaster(void* thread){ // need this to call all other functions so that i meet the n threads goal :>
    freqData* data = static_cast<freqData*>(thread);

    freqCalc(static_cast<void*>(data));
    binarytime(static_cast<void*>(data));
    hexatime(static_cast<void*>(data));
    ten_littleendian(static_cast<void*>(data));
    two_littleendians(static_cast<void*>(data));

    return nullptr;
}

//kill all zombies
void fireman(int){
    while(waitpid(-1, NULL, WNOHANG) > 0); 
}

//actual server handling loop for incoming client connections
int main(int argc, char* argv[]){
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    
    // Creating a signal to execute the fireman function when a child process end its execution
    signal(SIGCHLD, fireman); 
    
    // Check for the port number from the command line  
    if (argc < 2){
        std::cerr << "ERROR, no port provided\n";
        exit(1);
    }

    //Create a socket descriptor for the PARENT 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        std::cerr << "ERROR opening socket" << std::endl;
        exit(1);
    }

    portno = atoi(argv[1]);

    //Populate the sockaddr_in structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    // Bind the socket descriptor with the sockaddr_in structure
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        std::cerr << "ERROR on binding" << std::endl;
        exit(1);
    }

    // Set the maximum number of concurrent connections
    listen(sockfd, 20);
    signal(SIGCHLD, fireman);

    clilen = sizeof(cli_addr);

    while (true){
        // Create a socket descriptor for the CHILD 
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);

        if(newsockfd < 0){ //continuing so that the server keeps running even if accept fails
            continue;
        };
     

        // calling the fork to handle the client request
        if (fork() == 0){
            close(sockfd); // Close the listening socket in the child process

            char symbol;
            n = read(newsockfd, &symbol, sizeof(char)); //read(fd, address to store into, num bytes)
            if (n < 0){std::cerr << "ERROR reading from socket";exit(1); }

            int msgSize;
            n = read(newsockfd, &msgSize, sizeof(int));
            std::string message(msgSize, '\0'); // allocate a string of size msgSize to store the incoming message
            if(n <=0){std::cerr << "ERROR reading message size from socket"; exit(1);}
            
            n = read(newsockfd, &message[0], msgSize);
            if(n <=0) {std::cerr << "ERROR reading message from socket"; exit(1);}

            //checking that my data made it past the portal jump
            // std::cout << "Portal Jump Through--------" << std::endl;
            // std::cout << "Symbol: " << symbol << std::endl;
            // std::cout << "Message: " << message << std::endl;
            // std::cout << "Message size: " << msgSize << std::endl;

            //i need to run my threadmaster function
            freqData data;
            data.symbol = symbol;
            data.message = message;
            TheThreadMaster(&data);

            //sending everything back
            write(newsockfd, &data.frequency, sizeof(data.frequency));
            write(newsockfd, &data.binary, sizeof(data.binary));
            write(newsockfd, &data.hexa, sizeof(data.hexa));
            write(newsockfd, &data.tenLittle, sizeof(data.tenLittle));
            write(newsockfd, &data.twoLittle, sizeof(data.twoLittle));
            write(newsockfd, &data.sixteenLittle, sizeof(data.sixteenLittle));
          

            // Terminate the child process
            _exit(0);
        }
    }
     close(newsockfd); // child dealt with it, parent closes the socket after fork
}