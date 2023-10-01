#include "hw1.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
    Init();
    InsertData( "1", 1, "data1", 10000);
    InsertData( "2", 1, "data2", 5);
    InsertData( "3", 1, "data3", 5);
    char* pBuf = (char*)malloc(5);
    printf("getDataByKey size : %d\n", getDataByKey("1", 1, pBuf, 10000));
    printf("pBuf: %s\n", pBuf);
    printBlock();
    return 0;
}