/*********************************************************************
* Author: Ellen Yang
* Program name: keygen.c
* Description: This program creates a key file of specified length. The characters in 
*	in the file generated will be any of the 26 capital letters and the space ( ) character.
*	The last character keygen outputs shuold be a newline. 
* Date: 5/24/2020
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAXCHAR 27


/*********************************************************************
* Function: void createKeyFile(int numLength)
* Description: The function will create a key file of specified length with
*	newline at the end. 
* Parameters: int numLength, number of characters the program should create
* Return: none
*********************************************************************/
void createKeyFile(int numLength)
{
	//charArray to store the 27 characters use to genearte keygen
	char charArray[MAXCHAR] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	int i;

	for (i = 0; i < numLength; i++) 
	{
		printf("%c", charArray[rand()%MAXCHAR]);
	} 	

	//add newline at the end of keygen
	printf("\n");
}


int main (int argc, char *argv[])
{
	//seed random time
	srand(time(NULL));

	//check command syntax
	//syntax for keygen is keygen keyLength
	if (argc != 2)
	{
		fprintf(stderr, "Invalid command syntax\n" );

		return 1; 
	}

	//convert to int
	int numLength; 
	numLength = atoi(argv[1]);
	
	//check to see if is valid integer
	if (numLength <= 0)
	{
		fprintf(stderr, "Invalid string length\n");

		return 1; 
	}

	//generated random key file
	createKeyFile(numLength);
}
