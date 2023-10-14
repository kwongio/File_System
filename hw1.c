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
    lseek(fd, 0, SEEK_SET); // ������ ���ۺ��� �˻�
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
    // ������ ó������ ���������� �˻��Ͽ� �� ����� ã��
    int offset = 0;
    while (read(fd, &tempDocument, sizeof(Document)) > 0) {
        if (tempDocument.block.blockState == FREE_BLOCK && tempDocument.block.sizeHead >= block.sizeHead) {
            // �� ����� ã������ �ش� ��ġ�� �̵�
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
    return 0; // ����
}

//Key ���� ������ ����� ������ �����͸� pBuf�� ��ȯ�Ѵ�
//? ���� key ���� ã�� �� ���ٸ� -1�� ���� �Ѵ�

int GetDataByKey(char *key, int keySize, char *pBuf, int bufSize) {
    lseek(fd, 0, SEEK_SET); // ������ ���ۺ��� �˻�
    Document document;
    while (read(fd, &document, sizeof(Document)) > 0) {
        // ��� ���°� �Ҵ�Ǿ���, Ű�� �´� ��쿡�� �����͸� ����
        char *tempKey = document.blockKey;
        char *tempData = document.blockData;
//        printf("tempKey: %s\n", tempKey);
//        printf("tempData: %s\n", tempData);
        if (document.block.blockState == ALLOC_BLOCK && document.block.keySize == keySize & strcmp(tempKey, key) == 0) {
//            printf("key: %s data: %s\n", tempKey, tempData);
            // ������ ũ�Ⱑ ���� ũ�⸦ ���� �ʴ��� Ȯ��

            if (document.block.dataSize <= bufSize) {
//                printf("key: %s data: %s\n", tempKey, tempData);
                strcpy(pBuf, tempData); // ������ ����
                return document.block.dataSize; // �����ϸ� �������� ���� ��ȯ
            } else {
                // ���� ũ�Ⱑ ������ ��� ������ ��ȯ
                printf("Buffer size is too small\n");
                return -1;
            }
        }
//        lseek(fd, block.tail, SEEK_CUR); // ���� ������� �̵�
    }
    // Ű�� ã�� ���� ��� -1 ��ȯ
//    printf("Key not found\n");
    return -1;
}

int RemoveDataByKey(char *key, int keySize) {
    lseek(fd, 0, SEEK_SET); // ������ ���ۺ��� �˻�
//    printf("RemoveDataByKey: %s\n", key);
    Document prevDocument;
    Document document;
    Document nextDocument;
    while (read(fd, &document, sizeof(Document)) > 0) {
        // case 1: �� �� �� �� ���� �ִ� ���
        // case 2: �ڿ� ����� ����ִ� ���
        // case 3: �տ� ����� ����ִ� ���
        // case 4: �� �� �� �� ����ִ� ���
//        printf("RemoveDataByKey: %s\n", key);
//        printf("document.blockKey %s\n", document.blockKey);
        char *tempKey = document.blockKey;
        if (document.block.blockState == ALLOC_BLOCK && document.block.keySize == keySize &&
            strcmp(tempKey, key) == 0) {
            // ���� ����� FREE_BLOCK ���·� �ٲ�
            document.block.blockState = FREE_BLOCK;
            lseek(fd, -sizeof(Document), SEEK_CUR);
            if (write(fd, &document, sizeof(Document)) == -1) {
                // ������ ��� ���� ����
                printf("Error writing to file\n");
                return -1;
            }

            // �ڿ� ����� ����ִ��� Ȯ��r
            if (read(fd, &nextDocument, sizeof(Document)) > 0) {
                if (nextDocument.block.blockState == FREE_BLOCK) {
                    document.block.sizeHead += nextDocument.block.sizeHead;
                    lseek(fd, -sizeof(Document), SEEK_CUR);
                    if (write(fd, &document, sizeof(Document)) == -1) {
                        // ������ ��� ���� ����
                        printf("Error writing to file\n");
                        return -1;
                    }
                }
            }
            // �տ� ����� ����ִ��� Ȯ��

            lseek(fd, -sizeof(Document), SEEK_CUR);
            read(fd, &prevDocument, sizeof(Document));
            if (prevDocument.block.blockState == FREE_BLOCK) {
                prevDocument.block.sizeHead += document.block.sizeHead;
                lseek(fd, -sizeof(Document), SEEK_CUR);
                if (write(fd, &prevDocument, sizeof(Document)) == -1) {
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
    fd = open(STORAGE_NAME, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd == -1) {
        printf("fd file open error\n");
        return;
    }
    // MAX_STORAGE_SIZE ũ���� �� ���� ����
    lseek(fd, MAX_STORAGE_SIZE, SEEK_SET);
    lseek(fd, 0, SEEK_SET);

    Document initialDocument;
    // �ʱ� ��� ��Ÿ������ �߰�
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
    } // ������ ���ۺ��� �˻�

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
