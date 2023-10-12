#include <stdio.h>
#include <unistd.h>
#include "hw1.h"
#include <fcntl.h>
#include "block.h"
#include <stdlib.h>
#include <string.h>

int fd;

//Init() 함수 추가:
//구현에 필요한 전역 변수가 있다면 초기화를 하는 함수이다.
//main() 함수 내에서 testcase들의 호출하기 앞서 Init()가 호출됨.
void Init(void) {
    // 이 부분에서 전역 변수 및 초기화 작업 수행
    // 저장소 파일 초기화

}

int InsertData(char *key, int keySize, char *pBuf, int bufSize) {
    // 데이터를 저장할 블록 생성
    Blocks blocks;
    blocks.block.blockState = ALLOC_BLOCK;
    blocks.block.sizeHead = keySize + bufSize + sizeof(blocks.block.sizeHead) + sizeof(blocks.block.sizeTail);
    blocks.key = malloc(keySize);
    blocks.key = key;
    blocks.block.keySize = keySize;
    blocks.data = malloc(bufSize);
    blocks.data = pBuf;
    blocks.block.dataSize = bufSize;
    blocks.block.sizeTail = 0;
    printf("sizehEad %d\n", blocks.block.sizeHead);
    printf("Block size %d\n", sizeof(Blocks));

    // 파일의 처음부터 순차적으로 검색하여 빈 블록을 찾음
    lseek(fd, 0, SEEK_SET);
    Blocks tempBlock;
    int offset = 0;
    while (read(fd, &tempBlock, sizeof(Blocks)) > 0) {
        if (tempBlock.block.blockState == FREE_BLOCK && tempBlock.block.sizeHead >= blocks.block.sizeHead) {
            // 빈 블록을 찾았으면 해당 위치로 이동
            lseek(fd, offset, SEEK_SET);
            break;
        } else {
            blocks.block.sizeTail = tempBlock.block.sizeHead;
        }
        offset += sizeof(Blocks);
    }
    blocks.block.blockOffset = offset;
    blocks.block.blockState = ALLOC_BLOCK;
    if (write(fd, &blocks, sizeof(Blocks)) == -1) {
        printf("Error writing to file\n");
        return -1;
    }

    if (tempBlock.block.sizeHead - blocks.block.sizeHead > 0) {
        Blocks remains;
        remains.block.blockState = FREE_BLOCK;
        remains.block.sizeHead = tempBlock.block.sizeHead - blocks.block.sizeHead;
        remains.data = NULL;
        remains.block.keySize = 0;
        remains.key = NULL;
        remains.block.dataSize = 0;
        remains.block.sizeTail = blocks.block.sizeHead;

        if (write(fd, &remains, sizeof(Blocks)) == -1) {
            printf("Error writing to file remain\n");
            return -1;
        }
    }
    return 0; // 성공
}

//Key 값을 가지는 블록이 저장한 데이터를 pBuf로 반환한다
//? 만일 key 값을 찾을 수 없다면 -1를 리턴 한다

int GetDataByKey(char *key, int keySize, char *pBuf, int bufSize) {
    if (lseek(fd, 0, SEEK_SET) == -1) {
        printf("Error lseek\n");
        return -1;
    } // 파일의 시작부터 검색
    printf("GetDataByKey\n");
    printf("key: %s\n", key);

    Blocks blocks;
    while (read(fd, &blocks, sizeof(Blocks)) > 0) {
        printf("key: %s data: %s\n", blocks.key, blocks.data);
        // 블록 상태가 할당되었고, 키가 맞는 경우에만 데이터를 복사
        if (blocks.block.blockState == ALLOC_BLOCK && blocks.block.keySize == keySize &&
            strcmp(blocks.key, key) == 0) {

            // 데이터 크기가 버퍼 크기를 넘지 않는지 확인
            if (blocks.block.dataSize <= bufSize) {
                printf("copy...\n");
                printf("key: %s data: %s\n", blocks.key, blocks.data);
                strcpy(pBuf, blocks.data);
                printf("blocks.data:  %s\n", blocks.data);
                printf("pBufdddddddd: %s\n", pBuf);
                return blocks.block.dataSize; // 성공하면 데이터의 길이 반환
            } else {
                // 버퍼 크기가 부족한 경우 에러를 반환
                printf("Buffer size is too small\n");
                return -1;
            }
        }
//        lseek(fd, block.sizeTail, SEEK_CUR); // 다음 블록으로 이동
    }

    // 키를 찾지 못한 경우 -1 반환
    printf("Key not found\n");
    return -1;
}

