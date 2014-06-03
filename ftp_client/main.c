/*
 * main.c
 *
 *  Created on: 2013-2-24
 *      Author: hyq
 */


#include <stdio.h>
#include <string.h>

#include "my_client.h"

int main(int argc, char* argv[])
{
	if(argc == 2)
	{
		int isOK;
		char* ipAddr = argv[1];
		isOK = clientFunction(ipAddr);
		return isOK;
	}
	else
	{
		printf("please input valid arguments\n");
		return -1;
	}

}
