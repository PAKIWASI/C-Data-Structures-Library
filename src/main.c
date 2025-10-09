#include "String.h"
#include "parse.h"



int main(void)
{
    String str;
    string_create_onstack(&str, "hello");

    string_append_cstr(&str, "world");

    string_insert_char(&str, 5, ' ');

    string_print(&str);

    string_destroy_fromstk(&str);
    //return parse();
}
