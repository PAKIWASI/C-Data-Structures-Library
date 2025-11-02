

#include "bit_vector.h"
int main(void)
{
    bitVec* bvec = bitVec_create();    

    bitVec_push(bvec);
    bitVec_push(bvec);
    bitVec_push(bvec);
    bitVec_push(bvec);
    bitVec_push(bvec);
    bitVec_push(bvec);
    bitVec_push(bvec);
    bitVec_push(bvec);

    bitVec_pop(bvec);
    bitVec_pop(bvec);

    bitVec_push(bvec);
    bitVec_push(bvec);
    bitVec_push(bvec);

    bitVec_print(bvec, 0);
    bitVec_print(bvec, 1);

    bitVec_destroy(bvec);
}
