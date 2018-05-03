#ifndef HASH_NODE_H_
#define HASH_NODE_H_

#include <shared_mutex>
namespace CTSL  // Concurrent Thread Safe Library
{
    // Class representing a templatized hash node
    template <typename K, typename V>
    class HashNode
    {
        public:
            HashNode() : next(nullptr){}
            HashNode(K key_, V value_) : next(nullptr), key(key_), value(value_){}

            ~HashNode() 
            {
                next = nullptr;
            }

            const K& getKey() const {return key;}
            void setValue(V value_) {value = value_;}
            const V& getValue() const {return value;}

            HashNode* next; // Pointer to the next node in the same bucket

        private:
            K key;   // key
            V value; // value
    };


    // Class representing a hash bucket. The bucket is implemented as a singly linked list.
    // A bucket is always constructed with a dummy head node
    template <typename K, typename V>
    class HashBucket
    {
        public:
            HashBucket() : head(nullptr){}

            ~HashBucket() //delete the bucket
            {
                clear();
            }   

            bool find(const K &key, V &value) const
            {
                // A shared mutex is used to enable mutiple concurrent reads
                std::shared_lock<std::shared_timed_mutex> lock(mutex_); 
                HashNode<K, V> * node = head;

                while (node != nullptr)
                {
                    if (node->getKey() == key)
                    {
                        value = node->getValue();
                        return true;
                    }
                    node = node->next;
                }
                return false;
            }

            void insert(const K &key, const V &value)
            {
                // Exclusive lock to enable single write in the bucket
                std::unique_lock<std::shared_timed_mutex> lock(mutex_);
                HashNode<K, V> * prev = nullptr;
                HashNode<K, V> * node = head;

                while (node != nullptr && node->getKey() != key)
                {
                    prev = node;
                    node = node->next;
                }

                if (nullptr == node) // New entry, create a node and add to bucket
                {
                    if(nullptr == head)
                    {
                        head = new HashNode<K, V>(key, value);
                    }
                    else
                    {
                        prev->next = new HashNode<K, V>(key, value);                 
                    }
                }
                else
                {
                    node->setValue(value); // Key found in bucket, update the value
                }
        
            }

            void erase(const K &key)
            {
                // Exclusive lock to enable single write in the bucket
                std::unique_lock<std::shared_timed_mutex> lock(mutex_);
                HashNode<K, V> *prev  = nullptr;
                HashNode<K, V> * node = head;

                while (node != nullptr && node->getKey() != key)
                {
                    prev = node;
                    node = node->next;
                }

                if (nullptr == node) // Key not found, nothing to be done
                {
                    return;
                }
                else  // Remove the node from the bucket
                {
                    if(head == node)
                    {
                        head = node->next;
                    }
                    else
                    {
                        prev->next = node->next; 
                    }
                    delete node; // Free up the memory
                }
            }

            void clear()
            {
                // Exclusive lock to enable single write in the bucket
                std::unique_lock<std::shared_timed_mutex> lock(mutex_);
                HashNode<K, V> * prev = nullptr;
                HashNode<K, V> * node = head;
                while(node != nullptr)
                {
                    prev = node;
                    node = node->next;
                    delete prev;
                }
                head = nullptr;
            }

        private:
            HashNode<K, V> * head; //The head node of the bucket
            mutable std::shared_timed_mutex mutex_; //The mutex for this bucket
    };
}

#endif

