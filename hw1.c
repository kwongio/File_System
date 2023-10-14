#include <stdio.h>
#include <unistd.h>
#include "hw1.h"
#include <fcntl.h>
#include "block.h"
#include <stdlib.h>
#include <string.h>


int fd;
int blockOffset = 0;
#define MAX_BLOCK_NUM (1024)


void Init(void) {
    fd = open(STORAGE_NAME, O_RDWR, 0777);
}

int InsertData(char *key, int keySize, char *pBuf, int bufSize) {
    lseek(fd, 0, SEEK_SET); // 파일의 시작부터 검색
    Document document;
    Block block;
    block.blockOffset = blockOffset;
    block.blockState = ALLOC_BLOCK;
    block.sizeHead = sizeof(keySize) + sizeof(bufSize) + sizeof(block.sizeHead) + sizeof(block.sizeTail);
    block.keySize = keySize;
    block.dataSize = bufSize;
    block.sizeTail = 0;
    document.block = block;
    strcpy(document.blockKey, key);
    strcpy(document.blockData, pBuf);
    blockOffset += sizeof(Document);


    Document tempDocument;
    // 파일의 처음부터 순차적으로 검색하여 빈 블록을 찾음
    int offset = 0;
    while (read(fd, &tempDocument, sizeof(Document)) > 0) {
        if (tempDocument.block.blockState == FREE_BLOCK && tempDocument.block.sizeHead >= block.sizeHead) {
            // 빈 블록을 찾았으면 해당 위치로 이동
            lseek(fd, offset, SEEK_SET);
            break;
        } else {
            document.block.sizeTail = tempDocument.block.sizeHead;
        }
        offset += sizeof(Document);
    }
    document.block.blockState = ALLOC_BLOCK;
    if (write(fd, &document, sizeof(Document)) == -1) {
        printf("Error writing to file\n");
        return -1;
    }

    if (tempDocument.block.sizeHead - block.sizeHead > 0) {
        Document remainDocument;
        Block remain;
        remain.blockOffset = 0;
        remain.blockState = FREE_BLOCK;
        remain.sizeHead = tempDocument.block.sizeHead - block.sizeHead;
        remain.keySize = 0;
        remain.dataSize = 0;
        remain.sizeTail = block.sizeHead;
        remainDocument.block = remain;
        strcpy(remainDocument.blockKey, "");
        strcpy(remainDocument.blockData, "");

        if (write(fd, &remainDocument, sizeof(Document)) == -1) {
            printf("Error writing to file remain\n");
            return -1;
        }
    }
    return 0; // 성공
}

//Key 값을 가지는 블록이 저장한 데이터를 pBuf로 반환한다
//? 만일 key 값을 찾을 수 없다면 -1를 리턴 한다

int GetDataByKey(char *key, int keySize, char *pBuf, int bufSize) {
    lseek(fd, 0, SEEK_SET); // 파일의 시작부터 검색
    Document document;
    while (read(fd, &document, sizeof(Document)) > 0) {
        // 블록 상태가 할당되었고, 키가 맞는 경우에만 데이터를 복사
        char *tempKey = document.blockKey;
        char *tempData = document.blockData;
//        printf("tempKey: %s\n", tempKey);
//        printf("tempData: %s\n", tempData);
        if (document.block.blockState == ALLOC_BLOCK && document.block.keySize == keySize & strcmp(tempKey, key) == 0) {
//            printf("key: %s data: %s\n", tempKey, tempData);
            // 데이터 크기가 버퍼 크기를 넘지 않는지 확인

            if (document.block.dataSize <= bufSize) {
//                printf("key: %s data: %s\n", tempKey, tempData);
                strcpy(pBuf, tempData); // 데이터 복사
                return document.block.dataSize; // 성공하면 데이터의 길이 반환
            } else {
                // 버퍼 크기가 부족한 경우 에러를 반환
                printf("Buffer size is too small\n");
                return -1;
            }
        }
//        lseek(fd, block.tail, SEEK_CUR); // 다음 블록으로 이동
    }
    // 키를 찾지 못한 경우 -1 반환
//    printf("Key not found\n");
    return -1;
}

