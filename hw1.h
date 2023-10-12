#ifndef __HW1_H__

#define __HW1_H__
#define MAX_STORAGE_SIZE (65500)
#define STORAGE_NAME "storage"

typedef enum __BlockState{
    FREE_BLOCK = 0,
    ALLOC_BLOCK = 1
} BlockState;


typedef struct __Block {
    int blockOffset;		// 블록의 시작 offset
    int blockState;		// 블록의 상태
    int sizeHead;		// 블록 헤더에 있는 블록 크기 값
    int sizeTail;		// 블록 헤더에 있는 블록 크기 값
    int keySize;		// 키 크기
    int dataSize;		// 데이트 크기
} Block;

typedef struct __Blocks {
    Block block;// 블록
    char *key; // 키
    char *data; // 데이터
} Blocks;


extern void Init(void);
extern int InsertData(char* key, int keySize, char* pBuf, int bufSize);
extern int GetDataByKey(char* key, int keySize, char* pBuf, int bufSize);
extern int RemoveDataByKey(char* key, int keySize);
extern void InitStorage(void);
extern int GetBlocks(Blocks* pBuf, int bufSize);
extern void printBlock();

#endif