int RemoveDataByKey(char *key, int keySize) {
    lseek(fd, 0, SEEK_SET);
    Block prevBlock;
    Blocks blocks;
    Block nextBlock;

    while (read(fd, &blocks.block, sizeof(Blocks)) > 0) {
        // case 1: 앞 뒤 둘 다 블럭이 있는 경우
        // case 2: 뒤에 블록이 비어있는 경우
        // case 3: 앞에 블록이 비어있는 경우
        // case 4: 앞 뒤 둘 다 비어있는 경우
        if (blocks.block.blockState == ALLOC_BLOCK && blocks.block.keySize == keySize &&
            memcmp(blocks.key, key, keySize) == 0) {
            // 현재 블록을 FREE_BLOCK 상태로 바꿈
            blocks.block.blockState = FREE_BLOCK;
            lseek(fd, -sizeof(Blocks), SEEK_CUR);
            if (write(fd, &blocks.block, sizeof(Blocks)) == -1) {
                // 변경한 블록 저장 실패
                printf("Error writing to file\n");
                return -1;
            }

            // 뒤에 블록이 비어있는지 확인r
            if (read(fd, &nextBlock, sizeof(Blocks)) > 0) {
                if (nextBlock.blockState == FREE_BLOCK) {
                    blocks.block.sizeHead += nextBlock.sizeHead;
                    lseek(fd, -sizeof(Blocks), SEEK_CUR);
                    if (write(fd, &blocks.block, sizeof(Blocks)) == -1) {
                        // 병합한 블록 저장 실패
                        printf("Error writing to file\n");
                        return -1;
                    }
                }
            }
            // 앞에 블록이 비어있는지 확인

            lseek(fd, -sizeof(Blocks), SEEK_CUR);
            read(fd, &prevBlock, sizeof(Blocks));
            if (prevBlock.blockState == FREE_BLOCK) {
                prevBlock.sizeHead += blocks.block.sizeHead;
                lseek(fd, -sizeof(Blocks), SEEK_CUR);
                if (write(fd, &prevBlock, sizeof(Blocks)) == -1) {
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

void printBlock() {
    lseek(fd, 0, SEEK_SET); // 파일의 시작부터 검색
    Blocks blocks;
    while (read(fd, &blocks, sizeof(Blocks)) > 0) {
        //block 전체 출력
//        if (block.blockState == ALLOC_BLOCK) {
        printf("blockState: %d\n", blocks.block.blockState);
        printf("sizeHead: %u\n", blocks.block.sizeHead);
        printf("key: %s\n", (blocks.key != NULL) ? blocks.key : "NULL");
        printf("data: %s\n", (blocks.data != NULL) ? blocks.data : "NULL");
        printf("keySize: %d\n", blocks.block.keySize);
        printf("dataSize: %d\n", blocks.block.dataSize);
        printf("sizeTail: %u\n\n", blocks.block.sizeTail);
//        }
    }
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
    int removeResult = remove(STORAGE_NAME);
    if (removeResult != 0) {
        printf("Error removing existing file: %s\n", STORAGE_NAME);
    }

    fd = open(STORAGE_NAME, O_CREAT | O_RDWR | O_TRUNC, 0666);

    if (fd == -1) {
        printf("file open error\n");
        return;
    }

    // MAX_STORAGE_SIZE 크기의 빈 파일 생성
    lseek(fd, MAX_STORAGE_SIZE, SEEK_SET);
    lseek(fd, 0, SEEK_SET);

    // 초기 블록 메타데이터 추가
    Blocks initialBlock;
    initialBlock.block.blockOffset = 0;
    initialBlock.block.blockState = FREE_BLOCK;
    initialBlock.block.sizeHead = MAX_STORAGE_SIZE;
    initialBlock.key = NULL; // 초기 상태에서는 key와 data가 없습니다.
    initialBlock.block.keySize = 0;
    initialBlock.data = NULL;
    initialBlock.block.dataSize = 0;
    initialBlock.block.sizeTail = 0;

    if (write(fd, &initialBlock, sizeof(Blocks)) == -1) {
        printf("IniError writing to file\n");
        return;
    };

}

int GetBlocks(Blocks *pBuf, int bufSize) {

    lseek(fd, 0, SEEK_SET); // 파일의 시작부터 검색
    Blocks blocks;
    int i = 0;

    while (read(fd, &blocks, sizeof(Blocks)) > 0) {
        i++;
        pBuf[i].key = malloc(pBuf[i].block.keySize);
        pBuf[i].data = malloc(pBuf[i].block.dataSize);
        strcpy(pBuf[i].key, blocks.key);
        strcpy(pBuf[i].data, blocks.data);
        pBuf[i] = blocks;
    }
    return i;
}