#pragma comment(lib,"ws2_32.lib")
#include<Winsock2.h>
#include<iostream>
#include<conio.h>
#include<string>
using namespace std;

enum Packet
{
	P_ChatMessage, P_Test
};
SOCKET Connection; 


bool SendInt(int _int)
{
	int RetnCheck = send(Connection, (char*)&_int, sizeof(int), NULL); //send the integer int
	if (RetnCheck == SOCKET_ERROR) //If integer is failed to sent
		return false; //Connection failure - Socket issues
	
	return true; //Successful cnnection
}

bool RevInt(int & _int)
{
	int RetnCheck = recv(Connection, (char*)&_int, sizeof(int), NULL); //receive integer
	if (RetnCheck == SOCKET_ERROR) //If integer cannot be received
		return false; //Connection failure - Socket issues
	
	return true;//Successful connection
}

bool SendPacketType(Packet _packettype)
{
	int RetnCheck = send(Connection, (char*)&_packettype, sizeof(Packet), NULL); //Send packettype
	if (RetnCheck == SOCKET_ERROR) //If packettype failed to sent
		return false; //Connection failure - Socket issues
	
	return true; //Successful connection
}

bool RevPacketType(Packet & _packettype)
{
	int RetnCheck = recv(Connection, (char*)&_packettype, sizeof(Packet), NULL); //receive packet type
	if (RetnCheck == SOCKET_ERROR) //If packettype cannot be received
		return false;//Connection failure - Socket issues
	
	return true;//Successful connection
}

bool SendString(std::string & _string)
{
	if (!SendPacketType(P_ChatMessage)) //If send packet type(string value-chat messages) Fails...
		return false; //connection failure
	int bufferlength = _string.size(); //Find string buffer length
	if (!SendInt(bufferlength)) //If length of string buffer fails to send...
		return false; //connection failure
	int RetnCheck = send(Connection, _string.c_str(), bufferlength, NULL); //Send string buffer
	if (RetnCheck == SOCKET_ERROR) //If failed to send string buffer
		return false; //Return false: Failed to send string buffer
	
	return true;// every thing is sent successfully, hence connection is successful
}

bool RevString(std::string & _string)
{
	int bufferlength; //Stores length of the message
	if (!RevInt(bufferlength))  //Get length of buffer and store in "bufferlength"
		return false; //If get int fails, connection failure

	char * buffer = new char[bufferlength + 1]; //Allocate buffer
	buffer[bufferlength] = '\0'; //Set last character of buffer to null terminator (one of the character & strings rule)
	
	int RetnCheck = recv(Connection, buffer, bufferlength, NULL); //receive message and store the message in buffer array, set RetnCheck to be the value recv returns to see if there is an issue with the connection
	_string = buffer; //set string to received buffer message
	delete[] buffer; //Deallocate buffer memory (clean up buffer to prevent memory leak)
	
	if (RetnCheck == SOCKET_ERROR) //If connection is lost while getting message
		return false;//Connection has issues
	
	return true;// String is received , hence successful connection
}

