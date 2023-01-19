#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>

using namespace std;

class BoardImpl
{
  public:
    BoardImpl(const Game& g);
    void clear();
    void block();
    void unblock();
    bool placeShip(Point topOrLeft, int shipId, Direction dir);
    bool unplaceShip(Point topOrLeft, int shipId, Direction dir);
    void display(bool shotsOnly) const;
    bool attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId);
    bool allShipsDestroyed() const;

  private:
    const Game& m_game;
    int mRows, mCols, nShips; 
    char mGrid[MAXROWS][MAXCOLS];
    vector<int> shipLen; //note that shipID is not necessarily equal to its location in shipVec
    vector<int> shipHealth;
    vector<char> shipSymb; 
    vector<string> shipName; 
    vector<Point> shipPoint; 
};

BoardImpl::BoardImpl(const Game& g)
 : m_game(g)
{
    mRows = g.rows(); 
    mCols = g.cols(); 
    nShips = g.nShips();

    //copying features from each ship into the other ship
    for (int i = 0; i < g.nShips(); i++) {
        shipLen.push_back(g.shipLength(i)); //passing in ship length
        shipSymb.push_back(g.shipSymbol(i)); //passing ship symbol
        shipName.push_back(g.shipName(i)); //passing ship name

    }
    for (int i = 0; i < mRows; i++) {
        for (int j = 0; j < mCols; j++) {
            mGrid[i][j] = '.'; //clear the board location
        }
    }
    shipHealth = shipLen; 

}

void BoardImpl::clear()
{
    for (int i = 0; i < mRows; i++) {
        for (int j = 0; j < mCols; j++) {
            mGrid[i][j] = '.'; //clear the board location
        }
    }

    for (int i = 0; i < nShips; i++) {
        shipPoint[i].c = -1; //move the ship off the board
    }
    
    // This compiles, but may not be correct
}

void BoardImpl::block()
{
    int halfNum = m_game.rows() * m_game.cols() / 2; //
    int numBlocked = 0; 
    int r, c; 
    while (numBlocked < halfNum) {
        r = randInt(m_game.rows());
        c = randInt(m_game.cols());
        if (mGrid[r][c] != '#') {
            mGrid[r][c] = '#';
            numBlocked++; 
        }
    }
}

void BoardImpl::unblock()
{
    for (int r = 0; r < m_game.rows(); r++)
        for (int c = 0; c < m_game.cols(); c++)
        {
            if (mGrid[r][c]=='#')
                mGrid[r][c]='.'; //Unblock if blocked 
        }
}

bool BoardImpl::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    //Checking if initial square is on the board
    if (topOrLeft.c >= m_game.cols() || topOrLeft.c < 0
        || topOrLeft.c >= m_game.cols() || topOrLeft.c < 0)
        return false; 

    char mSymb = shipSymb[shipId];
    int mShipLen = shipLen[shipId];

    if (dir == HORIZONTAL) {
        //Checking if right edge is on board
        if (topOrLeft.c + mShipLen >= m_game.cols())
            return false; 
        
        //iterate through each square
        for (int c = 0; c < mShipLen; c++) { //check if cells are empty
            if (mGrid[topOrLeft.r][c + topOrLeft.c] != '.' || c==m_game.cols())
                return false; //can't be placed
        }
        //place the ship symbols on the board
        for (int c = 0; c < mShipLen; c++) { //place the ship symbol at each pt
            mGrid[topOrLeft.r][c + topOrLeft.c] = mSymb;
        }
        return true; 
    }

    if (dir == VERTICAL) {
        //check if the end of the ship is below the board
        if (topOrLeft.r + mShipLen >= m_game.rows())
            return false;

        //iterate to see if the squares are empty
        for (int r = 0; r < mShipLen; r++) {//check if the cells are empty
            if (mGrid[topOrLeft.r+r][topOrLeft.c] != '.' || r >= m_game.rows())
                return false; //can't be placed
        }
        //add ship symbols
        for (int r = 0; r < mShipLen; r++) {//place the ship symbol at each pt
            mGrid[topOrLeft.r+r][topOrLeft.c] = mSymb;
        }
        return true;
    }

    return false; // If the ship can't be placed
}

