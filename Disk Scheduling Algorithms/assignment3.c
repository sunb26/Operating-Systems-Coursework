/*
*
* Authors: Benjamin Sun, Matt Wilker
* Course: Operating Systems Comp Sci/SFWRWENG 3SH3
* Assignment: 3
*
* Citations: https://www.geeksforgeeks.org/merge-sort/
*/



#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/mman.h> 


#define MEMORY_SIZE 80 // number of requests * 4 byte integer
#define NUM_REQ 20 // Number of requests
#define MAX_CYLINDER 299 // largest cylinder index

// Function Declarations
void merge(int arr[], int l, int m, int r);
void mergeSort(int arr[], int l, int r);
void fcfs(int head, int requests[]);
void scan(int head, int requests[], char *direction);
void look(int head, int requests[], char* direction);
void printRequests(int first, int requests[], char* direction); // Used to print request order

int main(int argc, char *argv[])
{
    
    int head = atoi(argv[1]);
    char *direction = argv[2]; // Take command line inputs
    int requests[NUM_REQ]; // Requests int array
    int sorted_requests[NUM_REQ]; // Requests sorted in ascending order
    signed char *mmapPtr;

    printf("Total Requests = %d\n", NUM_REQ);
    printf("Initial Head Position: %d\n", head);
    printf("Direction of Head: %s\n", direction);

    int back_store = open("request.bin", O_RDONLY); // open bin file

    // This is a pointer to beginning section of RAM (main memory) that begin storing data in
    mmapPtr = mmap(0, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, back_store, 0);

    for (int i = 0; i < NUM_REQ; i++) {
        memcpy(requests+i, mmapPtr+4*i, 4);
        memcpy(sorted_requests+i, mmapPtr+4*i, 4); // Populating both request arrays
    }

    // Sorting
    mergeSort(sorted_requests, 0, NUM_REQ-1);
    
    // Algorithms
    fcfs(head, requests);
    scan(head, sorted_requests, direction);
    look(head, sorted_requests, direction);

    // Close files
    munmap(mmapPtr, MEMORY_SIZE);
    close(back_store);
}

// First come first serve request algorithm
void fcfs(int head, int requests[])
{
    printf("\nFCFS DISK SCHEDULING ALGORITHM\n");

    // Printing request order
    for (int i=0; i<NUM_REQ; i++)
    {
        printf("%d, ", requests[i]); // Print all requests in the order they are stored
    }

    int first = head;
    int movements = 0;
    for (int i=0; i<NUM_REQ; i++)
    {
        movements = movements + abs(first-requests[i]); // Calculate head movements
        first = requests[i];
    }
    printf("\nFCFS - Total head movements = %d\n\n", movements);
}

void scan(int head, int requests[], char* direction)
{
    printf("SCAN DISK SCHEDULING ALGORITHM\n");
    int first = head;
    int movements = 0;

    if (strcmp(direction, "LEFT") == 0)
    {
        
        if (first >= requests[NUM_REQ-1]) movements = first;
        else movements = first + requests[NUM_REQ-1];
    }
    else if (strcmp(direction, "RIGHT") == 0)
    {
        if (first <= requests[0]) movements = abs(requests[NUM_REQ-1] - first);
        else movements = abs(first-MAX_CYLINDER) + abs(MAX_CYLINDER-requests[0]);
    }
    printRequests(first, requests, direction); // Print requests in the correct order
    printf("\nSCAN - Total head movements = %d\n\n", movements);
}


void look(int head, int requests[], char* direction)
{
    printf("LOOK DISK SCHEDULING ALGORITHM\n");
    int first = head;
    int movements = 0;


    if (strcmp(direction, "LEFT") == 0)
    {
        if (first >= requests[NUM_REQ-1]) movements = abs(first-requests[0]);
        else movements = abs(first-requests[0]) + abs(requests[NUM_REQ-1]-requests[0]);
    }
    else if (strcmp(direction, "RIGHT") == 0)
    {
        if (first <= requests[0]) movements = abs(requests[NUM_REQ-1] - first);
        else movements = abs(first-requests[NUM_REQ-1]) + abs(requests[NUM_REQ-1]-requests[0]);
    }
    printRequests(first, requests, direction); // Print requests in the right order
    printf("\nLOOK - Total head movements = %d\n\n", movements);
}

// Merges two subarrays of arr[].
// First subarray is arr[l..m]
// Second subarray is arr[m+1..r]
void merge(int arr[], int l, int m, int r)
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;
 
    /* create temp arrays */
    int L[n1], R[n2];
 
    /* Copy data to temp arrays L[] and R[] */
    for (i = 0; i < n1; i++)
        L[i] = arr[l + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[m + 1 + j];
 
    /* Merge the temp arrays back into arr[l..r]*/
    i = 0; // Initial index of first subarray
    j = 0; // Initial index of second subarray
    k = l; // Initial index of merged subarray
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        }
        else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }
 
    /* Copy the remaining elements of L[], if there
    are any */
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }
 
    /* Copy the remaining elements of R[], if there
    are any */
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}
 
/* l is for left index and r is right index of the
sub-array of arr to be sorted */
void mergeSort(int arr[], int l, int r)
{
    if (l < r) {
        // Same as (l+r)/2, but avoids overflow for
        // large l and h
        int m = l + (r - l) / 2;
 
        // Sort first and second halves
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);
 
        merge(arr, l, m, r);
    }
}

void printRequests(int first, int requests[], char* direction)
{
    int index = 0;
    // Printing request order
    for (int i=0; i<NUM_REQ; i++)
    {
        if (requests[i] > first)
        { 
            index = i;
            break;
        } 
    }
    printf("%d", first);
    if (strcmp(direction, "LEFT") == 0) // print requests to the left of head first then print to the right of head
    {
        for (int i=index-1; i >= 0; i--) 
        {
            if (requests[i] != first) printf(", %d", requests[i]);
        }
        for (int i=index; i<NUM_REQ; i++) 
        {
            if (requests[i] != first) printf(", %d", requests[i]);
        }
    }
    else if (strcmp(direction, "RIGHT") == 0) // print requests to the right of head first then print to the left of head
    {
        for (int i=index; i<NUM_REQ; i++) 
        {
            if (requests[i] != first) printf(", %d", requests[i]);
        }
        for (int i=index-1; i >= 0; i--) 
        {
            if (requests[i] != first) printf(", %d", requests[i]);
        }
    }

}