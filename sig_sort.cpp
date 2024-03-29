#include <cstdint>    //for uint64_t
#include <cstddef>    //for nullptr
#include <stack>
#include <bitset>
#include <iostream>
#include "MSB64.cpp"
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <memory>
#include <cmath>
#include "packed_sort.cpp"

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
   
    cout << "oddchunk:" << endl;
    cout << bitset<64>(oddchunk) << endl;
    cout << "evenchunk: " << endl;
    cout << bitset<64>(evenchunk) << endl;

    //h(x) = (Ax mod 2^k) div 2^(k-l)     
    int sizeDigit = 2*log2(n);
    //cout << "sizeDigit: " << sizeDigit << endl;
    double phiInv = 0.6180339887498948482045868343656;
    uint64_t W = pow(2,sizeChunk);
    uint64_t A = phiInv * W;
    //cout << "A: " << A << endl;

    uint64_t sigOdd = A * oddchunk;
    uint64_t sigEven = A * evenchunk;

    cout << "sigOdd: " << endl;
    cout << bitset<64>(sigOdd) << endl;
    cout << "sigEven: " << endl;
    cout << bitset<64>(sigEven) << endl;

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

//This function computes the most significant bit of an integer 
unsigned int mss(uint64_t num){
  int mss = highestOneBitIn(num);
  return (mss+1);
}

//This functions computes the power of an integer
int pow(int base, int exp){
  if (exp == 0){
    return 1;
  }
  int temp = pow(base,exp/2);
  if (exp%2 == 0){
    return temp*temp;
  }else{
    return temp*temp*base;
  }
}

//Node structure of a patricia trie
struct Node{
  Node **children;
  int numChildren;
  int childArrSize;
  uint64_t val;
  uint64_t sort_val;
  int len;
  int node_id;
  int sig_idx;        //signature that the node corresponds to

  //Constructor
  Node (uint64_t val,int len,int sig_idx){
    this->children=nullptr;
    this->numChildren=0;
    this->childArrSize=0;
    this->val=val;
    this->len = len;
    this->sig_idx = sig_idx;
  };
};

//Patricia Trie structure
struct PatriciaTrie{
  Node *root;
  stack <Node *> *rightSpline;
  int numChunks;
  int chunkSize;
  int n;
  unsigned int mask;
  int num_edges;
  unsigned int id_bits;

  //Constructor
  PatriciaTrie(int numChunks,int chunkSize){
    root = new Node(0,0,0);
    rightSpline = new stack <Node *>;
    rightSpline->push(root);
    this->numChunks = numChunks;
    this->chunkSize = chunkSize;
    this->n = numChunks*chunkSize;
    this->mask = pow(2,chunkSize)-1;
    this->num_edges = 0;
  };

  //Expand child array size if needed
  void expandArr(Node *parent){
    if (parent->childArrSize==0){
      parent->children = new Node* [4];
      parent->childArrSize = 4;
    } else {
      Node **oldChildren = parent->children;
      parent->children = new Node* [2*(parent->childArrSize)];
      for (int i=0; i<parent->childArrSize;i++){
        parent->children[i] = oldChildren[i];
      }
      parent->childArrSize = 2*(parent->childArrSize);
      delete [] oldChildren;
    }
  }

  //Insert
  void insert(uint64_t sig, int diff, int sig_idx){
    //cout << "Inserting" << endl;
    //find position to insert new node
    cout << n << " " << diff << endl;
    int height = numChunks - (n-diff)/chunkSize;
    cout << height << endl;
    int orig_height = height;
    int popped_height = 0;

    Node *cur_node = nullptr;
    while (height>0){
      cur_node = rightSpline->top();
      rightSpline->pop();
      popped_height += cur_node->len;
      height -= cur_node->len;
    }
    cout << orig_height << " " << height << " " << popped_height << endl;

    if (height==0){
      //insert leaf node to the right of parent
      Node *parent = rightSpline->top();
      Node *node = new Node(sig,orig_height,sig_idx);
      if (parent->numChildren >= parent->childArrSize){
        expandArr(parent);
      }
      parent->children[parent->numChildren] = node;
      parent->numChildren += 1;

      //add node to right spline
      rightSpline->push(node);

    }else{
      //change the length of the popped off node
      cur_node->len = cur_node->len + height;
      //add intermediary node and the new leaf node
      Node *int_node = new Node((cur_node->val) >> (cur_node->len * chunkSize),-height,sig_idx);
      Node *node = new Node(sig,popped_height+height,sig_idx);
      
      //add cur_node and leaf node to intermediary node
      Node *parent = int_node;
      if (parent->numChildren >= parent->childArrSize){
        expandArr(parent);
      }
      parent->children[parent->numChildren] = cur_node;
      parent->numChildren += 1;
      parent->children[parent->numChildren] = node;
      parent->numChildren += 1;
      
      //add intermediary node to its parent on the right spline
      parent = rightSpline->top();
      parent->children[parent->numChildren - 1] = int_node;

      //add both the intermediary and the leaf node to the right spline
      rightSpline->push(int_node);
      rightSpline->push(node);
    }

  };

