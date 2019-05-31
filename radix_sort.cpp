
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>

using namespace std;
void radixsort(uint64_t a[],int low, int n){
	int count[10];
	uint64_t output[n];
	memset(output,0,sizeof(output));
	memset(count,0,sizeof(count));
	uint64_t max = 0;
	for (int i = 0; i < n; ++i)
	{
		if (a[i + low]>max)
		{
			max = a[i + low];
		}
	}
	int maxdigits = 0;
	while(max){
		maxdigits++;
		max/=10;
	}
	for(int j=0;j<maxdigits;j++){
		for(int i=0;i<n;i++){
			uint64_t t = pow(10,j);
			count[(a[i + low]%(10*t))/t]++;
		}
		int k = 0;
		for(int p=0;p<10;p++){
			for(int i=0;i<n;i++){
				uint64_t t = pow(10,j);
				if((a[i + low]%(10*t))/t==p){
					output[k] = a[i+ low];
					k++;
				}
			}
		}
		memset(count,0,sizeof(count));
		for (int i = 0; i < n; ++i)
		{
			a[i + low] = output[i];
		}
	}
}
