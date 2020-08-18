#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/ioctl.h>

#define MAX 70050

/*********************************************************************
* Author: Ellen Yang
* Program name: otp.c
* Description: This program connects to otp_d and asks it to store or
*	retrieve messages for a given user.
* Date: 5/26/2020
*********************************************************************/

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

/*********************************************************************
* Function: long int getFileLength(char* fileName)
* Description: This functions find and return the size of a file.
* Parameters: char* fileName, name of the file we want to know the size
* Return: The size of file 
*********************************************************************/
long int getFileLength(char* fileName)
{
	//open the file in read mode
	FILE* file = fopen(fileName, "r");

	//check to if file exit
	if (file == NULL)
	{
		fprintf(stderr, "File not found!\n"); 
	}

	//move file pointer at the end of the files
	fseek(file, 0L, SEEK_END);

	//calculat the size of file
	long int size = ftell(file);

	//close the file
	fclose(file);

	return size; 
}

/*********************************************************************
* Function: void checkBadCharacters(char buffer[])
* Description: This functions to each character of the string. If the 
*	string contain any chacter that is not upper case letter and it is
* 	not a space, there is bad character in the string. 
* Parameters: char buffer[], the string we can to check for bad 
*	characters.
* Return: none
*********************************************************************/
void checkBadCharacters(char buffer[], char file[])
{
	int i;
	for (i = 0; i < strlen(buffer); i++)
	{
		if (!isupper(buffer[i]) && buffer[i] != ' ')
		{
			fprintf(stderr, "Error, bad character in %s.\n", file);
			exit(1);
		}
	}

}

/*********************************************************************
* Function: void getFileContent(char* fileName, char buffer[])
* Description: This functions open a file and store the content in string.
* Parameters: char* fileName, name of the file we want to open
*			char buffer[], where we store the content of the file
* Return: none
*********************************************************************/
void getFileContent(char* fileName, char buffer[])
{
	int character;
	int i; 

	//open the file in read mode
	FILE* file = fopen(fileName, "r");

	//check to if file exit
	if (file == NULL)
	{
		fprintf(stderr, "File not found!\n"); 
	}

	//get first character in the file
	character = fgetc(file);
	i = 0; 

	//check each character in the file
	while (character != EOF )
	{
		buffer[i] = character;
		character = fgetc(file);
		i++; 
	}

	// Remove the trailing \n 
	buffer[strcspn(buffer, "\n")] = '\0'; 

	//close the file
	fclose(file);
}

/*********************************************************************
* Function: int getNumber(char character)
* Description: This functions return a number that represent the character
*	according to the One-Time Pads, except we use modulo 27 operation.
*	The 27 characters are 26 capital letters, and the space character().
* Parameters: char character, the character that we want to convert
*	to number. 
* Return: int number that represent the character.
*********************************************************************/
int getNumber(char character)
{
	int number; 

	if (character == ' ')
	{
		number = 26;
	}
	else
	{
		number = character - 'A'; //ascii number for A is 65
	}


	return number;
}

/*********************************************************************
* Function: char getCharacter(int number)
* Description: This functions return a character that represent the number
*	according to the One-Time Pads, except we use modulo 27 operation.
*	The 27 characters are 26 capital letters, and the space character().
* Parameters: char character, the character that we want to convert
*	to number. 
* Parameters: int number, the number that we want to convert to character.
* Return: char character that represent the number. 
*********************************************************************/
char getCharacter(int number)
{
	char character; 

	if ( number == 26)
	{
		character = ' ';
	}
	else
	{
		character = number + 'A'; 
	}

	return character;
}

/*********************************************************************
* Function: void encryptText(char plainBuffer[], char keyBuffer[], char encryptMsg[])
* Description: This function encrypt the plaintext.
* Parameters: char plainBuffer[], plaintext we want to encrypt.
*	char keyBuffer[], random sequence of characters that will be used to 
*		convert plaintext to ciphertext and back again.
*	char encryptMsg[], plaintext after encrytion. 
* Return: none
*********************************************************************/
void encryptText(char plainBuffer[], char keyBuffer[], char encryptMsg[])
{
	int i;
	int numP;
	int numK; 
	int numE;

	for (i = 0; i < strlen(plainBuffer); i++)
	{
		//get number from plain
		numP = getNumber(plainBuffer[i]);
		//get number from key
		numK = getNumber(keyBuffer[i]);
		//add plain and key then mod 27
		numE = (numP + numK) % 27; 
		//get char and put in encrytMsg
		encryptMsg[i] = getCharacter(numE); 

	}
}

/*********************************************************************
* Function: void decryptText(char buffer[], char keyFileName[])
* Description: This funtion decrypt the ciphertext. 
* Parameters: char buffer[], string that store the ciphertext
*	char keyFileName[]), file name that has key.
* Return: none
*********************************************************************/
void decryptText(char buffer[], char keyFileName[])
{
	int i;
	int numC;
	int numK; 
	int numD;
	
	//get the content of keyFile
	char key[MAX];
	memset(key,'\0', sizeof(key));
	getFileContent(keyFileName, key);

	for(i = 0; i < strlen(buffer); i++)
	{
		//get number from ciphertext
		numC = getNumber(buffer[i]);

		//get number from key
		numK = getNumber(key[i]);

		//get decrypt msg number
		numD = (numC - numK); 

		if (numD < 0)
		{
			numD = numD + 27;
		}

		//get char and print
		printf("%c", getCharacter(numD));
		fflush(stdout);
	}

	printf("\n");
	fflush(stdout);
	
}


