#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>        /* for getenv */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>    /* Internet domain header */
#include <iostream>
#include "mysql++/mysql++.h"
#include <string>
#include <sstream>
#include <ctype.h>
#include "base64.h"
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <fstream>

using namespace mysqlpp;
using namespace std;

//#define SERVER_PORT 32800
struct sockaddr_in self = {AF_INET, 0};
ofstream logFile;
int SERVER;
pthread_mutex_t mutex_log;


//---------------------------------------------------------------------------------//
//-------------------------------- HEADER PROCCESSING -----------------------------//
//---------------------------------------------------------------------------------//

string HTTP_200 = "HTTP/1.1 200 OK\r\nContent-Type: application/xml; charset=utf-8\r\nconnection: close\r\n\r\n";
string HTTP_400 = "HTTP/1.1 400 Bad Request\n\rContent-Type: application/xml; charset=utf-8\r\nconnection: close\r\n\r\n";
string HTTP_404 = "HTTP/1.1 404 Forbidden\r\nContent-Type: application/xml; charset=utf-8\r\nconnection: close\r\n\r\n";
string HTTP_500 = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: application/xml; charset=utf-8\r\nconnection: close\r\n\r\n";

string XML_HEADER = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n";

string genErr(string& status, string& url, string error)
{
	string message;
	message += status;
	message += XML_HEADER;
	message += "<hash>\r\n";
	message += "<request>";
	message += url;
	message += "</request>\r\n";
	message += "<error>";
	message += error;
	message += "</error>\r\n";
	message += "</hash>\r\n\n\r";
	return message;
};

string getUrl(string& request)
{
	if(request[0] == 'P')
	{
		int pos1 = request.find("POST");
		int pos2 = request.find(".xml");
		return request.substr(pos1+5, pos2-(pos1+1));
	}
	else
	{
		int pos1 = request.find("GET");
		int pos2 = request.find("HTTP");
		return request.substr(pos1+4, pos2-(pos1+5));
	}	
}


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


//---------------------------------------------------------------------------------//
//------------------------------- PROCESS OBJECT ----------------------------------//
//---------------------------------------------------------------------------------//

class Process
{
	public:
	Connection conn;
	int client;

	Process(int client) : client(client), conn(false) {};
	
	//connection functions
	string listenClient();
	void replyClient(string& message);
	void connectDB();
	void closeDB();
	void checkDB();
	
	//sub functions
	Query runQuery(string& request);
	bool isNum(string& data);
	bool authorized(string& request, string& url, string& screen_name);
	string getQueryValue(string& query, string var);
	int getID(string& screen_name);
	string getUsername(string& user_id);
	bool isUserExist(string& data);
	bool isFollowed(string& screen_name, string& following);

	//process request functions
	void login(string& request, string& message);
	void makeAcc(string& request, string& message);
	void updateTweet(string& request, string& message);
	void follow(string& request, string& message);
	void followers(string& request, string& message);
	void recentTweets(string& request, string& message);
	void processRequest();
};


//----------------------------------------------------------------------------------//
//-------------------------------------- MAIN FUNCTIONS--------------------------------------//
//----------------------------------------------------------------------------------//

string getTime(string timestamp)
{
	char datetime[31];
	struct tm tmp;
	time_t t;
	if(timestamp ==  "default")
	{
		t = time(NULL);
	}
	else
	{
		t = atoi(timestamp.c_str());
	}

	if (gmtime_r((time_t *)&t, &tmp) == NULL)
	{
		// do some error checking.
		// perror("gmtime_r");
	}
	// %z is not standardzied, but pdc has support for it.
	if(strftime(datetime, sizeof(datetime), "%a %b %d %H:%M:%S %z %Y", &tmp) == 0)
	{
		// more error checking.
		fprintf(stderr, "strftime returned 0");
	}

	string datetime_cpp(datetime);
	return datetime_cpp;
}	

void setupServer(int port)
{
	int val;
	self.sin_port = htons(port);
	
	/* set up listening SERVER Socket */
    SERVER = socket(AF_INET, SOCK_STREAM, 0);
    if (SERVER < 0)
		{  
			perror("server:socket"); 
			exit(1);
		}
    val = 1;
    if (setsockopt(SERVER, SOL_SOCKET, SO_REUSEADDR, &val, sizeof val) != 0)
	{ 	
		perror("server:setsockopt"); 
		exit(1);
	}
    if (bind(SERVER, (struct sockaddr *)&self, sizeof(self)) == -1) 
	{  
		perror("server:bind"); 
		close(SERVER);
        exit(1); 
    }
	
	listen(SERVER, 1000);
}

