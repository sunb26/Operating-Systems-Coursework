/*
*
* Authors: Benjamin Sun, Matt Wilker
* Course: Operating Systems Comp Sci/SFWRWENG 3SH3
* Assignment: 2
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>     // For memcpy function
#include <fcntl.h>      // For open() system call
#include <sys/mman.h> 


// Macros
#define BUFFER_SIZE 10 // Largest address is 65535
#define OFFSET_MASK 255
#define PAGES 256
#define OFFSET_BITS 8
#define PAGE_SIZE 256
#define MAX_TLB_SIZE 16
#define MEMORY_SIZE 65536
#define PHYS_MEMORY 128 


// Define TLB Entry Struct
struct TLBentry {
    int page_num;
    int frame_num;
};

// Declare TLB
struct TLBentry tlb[MAX_TLB_SIZE];

// Function Declarations
int search_TLB(int pg_num);
void TLB_Add(struct TLBentry *newEntry);
void TLB_Update(struct TLBentry *updateEntry, int old_page_num);
int page_Fault(int page_num);

// Global Declarations for Page Table and TLB
int current_tlb_size = 0;
int oldest_entry = 0;
int page_table[PAGES]; // the -1s represent the address in main memory each page resides in
	 // These are the frame numbers. The index of each is the page number



// Global Declarations needed for Part 3
signed char *mmapPtr;
signed char main_mem[32768];
int next_free_frame = 0;
int back_store;

// Statistics
int page_faults = 0;
int tlb_hits = 0;
int total_addr = 0;

int main(int argc, char *argv[]) {

    // Populate Page Table with -1
    for (int j = 0; j < PAGES; j++)
    {
        page_table[j] = -1;
    }
	
    /*Read characters from the file at a time.
	These characters are stored in buff to read the logical addresses*/
	FILE *fptr = fopen("addresses.txt", "r");
    char buff[BUFFER_SIZE];
    int page_offset;

	while (fgets(buff, BUFFER_SIZE, fptr) != NULL)
	{
		/*While reading the logical addresses using bitwise operators in C to 
        compute the physical address for each logical address read and print it*/
		int page_num = atoi(buff) >> OFFSET_BITS;
		page_offset = atoi(buff) & OFFSET_MASK;
        signed char byte_value;  
        struct TLBentry newEntry;
        int frame_num;

        // Search TLB for existing page
        int tlb_loc = search_TLB(page_num);


        if (tlb_loc != -1)         // TLB Hit
        {
            tlb_hits++;
            struct TLBentry entry = tlb[tlb_loc]; // grab frame number from TLB
            frame_num = entry.frame_num;
        }
        else if (page_table[page_num] != -1)    //TLB Miss - Check Page Table
        {
            frame_num = page_table[page_num];
            newEntry.page_num = page_num;   // Grab frame number from page table and 
            newEntry.frame_num = frame_num; // add new TLB entry with this page and frame pair
            TLB_Add(&newEntry);
            
        }
        else // Not found in TLB or Page Table - Page Fault
        {
            page_faults++;
            int old_page = page_Fault(page_num);

            frame_num = next_free_frame;
            page_table[page_num] = frame_num; // Update page table with new frame info
            newEntry.page_num = page_num;
            newEntry.frame_num = frame_num;

            if (old_page != -1) // if a new page replaced an old page
            {
                page_table[old_page] = -1;
                TLB_Update(&newEntry, old_page);   // Check if removed page is in TLB
            }                                       // and update with new page
            else
            {
                TLB_Add(&newEntry);     // Add new page to TLB
            }
            next_free_frame++;
        }

        // Compute Physical Address and signed byte value (indexed at physical address)
        int phys_addr = (frame_num << OFFSET_BITS)|page_offset;
        byte_value = (signed char) main_mem[phys_addr];
        printf("Virtual Address: %d, Physical Address: %d, Value: %d\n", 
                atoi(buff), phys_addr, byte_value);

        total_addr++;

	}

    // Print Statistics
    printf("Total Addresses: %d\n", total_addr);
    printf("Page Faults = %d\n", page_faults);
    printf("TLB Hits = %d\n", tlb_hits);

	/*Close the files and Free the memory map.*/

	fclose(fptr);
    munmap(mmapPtr, MEMORY_SIZE);
    close(back_store);

	return 0;
}