/*********************************************************************
* Main Function of the program
*********************************************************************/
int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char plainBuffer[MAX];
	char keyBuffer[MAX];
	char encryptMsg[MAX];
	char buffer[MAX];
	char completeMessage[MAX];
	char readBuffer[1000];

	//post mode
	if (strcmp(argv[1],"post") == 0)
	{
		// Check usage & args
		if (argc != 6) 
		{ 
			fprintf(stderr,"USAGE: %s post user plaintext key port\n", argv[0]); 
			exit(0); 
		} 
		
		//check file length for plaintext and key
		//using long lnt in case the file is very large
		long int plainLength = getFileLength(argv[3]);
		long int keyLength = getFileLength(argv[4]);

		if (keyLength < plainLength)
		{
			fprintf(stderr, "Error: key '%s' is too short\n", argv[4]);
			exit(1);
		}

		//clear out the buffer array
		memset(plainBuffer, '\0', sizeof(plainBuffer)); 
		memset(keyBuffer, '\0', sizeof(keyBuffer)); 

		// read text and store in string
		getFileContent(argv[3], plainBuffer);
		getFileContent(argv[4], keyBuffer);

		//check any bad characters (content of file, filename)
		checkBadCharacters(plainBuffer, argv[3]);
		checkBadCharacters(keyBuffer, argv[4]);

		//clear out the buffer array
		memset(encryptMsg, '\0', sizeof(encryptMsg)); 

		//encrypt plaintext using keys
		encryptText(plainBuffer,keyBuffer,encryptMsg);

		//buffer with formate: post+user+encrytTest
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array

		//otp_d can use strtok to parse string, method+user+encryMsg+@@
		strcat(buffer, "post+");
		strcat(buffer, argv[2]);
		strcat(buffer,"+");
		strcat(buffer, encryptMsg);
		strcat(buffer,"@@"); //terminal

		// Set up the server address struct
		memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
		portNumber = atoi(argv[5]); // Get the port number, convert to an integer from a string
		serverAddress.sin_family = AF_INET; // Create a network-capable socket
		serverAddress.sin_port = htons(portNumber); // Store the port number
		serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
		if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
		memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

		// Set up the socket
		socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
		if (socketFD < 0) error("CLIENT: ERROR opening socket");

		//make socket reusable
		int yes = 1;
		setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)); 

		// Connect socket to address
		if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) 
		{
			error("");
			fprintf(stderr, "otp: ERROR connecting to otp_d server, attempted port %s\n", portNumber);
			exit(2);
		}

		//send post+user+encrypMsg to otp_d
		size_t length = strlen(buffer);
		const char *p = buffer; 

		while (length > 0)
		{
			charsWritten = send(socketFD, p, length, 0);

			if (charsWritten < 0) error("CLIENT: ERROR writing to socket");

			p += charsWritten; 
			length -= charsWritten;
		}

		// Get return message from server
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
		if (charsRead < 0) error("CLIENT: ERROR reading from socket");
		//printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
		
	}
	//get mode
	else if (strcmp(argv[1],"get") == 0)
	{
		// Check usage & args
		if (argc != 5) 
		{ 
			fprintf(stderr,"USAGE: %s get user key port\n", argv[0]); 
			fprintf(stderr,"USAGE: %s get user key port > myciphertext\n", argv[0]);
			fprintf(stderr,"USAGE: %s get user key port > myciphertext &\n", argv[0]);
			exit(0); 
		} 

		// Set up the server address struct
		memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
		portNumber = atoi(argv[4]); // Get the port number, convert to an integer from a string
		serverAddress.sin_family = AF_INET; // Create a network-capable socket
		serverAddress.sin_port = htons(portNumber); // Store the port number
		serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
		if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
		memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

		// Set up the socket
		socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
		if (socketFD < 0) error("CLIENT: ERROR opening socket");


		// Connect socket to address
		if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) 
		{
			error("");
			fprintf(stderr, "otp: ERROR connecting to otp_d server, attempted port %s\n", portNumber);
			exit(2);
		}

		//prepare buffer to send
		//buffer with formate: get+user+none
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array

		//otp_d can use strtok to parse string
		strcat(buffer, "get+");
		strcat(buffer, argv[2]);
		strcat(buffer,"+none@@");
	
		//send buffer message to otp_d
		size_t length = strlen(buffer);
		const char *p = buffer; 

		while (length > 0)
		{
			charsWritten = send(socketFD, p, length, 0);

			if (charsWritten < 0) error("CLIENT: ERROR writing to socket");

			p += charsWritten; 
			length -= charsWritten;
		}

		// Get return message from server, message will be either NO if no message for user or encrypted msg
		memset(completeMessage,'\0', sizeof(completeMessage)); //clear the buffer
					
		while (strstr(completeMessage, "@@") == NULL) //as long as we haven't found the terminal
		{
			memset(readBuffer,'\0',sizeof(readBuffer));
			charsRead = recv(socketFD, readBuffer, sizeof(readBuffer) - 1, 0); // Read the client's message from the socket
			strcat(completeMessage, readBuffer); //add read chuck to msg

			if (charsRead == -1)
			{
				fprintf(stderr, "Error Reading\n");
				break;
			}
			if (charsRead == 0)
			{
				break;
			}
		}

		
		int terminalLocation = strstr(completeMessage, "@@") - completeMessage; // where is the terminal
		completeMessage[terminalLocation] = '\0'; //End the string early to wipe out the termianl

		if(strcmp(completeMessage, "No") == 0)
		{
			fprintf(stderr, "%s has no message.\n", argv[2]);
		}
		else
		{
			//decrpyt buffer and print to stdout 
			decryptText(completeMessage, argv[3]);
		}

		
	}
	//wrong usage
	else
	{
		fprintf(stderr,"USAGE: either post or get need to be after %s \n", argv[0]); exit(0); 	
	}

	close(socketFD); // Close the socket
	return 0;
}
