/**
 * 自定义用户报错函数 
 */
#include <stdio.h>
#include <stdlib.h>


#define errExit(msg) do {                                                            \
        printf("%s:%s:%d:%s\n",__FILE__, __func__, __LINE__ , msg); \
        exit(EXIT_FAILURE);                                         \
    } while (0);