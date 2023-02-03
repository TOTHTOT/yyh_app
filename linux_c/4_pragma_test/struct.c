#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma pack(1)
struct test_st
{
    char a;       // 1字节
    int b;        // 4字节
    char str[10]; // 10字节, 总计17字节
    short c;      // 2字节
} test;
#pragma pack()

int main()
{

    memset((void *)&test, 0, sizeof(test));

    printf("struct len:[%d]\n", sizeof(struct test_st));
    // char a:[49], int b:[892613426], short c:[54], char* str:[]
    strcpy((char *)&test, "1234567890123456");
                                                                    // 1      2345     67     890123456
    printf("char a:[%x], int b:[%x], short c:[%d], char* str:[%s]\ntes_st:[%s]\n", test.a, test.b, test.c, test.str, (char *)&test);

}