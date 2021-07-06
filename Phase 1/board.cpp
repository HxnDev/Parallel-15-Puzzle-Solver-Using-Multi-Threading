//--------------------------------------------------------------------------------------------------------------------------------
//  Hassan Shahzad
//  18i-0441
//  OS-Project
//  FAST NUCES
//  board.cpp
//--------------------------------------------------------------------------------------------------------------------------------
//================================================================================================================================
// 1: This file contains a class that will store the states of the game. It is a generalized class.
// 2: "Board" class inherits "States" and specifies it. Moreover this class also generates the board randomly everytime.
//================================================================================================================================

#include <algorithm>

//==============
//  State Class
//==============

class State {

protected:

    int f;                                // f = g+h 
    int g;                               // cost of start to current node/state
    int h;                              // estimated cost of current node/state to end

    State* prev;                      //Keeps track of previous state
    
    bool inVisited;                //not visited
    size_t hashvalue;

    void initNew(int gval)       //Initialize according to "g" 
    {
        g = gval;
        h = computeH();
        setF();
        prev = NULL;
        inVisited = false;
        hashvalue = computeHash();
    }

    void setF()                     //Calculate value of f
    {
        f = g+h;
    }

    virtual int computeH() = 0;

    virtual size_t computeHash() = 0;


public:

    int heapIndex;
    
    int getF() 
    {
        return f;
    }

    void setG(int gValue) 
    {
        g = gValue;
        setF();
    }

    int getG() 
    {
        return g;
    }

    int getH() 
    {
        return h;
    }

    void setPrev(State* s) 
    {
        prev = s;
    }

    State* getPrev() 
    {
        return prev;
    }

    void addVisited() 
    {
        inVisited = true;
    }

    void removeVisited() 
    {
        inVisited = false;
    }

    bool checkVisited() 
    {
        return inVisited;
    }

    size_t getHash() 
    {
        return hashvalue;
    }
    
    bool operator<(State& temp) 
    {
        if (getF() < temp.getF())
            return true;
        
        if ((getF() == temp.getF()) and (getG() > temp.getG()))
            return true;
        
        return false;
    }

    virtual std::vector<State*> getNeighbors() = 0;

    virtual bool operator==(const State& other) = 0;

    virtual std::string toString() = 0;

    virtual ~State() {}
};

struct stateHash {

public:

	size_t operator()(State* s) const                   //Operator Overloading
    {
		return s->getHash();
	}
};

struct stateEqual {

public:

	bool operator()(State* s1, State* s2) const         //Operator Overloading
    {
		return *s1 == *s2;
	}
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//==============
//  Board Class
//==============

class Board : public State {

private:

    int** board;
    int size;
    int emptyRow;
    int emptyCol;
    
    Board (Board* bd, int x, int y)     // copy board and make a move
    {
        size = bd->size;
        emptyRow = bd->emptyRow + x;
        emptyCol = bd->emptyCol + y;

        board = new int*[size];

        for (int i = 0; i < size; i++) 
        {
            if (i == emptyRow or i == bd->emptyRow) 
            {
                board[i] = new int[size];

                for (int j = 0; j < size; j++) 
                {
                    board[i][j] = bd->board[i][j];
                }
            }
            else 
            {
                board[i] = bd->board[i];
            }
        }

        //Copying empty columns and rows
        
        board[emptyRow][emptyCol] = bd->board[bd->emptyRow][bd->emptyCol];
        board[bd->emptyRow][bd->emptyCol] = bd->board[emptyRow][emptyCol];

        initNew(INT_MAX);
    }

    int computeH()              //Computes the value of h
    {
        int total = 0;
        
        for (int i = 0; i < size; i++) 
        {
            for (int j = 0; j < size; j++) 
            {
                int element = board[i][j];

                if (i == emptyRow && j == emptyCol) 
                    continue;

                int row = (element-1)/size;
                int col = (element-1)%size;

                int distance = abs(row-i) + abs(col-j);
                total += distance;
            }
        }
        return total;
    }

    size_t computeHash()        //Computing hash table so no two same boards repeat
    {
        size_t h = 0;

        int size2 = size * size;

        size_t mod = (1LL<<56)-5;       //Long long integer
        
        for (int i = 0; i < size; i++) 
        {
            for (int j = 0; j < size; j++) 
            {
                h = (h * size + board[i][j]) % mod;
            }
        }



        size_t v = h;
        v = v ^ (v >> 21);
        v = v ^ (v << 37);
        v = v ^ (v >> 4);
        v = v * 4768777513237032717;
        v = v ^ (v << 20);
        v = v ^ (v >> 41);
        v = v ^ (v <<  5);
        return v;
    }


public:

