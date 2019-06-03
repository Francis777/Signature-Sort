#include<stdio.h>
#include<memory>
#include<cmath>
#include<iostream>
#include<stdlib.h>
#include<time.h>
#include<bitset>
using namespace std;

/*
Currently we only accept n = 4,8,16,32 because of implementation details
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

//Hash the given integer to a signature
uint64_t getSig(uint64_t inputInt, int numChunk, int n, int w, uint64_t oddmask, uint64_t evenmask){
    int sizeChunk = w/numChunk;

    uint64_t  oddchunk = inputInt & oddmask;
    uint64_t  evenchunk = inputInt & evenmask;
   
    //cout << "oddchunk:" << endl;
    //cout << bitset<64>(oddchunk) << endl;
    //cout << "evenchunk: " << endl;
    //cout << bitset<64>(evenchunk) << endl;

    //h(x) = (Ax mod 2^k) div 2^(k-l)     
    int sizeDigit = 3*log2(n);
    //cout << "sizeDigit: " << sizeDigit << endl;
    double phiInv = 0.6180339887498948482045868343656;
    uint64_t W = pow(2,sizeChunk);
    uint64_t A = phiInv * W;
    //cout << "A: " << A << endl;

    uint64_t sigOdd = A * oddchunk;
    uint64_t sigEven = A * evenchunk;

    //cout << "sigOdd: " << endl;
    //cout << bitset<64>(sigOdd) << endl;
    //cout << "sigEven: " << endl;
    //cout << bitset<64>(sigEven) << endl;

    //concatenate odd and even digits
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

    return sig;
}

// hashing n *(log n)^epsilon "digit"
// each "digit"  OMEGA(w/(log n)^epsilon) bits -> O(log n) bits
uint64_t* hashing(uint64_t* inputInts,int n,int w)
{
    double epsilon = getepsilon(n);
    uint64_t* sigs = new uint64_t [n];
    int numChunk = rint(pow(log2(n),epsilon));
    int sizeChunk = w/numChunk;

    //construct odd and even masks
    uint64_t oddmask = 0;
    uint64_t evenmask = 0;

    for (int i = 1; i <= numChunk; i ++){
        if (i % 2 == 1){
            uint64_t a = 1;
            oddmask = (oddmask << (2*sizeChunk)) | ((a << sizeChunk) - 1);
        }else{
            uint64_t a = 1;
            evenmask = (evenmask << sizeChunk | ((a << sizeChunk) - 1)) << sizeChunk;
        }
    }

    //compute the hashed signatures
    for (int i = 0; i < n; i++){
        sigs[i] = getSig(inputInts[i],numChunk,n,w,oddmask,evenmask);
    } 
    return sigs;
}


int main()
{
    //input parameters
    int n = 16;
    int w = 64;

    //Generate random integer input
    uint64_t* randomArray = new uint64_t[n];
    cout <<"Input integers:" << endl;
    srand((uint64_t)time(NULL));
    for (int i = 0; i < n; i++){
      randomArray[i] = ((uint64_t)rand() << 32) | rand();
    }
    for (int i = 0; i<n; i++){
      cout << bitset<64>(randomArray[i]) << endl;  
    }
    cout << endl;

    double epsilon = getepsilon(n);
    int numChunk = rint(pow(log2(n),epsilon));
    int digitSize = 3*log2(n);
    int sigSize = digitSize * numChunk; 
    cout << "numChunk: " << numChunk << endl;
    cout << "digitSize: " << digitSize << endl;

    //Hash signatures
    uint64_t* sigs = hashing(randomArray,n,w);

    //Output hashed signatures
    cout << endl << "Signatures:" << endl;
    for (int i = 0; i < n; i++){
      cout << bitset<24>(sigs[i]) << endl;
    }
    cout << endl;

    return 0;    
}
