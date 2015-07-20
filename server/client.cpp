
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>    /* Internet domain header */
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <iostream>
#include "base64.h"
#include <sstream>

using namespace std;

#define SERVER_PORT  32800
struct sockaddr_in peer = {AF_INET, 0}; 
int CLIENT;
struct hostent *SERVER;

const string HOST = "Host: pdc.amd01.poly.edu\n";


//----------------------------------------------------------------------------------//
//---------------------------------  URL ENCODE ------------------------------------//
//----------------------------------------------------------------------------------//

const char SAFE[256] = {
    /*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
    /* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,
    
    /* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
    /* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
    /* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
    /* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
    
    /* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    
    /* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    /* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};

const char HEX2DEC[256] =  {
    /*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
    /* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,
    
    /* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    
    /* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    
    /* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};

string urlEncode(const string & sSrc) {
   const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
   const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
   const int SRC_LEN = sSrc.length();
   unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
   unsigned char * pEnd = pStart;
   const unsigned char * const SRC_END = pSrc + SRC_LEN;

   for (; pSrc < SRC_END; ++pSrc)
   {
      if (SAFE[*pSrc]) 
         *pEnd++ = *pSrc;
      else
      {
         // escape this char
         *pEnd++ = '%';
         *pEnd++ = DEC2HEX[*pSrc >> 4];
         *pEnd++ = DEC2HEX[*pSrc & 0x0F];
      }
   }

   std::string sResult((char *)pStart, (char *)pEnd);
   delete [] pStart;
   return sResult;
}

string urlDecode(const string & sSrc) {
   // Note from RFC1630: "Sequences which start with a percent
   // sign but are not followed by two hexadecimal characters
   // (0-9, A-F) are reserved for future extension"

   const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
   const int SRC_LEN = sSrc.length();
   const unsigned char * const SRC_END = pSrc + SRC_LEN;
   // last decodable '%' 
   const unsigned char * const SRC_LAST_DEC = SRC_END - 2;

   char * const pStart = new char[SRC_LEN];
   char * pEnd = pStart;

   while (pSrc < SRC_LAST_DEC)
   {
      if (*pSrc == '%')
      {
         char dec1, dec2;
         if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
            && -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
         {
            *pEnd++ = (dec1 << 4) + dec2;
            pSrc += 3;
            continue;
         }
      }

      *pEnd++ = *pSrc++;
   }

   // the last 2- chars
   while (pSrc < SRC_END)
      *pEnd++ = *pSrc++;

   std::string sResult(pStart, pEnd);
   delete [] pStart;
   return sResult;
}


//----------------------------------------------------------------------------------//
//------------------------------- CONNECTION FUNCTIONS -----------------------------//
//----------------------------------------------------------------------------------//

void setupConnection(int argc, char* argv[])
{
	peer.sin_port = htons(SERVER_PORT);

    if ( argc != 2 )
	{  
		fprintf(stderr, "Please specify server address\n", argv[0]);
		exit(1);
	}
	
    /* fill in peer address */
    SERVER = gethostbyname(argv[1]);
    if( SERVER == NULL )
	{  
		fprintf(stderr, "%s: %s unknow host\n", argv[0], argv[1]);
		exit(1);
	}
    bcopy(SERVER->h_addr_list[0], (char*)&peer.sin_addr, SERVER->h_length);
    
	/* create CLIENT socket */
    CLIENT = socket(AF_INET, SOCK_STREAM, 0);
	
    /* request connection to SERVER */
    if(connect(CLIENT, (struct sockaddr *)&peer, sizeof(peer)) == -1)
	{  
		perror("Cannot connect to server"); 
		close(CLIENT);
		exit(1); 
    }
}

