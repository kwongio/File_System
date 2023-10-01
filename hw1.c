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
    InitStorage();
    fd = open(STORAGE_NAME, O_RDWR, 0666); // 파일 열기
}

int InsertData(char *key, int keySize, char *pBuf, int bufSize) {
    // 데이터를 저장할 블록 생성
    Block block;
    block.blockState = ALLOC_BLOCK;
    block.blockSize = keySize + bufSize + sizeof(block.blockSize) + sizeof(block.tail);
    block.key = key;
    block.keySize = keySize;
    block.data = pBuf;
    block.dataSize = bufSize;
    block.tail = 0;

    // 파일의 처음부터 순차적으로 검색하여 빈 블록을 찾음
    lseek(fd, 0, SEEK_SET);
    Block tempBlock;
    int offset = 0;
    while (read(fd, &tempBlock, sizeof(Block)) > 0) {
        if (tempBlock.blockState == FREE_BLOCK) {
            // 빈 블록을 찾았으면 해당 위치로 이동
            lseek(fd, offset, SEEK_SET);
            break;
        }
        offset += sizeof(Block);
    }
    if (write(fd, &block, sizeof(Block)) == -1) {
        printf("Error writing to file\n");
        return -1;
    }

    Block remain;
    remain.blockState = FREE_BLOCK;
    remain.blockSize = tempBlock.blockSize - block.blockSize;
    remain.data = NULL;
    remain.keySize = 0;
    remain.key = NULL;
    remain.dataSize = 0;
    remain.tail = block.blockSize;

    if (write(fd, &remain, sizeof(Block)) == -1) {
        printf("Error writing to file remain\n");
        return -1;
    }

    return 0; // 성공
}

//Key 값을 가지는 블록이 저장한 데이터를 pBuf로 반환한다
//? 만일 key 값을 찾을 수 없다면 -1를 리턴 한다

int getDataByKey(char *key, int keySize, char *pBuf, int bufSize) {
    if(lseek(fd, 0, SEEK_SET) == -1){
        printf("Error lseek\n");
        return -1;
    } // 파일의 시작부터 검색
    printf("key: %s\n", key);
    printf("keySize: %d\n", keySize);
    printf("pBuf: %s\n", pBuf);
    printf("bufSize: %d\n", bufSize);

    Block block;
    while (read(fd, &block, sizeof(Block)) > 0) {
        printf("\n\n\n");
        // 블록 상태가 할당되었고, 키가 맞는 경우에만 데이터를 복사
        if (block.blockState == ALLOC_BLOCK && block.keySize == keySize && memcmp(block.key, key, keySize) == 0) {
            printf("key: %s data: %s\n", block.key, block.data);
            // 데이터 크기가 버퍼 크기를 넘지 않는지 확인
            if (block.dataSize <= bufSize) {
                printf("key: %s data: %s\n", block.key, block.data);
                memcpy(pBuf, block.data, block.dataSize); // 데이터 복사
                return block.dataSize; // 성공하면 데이터의 길이 반환
            } else {
                // 버퍼 크기가 부족한 경우 에러를 반환
                printf("Buffer size is too small\n");
                return -1;
            }
        }
//        lseek(fd, block.tail, SEEK_CUR); // 다음 블록으로 이동
    }

    // 키를 찾지 못한 경우 -1 반환
    printf("Key not found\n");
    return -1;
}

int RemoveDataByKey(char *key, int keySize) {

}

void printBlock() {
    lseek(fd, 0, SEEK_SET); // 파일의 시작부터 검색
    Block block;
    while (read(fd, &block, sizeof(Block)) > 0) {
        //block 전체 출력
        printf("blockState: %d\n", block.blockState);
        printf("blockSize: %u\n", block.blockSize);
        printf("key: %s\n", (block.key != NULL) ? block.key : "NULL");
        printf("keySize: %d\n", block.keySize);
        printf("data: %s\n", (block.data != NULL) ? block.data : "NULL");
        printf("dataSize: %d\n", block.dataSize);
        printf("tail: %u\n", block.tail);
        printf("\n\n\n");
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
    lseek(fd, MAX_STORAGE_SIZE - 1, SEEK_SET);
    lseek(fd, 0, SEEK_SET);

    // 초기 블록 메타데이터 추가
    Block initialBlock;
    initialBlock.blockState = FREE_BLOCK;
    initialBlock.blockSize = MAX_STORAGE_SIZE;
    initialBlock.key = NULL; // 초기 상태에서는 key와 data가 없습니다.
    initialBlock.keySize = 0;
    initialBlock.data = NULL;
    initialBlock.dataSize = 0;
    initialBlock.tail = 0;

    write(fd, &initialBlock, sizeof(Block));

    close(fd); // 파일 닫기
}

