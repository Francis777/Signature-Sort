#include "RobinHoodHashTable.h"

RobinHoodHashTable::RobinHoodHashTable(size_t numBuckets, std::shared_ptr<HashFamily> family) {
  // TODO: Implement this
    h = family->get();
    this->numBuckets = numBuckets;
    hashtable = new int [numBuckets];
    for (size_t i = 0; i < numBuckets; i++){
        hashtable[i] = -2;
    }    
}

RobinHoodHashTable::~RobinHoodHashTable() {
  // TODO: Implement this
    delete [] hashtable;
    hashtable = nullptr;
}

void RobinHoodHashTable::insert(int data) {
  // TODO: Implement this
    size_t index = h(data) % numBuckets;
    size_t dist = 0;
    size_t curr_dist = 0;
     cout << "Insert start: " << data << endl;
    if (!contains(data)){
        while (true){
            // insert at empty entry
            if (hashtable[index] == -2){
                hashtable[index] = data;
                cout << "find empty and insert " << data << " at " << index << endl;
                return;
            }
            // compare "home" distance of current key and new key
            curr_dist = probe_distance(index,h);
            if ( curr_dist < dist){
                int temp = hashtable[index];
                hashtable[index] = data;
                data = temp;
                dist = curr_dist;
                cout << "Swap " << data <<" with " <<hashtable[index] <<" at "<< index << endl;
            }
            if (index == numBuckets - 1){
                index = 0;
            }else{
                index ++;
            }
            dist ++;
        }
    }
    cout << "Inserted " << data <<" "<< index  << endl;
    
}

bool RobinHoodHashTable::contains(int data) const {
  // TODO: Implement this
    cout << "contains start: " <<endl;
    size_t notfound_indicator = -1;
    if (containsHelper(data,h) == notfound_indicator){
        cout << "not contain: " << data << endl;
        return false;
    }else{
        cout << "contain: " << data << endl;
        return true;
    }
}

/**
 * You should implement this operation using backward-shift deletion: once
 * you've found the element to remove, continue scanning forward until you
 * find an element that is at its home location or an empty cell, then shift
 * each element up to that point backwards by one step.
 */
void RobinHoodHashTable::remove(int data) {
  // TODO: Implement this

    cout << "remove " << data << endl;
    
    // find the position of element to remove 
    size_t pos = containsHelper(data,h);
    size_t notfound_indicator = -1;
    if (pos != notfound_indicator){
        // continue scanning until finding element that is at its home location or an empty cell
        size_t endpos = removeHelper(pos,h);
        // shift each element in the interval one step backward 
        if (endpos <=  pos+1){
            hashtable[pos] = -2;
        }else{
            for (size_t i = pos; i < endpos; i++){
                hashtable[i] = hashtable[i+1];
            }
        }
    }
    cout << "removed " << data << endl;
}
