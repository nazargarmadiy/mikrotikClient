#include "client.h"

void get_interfaces(int fdSock, struct Interface **pInterface)
{
    int count = 0;
    struct Sentence sentence;
    struct Block block;
    initializeSentence(&sentence);
    addWordToSentence(&sentence, "/interface/print");//TODO:make define or config file!
    addWordToSentence(&sentence, "");
    writeSentence(fdSock, &sentence);
    block = readBlock(fdSock);
    count = block.iLength;
    int i, tmp_len;
    struct Interface *pTmp;
    for(i = 0; i < count -1; i++)
    {
        if(!*pInterface)
            pTmp = *pInterface = calloc(sizeof(struct Interface), 1);
        else
        {
            pTmp->pNext = calloc(sizeof(struct Interface), 1);
            pTmp = pTmp->pNext;
        }
        tmp_len = strlen(block.stSentence[i]->szSentence[2]) - 6;//len without "=name=" and ";"
        pTmp->name =(char*) malloc(sizeof(char) * tmp_len + 1);
        strncpy(pTmp->name, (block.stSentence[i]->szSentence[2]) + 6, tmp_len);//skip "=name=" and copy interfase name
        *(pTmp->name + tmp_len) = '\0';
    }
    clearSentence(&sentence);
    clearBlock(&block);
}

void show_interface(struct Interface *pInterface)
{
    printf("\nAviable interfaces:");
    int count = 1;
    while (pInterface)
    {
        printf("\n%d: %s;", count++, pInterface->name);
        pInterface = pInterface->pNext;
    }
}

struct Interface *choose_intreface(struct Interface *pInterface)
{
    printf("\nchoose interfase to get statistics: ");
    int num;
    scanf("%d", &num);
    while (--num && pInterface) {
        pInterface = pInterface->pNext;
    }
    return pInterface;
}

void get_stat(int fdSock, struct Interface *pInterface, int count, int frequency, struct Stat **pStat)
{
    struct Sentence sentence;
    struct Block block;
    initializeSentence(&sentence);
    addWordToSentence(&sentence, "/interface/monitor-traffic");//TODO:make define or config file!
    addWordToSentence(&sentence, "=once=");
    int len = strlen(pInterface->name) + 11;//lenght name + lenght "=interface=" + '\0'
    char interfaceName[len];
    memset(interfaceName, '\0', sizeof(interfaceName));
    strcpy(interfaceName, "=interface=");
    strcat(interfaceName, pInterface->name);
    addWordToSentence(&sentence, interfaceName);
    addWordToSentence(&sentence, "");
    struct Stat *pTmp;
    char tmpBuf[32];
    char *numPos;
    while (count--) {
        writeSentence(fdSock, &sentence);
        block = readBlock(fdSock);
        if(!*pStat)
            pTmp = *pStat = calloc(1, sizeof(struct Stat));
        else
        {
            pTmp->pNext = calloc(1, sizeof(struct Stat));            
            pTmp = pTmp->pNext;
        }
        memset(tmpBuf, '\0', sizeof(tmpBuf));
        numPos = strrchr(block.stSentence[0]->szSentence[2], '=');
        pTmp->r_packets_per_sec = atoi(++numPos);
        memset(tmpBuf, '\0', sizeof(tmpBuf));
        numPos = strrchr(block.stSentence[0]->szSentence[3], '=');
        pTmp->r_bits_per_sec = atoi(++numPos);
        memset(tmpBuf, '\0', sizeof(tmpBuf));
        numPos = strrchr(block.stSentence[0]->szSentence[4], '=');
        pTmp->r_drops_per_sec = atoi(++numPos);
        memset(tmpBuf, '\0', sizeof(tmpBuf));
        numPos = strrchr(block.stSentence[0]->szSentence[5], '=');
        pTmp->r_errors_per_sec = atoi(++numPos);
        memset(tmpBuf, '\0', sizeof(tmpBuf));
        numPos = strrchr(block.stSentence[0]->szSentence[6], '=');
        pTmp->t_packets_per_sec = atoi(++numPos);
        memset(tmpBuf, '\0', sizeof(tmpBuf));
        numPos = strrchr(block.stSentence[0]->szSentence[7], '=');
        pTmp->t_bits_per_sec = atoi(++numPos);
        memset(tmpBuf, '\0', sizeof(tmpBuf));
        numPos = strrchr(block.stSentence[0]->szSentence[8], '=');
        pTmp->t_drops_per_sec = atoi(++numPos);
        memset(tmpBuf, '\0', sizeof(tmpBuf));
        numPos = strrchr(block.stSentence[0]->szSentence[9], '=');
        pTmp->t_errors_per_sec = atoi(++numPos);
        usleep(frequency);
        clearBlock(&block);
    }
    clearSentence(&sentence);
}

void free_interfase_list(struct Interface *pInterfase)
{
    struct Interface *pTmp;
    while (pInterfase) {
        free(pInterfase->name);
        pTmp = pInterfase;
        pInterfase = pInterfase->pNext;
        free(pTmp);
    }
}

void free_stat_list(struct Stat *pStat)
{
    struct Stat *pTmp;
    while (pStat) {
        pTmp = pStat;
        pStat = pStat->pNext;
        free(pTmp);
    }
}

void show_stat(int index, struct Stat *pStat, struct Interface *pInterface)
{
    switch (index) {
    case 0://recived packets per seconds
    {
        printf("\nStats for received packets per seconds (%s):", pInterface->name);
        int count = 1, i, maxVal = max_stat_val(index, pStat);
        while (pStat)
        {
            printf("\n%3d: ", count++);
            for(i = 0; i < maxVal; i++)
                printf("%s", pStat->r_packets_per_sec > i ? "*" : " ");
           pStat = pStat->pNext;
        }
        break;
    }
    default:
        break;
    }
}

void show_average_stat(int index, struct Stat *pStat, struct Interface *pInterface)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int widht = w.ws_col - 30;
    switch (index) {
    case 0://recived packets per seconds
    {
        printf("\n%-15s[", pInterface->name);
        int count = 1, i, total = 0;
        float average;
        while (pStat)
        {
            total += pStat->r_packets_per_sec;
            count++;
            pStat = pStat->pNext;
        }
        average = (float)total/count;
        for(i = 0; i < widht; i++)
        {
            printf(KGRN"%s", i < (int)average? "*" : " ");
        }
        printf(KNRM " (%4d)]", (int)average);
        break;
    }


    default:
        break;
    }

}

int max_stat_val(int index, struct Stat *pStat)
{
    int max_val = 0;
    while (pStat)
    {
        switch (index)
        {
            case 0:
            {
                if(pStat->r_packets_per_sec > max_val)
                    max_val = pStat->r_packets_per_sec;
                break;
            }

            default:
                break;
        }
        pStat = pStat->pNext;
    }
    return max_val;
}
