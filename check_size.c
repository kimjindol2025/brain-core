#include "brain_format.h"
#include <stdio.h>
int main() {
    printf("brain_header_t size: %zu bytes\n", sizeof(brain_header_t));
    printf("brain_index_entry_t size: %zu bytes\n", sizeof(brain_index_entry_t));
    printf("brain_data_entry_t size: %zu bytes\n", sizeof(brain_data_entry_t));
    return 0;
}
