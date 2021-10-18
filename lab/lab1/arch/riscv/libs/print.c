#include "defs.h"
extern uint64_t sbi_call();


int puts(char* str){
	int i = 0 ;
	while( str[i] != '\0' ){
		sbi_call(1, str[i],0,0 );
		i += 1;
	}
    return 0;
}

int put_num(uint64_t n){
	//convert  unsigned long long uint64_t to string
	char * str ;
	int i = 0;
	int k = 0; 
	char temp;//临时变量，交换两个值时用到
	char index[] = "0123456789";
	uint64_t unum = n ;
	unsigned int radix = 10 ;
	str[i++] = index[unum % radix];
	unum /= radix;
	while(unum > 0){
		str[i++] = index[unum % radix];
		unum = unum/  radix;
	}
    for(int j = k;j <= (i-1)/2; j++)//头尾一一对称交换，i其实就是字符串的长度，索引最大值比长度少1
    {
        temp=str[j];//头部赋值给临时变量
        str[j]=str[i-1+k-j];//尾部赋值给头部
        str[i-1+k-j]=temp;//将临时变量的值(其实就是之前的头部值)赋给尾部
    }
	str[i] = '\0';
	puts(str) ;
	return 0;
}