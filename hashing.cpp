#include<stdio.h>
#include<memory>
#include<cmath>
#include<iostream>
#include<stdlib.h>
#include<time.h>
#include<bitset>
using namespace std;

uint64_t*  hashing(uint64_t*  inputInts,int n);
uint64_t getSig(uint64_t inputInt, int numChunk, int n);
uint64_t createMask(int a, int b);
double getepsilon(int n);


// hashing n *(log n)^epsilon "digit"
// each "digit"  OMEGA(w/(log n)^epsilon) bits -> O(log n) bits

uint64_t* hashing(uint64_t*  inputInts,int n)
{
    double epsilon = getepsilon(n);
    uint64_t* sigs = new uint64_t [n];
    int numchunk = rint(pow(log2(n),epsilon));
    for (int i = 0; i < n; i++){
        sigs[i] = getSig(inputInts[i],numchunk,n);
    } 
    return sigs;
}

uint64_t getSig(uint64_t inputInt, int numChunk, int n){
    int w = 64;
    int sizeChunk = w/numChunk;

    /*
       odd and even chunk
     */    

    uint64_t oddmask = 0;
    uint64_t evenmask = 0;

    cout << "numChunk: " << numChunk << endl;
    cout << "sizeChunk: " << sizeChunk << endl;

    for (int i = 1; i <= numChunk; i ++){
        if (i % 2 == 1){
            // odd mask
            //oddmask += createMask((i-1)*sizeChunk, i*sizeChunk-1);
            uint64_t a = 1;
            oddmask = (oddmask << (2*sizeChunk)) | ((a << sizeChunk) - 1);
        }else{
            // even mask
            //evenmask += createMask((i-1)*sizeChunk, i*sizeChunk-1);
            uint64_t a = 1;
            evenmask = (evenmask << sizeChunk | ((a << sizeChunk) - 1)) << sizeChunk;
        }
    }
 
    cout << "oddmask:" << endl;
    cout << bitset<64>(oddmask) << endl;
    cout << "evenmask: " << endl;
    cout << bitset<64>(evenmask) << endl;
 
    uint64_t  oddchunk = inputInt & oddmask;
    uint64_t  evenchunk = inputInt & evenmask;
   
    cout << "oddchunk:" << endl;
    cout << bitset<64>(oddchunk) << endl;
    cout << "evenchunk: " << endl;
    cout << bitset<64>(evenchunk) << endl;


    /*
       h(x) = (ax mod 2^k) div 2^(k-l)
     */
     
    int sizeDigit = 3*log2(n);
    cout << "sizeDigit: " << sizeDigit << endl;
    double phiInv = 0.6180339887498948482045868343656;
    uint64_t W = pow(2,sizeChunk);
    uint64_t A = phiInv * W;

    uint64_t sigOdd = A * oddchunk;
    uint64_t sigEven = A * evenchunk;

    cout << "sigOdd: " << endl;
    cout << bitset<64>(sigOdd) << endl;
    cout << "sigEven: " << endl;
    cout << bitset<64>(sigEven) << endl;
    /*
      concatenate odd and even digits
     */
    uint64_t sig = 0;
    uint64_t sigtoadd = 0;
  
    uint64_t a = 1;
    uint64_t maskRecover = (a << sizeDigit) - 1;

    for (int i = 1; i <= numChunk; i++){
        if (i % 2 == 1){
            sigOdd = sigOdd >> (sizeChunk - sizeDigit);
            sigtoadd =  maskRecover & sigOdd;
            sigOdd = sigOdd >> (sizeChunk + sizeDigit);
        }else{
            sigEven = sigEven >> (2*sizeChunk -sizeDigit);
            sigtoadd = maskRecover & sigEven;
            sigEven = (sigEven >> sizeDigit);
        }
        sig = sig | (sigtoadd << ((i-1)*sizeDigit));
    }

    cout << "sig: " << endl;
    cout << bitset<64>(sig) << endl;
    return sig;
    
}

uint64_t createMask(int a, int b){
    uint64_t r = 0;
    for (int i=a; i<=b; i++)
        r |= 1 << i;
    return r;
}


/*
here we only accept n = 4,8,16,32
epsilon are  hard-coded for each input size
*/
double getepsilon(int n){
    if (n == 4){
        return 1.;
    }else if (n == 16){
        return 0.5;
    }else if (n == 8){
        return 1/log2(3);
    }else{
        return 0.05;
    }
}

int main()
{
    int sizeInput = 16;
    uint64_t* randomArray = new uint64_t[sizeInput];

    cout <<"Input integers:" << endl;
    srand((uint64_t)time(NULL));

    for (int i = 0; i < sizeInput; i++){
      randomArray[i] = rand();
    }
    randomArray[0] = 324737983498;
    randomArray[1] = 2342342341111;

    for (int i = 0; i< sizeInput;i++){
      cout << randomArray[i] << endl;  
    }

    cout << endl << "Signatures:" << endl;

    uint64_t* sigs = hashing(randomArray,sizeInput);


    for (int i = 0; i < sizeInput; i++)
        {
            cout << sigs[i] << endl;
        }
    
    return 0;    
}