void* process(void* data)
{
	int client = *(int*)data;
	Process server(client);
	server.processRequest();
}

int main(int count, char* argv[])
{ 
	//get port number
	int port;
	if( count == 2) 
		port = atoi(argv[1]);
	else if (count == 1) 
		port = 32800;
	setupServer(port);
	
	struct sockaddr_in peer = {AF_INET}; 
	socklen_t peer_len = sizeof(peer);
	
	//set thread attribute
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	
	//create a log file
	string fileName = getTime("default")+".txt";
	logFile.open(fileName.c_str());
	logFile << "+++++++++++++++++++++++++++++++++++++++++ SERVER START ++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
	
	//create thread and process request
	while(true)
	{
		/* accept connection request from CLIENT Socket*/
		int* client;
		client = new int(accept(SERVER, (struct sockaddr *)&peer, &peer_len));       
		if (*client < 0)
		{
			perror("server:accept"); 
			close(SERVER);
			exit(1);
		}
		
		//create a thread for the connection
		pthread_t thread;
		int* t_check;
		t_check = new int(pthread_create(&thread, &attr, &process, (void*)client));
		while(*t_check != 0)
		{
			t_check = new int(pthread_create(&thread, &attr, &process, (void*)client));
		}
	}
	
	logFile << "+++++++++++++++++++++++++++++++++++++++++ SERVER CLOSE ++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
	logFile.close();
	close(SERVER);
    return(0);
}


//---------------------------------------------------------------------------------//
//------------------------------- CONNECTION FUNCTION -----------------------------//
//---------------------------------------------------------------------------------//

string Process::listenClient()
{
	int sizeBuff, check;
	char buf[9999];
	
	
	/* data transfer on connected CLIENT Socket */
	sizeBuff = read(client, buf, sizeof(buf) - 1);
	check = sizeBuff;
	if(check == -1) //if getting error while recieving from client
	{
		cout << "Error recieving from client. Server terminated 1" << endl;
		exit(1);
	}
	buf[sizeBuff] = '\0';			/* null-terminate string */
	string temp(buf);

	while(temp.find("\r\n\r\n") == string::npos)
	{
		memset( buf, 0, sizeof(buf) );
		sizeBuff += read(client, buf, sizeof(buf) - 1);
		if(check > sizeBuff) //if getting error while recieving from client
		{
		cout << "Error recieving from client. Server terminated 2" << endl;
		exit(1);
		}
		buf[sizeBuff] = '\0';			/* null-terminate string */
		temp += buf;
		check = sizeBuff;
	}
	
	if(temp.find("Content-length") != string::npos) //if the request is POST and it has content-length
	{
		string request = temp;
		
		//get Content-length
		int pos1 = temp.find("Content-length") + 16;
		temp = temp.substr(pos1);

		int pos2 = temp.find("\r\n");
		int length = atoi(temp.substr(0, pos2).c_str());
		
		//get the content in the request
		pos1 = temp.find("\r\n\r\n");
		temp = temp.substr(pos1+4);
		while(length > temp.size())
		{
			memset( buf, 0, sizeof(buf) );
			sizeBuff += read(client, buf, sizeof(buf) - 1);
			if(check > sizeBuff) //if getting error while recieving from client
			{
				cout << "Error recieving from client. Server terminated 2" << endl;
				exit(1);
			}
			buf[sizeBuff] = '\0';			/* null-terminate string */
			temp += buf;
			request += buf;
			check = sizeBuff;
		}
		
		printf("SERVER RECEIVED: %s\n", request.c_str());
		
		//return request from CLIENT
		return request;
	}
	else //if the request is GET or POST without Content-length
	{
		printf("SERVER RECEIVED: %s\n",buf);
		
		//return request from CLIENT
		return temp;
	}
}

void Process::replyClient(string& message)
{
	cout << "\nSERVER REPLY:\n" << message;
	int check = write(client, message.c_str(), message.length());
	while(check == -1)
	{
		check = write(client, message.c_str(), message.length());
	}
};

