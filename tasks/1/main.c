#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>

#define MAX_FILE_SIZE 1024

static ucontext_t uctx_main, uctx_func1, uctx_func2;

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define stack_size 1024 * 1024

/**
 * Coroutine body. This code is executed by all the corutines at
 * the same time and with different stacks. So you can store any
 * local variables and arguments right here. Here you implement
 * your solution.
 */
static void
my_coroutine(int id)
{
	printf("func%d: started\n", id);
	if (id == 1) {
	        printf("coroutine1: swapcontext(&uctx_func1, &uctx_func2)\n");
		if (swapcontext(&uctx_func1, &uctx_func2) == -1)
			//sortFile(argv[1],&result[0],&result_size[0]);
	        	handle_error("swapcontext");
	} else {
		printf("coroutine2: swapcontext(&uctx_func2, &uctx_func1)\n");
		if (swapcontext(&uctx_func2, &uctx_func1) == -1)
			//sortFile(argv[2],&result[1],&result_size[1]);
			handle_error("swapcontext");
	}
	printf("func%d: returning\n", id);
}

/**
 * Below you can see 3 different ways of how to allocate stack.
 * You can choose any. All of them do in fact the same.
 */

static void *
allocate_stack_sig()
{
	void *stack = malloc(stack_size);
	stack_t ss;
	ss.ss_sp = stack;
	ss.ss_size = stack_size;
	ss.ss_flags = 0;
	sigaltstack(&ss, NULL);
	return stack;
}

static void *
allocate_stack_mmap()
{
	return mmap(NULL, stack_size, PROT_READ | PROT_WRITE | PROT_EXEC,
		    MAP_PRIVATE, -1, 0);
}

static void *
allocate_stack_mprot()
{
	void *stack = malloc(stack_size);
	mprotect(stack, stack_size, PROT_READ | PROT_WRITE | PROT_EXEC);
	return stack;
}

enum stack_type {
	STACK_MMAP,
	STACK_SIG,
	STACK_MPROT
};

/**
 * Use this wrapper to choose your favourite way of stack
 * allocation.
 */
