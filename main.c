/*#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include "mikrotik_api.h"*/

#include "client.h"

/********************************************************************
 * Print program usage
 ********************************************************************/
void usage()
{
    printf("Usage: mikrotik-tty [-u<username>] [-p<password>] [-P<portNum>] [--quiet] <ip_address>\n\n");
    printf("-u<username> the username to login as.  Default is admin\n");
    printf("-p<password> the password to use for login.  Default is empty string\n");
    printf("-P<port> TCP port to use for API connection.  Default is 8728.\n");
    printf("--quiet Suppress all non-API output.  Default is interactive mode.\n");
    printf("<ip_address> IP address to connect to.  REQUIRED\n\n");
}




/********************************************************************
 * main
 ********************************************************************/
int main(int argc, char *argv[])
{
    // declare variables
    int fdSock;
    char *szIPaddr = "172.20.17.200";
    char *szPort = "8728"; // default port string
    int iPort = 8728;             // default port int
    char *szUsername = "admin";  // default username
    char *szPassword = "";       // default password
    int iInteractiveMode = 1;    // interactive mode...if set to 0, will supress all non-API output

    int iLoginResult;
    int iIndex;
    char cWordInput[256];        // limit user word input to 256 chars
    char *szNewline;             // used for word input from the user
    struct Sentence stSentence;
    struct Block stBlock;

    // check number of args.  if not correct, call usage and exit
    /*if (argc < 2)
    {
        usage();
        exit(0);
    }

    // parse command line parameters
    else
    {
        for (iIndex=0; iIndex<argc; iIndex++)
        {
            if (strstr(argv[iIndex], "-u"))
            {
                szUsername = &argv[iIndex][2];
            }
            else if (strstr(argv[iIndex], "-p"))
            {
                szPassword = &argv[iIndex][2];
            }
            else if (strstr(argv[iIndex], "-P"))
            {
                szPort = &argv[iIndex][2];
            }
            else if (strstr(argv[iIndex], "--quiet"))
            {
                iInteractiveMode = 0;
            }
        }

        // assume the last parameter is the IP address
        szIPaddr = argv[argc-1];

        // convert port string to an int
        iPort = atoi(szPort);
    }*/

    iInteractiveMode ? printf("Connecting to API: %s:%d\n", szIPaddr, iPort) : 0;
    fdSock = apiConnect(szIPaddr, iPort);

    iLoginResult = login(fdSock, szUsername, szPassword);

    if (!iLoginResult)
    {
        apiDisconnect(fdSock);
        iInteractiveMode ? printf("Invalid username or password.\n") : 0;
        exit(1);
    }

    // initialize first sentence
    initializeSentence(&stSentence);

    ///////
    struct Interface *pIinterfase = NULL;
    get_interfaces(fdSock, &pIinterfase);
    show_interface(pIinterfase);
    struct Interface *pCurrenrInt = choose_intreface(pIinterfase);
    struct Stat *pStat = NULL;
    get_stat(fdSock, pIinterfase, 10, 1, &pStat);

    show_stat(0, pStat, pIinterfase);

    free_interfase_list(pIinterfase);
    free_stat_list(pStat);

    apiDisconnect(fdSock);
    printf("\n");
    exit(0);
    //////*/

    // main loop
    while (1)
    {
        // get input from stdin
        iInteractiveMode ? fputs("<<< ", stdout): 0;
        iInteractiveMode ? fflush(stdout): 0;

        if (fgets(cWordInput, sizeof cWordInput, stdin) != NULL)
        {
            szNewline = strchr(cWordInput, '\n');

            if (szNewline != NULL)
            {
                *szNewline = '\0';
            }
        }

        // check to see if we want to quit
        if (strcmp(cWordInput, "quit") == 0)
        {
            break;
        }

        // check for end of sentence (\n)
        else if (strcmp(cWordInput, "") == 0)
        {
            // write sentence to the API
            if (stSentence.iLength > 0)
            {
                writeSentence(fdSock, &stSentence);

                // receive and print response block from the API
                stBlock = readBlock(fdSock);
                printBlock(&stBlock);

                // clear the sentence
                clearSentence(&stSentence);
            }
        }

        // if nothing else, simply add the word to the sentence
        else
        {
            addWordToSentence(&stSentence, cWordInput);
        }
    }

    apiDisconnect(fdSock);
    exit(0);
}
