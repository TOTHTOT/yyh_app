#include <stdio.h>
#include <string.h>

void fun(char *str1 , char *str2)
{   
    int len_str,i,j = 0 ;
    len_str =  strlen(str1);
    // printf("len = %d", len_str);
    for (i = len_str;  i > 0;i--)
    {
        str2[j++] = str1[len_str];
        // j++;
        printf("%c ", str1[len_str]);
    }
}

int main(void)
{
    char str[] = "welcome to cvte";
    char str2[25] = {0};

    fun(&str, &str2);

    printf("str:%s\nstr2:%s\n", str, str2);
}
