//
// Created by rldh1 on 2023-09-30.
//

#include <sys/types.h>
#include <stdint.h>
#ifndef SYSTEM1_BLOCK_H
#define SYSTEM1_BLOCK_H

#endif //SYSTEM1_BLOCK_H


typedef struct __Block {
    BlockState blockState; // ����� ���� (�Ҵ�� �Ǵ� ������)
    uint16_t blockSize; // ����� ũ��
    char *key; // Ű
    int keySize; // Ű�� ����
    char *data; // ������
    int dataSize; // �������� ����
    uint16_t tail; // �� ����� ������ (���� �� ���)
} Block;
