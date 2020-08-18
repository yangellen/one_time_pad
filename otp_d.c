/*********************************************************************
* Author: Ellen Yang
* Program name: otp_d.c
* Description: Run in the background as a daemon and it's functions is
*	to store the encrypted data. 
* Date: 5/26/2020
*********************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>

#define MAX 70050
#define NUMCHILD 5

int numFork; 

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

/***************************************************************************
* Function: void createFiles(char msg[], char user[], char fileName[])
* Description: create file that store the ciphertext.
* Parameter: char msg[], the ciphertext
*			 char user[], the user of the ciphertext
*			 char fileName[], name of the file that is used to create the file
* Return: none
* Note: modified from the function I wrote in program 2.
******************************************************************************/
void createFiles(char msg[], char user[], char fileName[])
{
	//adding time at the end of file name, so each file has unique name
	char formateTime[256];	
	memset(formateTime, '\0', sizeof(formateTime));		

	//get current time and store in time structure
	struct tm *timeStructure;  
	time_t currentTime = time(0); // get currentTime
	timeStructure = gmtime (&currentTime); 

	//formate time per specification
	strftime(formateTime, sizeof(formateTime), "%j%H%M%S", timeStructure);

	//clear the buffer
	memset(fileName,'\0',sizeof(fileName));

	//concat the desire filename
	strcat(fileName, user);
	strcat(fileName, "ciphertext");
	strcat(fileName, formateTime);

	//create file for ciphertext
	FILE* file = fopen (fileName,"w");

	//store buffer and add newline in file
	fprintf(file, "%s\n", msg);

	//close file
	fclose(file);	
}

/*********************************************************************
* Function: void printFilePath(char fileName[])
* Description: This fucntion print out the path of file 
* Parameters: char fileName
* Return: none
*********************************************************************/
void printFilePath(char fileName[])
{
	char cwd[1024];
	memset(cwd, '\0', sizeof(cwd));	

	//get current working direcotry
	getcwd(cwd, sizeof(cwd));

	//add / and file name
	strcat(cwd, "/");
	strcat(cwd, fileName);

	printf("%s\n", cwd);
	fflush(stdout);
}

