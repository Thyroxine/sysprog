#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/time.h>

#define DEFAULT_FILE_SIZE 1024

static ucontext_t uctx_main;

static char** coroutine_stack;
static int nfiles;
static ucontext_t* coroutine_context;
static struct timeval* times;
static struct timeval* wall_times;
static double delay;
static int *finished;
static int *num_swaps;

#define handle_error(msg) \
   do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define stack_size 1024 * 1024

static double measure_wall_time(int id) {
       struct timeval tval_now, tval_spent;
	gettimeofday(&tval_now, NULL);
	timersub(&tval_now, &wall_times[id], &tval_spent);
	double time_us = 1000000.0*tval_spent.tv_sec + tval_spent.tv_usec;
	printf("Context %d Wall Time elapsed: %.1lf microseconds\n", id, time_us);
	return time_us;
} 

static void execute_swap_coroutines(int i)
{
    //If coroutine with last id
     if(i >= nfiles-1) {
           if(!finished[0]){
               num_swaps[i]++;
           //   printf("swapcontext(%d, %d)\n", nfiles-1, 0);
              if (swapcontext(&coroutine_context[nfiles-1], &coroutine_context[0]) == -1)
		    handle_error("swapcontext");
	    }
	   } else { 
	      if(!finished[i+1]){
              num_swaps[i]++;
            //  printf("swapcontext(%d, %d)\n", i, i+1);
              if (swapcontext(&coroutine_context[i],&coroutine_context[i+1]) == -1) 
		   handle_error("swapcontext");
	      }
	   }
}

static void swap_coroutines(int id) {
	struct timeval tval_now, tval_spent;
	gettimeofday(&tval_now, NULL);
	timersub(&tval_now, &times[id], &tval_spent);
    double time_us = 1000000.0*tval_spent.tv_sec + tval_spent.tv_usec;
	//printf("Context %d Time elapsed: %lf microseconds\n", id, time_us);
	if (time_us > delay) {
		execute_swap_coroutines(id);
		memcpy(&times[id],&tval_now,sizeof(tval_now));
	}
}

/**
 * Below you can see 3 different ways of how to allocate stack.
 * You can choose any. All of them do in fact the same.
 */

static void *
allocate_stack()
{
	void *stack = malloc(stack_size);
	stack_t ss;
	ss.ss_sp = stack;
	ss.ss_size = stack_size;
	ss.ss_flags = 0;
	sigaltstack(&ss, NULL);
	return stack;
}

static void
deallocate_stack(void *stack)
{
	free(stack);
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
 
/* Function to print an array */
void print_array(int A[], int size)
{
    int i;
    for (i = 0; i < size; i++)
        printf("%d ", A[i]);
    printf("\n");
}

void print_array_tofile(int A[], int size, char* filename)
{   
    char* outfile=(char*)malloc((strlen(filename)*2)*sizeof(char));
    strcpy(outfile,filename);
    strcat(outfile,".out");
    printf("Writing sorted array to %s\n",outfile);
    FILE *out=fopen(outfile,"w");
    if (out == NULL) {
    	fprintf(stderr, "Failed to open file %s for writing\n", outfile);
    	exit(1);
    }	    
    int i;
    for (i = 0; i < size; i++)
        fprintf(out,"%d ", A[i]);
    fprintf(out,"\n");
    fclose(out);
    free(outfile);
}
/* This is coroutine body */
/* Sort file */

static void sort_file(char* filename, int id, int** result, int* result_size)
{
    finished[id]=0;
    gettimeofday(&wall_times[id], NULL);
    memcpy(&times[id],&wall_times[id],sizeof(wall_times[id]));
    printf("Sorting file %s: with context id: %d\n",filename, id);
    int buffer_size = DEFAULT_FILE_SIZE;
    int* buffer =(int*)malloc(buffer_size * sizeof(int));
    swap_coroutines(id);
    FILE *file = fopen(filename,"r");
    if (file == NULL) {
    	fprintf(stderr,"Failed to open file %s\n", filename);
    	exit(1);
    }
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
        swap_coroutines(id);
    }
    swap_coroutines(id);
    fclose(file);
    int arr_size=count;
    int* arr=(int*)malloc(arr_size * sizeof(int));
    swap_coroutines(id);
    memcpy(arr,buffer,arr_size*sizeof(int));
    free(buffer);
    swap_coroutines(id);
  //  printf("Given array is \n");
  //  print_array(arr, arr_size);
    printf("Executing merge sorting of file %s\n",filename);
    mergeSort(arr, 0, arr_size - 1);
    swap_coroutines(id);
    *result_size=arr_size;
    *result=(int*)malloc(arr_size*sizeof(int));
    swap_coroutines(id);
    memcpy(*result,arr,arr_size*sizeof(int));
    swap_coroutines(id);
   // printf("\nSorted array is \n");
  //  print_array(arr, arr_size);
    print_array_tofile(arr, arr_size, filename);
//    swap_coroutines(id);
    free(arr);
    measure_wall_time(id);
    printf("exiting coroutine with context id: %d\n",id);
    num_swaps[id]++;
    finished[id]=1;
    coroutine_context[id].uc_link=&uctx_main;
    if (swapcontext(&coroutine_context[id], &uctx_main) == -1)
		    		handle_error("swapcontext");
}

