
/* C program for Merge Sort */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILE_SIZE 1024

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
 
/* UTILITY FUNCTIONS */
/* Function to print an array */
void printArray(int A[], int size)
{
    int i;
    for (i = 0; i < size; i++)
        printf("%d ", A[i]);
    printf("\n");
}
 
/* Sort file */

void sortFile(char* filename, int** result, int* result_size)
{
    FILE *file = fopen(filename,"r");
 //int arr_size = sizeof(arr) / sizeof(arr[0]);
    int buffer_size = MAX_FILE_SIZE;
    int* buffer =(int*)malloc(buffer_size * sizeof(int));
    int number,count;
    count = 0;
    while (fscanf(file, "%d", &number) > 0)
    {
        if (count == buffer_size)
        {
            buffer_size *= 2;
            buffer = (int*)realloc(buffer, buffer_size * sizeof(int));
        }
        buffer[count++] = number;
    }
    
    fclose(file);
    int arr_size=count;
    int* arr=(int*)malloc(arr_size * sizeof(int));
    memcpy(arr,buffer,arr_size*sizeof(int));
 //int arr[] = { 12, 11, 13, 5, 6, 7 };
    free(buffer);
    printf("Given array is \n");
    //printArray(arr, arr_size);
 
    mergeSort(arr, 0, arr_size - 1);
    *result_size=arr_size;
    *result=(int*)malloc(arr_size*sizeof(int));
    
    memcpy(*result,arr,arr_size*sizeof(int));
    
    printf("\nSorted array is \n");
    //printArray(arr, arr_size);
    printf("%d\n",*result_size);
    printf("%d\n",result);
    free(arr);
}
 
 
/* Driver code */
int main(int argc, char* argv[])
{
    int nfiles=argc-1;
    if (nfiles<1) {
	printf("No files specified!\n");
	exit(0);
    }
    
    int** result=(int**)malloc(nfiles*sizeof(int *));
    int* result_size=(int*)malloc(nfiles*sizeof(int));
    for(int i=0;i<nfiles;i++) {
       printf("Sorting file %s:\n",argv[i+1]);
       printf("%d\n",&result[i]);
       sortFile(argv[i+1],&result[i],&result_size[i]);
    }
    
    printf("Sorting merged file\n");
    int full_arr_size=0;
    for(int i=0;i<nfiles;i++) {
       full_arr_size+=result_size[i];
//       printf("%d\n",result_size[i]);
//       printf("%d\n",&result[i]);
//       printArray(result[i], result_size[i]);   
    }
    
    int* full_arr=(int*)malloc(full_arr_size*sizeof(int));
    int offset=0;
    for(int i=0;i<nfiles;i++) {
    	memcpy(&full_arr[offset], result[i],result_size[i]*sizeof(int));
    	offset+=result_size[i];
    	free(result[i]);
    }
    free(result);
    free(result_size);
    
    printf("Given array is \n");
    printArray(full_arr, full_arr_size);
 
    mergeSort(full_arr, 0, full_arr_size - 1);
    
    printf("\nSorted array is \n");
    printArray(full_arr, full_arr_size); 
    free(full_arr);
    return 0;
}