bool ProcessPacket(Packet _packettype)
{
	switch (_packettype)
	{
		case P_ChatMessage: //Packet Type represents : the chat message
		{
			std::string Message; //string to store our message we received
			if (!RevString(Message)) //Get the chat message and store it in variable: Message
				return false; //If chat message is not received properly
			if(Message.substr(0,4)==" .  ")
			{
				std::cout <<"\t>>\tTo register, enter the command below :"<<std::endl;
				std::cout <<"\t  \treguser -u username -p password"<<std::endl;
				
				std::cout <<"\n\t>>\tAlready have an account? Enter the command below to login : "<<std::endl;
				std::cout <<"\t  \tlogin -u username -p password";
				
			}
			if(Message.substr(0,8)==".       ")
			{
				std::cout <<"\n\nLogin successful!"<<std::endl;
				std::cout <<"\n-------------------------------------------------------------------";
				std::cout <<"\n                               MENU                        ";
				std::cout <<"\n-------------------------------------------------------------------";
					
				std::cout <<"\n\t>>\tVIEW ACTIVE USERS : ";
				std::cout <<"\n\t  \twho -login\n";
				
				std::cout <<"\n\t>>\tVIEW REGISTERED USERS :";
				std::cout <<"\n\t  \twho -all\n";
				
				std::cout <<"\n\t>>\tBROADCAST A MESSAGE";
				std::cout <<"\n\t  \tmsg -b message\n";
				
				std::cout <<"\n\t>>\tCHAT PRIVATELY";
				std::cout <<"\n\t  \tmsg -p message -u username\n";
				
				std::cout <<"\n\t>>\tCREATE A GROUP CHAT (MAX : 5 USERS) ";
				std::cout <<"\n\t  \tmsg -g message -u username1, username2, ...usernameN\n";
				
				std::cout <<"\n\t>>\tLOGOUT : ";
				std::cout <<"\n\t  \tbye\n";
			}
			//Command Error checking:
			if(Message.substr(0,41)=="!! WARNING !!" || Message.substr(0,35)=="<Login failed!"|| Message.substr(0,51)=="User registration failed!") 
			{std::cout <<"\a";
			}

			if(Message.substr(0,2)=="[ ")// message sended sound
			{
				Beep (1050,50);Sleep(100);Beep (1550,150);
			}

			if(Message.substr(0,42)=="You have successfully logged out.")
			{
		        Beep (2550,100);Sleep(50);
				Beep (2050,100);Sleep(50);
				Beep (1550,100);Sleep(50);
				Beep (1050,200);
			}

			
			std::cout << Message << std::endl; //Display the message to the user
			break;
		}
		default: //If packet type is not accounted for
			std::cout << "Unrecognized packet: " << _packettype << std::endl; //Display that packet was not found
			break;
	}
	return true;
}
void ClientThread()
{
	Packet PacketType;
	while (true)
	{
		if (!RevPacketType(PacketType)) //Get packet type
			break; //If there is an issue getting the packet type, exit this loop
		if (!ProcessPacket(PacketType)) //Process packet (packet type)
			break; //If there is an issue processing the packet, exit this loop
	}
	std::cout << "Lost connection to the server." << std::endl;
	closesocket(Connection);
}
int main()
{ 
	//Winsock Startup
	WSADATA wsaData;
	WORD DllVersion = MAKEWORD(2,1);
	if(WSAStartup(DllVersion, &wsaData) !=0)
	{MessageBox(NULL,
	L"Winsock startup failed",
	L"Error",MB_OK | MB_ICONERROR);
		exit(1);	
	}else
	{
	 
	cout << endl;
	cout <<"--------------------------------------------------------------------"<< endl;  
	cout <<"\t\t\t   WELCOME TO CHAT ROOM"<< endl;
	cout <<"--------------------------------------------------------------------\n"<< endl;
		for(int i = 0; i<=100; i++){
		int r = rand() % 100;
		cout<< "\r"<<"\t\tLoading "<< i << "% . . ." <<flush;				
	}
	
	
	
	cout << endl<<endl;
	}
	
	SOCKADDR_IN addr; //Address that we will blind out listening socket to
	int sizeofaddr = sizeof(addr); // length of the address (required for accept call)
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Broadcast locally
	addr.sin_port = htons(1111); // Port
	addr.sin_family = AF_INET;// IPv4 Socket

	Connection = socket(AF_INET,SOCK_STREAM,NULL); //Set Connection socket
	if(connect(Connection,(SOCKADDR*)&addr,sizeofaddr)!=0)//if unable to connect
	{ MessageBoxA(NULL , "Connection Failed.","Error",MB_OK | MB_ICONERROR);
	return 0; //failed to connect
	} 
	CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)ClientThread,NULL,NULL, NULL);// Create client thread

	std::string userinput; //store the user's chat message
	std::string inputname;
	std::string inputpassword;
	while (true)
	{
		std::getline(std::cin,userinput); //Get line if user presses enter and fill the buffer
		if (!SendString(userinput)) //Send string: userinput, If string fails to send... (Connection issue)
			break;
		Sleep(10);
	}

	system("pause");

	return 0;
}
