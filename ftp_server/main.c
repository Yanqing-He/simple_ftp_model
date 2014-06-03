/*
 * main.c
 *
 *  Created on: 2013-2-24
 *      Author: hyq
 */

#include <stdio.h>
#include <string.h>

#include "my_server.h"

int main(int argc, char* argv[])
{
	int isOK;
	isOK = serverFunction();
	return isOK;

}
