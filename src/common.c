#include "common.h"



void print_hex(const u8* ptr, u64 size, u32 bytes_per_line) 
{
    if (ptr == NULL | size == 0 | bytes_per_line == 0) { return; }

    // hex rep 0-15
    const char* hex = "0123456789ABCDEF";
    
    for (u64 i = 0; i < size; i++) 
    {
        u8 val1 = ptr[i] >> 4;      // get upper 4 bits as num b/w 0-15
        u8 val2 = ptr[i] & 0x0F;    // get lower 4 bits as num b/w 0-15
        
        printf("%c%c", hex[val1], hex[val2]);
        
        // Add space or newline appropriately
        if ((i + 1) % bytes_per_line == 0) {
            printf("\n");
        } else if (i < size - 1) {
            printf(" ");
        }
    }

    // Add final newline if we didn't just print one
    if (size % bytes_per_line != 0) {
        printf("\n");
    }
}
