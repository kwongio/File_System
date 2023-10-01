//
// Created by rldh1 on 2023-09-30.
//

#include <sys/types.h>
#include <stdint.h>
#ifndef SYSTEM1_BLOCK_H
#define SYSTEM1_BLOCK_H

#endif //SYSTEM1_BLOCK_H


typedef struct __Block {
    BlockState blockState; // 블록의 상태 (할당됨 또는 삭제됨)
    uint16_t blockSize; // 블록의 크기
    char *key; // 키
    int keySize; // 키의 길이
    char *data; // 데이터
    int dataSize; // 데이터의 길이
    uint16_t tail; // 앞 블록의 사이즈 (삭제 시 사용)
} Block;
