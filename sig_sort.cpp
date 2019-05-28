#include <cstdint>    //for uint64_t
#include <cstddef>    //for nullptr
#include <stack>
#include <bitset>
#include <iostream>
using namespace std;

//This function computes the most significant bit of an integer 
int mss(uint64_t num){
  int mss = 0;
  while (num > 0){
    num >>= 1;
    mss++;
  }
  return mss;
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

struct Node{
  Node **children;
  int numChildren;
  int childArrSize;
  uint64_t val;
  uint64_t sort_val;
  int len;
  int node_id;

  //Constructor
  Node (uint64_t val,int len){
    this->children=nullptr;
    this->numChildren=0;
    this->childArrSize=0;
    this->val=val;
    this->len = len;
  };
};

struct PatriciaTrie{
  Node *root;
  stack <Node *> *rightSpline;
  int numChunks;
  int chunkSize;
  int n;
  unsigned int mask;
  int num_edges;

  //Constructor
  PatriciaTrie(int numChunks,int chunkSize){
    root = new Node(0,0);
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
  void insert(uint64_t sig, int diff){
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
      Node *node = new Node(sig,orig_height);
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
      Node *int_node = new Node((cur_node->val) >> (cur_node->len * chunkSize),-height);
      Node *node = new Node(sig,popped_height+height);
      
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
    cout << "Node value: " << node->val << endl;
    cout << "Node bitmap: " << bitset<9>(node->val) << endl;
    cout << "Sort value: " << bitset<9>(node->sort_val) << endl;
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

  //construct the edge information for sorting edges
  uint64_t* getEdgeInfo(){
    uint64_t *edges = new uint64_t[num_edges];
    return edges;
  }
};

uint64_t* sortSignatures(int n,int numChunks,int chunkSize, uint64_t* sigs){
  uint64_t* sortedSigs = new uint64_t[n];
  PatriciaTrie PT = PatriciaTrie(numChunks,chunkSize);
  
  //insert first signature into the Patricia Trie
  (PT.root)->children = new Node*[4];
  (PT.root)->childArrSize = 4;
  Node *node = new Node(sigs[0],numChunks); 
  (PT.root)->children[0] = node;
  (PT.root)->numChildren += 1;
  (PT.rightSpline)->push(node);

  //insert the rest of the signatures into Patricia Trie
  for (int i = 1; i<n; i++){
    int diff = mss((sigs[i-1])^(sigs[i]));
    PT.insert(sigs[i],diff);
  }
  int cur_id = 1;
  PT.addInfo(PT.root,cur_id);

  cout << "Full Patricia Tree before sorting: " << endl;
  PT.printTree(PT.root);
  cout << "Num edges: " << PT.num_edges << endl;

  uint64_t *edges = PT.getEdgeInfo(); 
  /*
  for (int i = 0; i<PT.num_edges; i++){
    cout << bitset<16>(edges[i]) << endl;
  }
  */

  return sortedSigs;
}

int main(){
  //uint64_t num = 14;
  //cout << "num: " << num << endl;
  //cout << "hex: " << hex << num << endl;
  //cout << "mss: " << dec << mss(num) << endl;
  
  //Input data
  int n=9;
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


  cout << "Unsorted signatures: " << endl;
  
  for (int i=0;i<n;i++){
    cout << sigs[i] << endl;
    cout << bitset<9>(sigs[i]) << endl;
  }

  uint64_t* ans = sortSignatures(n,numChunks,chunkSize,sigs);
  cout << "Sorted signatures: " << endl;
  
  for (int i=0;i<n;i++){
    cout << ans[i] << endl;
  }
  delete [] ans;
}
