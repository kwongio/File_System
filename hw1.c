#include <stdio.h>
#include <unistd.h>
#include "hw1.h"
#include <fcntl.h>
#include "block.h"
#include <stdlib.h>
#include <string.h>

int fd;

//Init() �Լ� �߰�:
//������ �ʿ��� ���� ������ �ִٸ� �ʱ�ȭ�� �ϴ� �Լ��̴�.
//main() �Լ� ������ testcase���� ȣ���ϱ� �ռ� Init()�� ȣ���.
void Init(void) {
    // �� �κп��� ���� ���� �� �ʱ�ȭ �۾� ����
    // ����� ���� �ʱ�ȭ
    InitStorage();
    fd = open(STORAGE_NAME, O_RDWR, 0666); // ���� ����
}

int InsertData(char *key, int keySize, char *pBuf, int bufSize) {
    // �����͸� ������ ��� ����
    Block block;
    block.blockState = ALLOC_BLOCK;
    block.blockSize = keySize + bufSize + sizeof(block.blockSize) + sizeof(block.tail);
    block.key = key;
    block.keySize = keySize;
    block.data = pBuf;
    block.dataSize = bufSize;
    block.tail = 0;

    // ������ ó������ ���������� �˻��Ͽ� �� ����� ã��
    lseek(fd, 0, SEEK_SET);
    Block tempBlock;
    int offset = 0;
    while (read(fd, &tempBlock, sizeof(Block)) > 0) {
        if (tempBlock.blockState == FREE_BLOCK) {
            // �� ����� ã������ �ش� ��ġ�� �̵�
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

    return 0; // ����
}

//Key ���� ������ ����� ������ �����͸� pBuf�� ��ȯ�Ѵ�
//? ���� key ���� ã�� �� ���ٸ� -1�� ���� �Ѵ�

int getDataByKey(char *key, int keySize, char *pBuf, int bufSize) {
    if(lseek(fd, 0, SEEK_SET) == -1){
        printf("Error lseek\n");
        return -1;
    } // ������ ���ۺ��� �˻�
    printf("key: %s\n", key);
    printf("keySize: %d\n", keySize);
    printf("pBuf: %s\n", pBuf);
    printf("bufSize: %d\n", bufSize);

    Block block;
    while (read(fd, &block, sizeof(Block)) > 0) {
        printf("\n\n\n");
        // ��� ���°� �Ҵ�Ǿ���, Ű�� �´� ��쿡�� �����͸� ����
        if (block.blockState == ALLOC_BLOCK && block.keySize == keySize && memcmp(block.key, key, keySize) == 0) {
            printf("key: %s data: %s\n", block.key, block.data);
            // ������ ũ�Ⱑ ���� ũ�⸦ ���� �ʴ��� Ȯ��
            if (block.dataSize <= bufSize) {
                printf("key: %s data: %s\n", block.key, block.data);
                memcpy(pBuf, block.data, block.dataSize); // ������ ����
                return block.dataSize; // �����ϸ� �������� ���� ��ȯ
            } else {
                // ���� ũ�Ⱑ ������ ��� ������ ��ȯ
                printf("Buffer size is too small\n");
                return -1;
            }
        }
//        lseek(fd, block.tail, SEEK_CUR); // ���� ������� �̵�
    }

    // Ű�� ã�� ���� ��� -1 ��ȯ
    printf("Key not found\n");
    return -1;
}

int RemoveDataByKey(char *key, int keySize) {

}

void printBlock() {
    lseek(fd, 0, SEEK_SET); // ������ ���ۺ��� �˻�
    Block block;
    while (read(fd, &block, sizeof(Block)) > 0) {
        //block ��ü ���
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
//Storage ������ �����ϰ� �ʱ�ȭ ��Ű�ų�,
//
//���� �����Ͱ� ����Ǿ� �ִٸ� �� ������ġ�� �ʱ�ȭ ��Ų��.
//
//�̶�, storage ������ ������ �� MAX_STORAGE_SIZE�� �� ������ �����ؾ���.

//hw1.h�� �Ʒ�ó�� ������
//#define MAX_STORAGE_SIZE (65,536)
//#define STORAGE_NAME "storage"
//������ storage ���� �̸��� "storage"�� ������.
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

    // MAX_STORAGE_SIZE ũ���� �� ���� ����
    lseek(fd, MAX_STORAGE_SIZE - 1, SEEK_SET);
    lseek(fd, 0, SEEK_SET);

    // �ʱ� ��� ��Ÿ������ �߰�
    Block initialBlock;
    initialBlock.blockState = FREE_BLOCK;
    initialBlock.blockSize = MAX_STORAGE_SIZE;
    initialBlock.key = NULL; // �ʱ� ���¿����� key�� data�� �����ϴ�.
    initialBlock.keySize = 0;
    initialBlock.data = NULL;
    initialBlock.dataSize = 0;
    initialBlock.tail = 0;

    write(fd, &initialBlock, sizeof(Block));

    close(fd); // ���� �ݱ�
}

