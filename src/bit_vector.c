#include "bit_vector.h"



bitVec* bitVec_create(void)
{
    bitVec* bvec = malloc(sizeof(bitVec));
    CHECK_FATAL(!bvec, "bvec init failed");

    bvec->arr = genVec_init(0, sizeof(u8), NULL, NULL, NULL);

    bvec->size = 0;

    return bvec;
}

void bitVec_destroy(bitVec* bvec)
{
    CHECK_FATAL(!bvec, "bvec is null");

    genVec_destroy(bvec->arr);

    free(bvec);
}

// Set bit i to 1
void bitVec_set(bitVec* bvec, u32 i)
{
    CHECK_FATAL(!bvec, "bvec is null");
    
    u32 byte_index = i / 8; // which byte (elm) 
    u32 bit_index = i % 8; // which bit in the byte
    
    // Ensure byte capacity
    while (byte_index >= bvec->arr->size) {
        u8 zero = 0;
        genVec_push(bvec->arr, &zero);
    }

    u8* byte = (u8*)genVec_get_ptr(bvec->arr, byte_index);
    *byte |= (1 << bit_index);  // Set the bit
    // we create a new 8 bit arr with left shift 
    // it has 1 at the pos we want to set
    // we or it with the arr so the 1 is set
    // we set the bits till the pos where we want 1
    // rest are 0, rhs of 1 is implicitly set, lhs of 1 is explici by <<

    if (i + 1 > bvec->size) { // bits upto ith are considered allocated
        bvec->size = i + 1;  // ith bit is 1 (set)
    }  
}

// Clear bit i (set to 0)
void bitVec_clear(bitVec* bvec, u32 i)
{
    CHECK_FATAL(!bvec, "bvec is null");

    CHECK_FATAL(i >= bvec->size, "index out of bounds");     

    u32 byte_index = i / 8;
    u32 bit_index = i % 8;

    u8* byte = (u8*)genVec_get_ptr(bvec->arr, byte_index);
    *byte &= ~(1 << bit_index);  // Clear the bit
    // we create a new 8 bit arr with left shift 
    // it has 0 at the pos we want to clear (the not puts 0 there and 1 everywhere else)
    // we and it with the arr so 0 is cleared
}

// Test bit i (returns 1 or 0)
u8 bitVec_test(bitVec* bvec, u32 i)
{
    CHECK_FATAL(!bvec, "bvec is null");
    
    CHECK_FATAL(i >= bvec->size, "index out of bounds");

    u32 byte_index = i / 8;
    u32 bit_index = i % 8;

    //u8* byte = (u8*)genVec_get_ptr(bvec->arr, byte_index); 
    return (*genVec_get_ptr(bvec->arr, byte_index) >> bit_index) & 1;  // copy of dereferenced byte data returned
     // create new arr, move needed bit to LSB
    //The `& 1` masks off everything except the LSB: // 1 = 00000001
}

// Toggle bit i
void bitVec_toggle(bitVec* bvec, u32 i)
{
    CHECK_FATAL(!bvec, "bvec is null");
    
    CHECK_FATAL(i >= bvec->size, "index out of bounds");

    u32 byte_index = i / 8;
    u32 bit_index = i % 8;

    u8* byte = (u8*)genVec_get_ptr(bvec->arr, byte_index);
    *byte ^= (1 << bit_index); // lvalue so byte is modified
    // xor with 1 toggles the bit 
    // while with 0 it does nothing
}


void bitVec_push(bitVec* bvec)
{
    bitVec_set(bvec, bvec->size); 
}


void bitVec_pop(bitVec* bvec)
{
    CHECK_FATAL(!bvec, "bvec is null");

    bvec->size--;
    if (bvec->size % 8 == 0) {
        genVec_pop(bvec->arr, NULL);
    }
}

void bitVec_print(bitVec *bvec, u32 byteI)
{
    CHECK_FATAL(!bvec, "bvec is null");

    CHECK_FATAL(byteI >= bvec->arr->size, "index out of bounds");
    
    u8 bits_to_print = 8;
    // If this is the last byte, only print the valid bits
    if (byteI == bvec->arr->size - 1) {
        u32 remaining = bvec->size % 8;
        bits_to_print = (remaining == 0) ? 8 : (u8)remaining;
    }

    for (u8 i = 0; i < bits_to_print; i++) {
        // we print from 0th bit to 7th bit (there are no lsb, msb)
        printf("%d", ((*genVec_get_ptr(bvec->arr, byteI)) >> i) & 1);// we lose data from right
    }

    printf("\n");
}