  //prints the structure of the Patricia Trie
  void printTree(Node *node){
    cout << "Node ID: " << node->node_id << endl;
    //cout << "Node value: " << node->val << endl;
    cout << "Node bitmap: " << bitset<9>(node->val) << endl;
    //cout << "Sort value: " << bitset<9>(node->sort_val) << endl;
    cout << "Sig idx: " << node->sig_idx << endl;
    cout << "len: " << node->len << endl << endl;
    if (node->children != nullptr){
      for (int i = 0; i<node->numChildren;i++){
        printTree((node->children)[i]);
      }
    }
  };

  //label the node id's and chunk values for sorting
  void addInfo(Node *node, int &cur_id){
    //extract the edge value and store it in sort_val
    int sort_val = node->val;

    int shift_len = node->len - 1;
    if (shift_len > 0){
      sort_val = sort_val >> (shift_len*chunkSize);  
    }
    node->sort_val = sort_val & mask;

    //label node id
    node->node_id = cur_id;
    cur_id += 1;

    //recurse on children
    if (node->children != nullptr){
      for (int i = 0; i<node->numChildren; i++){
        num_edges+=1;
        addInfo(node->children[i],cur_id);
      }
    }
  }

  //label each edge for sorting
  void labelEdges(uint64_t *edges, Node *node, int &cur_edge){
    if (node->children != nullptr){
      for (int i = 0; i<node->numChildren; i++){
        edges[cur_edge] = (node->node_id) << (chunkSize+id_bits) | (node->children[i]->sort_val) << id_bits | i;
        cur_edge+=1;
      }
      for (int i = 0; i<node->numChildren; i++){
        labelEdges(edges,node->children[i],cur_edge);
      }
    }
  }

  //construct the edge information for sorting edges
  uint64_t* getEdgeInfo(){
    uint64_t *edges = new uint64_t[num_edges];
    uint64_t num_nodes = num_edges + 1;
    int cur_edge = 0;
    id_bits = mss(num_nodes);
    cout << "Num Nodes: " << num_nodes << endl;
    cout << "Node id bits: " << id_bits << endl;
    labelEdges(edges,root,cur_edge);
    return edges;
  }

  //Reorders the edges in the Patricia Trie (to the correct ordering) from the information obtained after sorting the edge information
  void permuteEdges(uint64_t *edges, int &edge_idx,Node *node){
    cout << "permuting: " << node->node_id << endl;
    if (edge_idx < num_edges){
      unsigned int node_id = edges[edge_idx] >> (id_bits + chunkSize);
      if (node_id == node->node_id){
        cout << "edges permuted: " << node->numChildren << endl;
        int idx = edge_idx;
        edge_idx += node->numChildren;
        //permute children
        for (int i = 0;i<node->numChildren;i++){
          permuteEdges(edges, edge_idx, node->children[i]);
        }
        //permute current node
        Node **newChildren = new Node*[node->childArrSize];
        for (int i = 0;i<node->numChildren;i++){
          unsigned int edge_num = edges[idx+i] & ((1 << id_bits) - 1);
          newChildren[i] = node->children[edge_num];
        }
        delete [] node->children;
        node->children = newChildren;
      }
    }
  }

  //Inorder traversal of the Patricia Trie to retrive the signatures
  void traverse(uint64_t *sortedSigs, int &cur_sig, Node *node){
    if (node->children == nullptr){
      sortedSigs[cur_sig] = node->val;
      cur_sig += 1;
    } else {
      for (int i = 0; i<node->numChildren; i++){
        traverse(sortedSigs, cur_sig, node->children[i]);  
      }
    }
  }
};

