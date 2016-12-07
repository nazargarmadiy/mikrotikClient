#ifndef CLIENT_H
#define CLIENT_H

#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include "mikrotik_api.h"

struct Interface
{
    char *name;
    struct Interface *pNext;
};

struct Stat
{
    int r_packets_per_sec;
    int r_drops_per_sec;
    int r_errors_per_sec;
    int r_bits_per_sec;
    int t_packets_per_sec;
    int t_drops_per_sec;
    int t_errors_per_sec;
    int t_bits_per_sec;
    struct Stat *pNext;
};

void get_interfaces(int fdSock, struct Interface **pInterface);
void show_interface(struct Interface *pInterface);
struct Interface *choose_intreface(struct Interface *pInterface);
void free_interfase_list(struct Interface *pInterfase);
void free_stat_list(struct Stat *pStat);
void get_stat(int fdSock, struct Interface *pInterface, int count, int frequency, struct Stat **pStat);
void show_stat(int index, struct Stat *pStat, struct Interface *pInterface);
int max_stat_val(int index, struct Stat *pStat);

#endif // CLIENT_H