/*********************************************************************
* Function: void findOldestFile(char oldestFile[], char user[])
* Descriptions: 
* Parameter: char oldestFile[], use to store the name of oldest file of user
*			 char user[], the user that file belong to
* Return: none
* Note: modified from 2.4 lecture notes
*********************************************************************/
void findOldestFile(char oldestFile[], char user[])
{	
	memset(oldestFile, '\0', sizeof(oldestFile));

	int oldestFileTime = -5; //modifed timestamp of oldest subdir examined
	int count = 0;
	char targetDirPrefix[32];; //prefix we are looking for
	memset(targetDirPrefix, '\0', sizeof(targetDirPrefix));
	strcat(targetDirPrefix, user);

	DIR* dirToCheck; //holds the dir we are looking for
	struct dirent *fileInDir; //holds the current subdir of the starting starting dir
	struct stat dirAttributes; // Holds information we've gained about subdir
	
	dirToCheck = opendir("."); // Open up the directory this program was run in

	if (dirToCheck > 0) // Make sure the current directory could be opened
	{ 
		while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in the directory
		{
			if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has the prefix
			{
				count++;
				
		 		stat(fileInDir->d_name, &dirAttributes); // Get attributes of the 

		 		//set the time of first found file as oldestFileTime
		 		if(count == 1)
		 		{
		 			oldestFileTime = (int)dirAttributes.st_mtime; 
		 		}
				
				if ((int)dirAttributes.st_mtime <= oldestFileTime) // If this time is smaller or equal
				{
					oldestFileTime = (int)dirAttributes.st_mtime;
					memset(oldestFile, '\0', sizeof(oldestFile));
					strcpy(oldestFile, fileInDir->d_name);
				}
			}
		} 
		/*
		if (count == 0)
		{
			fprintf(stderr, "NO file for %s\n", user );
		}*/
	}   		

	closedir(dirToCheck); // Close the directory we 
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
* Function: void CatchSIGCHLD(int signo)
* Description: Catch SIGCHILD when child terminate, then decrease 
*	number of fork and wait for child. 
* Parameter: int signo
* Return:none
*********************************************************************/
void CatchSIGCHLD(int signo)
{
	numFork--;
	wait(NULL);
}

/*********************************************************************
* Main Function of the program
*********************************************************************/
int main(int argc, char *argv[])
{
	// signal handler for SIGCHILD
    struct sigaction SIGCHLD_action = {0};    
    SIGCHLD_action.sa_handler = CatchSIGCHLD;   
    sigfillset(&SIGCHLD_action.sa_mask);        
    SIGCHLD_action.sa_flags = SA_RESTART;       
    sigaction(SIGCHLD, &SIGCHLD_action, NULL);  

	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char completeMessage[MAX];
	char readBuffer[1000];
	char buffer[MAX];
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s listening_port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process
	
	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	
	//fork pid
	pid_t spawnpid = -5; 
	int exitStatus = 0; 
	char method[10];
	char user[20];
	char msg[MAX];
	char* token = NULL; 

	while(1)
	{
		if (numFork < 5)
		{
			// Accept a connection, blocking if one is not available until one connects
			sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
			establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
			if (establishedConnectionFD < 0) error("ERROR on accept");

			//start fork
			spawnpid = fork();

			switch (spawnpid)
			{
				//not able to fork()
				case -1: 
				{
					fprintf(stderr, "Error on fork\n");
					exit(1);
					break;
				}

				//In child process, where we do the work
				case 0:
				{
					//first thing the child of otp_d must do after a connection
					sleep(2);

					numFork++; 
		
					memset(completeMessage,'\0', sizeof(completeMessage)); //clear the buffer
					
					while (strstr(completeMessage, "@@") == NULL) //as long as we haven't found the terminal
					{
						memset(readBuffer,'\0',sizeof(readBuffer));
						charsRead = recv(establishedConnectionFD, readBuffer, sizeof(readBuffer) - 1, 0); // Read the client's message from the socket
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

					//printf("SERVER: I received this from the client: \"%s\"\n", completeMessage); //

					//memset vairalbes
					memset(method, '\0', sizeof(method));
					memset(user,'\0', sizeof (user));
					memset(msg, '\0', sizeof(msg));


					//parsing buffer with token, formate of buffer is method+user+msg
					//break buffer in chucks wherever there is +
					token = strtok(completeMessage, "+");
					strcpy(method, token);
					token = strtok(NULL, "+");
					strcpy(user, token);
					token = strtok(NULL, "\0");
					strcpy(msg, token);
			
					if (strcmp(method,"post") == 0)
					{
						//write encrypted message to file
						char fileName[256];
						memset(fileName,'\0',sizeof(fileName));
						createFiles(msg, user,fileName);		

						//print the path to this file to stdout
						printFilePath(fileName);

						// Send a Success message back to the client
						charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
						if (charsRead < 0) error("ERROR writing to socket");

					}
					else
					{		
						//find the oldest file of user 
						char oldestFile[256];
						findOldestFile(oldestFile, user);		

						//prepare buffer to send
						memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array

						//only get the content and send if oldestfile for user has found
						if (oldestFile[0] != '\0')
						{
							//get the content of oldest file
							getFileContent(oldestFile, buffer);

							//add @@
							strcat(buffer,"@@");

							//send content of file to otp_d
							size_t length = strlen(buffer);
							const char *p = buffer; 

							while (length > 0)
							{
								charsRead = send(establishedConnectionFD, p, length, 0);

								if (charsRead < 0) error("ERROR writing to socket");

								p += charsRead; 
								length -= charsRead;
							}

							//delete the ciphertext file
							if (remove(oldestFile) != 0)
							{
								fprintf(stderr, "Unable to delete the file\n");
							}
						}

						else
						{
							// Send No message back to the client
							charsRead = send(establishedConnectionFD, "No@@", 4, 0); // Send no file back
							if (charsRead < 0) error("ERROR writing to socket");
						}

						

					}			
					close(establishedConnectionFD); // Close the existing socket which is connected to the client
					exit(0);
					break;
				}

				//In parent process now
				default:
				{
					break;
				}
				close(establishedConnectionFD); // Close the existing socket which is connected to the client

			}
		}
		
	}

	close(listenSocketFD); // Close the listening socket
	return 0; 
}