string sendAndListen(string& request)
{
	char buf[9999];
	
	int check;
	check = write(CLIENT, request.c_str(), request.length()); //send requet
	if(check == -1)
	{
		cout << "Error while sending to the server. Client terminated.\n";
		exit(1);
	}
	
    read(CLIENT, buf, sizeof(buf)); //receive reply
	if(check == -1)
	{
		cout << "Error while recieving from the server. Client terminated.\n";
		exit(1);
	}
	    
	string reply(buf);
	return reply;
}


//----------------------------------------------------------------------------------//
//-------------------------------- SUB FUNCTIONS -----------------------------------//
//----------------------------------------------------------------------------------//

void displayMenu()
{
	cout << "Welcome to Ptwitter. Please choose one of these action below:\n"
		 << "1. Register\n"
		 << "2. Follow a friend\n"
		 << "3. View your followers\n"
		 << "4. View recent tweets of any users\n"
		 << "5. Update your tweet\n"
		 << "6. Exit\n";
}

string authorization()
{
	//get screen_name and password
	string screen_name, password, authorize;
	cout << "Please enter your screen name (cannot be all numbers): ";
	cin >> screen_name;
	cout << "Please enter your password: ";
	cin >> password;
	
	//encode into Authorization header
	string temp = screen_name;
	temp += ":";
	temp += password;
	authorize = "Authorization: Basic ";
	authorize += base64_encode_cpp(temp);
	authorize += "\r\n";
	
	return authorize;
}

string getStatus(string& reply)
{
	int pos = reply.find("\r");
	return reply.substr(9, pos-10);
}

string getErr(string& reply)
{
	int pos1 = reply.find("<error>");
	int pos2 = reply.find("</error>");
	return reply.substr(pos1+7, pos2-(pos1+7));
}

//----------------------------------------------------------------------------------//
//------------------------------- REQUEST FUNCTIONS --------------------------------//
//----------------------------------------------------------------------------------//

string registerReq()
{
	//get screen_name and password
	string screen_name, password, request;
	cout << "Please enter your screen name (cannot be all numbers): ";
	cin >> screen_name;
	cout << "Please enter your password: ";
	cin >> password;
	
	//make request
	request = "POST /account/register.xml\r\n";
	request += HOST;
	request += "screen_name=";
	request += screen_name;
	request += "&password=";
	request += password;
	request += "\r\n\r\n";
	
	return request;
}

string following()
{
	//get screen_name
	string user, request;
	cout << "Please enter the user you want to follow: ";
	cin >> user;
	
	//make request
	request = "POST /friendships/create/";
	request += user;
	request += ".xml HTTP/1.1\r\n";
	request += authorization(); //Authorization header
	request += HOST; //Host header
	request += "\r\n";
	
	return request;
}

string update()
{
	//get new tweet
	string request, tweet;
	char temp[1];
	cout << "Please enter your new tweet(enter | to finish): ";
	cin.getline(temp, '1');
	getline(cin, tweet,'|');
	
	//make request
	request = "POST /statuses/update.xml HTTP/1.1\r\n";
	request += authorization(); //Authorization header
	request += HOST; //Host header
	request += "status=" + urlEncode(tweet);
	request += "\r\n\r\n";
	
	return request;
}

string recentTweets()
{
	//get screen_name and number of tweets
	string user, count;
	cout << "Please enter the user whose tweets you want to see: ";
	cin >> user;
	cout << "Please enter the number of tweets you want to see: ";
	cin >> count;
	
	//make request
	stringstream ss;
	ss << "GET /statuses/user_timeline/" << user << ".xml?count=" << count << " HTTP/1.1\r\n";
	ss << authorization(); //Authorization header
	ss << HOST; //Host header
	ss << "\r\n\r\n";
	
	return ss.str();
}

string followers(int cursor)
{
	//make request
	stringstream ss;
	ss << "GET /statuses/followers.xml?cursor=" << cursor << " HTTP/1.1\r\n";
	ss << authorization(); //Authorization header
	ss << HOST; //Host header
	ss << "\r\n\r\n";
	return ss.str();
}


