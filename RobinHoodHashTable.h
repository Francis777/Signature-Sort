#ifndef RobinHoodHashTable_Included
#define RobinHoodHashTable_Included
#include <iostream>
using namespace std;
#include "Hashes.h"

class RobinHoodHashTable {
public:
  /**
   * Constructs a new Robing Hood table with the specified number of buckets,
   * using hash functions drawn from the indicated family of hash functions.
   * Because our testing harness attempts to exercise a number of different
   * load factors, you should not change the number of buckets once the hash
   * table has initially be created.
   *
   * You can choose a hash function out of the family of hash functions by
   * declaring a variable of type HashFunction and assigning it the value
   * family->get(). For example:
   *
   *    HashFunction h;
   *    h = family->get();
   */
  RobinHoodHashTable(size_t numBuckets, std::shared_ptr<HashFamily> family);
  
  /**
   * Cleans up all memory allocated by this hash table.
   */
  ~RobinHoodHashTable();
  
  /**
   * Inserts the specified element into this hash table. If the element already
   * exists, this operation is a no-op.
   */
  void insert(int key);
  
  /**
   * Returns whether the specified key is contained in this hash tasble.
   */
  bool contains(int key) const;
  
  /**
   * Removes the specified element from this hash table. If the element is not
   * present in the hash table, this operation is a no-op.
   *
   * You should implement this operation using backward-shift deletion: once
   * you've found the element to remove, continue scanning forward until you
   * find an element that is at its home location or an empty cell, then shift
   * each element up to that point backwards by one step.
   */
  void remove(int key);
  
private:
  /* TODO: Add any data members or private helper functions that you'll need,
   * then delete this comment.
   */
    int* hashtable;
    size_t numBuckets;
    HashFunction h;
  
    size_t probe_distance(size_t index,HashFunction h) const {
        size_t nominal_index = h(hashtable[index]) % numBuckets;
        return index - nominal_index;
    }

        
    size_t containsHelper(int data, HashFunction h) const {
        size_t index = h(data) % numBuckets;
        size_t dist = 0;
        cout << "data: " << data << " index: " << index << endl;
        while(true){   
            if (hashtable[index] == -2){
                cout << "index: " << hashtable[index] << " data: " << data <<endl;
                // can't exist if the entry it should be hashed to is empty
                return -1;
            }else if (dist > probe_distance(index,h)){
                cout << dist << " , " << probe_distance(index,h) <<endl;
                // can't exist if still haven't found at index it should be swapped if exists
                return -1;
            }else if (hashtable[index] == data){
                return index;
            }

            if (index == numBuckets - 1){
                index = 0;
            }else{
                index ++;
            }
            dist ++;
        }
    }

    size_t removeHelper(size_t start_pos, HashFunction h) const {
        size_t pos = start_pos;
        while (true){
            if (hashtable[pos] == -2 || pos == h(hashtable[pos])){
                return pos;
            }else{
                if (pos == numBuckets -1){
                    pos = 0;
                }else{
                    pos ++;
                }
            }
        }
    }
    
    
  /* Fun with C++: these next two lines disable implicitly-generated copy
   * functions that would otherwise cause weird errors if you tried to
   * implicitly copy an object of this type. You don't need to touch these
   * lines.
   */
  RobinHoodHashTable(RobinHoodHashTable const &) = delete;
  void operator=(RobinHoodHashTable const &) = delete;
};

#endif
