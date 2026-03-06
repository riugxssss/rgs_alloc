#include <stdio.h>
#include "rgs_alloc.h"
#include <string.h>

int main(void){

    char *p1 = (char *)rgs_allocator(20);
    if (p1 == NULL){
        printf("alloc error\n");
        return -1;
    }
    
    int *p2 = (int *)rgs_callocator(3, sizeof(int));
    
    if (!p1 || !p2) {
        printf("Failed allocation\n");
        return 1;
    }
    printf("%s | %d\n", FIT_NAME, ALLOC_STRATEGY);
    printf("%d\n", p2[0]);
    strcpy(p1, "blocdaiwdiaaiwdawi");
    rgs_free(p1);
    
    
   printf("all good\n");
 

    return 0;
}
