//#include "hw1.h"
//#include "block.h"
//#include <stdio.h>
//
//
//#define MAX_BLOCK_NUM      (1024)
//
//
//int main() {
//    Init();
//    InitStorage();
//    InsertData("1", 1, "data1", 5);
//    InsertData("2", 1, "data2", 5);
//    InsertData("3", 1, "data3", 5);
//
//    PrintBlocks();
//}
////
//////    printBlocks();
//////    InsertData( "1", 1, "data1", 5);
//////    InsertData( "2", 1, "data2", 5);
//////    InsertData( "3", 1, "data3", 5);
//////    InsertData( "4", 1, "data4", 5);
//////    InsertData( "5", 1, "data5", 5);
//////    InsertData( "6", 1, "data6", 5);
//////    InsertData( "7", 1, "data7", 5);
//////    InsertData( "8", 1, "data8", 5);
//////    InsertData( "9", 1, "data9", 5);
//////    InsertData( "10", 2, "data10", 6);
//////    InsertData( "11", 2, "data11", 6);
//////    InsertData( "12", 2, "data12", 6);
//////    InsertData( "13", 2, "data13", 6);
//////
//////
//////
//////
//////    // case 1: 앞 뒤 둘 다 블럭이 있는 경우
//////
//////    printf("====== Case 1 ===============\n");
//////    RemoveDataByKey("2", 1);
//////    printBlock();
//////
//////
//////
//////    // case 2: 뒤에 블록이 비어있는 경우
//////    printf("====== Case 2 ===============\n");
//////    RemoveDataByKey("9", 1);
//////    RemoveDataByKey("8", 1);
//////    printBlock();
//////
//////
//////    // case 3: 앞에 블록이 비어있는 경우
//////    printf("====== Case 3 ===============\n");
//////    RemoveDataByKey("10", 2);
//////    printBlock();
////
//////    // case 4: 앞 뒤 둘 다 비어있는 경우
//////    printf("====== Case 4 ===============\n");
//////    RemoveDataByKey("6", 1);
//////    RemoveDataByKey("4", 1);
//////    RemoveDataByKey("5", 1);
//////
////
//////    printBlock();
//////
////
////
////
////
////
////
//////    printf("removeDataByKey: %d\n\n", RemoveDataByKey("2", 1));
//////    char* pBuf = (char*)malloc(5);
//////    printf("getDataByKey size : %d\n", getDataByKey("1", 1, pBuf, 10000));
//////    printf("pBuf: %s\n", pBuf);
////
////    return 0;
////}
//
//int PrintBlocks(void) {
//
//    Block pBuf[MAX_BLOCK_NUM];
//
//    int numBlocks = 0;
//
//    printf("Blocks: ");
//    numBlocks = GetBlocks(pBuf, MAX_BLOCK_NUM);
//    for (int i = 0; i < numBlocks; i++) {
//        printf("[%d %d %d %d %d %d], ",
//               pBuf[i].blockOffset, pBuf[i].blockState,
//               pBuf[i].sizeHead, pBuf[i].sizeTail,
//               pBuf[i].keySize, pBuf[i].dataSize);
//    }
//    printf("\n");
//    return 0;
//
//}
//