void Process::connectDB()
{
	if(!conn.connect("mtruon01", "localhost", "mtruon01", "p0356860"))
	{
		string url = "";
		string message = genErr(HTTP_500, url, "Cannot connect to the database. Server terminated.");
		replyClient(message);
		cout << "Cannot connect to the database\n";
		exit(1);
	}	
};

void Process::closeDB()
{
	conn.disconnect();
}

void Process::checkDB()
{
	int i(0);
	while(!conn.ping() && i != 2)
	{
		sleep(5);
		cout << "Try to reconnect to the database\n";
		conn.connect("mtruon01", "localhost", "mtruon01", "p0356860");
		i++;
	}
	if(!conn.ping())
	{
		string url = "";
		string message = genErr(HTTP_500, url, "Cannot connect to the database. Server terminated.");
		replyClient(message);
		cout << "Cannot connect to the database\n";
		exit(1);
	}
};


//---------------------------------------------------------------------------------//
//------------------------------- SUB FUNCTIONS -----------------------------------//
//---------------------------------------------------------------------------------//

Query Process::runQuery(string& request)
{
	checkDB();
	
	//execute query
	Query query = conn.query(request);
	return query;
}

bool Process::isNum(string& data)
{
	//check for user_id or screen_name
	bool isInt(true);
	for(size_t i = 0; i < data.length(); i++)
	{
		if(!isdigit(data[i]))
		{
				isInt = false;
		}
	}
	return isInt;
}

bool Process::authorized(string& request, string& url, string& screen_name)
{
	//get authorization
	int pos1(0), pos2(0), pos3(0);
	pos1 = request.find("Authorization:");
	string temp = request.substr(pos1+21);
	pos2 = temp.find("\r");
	string code = temp.substr(0, pos2);
	
	//decode and get screen_name and password
	temp = base64_decode_cpp(code);
	pos3 = temp.find(":");
	string password;
	screen_name = temp.substr(0, pos3);
	password = temp.substr(pos3+1);
	
	//get url
	url = getUrl(request);
	
	// authorize
	temp = "SELECT * FROM Users WHERE Usernames='";
	temp += screen_name;
	temp += "' AND Passwords='";
	temp += password;
	temp += "'";
	
	Row row;
	try 
	{
		Query test = runQuery(temp);
		UseQueryResult testRes = test.use();
		row = testRes.fetch_row();
	}
	catch(Exception& e) { cerr << "\nSomething's wrong with the authorized query: " << temp << endl; }
	
	if(!row.empty())
	{
		return true;
	}
	else
	{
		return false;
	}
}

string Process::getQueryValue(string& query, string var)
{
	int pos1, pos2;
	pos1 = query.find("xml?");
	string temp = query.substr(pos1+4);
	if(temp.find(var)!=string::npos)
	{
		pos1 = temp.find(var);
		temp = temp.substr(pos1);
		pos1 = temp.find("=");
		pos2 = temp.find("&");
		if(pos2 != string::npos) //if there is more query after
		{
			return temp.substr(pos1+1, pos2-(pos1+1));
		}
		else //there is no more query after
		{
			return temp.substr(pos1+1, temp.find(" ")-(pos1+1));
		}
	}
	return "";
}

int Process::getID(string& screen_name)
{
	int user_id;
	string temp;
	
	//make query
	temp = "SELECT IDs FROM Users WHERE Usernames='";
	temp += screen_name;
	temp += "'";

	Query query1 = runQuery(temp);

	if(UseQueryResult res = query1.use())
	{
		Row row = res.fetch_row();
		user_id = row["IDs"];	

		/*if(conn.errnum())
		{
			cerr << "Error fetching a row: " << conn.error() << std::endl;
		}*/
	}
	else
	{
			cerr << "Failed to query SQL: adasd" << endl;
	}
	return user_id;
}

string Process::getUsername(string& user_id)
{
	string temp;
	
	//make query
	temp = "SELECT Usernames FROM Users WHERE IDs='";
	temp += user_id;
	temp += "'";
		
	Query query1 = runQuery(temp);

	if(UseQueryResult res = query1.use())
	{
		Row row = res.fetch_row();
		string screen_name(row["Usernames"]);
		temp = screen_name;

		/*if(conn.errnum())
		{
			cerr << "Error fetching a row: " << conn.error() << std::endl;
		}*/
	}
	else
	{
			cerr << "Failed to query SQL: adasd" << endl;
	}
	return temp;
};