//----------------------------------------------------------------------------------//
//---------------------------- REPLY PROCESSING FUNCTIONS --------------------------//
//----------------------------------------------------------------------------------//

void replyReg(string& reply)
{
	if(reply.find("200 OK") == string::npos) //If receive error
	{
		cout << getStatus(reply) << endl;
		cout << "Error: " << getErr(reply) << endl << endl;
	}
	else //register successful
	{
		string screen_name, user_id;
		
		//get new user_id
		int pos1 = reply.find("<id>");
		int pos2 = reply.find("</id>");
		user_id = reply.substr(pos1+4, pos2-(pos1+4));
		
		//get new screen_name
		pos1 = reply.find("<screen_name>");
		pos2 = reply.find("</screen_name>");
		screen_name = reply.substr(pos1+13, pos2-(pos1+13));
		
		//display upon success
		cout << "Congratulation, " << screen_name << "! You have successfully registered. Your ID number is " << user_id << ".\n\n";
	}
}

void replyFollowing(string& reply)
{
	if(reply.find("200 OK") == string::npos) //If receive error
	{
		cout << getStatus(reply) << endl;
		cout << "Error: " << getErr(reply) << endl << endl;
	}
	else
	{
		//get screen_name
		int pos1 = reply.find("<screen_name>");
		int pos2 = reply.find("</screen_name>");
		string screen_name = reply.substr(pos1+13, pos2-(pos1+13));
		
		//display upon success
		cout << "Congratulation! You have successfully followed " << screen_name << ".\n\n";
	}
}

void replyTweets(string& reply)
{
	if(reply.find("200 OK") == string::npos) //If receive error
	{
		cout << getStatus(reply) << endl;
		cout << "Error: " << getErr(reply) << endl << endl;
	}
	else
	{
		string temp, screen_name;
		int pos1 = reply.find("<status>");
		int pos2 = reply.find("</status>");
		if(pos1 == string::npos)
		{
			cout << "This user has no tweet.\n";
		}
		else
		{
			temp = reply.substr(pos1);
			
			//get screen_name
			pos1 = temp.find("<screen_name>");
			pos2 = temp.find("</screen_name>");
			screen_name = temp.substr(pos1+13, pos2-(pos1+13));
			cout << screen_name << "'s lastest tweets: \n";
			
			//display tweets
			while( temp.find("<status>") != string::npos )
			{
				//get time
				pos1 = temp.find("<created_at>");
				pos2 = temp.find("</created_at>");
				string time = temp.substr(pos1+12, pos2-(pos1+12));
				
				//get tweet's content
				pos1 = temp.find("<text>");
				pos2 = temp.find("</text>");
				string text = urlDecode(temp.substr(pos1+6, pos2-(pos1+6)));
				
				//display tweet
				cout << time << ": " << text << endl;
				
				pos1 = temp.find("</status>");
				temp = temp.substr(pos1+9);
			}
			
			cout << endl;
		}
	}
}

void replyUpdate(string& reply)
{
	if(reply.find("200 OK") == string::npos) //If receive error
	{
		cout << getStatus(reply) << endl;
		cout << "Error: " << getErr(reply) << endl << endl;
	}
	else
	{
		//get time
		int pos1 = reply.find("<created_at>");
		int pos2 = reply.find("</created_at>");
		string time = reply.substr(pos1+12, pos2-(pos1+12));
		
		//get tweet's content
		pos1 = reply.find("<text>");
		pos2 = reply.find("</text>");
		string text = urlDecode( reply.substr(pos1+6, pos2-(pos1+6)) );
		
		//display tweet
		cout << "Your new tweet:\n";
		cout << time << ": " << text << endl;
	}
}