int main(int argc, char* argv[])
{
    struct timeval main_start, main_end, main_spent;
    gettimeofday(&main_start, NULL);
    nfiles=argc-2;
    if (nfiles<1)
    {
	    printf("No files specified!\n");
	    printf("Usage: %s <target latency> <file 1> <file 2> ... <file N>\n",argv[0]);
	    exit(0);
    }	
    delay=strtod(argv[1],NULL);
    printf("Target latency is %lf microseconds\n", delay);
    delay=delay/(double)nfiles;
    printf("Specified coroutine latency is %lf microseconds\n", delay);

    int i;
    int** result=(int**)malloc(nfiles*sizeof(int *));
    int* result_size=(int*)malloc(nfiles*sizeof(int));
    
    finished=(int*)calloc(nfiles,sizeof(int));
    num_swaps=(int*)calloc((nfiles+1),sizeof(int));
    times=(struct timeval*)malloc(nfiles*sizeof(struct timeval));
    wall_times=(struct timeval*)malloc(nfiles*sizeof(struct timeval));
    
    coroutine_stack=(char**)malloc(nfiles*sizeof(char*));
    coroutine_context=(ucontext_t*)malloc(nfiles*sizeof(ucontext_t));
    
    printf("Creating coroutines\n");
    for(i=0;i<nfiles;i++)
    {
	coroutine_stack[i]=allocate_stack();
	if(getcontext(&coroutine_context[i]) == -1) 
		handle_error("getcontext");
       	coroutine_context[i].uc_stack.ss_sp=coroutine_stack[i];
	    coroutine_context[i].uc_stack.ss_size = stack_size;
        if (i==0)
        {
            coroutine_context[i].uc_link=&uctx_main;
	    } else {
           coroutine_context[i].uc_link=&coroutine_context[i-1];
	    }
	makecontext(&coroutine_context[i],sort_file,4, argv[i+2], i,&result[i],&result_size[i]);

    }	    
    printf("Starting coroutines\n");
    num_swaps[nfiles]++;
	if (swapcontext(&uctx_main, &coroutine_context[0]) == -1)
		    handle_error("swapcontext"); 
		    
	int all_finished=0;
	do
	{
		all_finished=1;
		for(i=0;i<nfiles;i++)	
			if(!finished[i])
			{
			printf("Coroutine with context %d not finished, switching to it\n", i);
				all_finished=0;
				num_swaps[nfiles]++;
				if (swapcontext(&uctx_main, &coroutine_context[i]) == -1)
		    		handle_error("swapcontext");
		    } 
	} while (!all_finished);
	if (all_finished) 
	    printf ("All coroutines finished! \n");
    printf("Total swaps:\n");
    for(int i=0;i<=nfiles;i++) {
        if(i<nfiles)
            printf("Coroutine with context id %d: %d\n",i, num_swaps[i]);
        else
            printf("Main: %d\n", num_swaps[i]);
    }
	/*for(i=nfiles;i>=0;i--)
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
        } */


    printf("Sorting merged file\n");
    int full_arr_size=0;
    for(int i=0;i<nfiles;i++) {
       full_arr_size+=result_size[i]; 
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
    
    //printf("Given array is \n");
   // print_array(full_arr, full_arr_size);
 
    mergeSort(full_arr, 0, full_arr_size - 1);
    
    //printf("\nSorted array is \n");
    //print_array(full_arr, full_arr_size); 
    print_array_tofile(full_arr, full_arr_size, "result_full.txt");
    for(i=0;i<nfiles;i++)
    	deallocate_stack(coroutine_stack[i]);
    free(full_arr);
    free(num_swaps);
    free(finished);
    free(times);
    free(wall_times);
    free(coroutine_stack);
    free(coroutine_context);
    gettimeofday(&main_end, NULL);
    timersub(&main_end, &main_start, &main_spent);
    double time_us = 1000000.0*main_spent.tv_sec + main_spent.tv_usec;
    printf("Wall Time elapsed: %.1lf microseconds\n", time_us);
    printf("main: exiting\n");
    return 0;
}