    Board(int n)                // Generates random board
    {
        size = n;               //length of puzzle

        int size2 = size * size;

        int* per = new int[size2];          // creates random permutation
        
        for (int i = 0; i < size2; i++) 
        {
            per[i] = i;
        }

        srand(time(NULL));
        
        for (int i = size2; i > 0; i--) 
        {
            int index1 = rand() % i;         //Swapping at random places
            int temp = per[index1];
            per[index1] = per[i-1];

            per[i-1] = temp;
        }

        board = new int*[size];            // inserting into the board

        for (int i = 0; i < size; i++) 
        {
            board[i] = new int[size];
        
            int row = i*size;

            for (int j = 0; j < size; j++) 
            {
                board[i][j] = per[row+j];
                if (board[i][j] == 0)       //When finds zero, assign the indexes to emptyrow and empty column
                {
                    emptyRow = i;
                    emptyCol = j;
                }
            }
        }
        delete[] per;                   //No use left of permutation

        initNew(0);
    }

    Board(int n, int moves)             // make moves from board
    {
        size = n;

        board = new int*[size];

        for (int i = 0; i < size; i++) 
        {
            board[i] = new int[size];
            int row = 1+i*size;
        
            for (int j = 0; j < size; j++) 
            {
                board[i][j] = row+j;
            }
        }

        board[size-1][size-1] = 0;
        emptyRow = size-1;
        emptyCol = size-1;

        srand(time(NULL));              //make random moves
        
        while(moves--) 
        {
            int x[4] = {-1, 1, 0, 0};
            int y[4] = {0, 0, -1, 1};
            int valid[4] = {0, 0, 0, 0};

            //Conditional checks
            
            int numvalid = 0;
            if (emptyRow > 0) 
            { 
                valid[0] = 1; numvalid++;
            }
            
            if (emptyRow < size-1) 
            { 
                valid[1] = 1; numvalid++;
            }
            
            if (emptyCol > 0) 
            { 
                valid[2] = 1; numvalid++;
            }
            
            if (emptyCol < size-1) 
            { 
                valid[3] = 1; numvalid++;
            }
            
            if (numvalid == 0) 
            { 
                initNew(0); 
                return; 
            }

            int index = 0;

            int index1 = rand() % numvalid;

            for (int i = 0; i <= index1; i++) 
            {
                if (i > 0) index++;
                while (valid[index] == 0) 
                {
                    index++;
                }
            }

            int newEmptyRow = emptyRow + x[index];
            int newEmptyCol = emptyCol + y[index];

            board[emptyRow][emptyCol] = board[newEmptyRow][newEmptyCol];
            board[newEmptyRow][newEmptyCol] = 0;
            emptyRow = newEmptyRow;
            emptyCol = newEmptyCol;
        }

        initNew(0);
    }

//FOR TESTING PURPOSE ONLY. (BENCHMARK)//

    // initialize board based on file
    Board(std::string s) 
    {
        FILE *fp = fopen(s.c_str(), "r");
        fscanf(fp, "%d", &size);

        board = new int*[size];
        for (int i = 0; i < size; i++) {
            board[i] = new int[size];
            int row = i*size;
            for (int j = 0; j < size; j++) {
                fscanf(fp, "%d", &board[i][j]);
                if (board[i][j] == 0) {
                    emptyRow = i;
                    emptyCol = j;
                }
            }
        }
        fclose(fp);

        initNew(0);
    }

    
    vector<State*> getNeighbors() 
    {
        vector<State*> neighbors;
        
        if (emptyRow > 0) 
        {
            Board* nbr = new Board(this, -1, 0);    //left
            neighbors.push_back((State*)nbr);
        }

        if (emptyRow < size-1) 
        {
            Board* nbr = new Board(this, 1, 0);     //right
            neighbors.push_back((State*)nbr);
        }
        
        if (emptyCol > 0) 
        {
            Board* nbr = new Board(this, 0, -1);    //down
            neighbors.push_back((State*)nbr);
        }
        
        if (emptyCol < size-1) 
        {
            Board* nbr = new Board(this, 0, 1);     //up
            neighbors.push_back((State*)nbr);
        }
        return neighbors;
    }

    bool operator==(const State& other) 
    {
        Board* bd = (Board*)(&other);
        
        if (size != bd->size) 
            return false;
        
        for (int i = 0; i < size; i++) 
        {
            for (int j = 0; j < size; j++) 
            {
                if (board[i][j] != bd->board[i][j]) 
                {
                    return false;
                }
            }
        }
        return true;
    }

    string toString() 
    {
        string s;
        
        s += "Row " + to_string(emptyRow) + " & ";
        s += "Column " + to_string(emptyCol) + " have '0'\n";

        for (int i = 0; i < size; i++) 
        {
            for (int j = 0; j < size; j++) 
            {
                s += to_string(board[i][j]);
                s += " ";
            }
            s += "\n";
        }
        return s;
    }
};