void replyFollowers(string& reply, int& next, int& previous)
{
	if(reply.find("200 OK") == string::npos) //If receive error
	{
		cout << getStatus(reply) << endl;
		cout << "Error: " << getErr(reply) << endl << endl;
	}
	else
	{
		//get next cursor
		int pos1 = reply.find("<next_cursor>");
		int pos2 = reply.find("</next_cursor>");
		string temp = reply.substr(pos1+13, pos2-(pos1+13));
		next = atoi(temp.c_str());
		
		//get previous cursor
		pos1 = reply.find("<previous_cursor>");
		pos2 = reply.find("</previous_cursor>");
		temp = reply.substr(pos1+17, pos2-(pos1+17));
		previous = atoi(temp.c_str());
		
		//get followers
		pos1 = reply.find("<user>");
		pos2 = reply.find("</user>");
		if(pos1 == string::npos) //if there is no follower
		{
			cout << "You have no follower.\n";
		}
		else //there are followers
		{
			temp = reply.substr(pos1);
			
			cout << "Your followers:\n";
			
			while(temp.find("<user>") != string::npos)
			{
				//get follower's screen_name
				pos1 = temp.find("<screen_name>");
				pos2 = temp.find("</screen_name>");
				
				//display follower
				cout << temp.substr(pos1+13, pos2-(pos1+13)) << endl;
				
				pos1 = temp.find("</user>");
				temp = temp.substr(pos1+9);				
			}
		}
		cout << endl;
	}
}


//----------------------------------------------------------------------------------//
//-------------------------------------- MAIN --------------------------------------//
//----------------------------------------------------------------------------------//

int main(int argc, char* argv[])
{ 
	while(true)
	{
		string request, reply;
		int choice;
		
		//display option for user
		displayMenu();
		cout << "Please enter your choice: ";
		cin >> choice;
		
		if(choice == 1) //register
		{
			//connect to server
			setupConnection(argc, argv);
			
			//send request, get and process reply message
			request = registerReq();
			reply = sendAndListen(request);
			cout << endl;
			replyReg(reply);
			
			//close connection
			close(CLIENT); 
		}
		else if(choice == 2) //follow a friend
		{
			//connect to server
			setupConnection(argc, argv);
			
			//send request, get and process reply message
			request = following();
			reply = sendAndListen(request);
			cout << endl;
			replyFollowing(reply);
			
			//close connection
			close(CLIENT); 
		}
		else if(choice == 3) //see followers
		{
			int next(-1), previous(-1), cursor(-1);
					
			while(cursor != 3)
			{
				//connect to server
				setupConnection(argc, argv);
				
				if(cursor == 1)
				{
					cursor = next;
				}
				else
				{
					cursor = previous;
				}
				
				//send request, get and process reply message
				request = followers(cursor);
				reply = sendAndListen(request);
				cout << endl;
				replyFollowers(reply, next, previous);
				
				//navigating through list of followers
				cout << "Navigation through list of your followers: \n";
				if(next != 0)
				{
					cout << "1. Next\n";
				}
				if(previous != 0)
				{
					cout << "2. Previous\n";
				}
				cout << "3. Go back to main menu\n" << "Please enter your choice:";
				cin >> cursor;
				
				//close connection
				close(CLIENT); 
			}
			cout << endl;
		}
		else if(choice == 4) //view recent tweets
		{
			//connect to server
			setupConnection(argc, argv);
			
			//send request, get and process reply message
			request = recentTweets();
			reply = sendAndListen(request);
			cout << endl;
			replyTweets(reply);
			
			//close connection
			close(CLIENT); 
		}
		else if(choice == 5) //update tweet
		{
			//connect to server
			setupConnection(argc, argv);
			
			//send request, get and process reply message
			request = update();
			reply = sendAndListen(request);
			cout << endl;
			replyUpdate(reply);
			
			//close connection
			close(CLIENT); 
		}
		else if(choice == 6) //exit
		{
			return(0);
		}
		else //invalid choice
		{
			cout << "\nInvalid choice.\n\n";
		}
	}   
}