bool Process::isUserExist(string& data)
{
	string temp;

	if(!isNum(data))
	{
		temp = "SELECT * FROM Users WHERE Usernames='";
		temp += data;
		temp += "'";
	}
	else
	{
		temp = "SELECT * FROM Users WHERE IDs='";
		temp += data;
		temp += "'";
	}
	
	Row row; 
	try 
	{
		Query test = runQuery(temp);
		UseQueryResult testRes = test.use();
		row = testRes.fetch_row();
	}
	catch(Exception& e) { cerr << "\nSomething's wrong with the isUserExist query: " << temp << endl; }
	
	if(!row.empty())
	{
		return true;
	}
	return false;
}

bool Process::isFollowed(string& screen_name, string& following)
{
	string temp = "SELECT * FROM Followers WHERE Users='";
	temp += screen_name;
	temp += "' AND Followings='";
	temp += following;
	temp += "'";
	
	Row row;
	try
	{
		Query test = conn.query(temp);
		UseQueryResult testRes = test.use();
		row = testRes.fetch_row();
	}
	catch(Exception& e) { cerr << "\nSomething's wrong with the isFollowed query\n"; }
	
	if(!row.empty())
	{
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------------//
//-------------------------- PROCESS REQUEST FUNCTIONS -----------------------------//
//----------------------------------------------------------------------------------//

void Process::login(string& request, string& message)
{
	string screen_name, url;
	if(!authorized(request, url, screen_name)) //if authorization fails
	{
		message = genErr(HTTP_404, url, "Unauthorized: Username or password do not match.");
	}
	else
	{
		message = HTTP_200;
	}

}

void Process::makeAcc(string& request, string& message)
{
	string temp, password, screen_name;
	
	// get url, screen_name and password from request
	int pos1(0), pos2(0), pos3(0), pos4(0);
	pos1 = request.find("screen_name=");
	pos2 = request.find("&password=");
	screen_name = request.substr(pos1+12, pos2-(pos1+12));
	temp = request.substr(pos2+10);
	pos3 = temp.find("\r");
	if(pos3 == string::npos)
	{
		password = temp.substr(0);
	}
	else
	{
		password = temp.substr(0, pos3);
	}
		
	//get url
	string url = getUrl(request);
		
	//check screen_name if it is a number
	
	if(isNum(screen_name)) //screen_name is a number
	{
		message = genErr(HTTP_400, url, "Username cannot be a number or empty");
	}
	else if(isUserExist(screen_name)) //screen_name already taken
	{
		message = genErr(HTTP_400, url, "Username is already taken");
	}
	else if(password.empty()) //if password is not entered
	{
		message = genErr(HTTP_400, url, "Please fill in your password.");
	}
	else // screen_name is valid => add user and reply
	{
		// insert new screen_name and password
		temp = "INSERT INTO Users (Usernames, Passwords) VALUES (";
		temp += "'";
		temp += screen_name;
		temp += "', '";
		temp += password;
		temp += "')";

		Query query = runQuery(temp);
		if(!query.exec())
		{
			cerr << "\nFailed adding a user in makeAcc function. Query: "<< temp << endl;
		}
		
		// get user_id of screen_name
		int user_id = getID(screen_name);		
		
		// reply message
		stringstream reply;
		reply << HTTP_200 << XML_HEADER;
		reply << "<user>\r\n";
		reply << "\t<id>" << user_id << "</id>\r\n";
		reply << "\t<screen_name>" << screen_name << "</screen_name>\r\n";
		reply << "</user>\r\n";
		message = reply.str();
	}	
}

void Process::updateTweet(string& request, string& message)
{
	string temp, url, screen_name;
	int pos1(0), pos2(0);
	
	if(!authorized(request, url, screen_name)) //if authorization fails
	{
		message = genErr(HTTP_404, url, "Unauthorized: Username or password do not match.");
	}
	else //if authorization successes
	{
		//get status
		string status;
		pos1 = request.find("status=");
		temp = request.substr(pos1+7);
		pos2 = temp.find("\n");
		status = temp.substr(0, pos2);
		
		//check status's length
		if(status.length() > 140)
		{
			message = genErr(HTTP_400, url, "Status cannot be longer than 140 characters.");
		}
		else
		{
			// insert new status
			temp = "INSERT INTO Tweets (Contents, Users) VALUES (";
			temp += "'";
			temp += status;
			temp += "', '";
			temp += screen_name;
			temp += "')";
			
			Query query = runQuery(temp);
			if(!query.exec())
			{
				cerr << "Failed updating status in updateTweet function. Query: " << temp << endl;
			}
			
			//get new tweet info
			temp = "SELECT * FROM Tweets WHERE Users='";
			temp += screen_name;
			temp += "' AND Contents='";
			temp += status;
			temp += "'"; 
			
			Row row;
			try
			{
			Query query1 = runQuery(temp);
			UseQueryResult res = query1.use();
			row = res.fetch_row();
			}
			catch(Exception& e) { cerr << "\nSomething's wrong with the get new tweet info query in updateTweet: " << temp << endl; }
			
			//reply message
			int user_id = getID(screen_name);
			stringstream reply;
			reply << HTTP_200 << XML_HEADER;
			reply << "<status>\r\n";
			reply << "<created_at>" << row["Time"] << "</created_at>\r\n";
			reply << "<id>" << row["TweetIDs"] << "</id>\r\n";
			reply << "<text>" << urlEncode(status) << "</text>\r\n";
			reply << "<user>\r\n";
			reply << "<id>" << user_id << "</id>\r\n";
			reply << "<screen_name>" << screen_name << "</screen_name>\r\n";
			reply << "</user>\r\n";
			reply << "</status>\r\n\r\n";
			message = reply.str();			
		}		
	}
}

void Process::follow(string& request, string& message)
{
	string temp, url, screen_name;
		
	if(!authorized(request, url, screen_name)) //if authorization fails
	{
		message = genErr(HTTP_404, url, "Unauthorized: Username or password do not match.");
	}
	else //if authorization successes
	{
		//get and check user_id or screen_name
		int pos1 = url.find("create");
		int pos2 = url.find(".xml");
		string data = url.substr(pos1+7, pos2-(pos1+7));
				
		if(!isUserExist(data)) //if the followee does not exist
		{
			message = genErr(HTTP_400, url, "The user you want to follow does not exist.");
		}
		else
		{
			string following;
			int following_id;
			bool isInt = isNum(data);
			
			//get the other screen_name or user-id
			if(isInt)
			{
				following_id = atoi(data.c_str());
				following = getUsername(data);
			}
			else
			{
				following_id = getID(data);
				following = data;
			}
		
			if(following == screen_name) //if you follow yourself
			{
				message = genErr(HTTP_400, url, "You cannot follow yourself.");
			}
			else if(isFollowed(screen_name, following)) //if you already follow this user
			{
				message = genErr(HTTP_400, url, "You already follow this user.");
			}
			else //everything is fine
			{			
				//insert into database
				temp = "INSERT INTO Followers (Users, Followings) VALUES (";
				temp += "'";
				temp += screen_name;
				temp += "', '";
				temp += following;
				temp += "')";
				
				Query query = runQuery(temp);
				if(!query.exec())
				{
					cerr << "Failed inserting into Followers table in follow function. Query: " << temp << endl;
				}
				
				// reply message
				stringstream reply;
				reply << HTTP_200 << XML_HEADER;
				reply << "<user>\r\n";
				reply << "\t<id>" << following_id << "</id>\r\n";
				reply << "\t<screen_name>" << following << "</screen_name>\r\n";
				reply << "</user>\r\n\r\n";
				message = reply.str();
			}
		}
	}
};

void Process::followers(string& request, string& message)
{
	string temp, url, screen_name;
	int pos1(0), pos2(0);
	
	if(!authorized(request, url, screen_name)) //if authorization fails
	{
		message = genErr(HTTP_404, url, "Unauthorized: Username or password do not match.");
	}
	else //if authorization successes
	{
		//get cursor
		string cursor = getQueryValue(url, "cursor");
		
		//check cursor
		if(cursor == "" || atoi(cursor.c_str()) < -1) //cursor is less then -1
		{
			message = genErr(HTTP_400, url, "Invalid value for cursor.");
		}
		else //everything is fine so far
		{
			int pageLimit = 100;
			
			//make query to find number of rows of data
			temp = "SELECT COUNT(*) FROM Followers WHERE Followings='";
			temp += screen_name;
			temp += "'";
						
			int totalRow(0);
			try {
				Query query = conn.query(temp);
				UseQueryResult res = query.use();
				Row row = res.fetch_row();
				totalRow = row[0];
			}
			catch(Exception& e) { cerr << "\nSomething's wrong with the number of rows query in followers function: " << temp << endl; }
			
			//get total page and current page
			int totalPage = totalRow/pageLimit;
			if(totalRow % pageLimit != 0)
			{
				totalPage ++;
			}
			
			int getPage;
			if(atoi(cursor.c_str()) < 2)
			{
				getPage = 1;
			}
			else
			{
				getPage = atoi(cursor.c_str());
			}
					
			//check valid of page wanted to get
			if(totalPage == 0)
			{
				message = HTTP_200;
				message += XML_HEADER;
				message += "<users_list>\r\n";
				message += "<users type=\"array\">\r\n";
				message += "</users>\r\n";
				message += "<next_cursor>0</next_cursor>\r\n";
				message += "<previous_cursor>0</previous_cursor>\r\n";
				message += "</users_list>\r\n\r\n";
			}
			else if(getPage > totalPage)
			{
					message = genErr(HTTP_400, url, "Invalid value for cursor.");
			}
			else
			{			
				//get all followers
				temp = "SELECT * FROM Followers WHERE Followings='";
				temp += screen_name;
				temp += "'";
				
				StoreQueryResult data;
				try
				{
					Query query = runQuery(temp);
					data = query.store();
				}
				catch(Exception& e) { cerr << "Something's wrong with the get followers query in the followers function: " << temp << endl; }
							
				//get position of the wanted first and last row in the data
				int firstRow = ((getPage-1) * pageLimit);
				int lastRow = getPage * pageLimit;
				if(lastRow >= totalRow)
				{
					lastRow = totalRow;
				}
				
				//get previous and next cursor
				int previous = getPage - 1;
				if(previous < 0)
				{
					previous = 0;
				}
				int next = getPage + 1;
				if(next > totalPage)
				{
					next = 0;
				}
				
				//reply message
				stringstream reply;
				reply << HTTP_200 << XML_HEADER;
				reply << "<users_list>\r\n" << "<users type=\"array\">\r\n";
								
				for(int i = firstRow; i < lastRow; i++)
				{
					stringstream user;
					user << data[i]["Users"];
					string users = user.str();
					reply << "<user>\r\n";
					reply << "<id>" << getID(users) << "</id>\r\n";
					reply << "<screen_name>" << data[i]["Users"] << "</screen_name>\r\n";
					reply << "</user>\r\n";
				}
				
				reply << "</users>\r\n";
				reply << "<next_cursor>" << next << "</next_cursor>\r\n";
				reply << "<previous_cursor>" << previous << "</previous_cursor>\r\n";
				reply << "</users_list>\r\n\r\n";
				
				message = reply.str();				
			}
		}			
	}
}

void Process::recentTweets(string& request, string& message)
{
	string temp, url, screen_name;
		
	if(!authorized(request, url, screen_name)) //if authorization fails
	{
		message = genErr(HTTP_404, url, "Unauthorized: Username or password do not match.");
	}
	else //if authorization successes
	{
		//get and check screen_name or user_id
		string username;
		int	user_id;
		int pos1 = url.find("timeline");
		int pos2 = url.find(".xml");
		string data = url.substr(pos1+9, pos2-(pos1+9));
		
		if(!isUserExist(data))
		{
			message = genErr(HTTP_400, url, "This user does not exist.");
		}
		else
		{
			//get query value
			string count, since_id, max_id;
			count = getQueryValue(url, "count");
			since_id = getQueryValue(url, "since_id");
			max_id = getQueryValue(url, "max_id");
			
			//check for bad query value
			if( count != "" && !isNum(count) ) //count is not a number or is negative
			{
				message = genErr(HTTP_400, url, "Unvalid value for count.");
			}
			else if( since_id != "" && !isdigit(since_id[0]) ) //since_id is not a number or is negative
			{
				message = genErr(HTTP_400, url, "Unvalid value for since_id.");
			}
			else if( max_id != "" && !isdigit(max_id[0])) //max_id is not a number or is negative
			{
				message = genErr(HTTP_400, url, "Unvalid value for max_id.");
			}
			else //everything is fine so far
			{		
				if( count== "0" || atoi(max_id.c_str()) == 1) //if count = 0 or max_id < 2
				{
					stringstream reply;
					reply << HTTP_200 << XML_HEADER;
					reply << "<statuses></statuses>\r\n";
					message = reply.str();
				}
				else
				{				
					//get user_id or screen_name
					if(isNum(data))
					{
						user_id = atoi(data.c_str());
						username = getUsername(data);
					}
					else
					{
						username = data;
						user_id = getID(data);
					}

					//make query
					temp = "SELECT Contents, TweetIDs, UNIX_TIMESTAMP(Time) as datetime FROM Tweets WHERE Users='";
					temp += username;
					temp += "' ";
					
					//get condition for query
					if(since_id != "")
					{
						temp += "AND TweetIDs > '";
						temp += since_id;
						temp += "' ";
					}
					if(max_id != "")
					{
						temp += "AND TweetIDs < '";
						temp += max_id;
						temp += "' ";
					}
					temp += "ORDER BY Time DESC ";
					if(count != "" && atoi(count.c_str()) <= 30)
					{
						temp += "LIMIT ";
						temp += count;
					}
					else
					{
						temp += "LIMIT 30";
					}
			
					//run query to database
					UseQueryResult res;
					try
					{
						Query query = runQuery(temp);
						res = query.use();
					}
					catch(Exception& e) { cerr << "\nSomething's wrong with the get recent tweets query in recentTweets function: " << temp << endl; }
					
					//reply message
					stringstream reply;
					reply << HTTP_200 << XML_HEADER;
					reply << "<statuses>\r\n";
					while(Row row = res.fetch_row())
					{
						string time(row["datetime"]);
						reply << "<status>\r\n";
						reply << "<created_at>" << getTime(time) << "</created_at>\r\n";
						reply << "<id>" << row["TweetIDs"] << "</id>\r\n";
						reply << "<text>" << row["Contents"] << "</text>\r\n";
						reply << "<user>\r\n";
						reply << "<id>" << user_id << "</id>\r\n";
						reply << "<screen_name>" << username << "</screen_name>\r\n";
						reply << "</user>\r\n" << "</status>\r\n";
					}
					reply << "</statuses>\r\n\r\n";					
					message = reply.str();					
				}	
			}
		}
	}
}

void Process::processRequest()
{
	//write new thread id to the log file
	pthread_mutex_lock(&mutex_log);
	logFile << "New thread created: " << pthread_self() << endl;
	pthread_mutex_unlock(&mutex_log);
	
	//connect to database
	connectDB();
	
	//get request from client				
	string request = listenClient();
	
	//write request in the log file
	pthread_mutex_lock(&mutex_log);
	logFile << endl;
	logFile << "---------------------------REQUEST--------------------------" << endl;
	logFile << request << endl;
	pthread_mutex_unlock(&mutex_log);
	
	//sort and process request from client
	string message;
	if(request.find("POST /account/register.xml") != string::npos)
	{
		makeAcc(request, message);
	}
	else if(request.find("POST /statuses/update.xml") != string::npos)
	{
		updateTweet(request, message);
	}
	else if(request.find("POST /friendships/create/") != string::npos)
	{
		follow(request, message);
	}
	else if(request.find("POST /login.xml") != string::npos)
	{
		login(request, message);
	}
	else if(request.find("GET /statuses/followers.xml") != string::npos)
	{
		followers(request, message);
	}
	else if(request.find("GET /statuses/user_timeline/") != string::npos)
	{
		recentTweets(request, message);
	}
	else
	{
		cout << "Invalid request\n";
		string url = "";
		message = genErr(HTTP_400, url, "Invalid Request");
	}
	
	//write reply message in the log file
	pthread_mutex_lock(&mutex_log);
	logFile << "---------------------------REPLY--------------------------" << endl;
	logFile << message << endl;
	pthread_mutex_unlock(&mutex_log);
	
	//reply and close connection with client and database
	replyClient(message);
		
	closeDB();
	close(client);
}