bool BoardImpl::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    //check if valid shipID
    if (shipId > shipSymb.size())
        return false; 

    char mSymb = shipSymb[shipId];
    int mShipLen = shipLen[shipId];

    if (dir == HORIZONTAL) {
        // check if cells match ship
        for (int c = 0; c < mShipLen; c++) { 
            if (mGrid[topOrLeft.r][c + topOrLeft.c] != mSymb)
                return false; 
        }
        //unblock each cell
        for (int c = 0; c < mShipLen; c++) {
                mGrid[topOrLeft.r][c + topOrLeft.c] = '.';
        }
        return true; 
    }

    if (dir == VERTICAL) {
        //check if the cells match ship
        for (int r = 0; r < mShipLen; r++) {
            if (mGrid[topOrLeft.r + r][topOrLeft.c] != mSymb)
                return false; 
        }

        //unblock each cell
        for (int r = 0; r < mShipLen; r++) {
                mGrid[topOrLeft.r + r][topOrLeft.c] = '.';
        }
        return true;//return true if all values 
    }

    return false; // In case there exist uncaught cases
}

void BoardImpl::display(bool shotsOnly) const
{
    cout << "  "; //start of string text

    for (int c = 0; c < m_game.cols(); c++) {
        cout << c; //printing numbers on top of board
    }
    cout << endl; 

    for (int r = 0; r < m_game.rows(); r++){
        cout << r << ' '; //printing row numbers
            for (int c = 0; c < m_game.cols(); c++)
        {
            //Printing X, o, or .; or the ship symbol of not human
            if (mGrid[r][c] == 'X' || mGrid[r][c] == 'o' || mGrid[r][c] == '.' || !shotsOnly)
                cout << mGrid[r][c]; // print each cell
            else
                cout << '.'; //printing dots in place of ship symbols if human
        }
        cout << endl; 
    }
}

bool BoardImpl::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    shotHit = false; 
    shipDestroyed = false; 

    //return false if outside region or already shot at point
    if (p.c < 0 || p.c >= m_game.cols() || p.r < 0 || p.r >= m_game.rows() || mGrid[p.r][p.c]=='o' || mGrid[p.r][p.c] == 'X')
        return false; 

    //return shothit false if the shot misses all boats
    if (mGrid[p.r][p.c] == '.') {
        mGrid[p.r][p.c] = 'o';
        return true;
    }

    //the attack must have hit a ship
    shotHit = true; 
    char symb = mGrid[p.r][p.c]; 
    mGrid[p.r][p.c] = 'X';
    for (int i = 0; i < shipSymb.size(); i++) {
        if (shipSymb[i] == symb) {
            shipId = i; //setting ship id
            shipHealth[i]--; //ship has one fewer hit remaining
            if (shipHealth[i] <= 0) { //if the ship has taken as many hits as its length
                shipDestroyed = true;
                return true;
            }
            else {
                shipDestroyed = false;
                return true; 
            }
        }
    }
    //This line should never be acessed
    cerr << "There is an issue with the attack function" << endl; 
    return false; //uncaught issue
}

bool BoardImpl::allShipsDestroyed() const
{
    for (int i = 0; i < shipHealth.size(); i++) {
        if (shipHealth[i] > 0)
            return false; //there is a ship with a non-zero number of hits remaining
    }
    return true; // All ships have been hit
}

//******************** Board functions ********************************

// These functions simply delegate to BoardImpl's functions.
// You probably don't want to change any of this code.

Board::Board(const Game& g)
{
    m_impl = new BoardImpl(g);
}

Board::~Board()
{
    delete m_impl;
}

void Board::clear()
{
    m_impl->clear();
}

void Board::block()
{
    return m_impl->block();
}

void Board::unblock()
{
    return m_impl->unblock();
}

bool Board::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->placeShip(topOrLeft, shipId, dir);
}

bool Board::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->unplaceShip(topOrLeft, shipId, dir);
}

void Board::display(bool shotsOnly) const
{
    m_impl->display(shotsOnly);
}

bool Board::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    return m_impl->attack(p, shotHit, shipDestroyed, shipId);
}

bool Board::allShipsDestroyed() const
{
    return m_impl->allShipsDestroyed();
}
