#include "bit_vector.h"

#include <stdio.h>
#include <stdlib.h>



bitVec* bitVec_create(void)
{
    bitVec* bvec = malloc(sizeof(bitVec));     

    bvec->arr = genVec_init(1, sizeof(u8), NULL);
    if (!bvec->arr) {
        printf("bvec init: genVec_init failed\n");
        return NULL;
    }

    return bvec;
}