static void *
allocate_stack(enum stack_type t)
{
	switch(t) {
	case STACK_MMAP:
		return allocate_stack_mmap();
	case STACK_SIG:
		return allocate_stack_sig();
	case STACK_MPROT:
		return allocate_stack_mprot();
	}
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
 
/* UTILITY FUNCTIONS */
/* Function to print an array */
void printArray(int A[], int size)
{
    int i;
    for (i = 0; i < size; i++)
        printf("%d ", A[i]);
    printf("\n");
}

void printArrayFile(int A[], int size, char* filename)
{   
    char* outfile=(char*)malloc((strlen(filename)+1)*sizeof(char));
    strcpy(outfile,filename);
    strcat(outfile,".out");
    printf("Writing array to %s\n",outfile);
    FILE *out=fopen(outfile,"w");	    
    int i;
    for (i = 0; i < size; i++)
        fprintf(out,"%d ", A[i]);
    fprintf(out,"\n");
    fclose(out);
}

/* Sort file */

static void sortFile(char* filename, int** result, int* result_size)
{
    printf("Sorting file %s:\n",filename);
    FILE *file = fopen(filename,"r");
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
    free(buffer);
    printf("Given array is \n");
  //  printArray(arr, arr_size);
 
    mergeSort(arr, 0, arr_size - 1);
    *result_size=arr_size;
    *result=(int*)malloc(arr_size*sizeof(int));
    
    memcpy(*result,arr,arr_size*sizeof(int));
    
    printf("\nSorted array is \n");
  //  printArray(arr, arr_size);
    printArrayFile(arr, arr_size, filename);
//    printf("%d\n",*result_size);
//    printf("%d\n",result);
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
    int i;
    int** result=(int**)malloc(nfiles*sizeof(int *));
    int* result_size=(int*)malloc(nfiles*sizeof(int));
    char* coroutine_stack[nfiles];
    ucontext_t coroutine_context[nfiles];
    
    printf("Creating coroutines\n");
    for(i=0;i<nfiles;i++)
    {
	coroutine_stack[i]=allocate_stack(STACK_SIG);
	if(getcontext(&coroutine_context[i]) == -1) 
		handle_error("getcontext");
       	coroutine_context[i].uc_stack.ss_sp=coroutine_stack[i];
	coroutine_context[i].uc_stack.ss_size = stack_size;
	coroutine_context[i].uc_link=&uctx_main;
        /*if (i==0) {
	   coroutine_context[i].uc_link=&uctx_main;
	} else {
           coroutine_context[i].uc_link=&coroutine_context[i-1];
	}*/
	makecontext(&coroutine_context[i],sortFile,3, argv[i+1],&result[i],&result_size[i]);

    }	    
    /*for(int i=0;i<nfiles;i++) {
       printf("%d\n",&result[i]);
       sortFile(argv[i+1],&result[i],&result_size[i]);
    }*/
    
    /* First of all, create a stack for each coroutine. */
/*	char *func1_stack = allocate_stack(STACK_SIG);
	char *func2_stack = allocate_stack(STACK_MPROT);

	
	 * Below is just initialization of coroutine structures.
	 * They are not started yet. Just created.
	 */
	//if (getcontext(&uctx_func1) == -1)
	//	handle_error("getcontext");
	/*
	 * Here you specify a stack, allocated earlier. Unique for
	 * each coroutine.
	 */
	//uctx_func1.uc_stack.ss_sp = func1_stack;
	//uctx_func1.uc_stack.ss_size = stack_size;
	/*
	 * Important - here you specify, to which context to
	 * switch after this coroutine is finished. The code below
	 * says, that when 'uctx_func1' is finished, it should
	 * switch to 'uctx_main'.
	 */
	/*uctx_func1.uc_link = &uctx_main;
	makecontext(&uctx_func1, sortFile, 3, argv[1],&result[0],&result_size[0]);

	if (getcontext(&uctx_func2) == -1)
		handle_error("getcontext");
	uctx_func2.uc_stack.ss_sp = func2_stack;
	uctx_func2.uc_stack.ss_size = stack_size;*/
	/* Successor context is f1(), unless argc > 1. */
        //uctx_func2.uc_link = (argc < 1) ? NULL : &uctx_func1;
	//makecontext(&uctx_func2, sortFile, 3, argv[2],&result[1],&result_size[1]); 

	/*
	 * And here it starts. The first coroutine to start is
	 * 'uctx_func2'.
	 */
	//printf("main: swapcontext(&uctx_main, &uctx_func2)\n");
	for(i=nfiles;i>=0;i--)
	{  
	   
	   
	   if(i == nfiles) {
              printf("main: swapcontext(main, %s)\n", argv[i]); 
	      if (swapcontext(&uctx_main, &coroutine_context[i-1]) == -1)
		    handle_error("swapcontext");
	   } else if(i == 0) { 
              printf("main: swapcontext(%s, main)\n", argv[i+1]); 
	      if (swapcontext(&coroutine_context[i], &uctx_main) == -1)
                    handle_error("swapcontext");
	   } else { 
              printf("main: swapcontext(%s, %s)\n", argv[i+1],argv[i]); 
              if (swapcontext(&coroutine_context[i],&coroutine_context[i-1]) == -1) 
		   handle_error("swapcontext");
	   }
        } 
    
    
    
    printf("Sorting merged file\n");
    int full_arr_size=0;
    for(int i=0;i<nfiles;i++) {
       full_arr_size+=result_size[i];
    //   printf("%d\n",result_size[i]);
    //   printf("%d\n",&result[i]);
    //   printArray(result[i], result_size[i]);   
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
   // printArray(full_arr, full_arr_size);
 
    mergeSort(full_arr, 0, full_arr_size - 1);
    
    printf("\nSorted array is \n");
    //printArray(full_arr, full_arr_size); 
    printArrayFile(full_arr, full_arr_size, "result_full"); 
    free(full_arr);
    printf("main: exiting\n");
    return 0;
}
