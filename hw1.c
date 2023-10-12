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

}

int InsertData(char *key, int keySize, char *pBuf, int bufSize) {
    // �����͸� ������ ��� ����
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

    // ������ ó������ ���������� �˻��Ͽ� �� ����� ã��
    lseek(fd, 0, SEEK_SET);
    Blocks tempBlock;
    int offset = 0;
    while (read(fd, &tempBlock, sizeof(Blocks)) > 0) {
        if (tempBlock.block.blockState == FREE_BLOCK && tempBlock.block.sizeHead >= blocks.block.sizeHead) {
            // �� ����� ã������ �ش� ��ġ�� �̵�
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
    return 0; // ����
}

//Key ���� ������ ����� ������ �����͸� pBuf�� ��ȯ�Ѵ�
//? ���� key ���� ã�� �� ���ٸ� -1�� ���� �Ѵ�

int GetDataByKey(char *key, int keySize, char *pBuf, int bufSize) {
    if (lseek(fd, 0, SEEK_SET) == -1) {
        printf("Error lseek\n");
        return -1;
    } // ������ ���ۺ��� �˻�
    printf("GetDataByKey\n");
    printf("key: %s\n", key);

    Blocks blocks;
    while (read(fd, &blocks, sizeof(Blocks)) > 0) {
        printf("key: %s data: %s\n", blocks.key, blocks.data);
        // ��� ���°� �Ҵ�Ǿ���, Ű�� �´� ��쿡�� �����͸� ����
        if (blocks.block.blockState == ALLOC_BLOCK && blocks.block.keySize == keySize &&
            strcmp(blocks.key, key) == 0) {

            // ������ ũ�Ⱑ ���� ũ�⸦ ���� �ʴ��� Ȯ��
            if (blocks.block.dataSize <= bufSize) {
                printf("copy...\n");
                printf("key: %s data: %s\n", blocks.key, blocks.data);
                strcpy(pBuf, blocks.data);
                printf("blocks.data:  %s\n", blocks.data);
                printf("pBufdddddddd: %s\n", pBuf);
                return blocks.block.dataSize; // �����ϸ� �������� ���� ��ȯ
            } else {
                // ���� ũ�Ⱑ ������ ��� ������ ��ȯ
                printf("Buffer size is too small\n");
                return -1;
            }
        }
//        lseek(fd, block.sizeTail, SEEK_CUR); // ���� ������� �̵�
    }

    // Ű�� ã�� ���� ��� -1 ��ȯ
    printf("Key not found\n");
    return -1;
}

int RemoveDataByKey(char *key, int keySize) {
    lseek(fd, 0, SEEK_SET);
    Block prevBlock;
    Blocks blocks;
    Block nextBlock;

    while (read(fd, &blocks.block, sizeof(Blocks)) > 0) {
        // case 1: �� �� �� �� ���� �ִ� ���
        // case 2: �ڿ� ����� ����ִ� ���
        // case 3: �տ� ����� ����ִ� ���
        // case 4: �� �� �� �� ����ִ� ���
        if (blocks.block.blockState == ALLOC_BLOCK && blocks.block.keySize == keySize &&
            memcmp(blocks.key, key, keySize) == 0) {
            // ���� ����� FREE_BLOCK ���·� �ٲ�
            blocks.block.blockState = FREE_BLOCK;
            lseek(fd, -sizeof(Blocks), SEEK_CUR);
            if (write(fd, &blocks.block, sizeof(Blocks)) == -1) {
                // ������ ��� ���� ����
                printf("Error writing to file\n");
                return -1;
            }

            // �ڿ� ����� ����ִ��� Ȯ��r
            if (read(fd, &nextBlock, sizeof(Blocks)) > 0) {
                if (nextBlock.blockState == FREE_BLOCK) {
                    blocks.block.sizeHead += nextBlock.sizeHead;
                    lseek(fd, -sizeof(Blocks), SEEK_CUR);
                    if (write(fd, &blocks.block, sizeof(Blocks)) == -1) {
                        // ������ ��� ���� ����
                        printf("Error writing to file\n");
                        return -1;
                    }
                }
            }
            // �տ� ����� ����ִ��� Ȯ��

            lseek(fd, -sizeof(Blocks), SEEK_CUR);
            read(fd, &prevBlock, sizeof(Blocks));
            if (prevBlock.blockState == FREE_BLOCK) {
                prevBlock.sizeHead += blocks.block.sizeHead;
                lseek(fd, -sizeof(Blocks), SEEK_CUR);
                if (write(fd, &prevBlock, sizeof(Blocks)) == -1) {
                    // ������ ��� ���� ����
                    printf("Error writing to file\n");
                    return -1;
                }
            }
            return 0; // ����
        }
    }

    // ã�� Ű�� ���� ���
    return -1;
}

void printBlock() {
    lseek(fd, 0, SEEK_SET); // ������ ���ۺ��� �˻�
    Blocks blocks;
    while (read(fd, &blocks, sizeof(Blocks)) > 0) {
        //block ��ü ���
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
    lseek(fd, MAX_STORAGE_SIZE, SEEK_SET);
    lseek(fd, 0, SEEK_SET);

    // �ʱ� ��� ��Ÿ������ �߰�
    Blocks initialBlock;
    initialBlock.block.blockOffset = 0;
    initialBlock.block.blockState = FREE_BLOCK;
    initialBlock.block.sizeHead = MAX_STORAGE_SIZE;
    initialBlock.key = NULL; // �ʱ� ���¿����� key�� data�� �����ϴ�.
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

    lseek(fd, 0, SEEK_SET); // ������ ���ۺ��� �˻�
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