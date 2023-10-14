//
// Created by rldh1 on 2023-09-30.
//

#include <sys/types.h>
#include <stdint.h>
#include "hw1.h"

#ifndef SYSTEM1_BLOCK_H
#define SYSTEM1_BLOCK_H

#endif //SYSTEM1_BLOCK_H

typedef struct Document {
    Block block;
    char blockKey[100] ;
    char blockData[100];
} Document;