uint64_t* sortSignatures(int n,int numChunks,int chunkSize, uint64_t* sigs){
  uint64_t* sortedSigs = new uint64_t[n];
  PatriciaTrie PT = PatriciaTrie(numChunks,chunkSize);
  
  //insert first signature into the Patricia Trie
  (PT.root)->children = new Node*[4];
  (PT.root)->childArrSize = 4;
  Node *node = new Node(sigs[0],numChunks,0); 
  (PT.root)->children[0] = node;
  (PT.root)->numChildren += 1;
  (PT.rightSpline)->push(node);

  //insert the rest of the signatures into Patricia Trie
  for (int i = 1; i<n; i++){
    int diff = mss((sigs[i-1])^(sigs[i]));
    PT.insert(sigs[i],diff,i);
  }
  int cur_id = 1;
  PT.addInfo(PT.root,cur_id);

  cout << "Full Patricia Tree before sorting: " << endl;
  PT.printTree(PT.root);
  cout << "Num edges: " << PT.num_edges << endl;

  //Get value of edges (node id, actual chunk, edge id) to sort
  uint64_t *edges = PT.getEdgeInfo(); 

  //print edge info
  cout << "Edges to sort:" << endl;  
  for (int i = 0; i<PT.num_edges; i++){
    cout << bitset<16>(edges[i]) << endl;
  }
  cout << endl;

  //recursively sort edges
  //#################REPLACE WITH SIGSORT AND PACKED SORT################################
  sort(edges,edges+PT.num_edges);
  
  //Print sorted edges
  cout << "Sorted edges" << endl;
  for (int i = 0; i<PT.num_edges; i++){
    cout << bitset<16>(edges[i]) << endl;
  } 
  cout << endl;

  //Permute edges in original patricia trie
  int edge_idx = 0;
  PT.permuteEdges(edges, edge_idx, PT.root);
  
  //Inorder traversal to fill out sorted signatures
  int cur_sig = 0;
  PT.traverse(sortedSigs,cur_sig,PT.root); 

  return sortedSigs;
}

int main(){
  //Input data
  int n = 8;
  int w = 64;

  //Generate integer input
  uint64_t* randomArray = new uint64_t[n];
  cout <<"Input integers:" << endl;

  /*
  srand((uint64_t)time(NULL));
  for (int i = 0; i < n; i++){
    randomArray[i] = ((uint64_t)rand() << 32) | rand();
  }*/

  randomArray[0] = 123423423467;
  randomArray[1] = 908567975396664;
  randomArray[2] = 23987897454869;
  randomArray[3] = 9871128937934;
  randomArray[4] = 1234;
  randomArray[5] = 758567975394;
  randomArray[6] = 3455345609892;
  randomArray[7] = 6598;

  for (int i = 0; i<n; i++){
    cout << bitset<64>(randomArray[i]) << endl;  
  }
  cout << endl;

  double epsilon = getepsilon(n);
  int numChunk = rint(pow(log2(n),epsilon));
  int digitSize = 2*log2(n);
  int sigSize = digitSize * numChunk; 
  cout << "numChunk: " << numChunk << endl;
  cout << "digitSize: " << digitSize << endl;
  cout << endl;

  //Hash signatures
  uint64_t* hashed_sigs = hashing(randomArray,n,w);

  //Output hashed signatures
  cout << endl << "Signatures:" << endl;
  for (int i = 0; i < n; i++){
    cout << bitset<12>(hashed_sigs[i]) << endl;
  }
  cout << endl;

  //Sort signatures using packed sort


  //Input data
  n=9;
      int numChunks=3;
        int chunkSize=3;
          uint64_t sigs [n];
            sigs[0] = 0;
              sigs[1] = 56;
                sigs[2] = 57;
                  sigs[3] = 62;
                    sigs[4] = 320;
                      sigs[5] = 391;
                        sigs[6] = 447;
                          sigs[7] = 449;
                            sigs[8] = 454;
  
  cout << "Sorted signatures: " << endl;
  
  for (int i=0;i<n;i++){
    cout << sigs[i] << endl;
    cout << bitset<24>(sigs[i]) << endl;
  }

  uint64_t* ans = sortSignatures(n,numChunks,chunkSize,sigs);
  cout << "Sorted integers: " << endl;
  
  for (int i=0;i<n;i++){
    cout << ans[i] << endl;
  }
  
}