/*
*   Function takes a page number as an input. The TLB is iterated through
*   and each entry's page number is compared to the input page number.
*   Return the index of the entry if found otherwise return -1.
*/
int search_TLB(int pg_num)
{
    int entryIndex = 0;

    while (entryIndex < current_tlb_size)
    {
        struct TLBentry current = tlb[entryIndex];
        if (current.page_num == pg_num)
        {
            return entryIndex;
        }
        entryIndex++;
    }
    return -1;
}

/*
*   Function takes a TLB Entry as an input. The replacement policy follows FIFO
*   therefore the TLB is first checked to see if it is full. If so, the oldest
*   entry is replaced. The oldest_entry variable keeps track of which index is 
*   the oldest entry to be replaced following FIFO. If the TLB is not full,
*   add the new entry to the next available index and increment the size counter.
*/

void TLB_Add(struct TLBentry *newEntry)
{
    if (current_tlb_size == MAX_TLB_SIZE)
    {
        tlb[oldest_entry] = *newEntry;
        oldest_entry++;
        if (oldest_entry == MAX_TLB_SIZE) oldest_entry = 0; // Once the end of the TLB is reached, the first
                            // element is the oldest so oldest_entry is reset to the start of the TLB.
    }
    else
    {
        tlb[current_tlb_size] = *newEntry;
        current_tlb_size++;
    }
}

/*
*   Function takes a TLB Entry and the page number that was replaced
*   in physical memory as inputs. The page number being replaced is searched for 
*   in the TLB. Once the index of the replaced page is found in the TLB, the entry
*   in the TLB is replaced with the new TLB entry containing the new page:frame pair.
*   If the removed page was not already in TLB, just add the new page in normally
*   applying FIFO if necessary.
*/
void TLB_Update(struct TLBentry *updateEntry, int old_page_num)
{
    int entryLocation = search_TLB(old_page_num);

    if (entryLocation != -1)
    {
        tlb[entryLocation] = *updateEntry;
    }
    else
    {
        TLB_Add(updateEntry);
    }
}

/*
*   Function takes a page number as an input. The .bin file is opened and the contents
*   are mapped to RAM. Since the physical memory follows FIFO replacement policy
*   the program checks if physical memory is full and resets the next free frame
*   to be overwritten to the start of the physical memory. If a page is replaced
*   in memory due to FIFO, the current page number is noted down in oldPage and then
*   overwritten with the new page. The old page number is returned such that the TLB
*   can be updated if needed. Frame number is globally tracked.
*/
int page_Fault(int page_num)
{
    
    int index = 0;
    int oldPage = -1;

    // Open binary file as read-ony 
    back_store = open("BACKING_STORE.bin", O_RDONLY);

    // This is a pointer to beginning section of RAM (main memory) that begin storing data in
    mmapPtr = mmap(0, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, back_store, 0); 
    
    if (next_free_frame == PHYS_MEMORY) // Reset next frame to be written to if memory is full.
    {
        next_free_frame = 0;
    }
    
    // Check if page is being replaced. If so, keep track of the page number being replaced
    while(index < PAGES)
    {
        if (page_table[index] == next_free_frame)
        {
            oldPage = index;
            break;
        }
        index++;
    }

    // Copy specified page contents from memory map to our "main memory"
    memcpy(main_mem + PAGE_SIZE*next_free_frame, mmapPtr + PAGE_SIZE*page_num, PAGE_SIZE);
    return oldPage;
}