int RemoveDataByKey(char *key, int keySize) {
    lseek(fd, 0, SEEK_SET); // 파일의 시작부터 검색
//    printf("RemoveDataByKey: %s\n", key);
    Document prevDocument;
    Document document;
    Document nextDocument;
    while (read(fd, &document, sizeof(Document)) > 0) {
        // case 1: 앞 뒤 둘 다 블럭이 있는 경우
        // case 2: 뒤에 블록이 비어있는 경우
        // case 3: 앞에 블록이 비어있는 경우
        // case 4: 앞 뒤 둘 다 비어있는 경우
//        printf("RemoveDataByKey: %s\n", key);
//        printf("document.blockKey %s\n", document.blockKey);
        char *tempKey = document.blockKey;
        if (document.block.blockState == ALLOC_BLOCK && document.block.keySize == keySize &&
            strcmp(tempKey, key) == 0) {
            // 현재 블록을 FREE_BLOCK 상태로 바꿈
            document.block.blockState = FREE_BLOCK;
            lseek(fd, -sizeof(Document), SEEK_CUR);
            if (write(fd, &document, sizeof(Document)) == -1) {
                // 변경한 블록 저장 실패
                printf("Error writing to file\n");
                return -1;
            }

            // 뒤에 블록이 비어있는지 확인r
            if (read(fd, &nextDocument, sizeof(Document)) > 0) {
                if (nextDocument.block.blockState == FREE_BLOCK) {
                    document.block.sizeHead += nextDocument.block.sizeHead;
                    lseek(fd, -sizeof(Document), SEEK_CUR);
                    if (write(fd, &document, sizeof(Document)) == -1) {
                        // 병합한 블록 저장 실패
                        printf("Error writing to file\n");
                        return -1;
                    }
                }
            }
            // 앞에 블록이 비어있는지 확인

            lseek(fd, -sizeof(Document), SEEK_CUR);
            read(fd, &prevDocument, sizeof(Document));
            if (prevDocument.block.blockState == FREE_BLOCK) {
                prevDocument.block.sizeHead += document.block.sizeHead;
                lseek(fd, -sizeof(Document), SEEK_CUR);
                if (write(fd, &prevDocument, sizeof(Document)) == -1) {
                    // 병합한 블록 저장 실패
                    printf("Error writing to file\n");
                    return -1;
                }
            }
            return 0; // 성공
        }
    }
    // 찾는 키가 없는 경우
    return -1;
}


//InitStorage():
//
//Storage 파일을 생성하고 초기화 시키거나,
//
//만일 데이터가 저장되어 있다면 빈 저장장치로 초기화 시킨다.
//
//이때, storage 파일을 생성할 때 MAX_STORAGE_SIZE의 빈 파일을 생성해야함.

//hw1.h에 아래처럼 정의함
//#define MAX_STORAGE_SIZE (65,536)
//#define STORAGE_NAME "storage"
//생성할 storage 파일 이름은 "storage"로 정의함.
void InitStorage(void) {
    fd = open(STORAGE_NAME, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd == -1) {
        printf("fd file open error\n");
        return;
    }
    // MAX_STORAGE_SIZE 크기의 빈 파일 생성
    lseek(fd, MAX_STORAGE_SIZE, SEEK_SET);
    lseek(fd, 0, SEEK_SET);

    Document initialDocument;
    // 초기 블록 메타데이터 추가
    Block initialBlock;
    initialBlock.blockOffset = 0;
    initialBlock.blockState = FREE_BLOCK;
    initialBlock.sizeHead = MAX_STORAGE_SIZE;
    initialBlock.keySize = 0;
    initialBlock.dataSize = 0;
    initialBlock.sizeTail = 0;
    initialDocument.block = initialBlock;
    strcpy(initialDocument.blockKey, "");
    strcpy(initialDocument.blockData, "");

    if (write(fd, &initialDocument, sizeof(Document)) == -1) {
        printf("Error writing to file\n");
        return;
    }

}


int GetBlocks(Block *pBuf, int bufSize) {
    if (lseek(fd, 0, SEEK_SET) > 0) {
        printf("lseek error\n");
        return -1;
    } // 파일의 시작부터 검색

    Document document;

    int i = 0;
    while (read(fd, &document, sizeof(Document)) > 0) {
//        printf("document.blockKey: %s\n", document.blockKey);
//        printf("document.blockData: %s\n", document.blockData);
        pBuf[i] = document.block;
//        printf("pBuf[i].blockOffset: %d\n pBuf[i].blockState: %d\n pBuf[i].sizeHead: %d\n pBuf[i].sizeTail: %d\n pBuf[i].keySize: %d\n pBuf[i].dataSize: %d\n\n ",
//               pBuf[i].blockOffset, pBuf[i].blockState,
//               pBuf[i].sizeHead, pBuf[i].sizeTail,
//               pBuf[i].keySize, pBuf[i].dataSize);
        i++;

    }
    return i;
}
