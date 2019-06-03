#include<stdio.h>
#include<vector>
#include <cstdint>
#include <new>
#include <cmath>
#include <iostream>
#include "radix_sort.cpp"

#include<time.h>
#include<stdlib.h>

using namespace std; 

// const uint64_t bit64 = 1 << 63;
const uint64_t biggest = 9223372036854775807;
//helper function
void compAndSwap(uint64_t  a[], int i, int j, int dir) 
{ 
    if (dir == (a[i]>a[j])){
        uint64_t  temp = a[j];
        a[j] = a[i];
        a[i] = temp; 
    }
        
} 


void de_pack(uint64_t a[], uint64_t p[], int low, int cnt, int b){

	int k = 64/2/(b+1);
	int num = cnt/k;
	uint64_t mask_num = pow(2,b) - 1;

	for (int i = 0; i<cnt; i++){
		a[i]=0;
	}

	//unpack
	int count;
	for (int i = 0; i < num; i++){
		uint64_t temp = 0;
		for(int j = 0; j <  k; j++){
			count = (i + 1)* k -j + low -1;
			temp = p[i] & mask_num; 
			p[i] = p[i] >> (b+1);
			a[count] = temp;
		}
	}

}


void pack (uint64_t a[],  uint64_t p[], int low, int cnt, int b){
	int k = 64/2/(b+1);
	int num = cnt/k;

	int count = low;
	for (int i = 0; i < num; i++){
		uint64_t temp = 0;
		for(int j = 0; j <  k; j++){
			temp = temp | a[count];
			temp = temp << (b+1);
			count += 1;
		}
		temp = temp >> (b+1);
		p[i] = temp;

	}
}


uint64_t reverse(uint64_t in, uint64_t mask[], int layer,int b){

	int k = 64/2/(b+1);
	int shift = (k/2) * ( b + 1);
	uint64_t result = in;
	for (int i = 0; i < layer; i++){

		uint64_t left = result & (~(mask[i]));
		left = left >> shift;
		uint64_t right = result & (mask [i]);
		right = right << shift;
		result = left | right;

		shift /= 2;
	}
	return result;
}

//create masks
void create_masks(uint64_t mask[], uint64_t mask_swap[], uint64_t mask_pair[], int b){
	uint64_t mask_num = pow(2,b) - 1;//b 1s
	int k = 64/2/(b+1);
	int layer =  log2(k);

	//reverse mask
	int layer_num = 1; //num of small parts
	int shift  = k /2 *(b + 1);
	for (int i = 0; i < layer; i++){
		uint64_t temp = 0;
		for (int j = 0; j < layer_num; j++){
			for (int m = 0; m < shift/(b+1); m++){
				temp = temp | mask_num;
				temp = temp << (b + 1);
				// printf("%\n");
			}
			temp = temp << shift;
		}
		temp = temp >> (b+1);//back for the extra shift 
		temp = temp >> shift;//shift back
		layer_num *= 2;
		shift /= 2;
		mask[i] = temp;
	}

	uint64_t shift_swap = k;
	uint64_t pairs = 1;
	//this mask add 1 to left A words
	for (int i = 0; i < layer + 1 ; i++){
		uint64_t temp = 0;
		for (int j = 0; j < pairs; j ++){
			for (int m = 0; m < shift_swap; m++){
				temp = temp << 1;
				temp = temp | 1;
				temp = temp << b;
			}
			temp = temp << (shift_swap * (b + 1));
		}
		shift_swap /=  2;
		pairs *= 2;
		mask_swap[i] = temp;
	}

	//this mask is used to mask right part of pairs
	pairs = 1;
	shift_swap = k;
	for (int i = 0; i < layer + 1 ; i++){
		uint64_t temp = 0;
		for (int j = 0; j < pairs; j ++){
			for (int m = 0; m < shift_swap; m++){
				temp = temp << (b + 1);
				temp = temp | mask_num;
			}
			temp = temp << (shift_swap * (b + 1));
		}
		shift_swap /=  2;
		pairs *= 2;
		mask_pair[i] = temp;
	}

}

//Merge two soerted words in O(lgk) time
void Merge2(uint64_t input_0, uint64_t input_1, uint64_t result[], int b, uint64_t mask[], uint64_t mask_swap[], uint64_t mask_pair[]){
	
	uint64_t mask_num = pow(2,b) - 1;//b 1s
	int k = 64/2/(b+1);

	//reverse
	int layer =  log2(k);

	uint64_t shift_swap = k;
	uint64_t pairs = 1;


	uint64_t word_combined = input_0 << 32 | (reverse(input_1,mask,layer,b));
	pairs = 1;
	shift_swap = k;
	for (int i = 0; i < layer + 1; i ++){

		// mask out number
		uint64_t input_a = word_combined & (mask_pair[i]);
		uint64_t input_b = word_combined & (~mask_pair[i]);
		// add 1 to front of a
		input_a = input_a | mask_swap[i];
		//remove former 1 before front of b
		input_b = input_b & (~mask_swap[i] >> shift_swap * (b + 1));
		//shift a to the same position
		input_a = input_a >> (shift_swap *(b+1));


		
		//add
		uint64_t mask_a = input_a - input_b;
		mask_a = mask_a & (mask_swap[i] >> (shift_swap *(b+1)));
		mask_a = mask_a - (mask_a>>b);


		uint64_t temp_a = (input_a & (~mask_a)) | (input_b &(mask_a));
		uint64_t temp_b = (input_b & (~mask_a)) | (input_a &(mask_a));
		temp_a = temp_a << (shift_swap*(b+1));
		word_combined = temp_a | temp_b;

		shift_swap /= 2;
	}

	uint64_t num_a = word_combined >> (k * (b+1));
	uint64_t num_b = word_combined << (k * (b+1));
	num_b = num_b >> (k * (b+1));

	result[0] = num_a;
	result[1] = num_b;
	
}

