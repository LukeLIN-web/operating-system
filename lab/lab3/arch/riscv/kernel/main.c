#include "test.h"
extern test();
extern init();

int main(){
	puts("ZJU OS LAB 3 \n");
    init();
    test();
    while(1){}
    return 0;
}