//--------------------------------------------------------------------------------------------------------------------------------
//  Hassan Shahzad
//  18i-0441
//  OS-Project
//  FAST NUCES
//  multithreading.cpp
//--------------------------------------------------------------------------------------------------------------------------------
//================================================================================================================================
// 1: This file contains complete generic implementation of Multi-threading/Paralellization with 
//      A* algorithm to solve 15 Puzzle Problem
//================================================================================================================================

threadsQueue <State*, stateHash, stateEqual> visited;

int lock = 0;

bool handle_tick()                                  //Checks if goal state has been visited. If yes then check its key value and compare with overall minimum key value
{
    State* element;
    if ((element = visited.find(goal)) == NULL) 
    {
        return false;
    }

    int fval = element->getF();

    for (int i = 0; i < numThreads; i++) 
    {
        if (visited.getMinKey(i) < fval) 
        {
            return false;
        }
    }
    return true;
}

void* parallelThread(void* arg)                     //Thread Function (Multithreading implemnentation)
{
    int thread_id = (int)(long long)(arg);
    int visit = 0;
    int x=1;
    while (1) 
    {    
        if (thread_id == 0 and visit%10000 == 0) 
        {
            if (visit % 100000 == 0) 
            {
                cout<<"Finding optimal solution..."<<x<<endl;
                x++;
            }

            if (handle_tick()) 
            {
                lock = 1;
                return NULL;
            }
        }

        visit++;

        State* current = NULL;

        while (current == NULL) 
        {
            if (lock == 1) 
            {
                return NULL;
            }

                current = visited.pop(thread_id);

        }
        
        current->removeVisited();
        
        vector<State*> neighbors = current->getNeighbors();     //Storing neighbors in vectors
        
        for (int i = 0; i < neighbors.size(); i++) 
        {
            State* neighbor = neighbors[i];
            visited.push(neighbor, current);
        }
    }

    return NULL;
}

void createPath()                       //Keeps track of previous states
{
    vector<State*> tempPath;

    State* current = visited.find(goal);

    while (current != NULL && current != start) 
    {
        tempPath.push_back(current);
        current = current->getPrev();
    }
            
    tempPath.push_back(start);
            
    int pathLength = tempPath.size();

    for (int i = 0; i < pathLength; i++) 
    {
        path.push_back(tempPath[pathLength-1-i]);
    }

    return;
}


void parallel(int numThreads)                       //Runs everything. Called by main
{

    srand(time(NULL));

    visited.init(numThreads); // initializing 

    pthread_t threads[numThreads];                  //Initializing threads

    visited.push(start, NULL);
    for (int i = 0; i < numThreads; i++)            //Creating threads as provided by user
    {
        pthread_create(&threads[i], NULL, &parallelThread, (void*)(long long)i);
    }

    for (int i = 0; i < numThreads; i++)            //Joining thread
    {
        pthread_join(threads[i], NULL);
    }

    createPath();                                   //Creating path
}