//for debug
void print_packed_array(uint64_t a[], int low, int cnt, int b){
	int k = 64/2/(b+1);
	int num = cnt/k;

	uint64_t a_backup[num];
	for(int i = 0; i < num; i++){
		a_backup[i] = a[low + i];
	}	
	uint64_t result[cnt];
	de_pack(result,a_backup,0,cnt,b);
	printf("it's: "); 
    for (int i=0; i < cnt; i++) 
        printf("%llu,", (long long unsigned int)result[i]); 
    printf("\n");
    return; 
}

void Merge(uint64_t p[], int low, int cnt, int b, uint64_t result[], uint64_t mask[], uint64_t mask_swap[], uint64_t mask_pair[]){
	if (cnt == 1) {
		result[0] = p[low];
		return;
	} if (cnt == 2){
		Merge2(p[low],p[low + 1], result,b, mask, mask_swap, mask_pair);
	}
	else{
		uint64_t mask_num = pow(2,b) - 1;

		int k = cnt/2; //midde num
		uint64_t list1[k];
		uint64_t list2[k];
	
		Merge(p, low, k, b, list1,mask, mask_swap, mask_pair);
		Merge(p, low + k, k, b, list2, mask, mask_swap, mask_pair);

		// print_packed_array(list1, 0, k * (32/(b+1)), b);
		// print_packed_array(list2, 0, k * (32/(b+1)), b);

		int i1 = 0;
		int i2 = 0;
		int count = 0;
		uint64_t buffer[2];

		while (i1 != k && i2 != k){
			Merge2(list1[i1],list2[i2], buffer, b, mask, mask_swap, mask_pair);
			result[count] = buffer[0];
			count += 1;
			uint64_t big = buffer[1] & mask_num;
			if (i1 < k -1){
				uint64_t small1 = (list1[i1 + 1] >> (32 - b -1)) & mask_num;

				//put back
				if (big <= small1){
					i2 += 1;
					list1[i1] = buffer[1];
				}else{
					i1 += 1;
					list2[i2] = buffer[1];
				}
			} else if( i1 == k -1) { // i1 = k-1
				uint64_t small2 = (list2[i2 + 1] >> (32 - b -1)) & mask_num;

				//put back
				if (big <= small2){
					i1 += 1;
					list2[i2] = buffer[1];
				}else{
					i2 += 1;
					list1[i1] = buffer[1];
				}

			}

		}

		if (i1 != k){
			for (int j = i1; j < k; j++){
				result[count] = list1[j];
				count += 1;
			}
		} if (i2 != k){
			for (int j = i2; j <k; j++){
				result[count] = list2[j];
				count += 1;
			}
		}

 		return;
	}
}

void sort_k(uint64_t a[],  int low, int cnt, int b) {
	int k = 64/2/(b+1);
	int num = cnt/k;

	for(int i = 0; i < num; i++){
		radixsort(a,i * k,k);
	}
}

//we pack k b-bit interger into a pack
//b can choose 3,7,15
//choose cnt can be divided by k
//{3, 7, 4, 8, 6, 2, 1, 5}
void packed_sort(uint64_t a[],  int low, int cnt, int b){
	
	int k = 64/2/(b+1);
	int num = cnt/k;
	sort_k(a,  low,cnt,b);
	int layer =  log2(k);

	uint64_t mask[layer];
	uint64_t mask_swap[layer + 1];
	uint64_t mask_pair[layer + 1];
	create_masks(mask, mask_swap, mask_pair, b);

	uint64_t p[num];
	pack(a,p,low,cnt,b);
	uint64_t result[num];
	Merge(p, low, num, b, result, mask, mask_swap, mask_pair);
	de_pack(a,result,low,cnt,b);

}


void rand_n(uint64_t a[], int n, uint64_t x){
	srand((int)time(0));
	for (int i = 0; i<n; i++){
		a[i] = rand()%x;
	}
}
/*
// Driver code 
int main() 
{ 
	int n = 128;
	int b = 7;
	uint64_t mask_num = pow(2,b) - 1;
	uint64_t a[n];
	rand_n(a,n, mask_num);

    int N = sizeof(a)/sizeof(a[0]); 
  	
  	printf("Original array: \n"); 
    for (int i=0; i<N; i++) 
    	printf("%lu ", a[i]); 
    printf("\n");

    int up = 1;   // means sort in ascending order 

    // radixsort(a,0,N);
    packed_sort(a, 0, N, 7);
  
    printf("Sorted array: \n"); 
    for (int i=0; i<N; i++) 
        printf("%lu ", a[i]); 
    printf("\n");
    return 0; 
}
*/
