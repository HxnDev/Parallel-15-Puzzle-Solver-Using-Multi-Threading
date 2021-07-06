//--------------------------------------------------------------------------------------------------------------------------------
//  Hassan Shahzad
//  18i-0441
//  OS-Project
//  FAST NUCES
//  queues.cpp
//--------------------------------------------------------------------------------------------------------------------------------
//================================================================================================================================
// 1: This file contains a data structure "Priority Queue" which will simply be used to get the minimum node.
// 2: Another modified data structure that I am using is "Thread Priority Queue" in which I am creating a priority queue
//      for each thread so they may access from their own queues which will prevent overlapping of threads and enhance parallelization.
// 3: In this implementation, hash function has a function that it stores the board object's hash value to check that no combination 
//      is repeated.
//================================================================================================================================

//==================
// Priority Queue DS
//==================

template <class T>

class priorityQueue
{

private:
    vector<T> heap;                             //Vector

    void swap(int index1, int index2)
    {
        T temp = heap[index1];
        heap[index1] = heap[index2];
        heap[index2] = temp;

        heap[index1]->heapIndex = index1;       //Assigning values
        heap[index2]->heapIndex = index2;
    }

    void siftUp(int index)                      //Swaps the larger node with its smaller parent
    {
        int parentIndex = index / 2;

        if ((parentIndex >= 1) and (*(heap[index]) < *(heap[parentIndex])))
        {
            swap(index, parentIndex);
            siftUp(parentIndex);
        }
    }

    void siftDown(int index)                    //Swap a smaller node with its larger child
    {
        int childIndex = index;

        if ((2 * index < heap.size()) and (*(heap[2 * index]) < *(heap[childIndex])))
        {
            childIndex = 2 * index;
        }

        if ((2 * index + 1 < heap.size()) and (*(heap[2 * index + 1]) < *(heap[childIndex])))
        {
            childIndex = 2 * index + 1;
        }

        if (index != childIndex)
        {
            swap(index, childIndex);
            siftDown(childIndex);
        }
    }

public:
    priorityQueue()
    {
        heap.push_back(NULL);
    }

    bool empty()
    {
        return heap.size() == 1;
    }

    int size()
    {
        return heap.size() - 1;
    }

    void push(T val)
    {
        heap.push_back(val);
        val->heapIndex = heap.size() - 1;
        siftUp(heap.size() - 1);
    }

    T pop()
    {
        if (!empty())
        {
            T val = heap[1];
            swap(1, heap.size() - 1);
            heap.pop_back();

            if (1 < heap.size())
            {
                siftDown(1);
            }
            return val;
        }
        return NULL;
    }

    void remove(T val)
    {
        int index = val->heapIndex;
        swap(index, heap.size() - 1);
        heap.pop_back();

        if (index < heap.size())
        {
            siftUp(index);
            siftDown(index);
        }
    }

    int getMinKey()                 //Returns cost/key
    {
        if (!empty())
        {
            return heap[1]->getF();
        }
        return INT_MAX;
    }

    void update(T val)              //Updating value
    {
        int index = val->heapIndex;
        siftUp(index);
        siftDown(index);
    }
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//=====================
//Queue DS for threads
//=====================
int numThread1;

template <class T, class stateHash, class stateEqual>

class threadsQueue
{

public:
    typedef typename std::unordered_set<T, stateHash, stateEqual>::iterator iterator;

private:
    pthread_mutex_t *lock;
    vector<priorityQueue<T>> pq;                                    //Vector of priority queue
    vector<unordered_set<T, stateHash, stateEqual>> hashes;         //Vector of hash

    int hashVal(T val)
    {
        size_t hashvalue = stateHash()(val);
        return hashvalue % numThread1;
    }

public:
    threadsQueue()
    {
    }

    void init(int n)
    {
        numThread1 = n;
        lock = (pthread_mutex_t *)malloc(n * sizeof(pthread_mutex_t));

        for (int i = 0; i < n; i++)
        {
            pthread_mutex_init(&lock[i], NULL);
            pq.push_back(priorityQueue<T>());
            hashes.push_back(unordered_set<T, stateHash, stateEqual>());
        }
    }

    bool empty(int k)
    {
        pthread_mutex_lock(&lock[k]);
        bool b = pq[k].empty();
        pthread_mutex_unlock(&lock[k]);
        return b;
    }

    T find(T val)
    {
        int k = hashVal(val);
        T e = NULL;

        pthread_mutex_lock(&lock[k]);
        iterator it = hashes[k].find(val);

        if (it != hashes[k].end())
        {
            e = *it;
        }
        pthread_mutex_unlock(&lock[k]);
        return e;
    }

    int size(int k)
    {
        pthread_mutex_lock(&lock[k]);
        int size = pq[k].size();
        pthread_mutex_unlock(&lock[k]);
        return size;
    }

    int hashSize(int k)
    {
        pthread_mutex_lock(&lock[k]);
        int size = hashes[k].size();
        pthread_mutex_unlock(&lock[k]);
        return size;
    }

    void push(T val, State *current)
    {
        int k = hashVal(val);
        pthread_mutex_lock(&lock[k]);
        int altG;

        if (current == NULL)
        {
            altG = 0;
        }

        else
        {
            altG = current->getG() + 1;
        }

        iterator it = hashes[k].find(val);
        if (it != hashes[k].end())                  // found existing state
        {
            delete val;
            val = *it;

            if (altG < val->getG())
            {
                val->setPrev(current);
                val->setG(altG);

                if (val->checkVisited())
                {
                    pq[k].update(val);
                }
                else
                {
                    pq[k].push(val);
                }
            }
        }
        else                                            // new state
        {
            hashes[k].insert(val);
            val->addVisited();

            val->setPrev(current);
            val->setG(altG);
            pq[k].push(val);
        }

        pthread_mutex_unlock(&lock[k]);
    }

    T pop(int k)
    {
        pthread_mutex_lock(&lock[k]);
        T val = pq[k].pop();
        pthread_mutex_unlock(&lock[k]);
        return val;
    }

    void remove(T val)
    {
        int k = hashVal(val);
        pthread_mutex_lock(&lock[k]);
        pq[k].remove(val);
        pthread_mutex_unlock(&lock[k]);
    }

    void update(T val)
    {
        int k = hashVal(val);
        pthread_mutex_lock(&lock[k]);
        pq[k].update(val);
        pthread_mutex_unlock(&lock[k]);
    }

    int getMinKey(int k)
    {
        pthread_mutex_lock(&lock[k]);
        int key = pq[k].getMinKey();
        pthread_mutex_unlock(&lock[k]);
        return key;
    }
};