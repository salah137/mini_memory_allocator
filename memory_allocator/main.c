#include <sys/mman.h>
#include <stdio.h>
#include <stddef.h>

struct
{
    void *address;
    size_t length;
    int freed;

} typedef AllocatedRegion;

AllocatedRegion allocated_regions[1024];

int allocated_regions_count = 0;
int total_allocated_memory = 0;

int last_allocated_index = 0;

void *mapMemory()
{
    size_t length = 4096; // Length of the memory region
    void *addr = mmap(
        0,
        length,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0);

    if (addr == MAP_FAILED)
    {
        return (void *)-1; // mmap failed
    }

    return addr;
}

void *baseAdress = 0;

AllocatedRegion *allocate(size_t size)
{
    if (baseAdress == 0)
    {
        baseAdress = mapMemory();
        if (baseAdress == (void *)-1)
        {
            perror("dafakkkk");
        }
    }

    // fill the first place
    if (allocated_regions_count == 0)
    {
        AllocatedRegion first_region = {
            .address = baseAdress,
            .length = size,
            .freed = 0};

        allocated_regions[0] = first_region;
        allocated_regions_count++;
        return &allocated_regions[0];
    }
   
    // see if there is any free already initialezed container
    for (int i = 0; i < allocated_regions_count; i++)
    {
        if (allocated_regions[i].freed &&
            allocated_regions[i].length >= size)
        {
            allocated_regions[i].freed = 0;
            allocated_regions[i].length = size;
            return &allocated_regions[i];
        }
    }

    // fill the gaps
    for (int i = 0; i + 1 < allocated_regions_count; i++)
    {
        char *end_i = (char *)allocated_regions[i].address +
                      allocated_regions[i].length;

        char *start_next = (char *)allocated_regions[i + 1].address;

        if (start_next - end_i >= (ptrdiff_t)size)
        {
            if (end_i + size > (char *)baseAdress + 4096)
                return NULL;

            for (int j = allocated_regions_count; j > i + 1; j--)
                allocated_regions[j] = allocated_regions[j - 1];

            allocated_regions[i + 1] = (AllocatedRegion){
                .address = end_i,
                .length = size,
                .freed = 0};

            allocated_regions_count++;
            return &allocated_regions[i + 1];
        }
    }

    // fill up in the end
    AllocatedRegion *last =
        &allocated_regions[allocated_regions_count - 1];

    char *new_addr =
        (char *)last->address + last->length;

    if (new_addr + size > (char *)baseAdress + 4096)
        return NULL;

    allocated_regions[allocated_regions_count] = (AllocatedRegion){
        .address = new_addr,
        .length = size,
        .freed = 0};

    allocated_regions_count++;
    return &allocated_regions[allocated_regions_count - 1];
}

void freeTheContainer(void *addess)
{
    for (int i = 0; i < allocated_regions_count; i++)
    {
        if (allocated_regions[i].address == addess)
        {
            allocated_regions[i].freed = 1;
            if ( i+1 < allocated_regions_count && allocated_regions[i+1].freed == 1 ){
                size_t new = allocated_regions[i+1].length + allocated_regions[i].length;
                for(int j  = i + 1; j<allocated_regions_count-1; j++){
                    allocated_regions[j] = allocated_regions[j+1];
                }
                
                allocated_regions[i].length = new;
            }

            if (i> 0 && allocated_regions[i-1].freed == 1 ){
                size_t new = allocated_regions[i-1].length + allocated_regions[i].length;
                allocated_regions[i-1].length = new;
               
                for(int j  = i; j<allocated_regions_count-1; j++){
                    allocated_regions[j] = allocated_regions[j+1];
                }
                
            }
            break;
        }
    }
}

int main() {
    printf("=== Allocation Test ===\n");

    // Allocate blocks
    char *a = allocate(4)->address;
    char *b = allocate(6)->address;
    char *c = allocate(2)->address;

    printf("Allocated blocks:\n");
    printf("A: %p\n", a);
    printf("B: %p\n", b);
    printf("C: %p\n", c);

    // Free the middle block
    freeTheContainer(b);
    printf("\nFreed block B\n");

    // Allocate smaller block (tests splitting)
    char *d = allocate(3)->address;
    printf("Allocated D (3 bytes) in place of B: %p\n", d);

    // Allocate another block to check leftover free space
    char *e = allocate(2)->address;
    printf("Allocated E (2 bytes) in leftover space: %p\n", e);

    // Free adjacent blocks to test coalescing
    freeTheContainer(d);
    freeTheContainer(e);
    printf("\nFreed D and E to test coalescing\n");

    // Allocate large block that fits into coalesced space
    char *f = allocate(5)->address;
    printf("Allocated F (5 bytes) into coalesced space: %p\n", f);

    return 0;
}
