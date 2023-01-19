#include "Game.h"
#include "Board.h"
#include "Player.h"
#include "globals.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>

using namespace std;



class GameImpl
{
  public:
    GameImpl(int nRows, int nCols);
    int rows() const;
    int cols() const;
    bool isValid(Point p) const;
    Point randomPoint() const;
    bool addShip(int length, char symbol, string name);
    int nShips() const;
    int shipLength(int shipId) const;
    char shipSymbol(int shipId) const;
    string shipName(int shipId) const;
    Player* play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause=true);

private: 
    int m_rows, m_cols; 
    vector<char> symbVec;
    vector<int> healthVec; 
    vector<int> lenVec;
    vector<string> nameVec; 


};

void waitForEnter()
{
    cout << "Press enter to continue: ";
    cin.ignore(10000, '\n');
}

GameImpl::GameImpl(int nRows, int nCols)
{
    m_rows = nRows; 
    m_cols = nCols; 

}

int GameImpl::rows() const
{
    return m_rows; 
}

int GameImpl::cols() const
{
    return m_cols; 
}

bool GameImpl::isValid(Point p) const
{
    return (p.r >= 0  &&  p.r < rows()  &&  p.c >= 0  &&  p.c < cols());
}

Point GameImpl::randomPoint() const
{
    return Point(randInt(rows()), randInt(cols()));
}

bool GameImpl::addShip(int length, char symbol, string name)
{
    // Ship can't be too long to fit or negative
    if (length > max(m_rows, m_cols) || length < 1)
        return false; 

    // Default characters can't be used as symbols
    if (symbol == *"X" || symbol == *"." || symbol == *"o")
        return false; 

    // Checking if printable
    if (isprint(symbol) <= 0)
        return false; 

    //checking if another ship has the same symbol
    vector<char>::iterator it = symbVec.begin();
    while (it != symbVec.end()) { //using iterator to iterate through each ship
        if ((* it) == symbol)
            return false; 
        it++; 
    }

    //creating a ship and initializing some of the values
    lenVec.push_back(length);
    healthVec.push_back(length);
    nameVec.push_back(name);
    symbVec.push_back(symbol);

    return true;  // Sucessful addition of ship
}

int GameImpl::nShips() const
{
    return healthVec.size();
}

int GameImpl::shipLength(int shipId) const
{
    if (shipId >= lenVec.size() || shipId < 0)
        return -1;  //checking if that ship exists

    return(lenVec[shipId]); // returning that ship's length
}

char GameImpl::shipSymbol(int shipId) const
{
    if (shipId >= lenVec.size() || shipId < 0)
        return -1;  //checking if that ship exists

    return(symbVec[shipId]); // Returning that ship's symbol
}

string GameImpl::shipName(int shipId) const
{
    if (shipId >= lenVec.size() || shipId < 0)
        return "Error";  //checking if that ship exists

    return(nameVec[shipId]); // Returning that ship's name
}

Player* GameImpl::play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause)
{
    //place ships
    if (!p1->placeShips(b1) || !p2->placeShips(b2))
        return nullptr; 

    //save if human for display purposes
    bool humanCheck1(false), humanCheck2(false);
    if (p1->isHuman())
        humanCheck1 = true; 
    if (p2->isHuman())
        humanCheck2= true;

    //initialize values for future use
    Point attack, attack2; 
    bool shotHit, shipDestroyed, validShot; 
    int shipId; 

    while (!b1.allShipsDestroyed() && !b2.allShipsDestroyed()) {
        // Player one's move
        cout << p1->name() << "'s turn. Board for " << p2->name() << ":" << endl; 
        b2.display(humanCheck1); //display, only shows the shots if b2 human
        
        //perform attack
        attack = p1->recommendAttack(); //save the point to be attacked
        validShot = b2.attack(attack, shotHit, shipDestroyed, shipId); //evaluates if valid shot, changes values if needed

        //save attack results
        p1->recordAttackResult(attack, validShot, shotHit, shipDestroyed, shipId); //
        p2->recordAttackByOpponent(attack);

        //display outcome of attack
        cout << p1->name();
        if (!validShot) {
            cout << "wasted a shot at (";
            cout << attack.r << "," << attack.c << ")." << endl; 
        }
        else {
           cout  << " attacked (" << attack.r << "," << attack.c << ") and ";
            if (shotHit)
                if (shipDestroyed)
                    cout << "destroyed the " << shipName(shipId);
                else
                    cout << "hit something";
            else
                cout << "and missed";
            cout << ", resulting in:" << endl;

            b2.display(humanCheck1); //display, only shows the shots if b2 human
        }

        // Check if p1 is winner
        if (b2.allShipsDestroyed()) {//Player 1 is the winner
            cout << p1->name() << " wins!" << endl;
            if (humanCheck2) { //if player 2 is human, display the board with everything
                cout << "Here is where " << p1->name() << "'s ships were: " << endl;
                b1.display(false);

            }
            return p1; //if p1 wins, return p1; 
        }

        //wait for enter as needed
        if (shouldPause)
            waitForEnter();


        //Player 2's turn
        cout << p2->name() << "'s turn. Board for " << p1->name() << ":" << endl;
        b1.display(humanCheck2); //display, with only shots

        //perform attack
        attack = p2->recommendAttack(); //save the point to be attacked

        //perform attack
        validShot = b1.attack(attack, shotHit, shipDestroyed, shipId);

        //save attack results
        p2->recordAttackResult(attack, validShot, shotHit, shipDestroyed, shipId);
        p1->recordAttackByOpponent(attack);
        
        //display outcome of attack
        cout << p2->name();
        if (!validShot) {
            cout << " wasted a shot at (";
            cout << attack.r << "," << attack.c << ")." << endl;
        }
        else {
            cout << " attacked (" << attack.r << "," << attack.c << ") and ";
            if (shotHit)
                if (shipDestroyed)
                    cout << "destroyed the " << shipName(shipId);
                else
                    cout << "hit something";
            else
                cout << "and missed";
            cout << ", resulting in:" << endl;

            b1.display(humanCheck2); //display, only shows the shots if b2 human
        }

        //check if p2 is winner
        if (b1.allShipsDestroyed()) {
            cout << p2->name() << " wins!" << endl;
            if (humanCheck1) {//if player 1 is human, display P2's baord board with everything
                cout << "Here is where " << p2->name() << "'s ships were: " << endl;
                b2.display(false);
            }

            return p2; //if p2 wins, return p2; 
        }

        //wait for enter as needed
        if (shouldPause)
            waitForEnter(); 
    }
    
    cerr << "We have encountered an unexpected end to the game"; 
    return nullptr;  // Catching any unexpected behavior
}

//******************** Game functions *******************************

// These functions for the most part simply delegate to GameImpl's functions.
// You probably don't want to change any of the code from this point down.

Game::Game(int nRows, int nCols)
{
    if (nRows < 1  ||  nRows > MAXROWS)
    {
        cout << "Number of rows must be >= 1 and <= " << MAXROWS << endl;
        exit(1);
    }
    if (nCols < 1  ||  nCols > MAXCOLS)
    {
        cout << "Number of columns must be >= 1 and <= " << MAXCOLS << endl;
        exit(1);
    }
    m_impl = new GameImpl(nRows, nCols);
}

Game::~Game()
{
    delete m_impl;
}

int Game::rows() const
{
    return m_impl->rows();
}

int Game::cols() const
{
    return m_impl->cols();
}

bool Game::isValid(Point p) const
{
    return m_impl->isValid(p);
}

Point Game::randomPoint() const
{
    return m_impl->randomPoint();
}

bool Game::addShip(int length, char symbol, string name)
{
    if (length < 1)
    {
        cout << "Bad ship length " << length << "; it must be >= 1" << endl;
        return false;
    }
    if (length > rows()  &&  length > cols())
    {
        cout << "Bad ship length " << length << "; it won't fit on the board"
             << endl;
        return false;
    }
    if (!isascii(symbol)  ||  !isprint(symbol))
    {
        cout << "Unprintable character with decimal value " << symbol
             << " must not be used as a ship symbol" << endl;
        return false;
    }
    if (symbol == 'X'  ||  symbol == '.'  ||  symbol == 'o')
    {
        cout << "Character " << symbol << " must not be used as a ship symbol"
             << endl;
        return false;
    }
    int totalOfLengths = 0;
    for (int s = 0; s < nShips(); s++)
    {
        totalOfLengths += shipLength(s);
        if (shipSymbol(s) == symbol)
        {
            cout << "Ship symbol " << symbol
                 << " must not be used for more than one ship" << endl;
            return false;
        }
    }
    if (totalOfLengths + length > rows() * cols())
    {
        cout << "Board is too small to fit all ships" << endl;
        return false;
    }
    return m_impl->addShip(length, symbol, name);
}

int Game::nShips() const
{
    return m_impl->nShips();
}

int Game::shipLength(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipLength(shipId);
}

char Game::shipSymbol(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipSymbol(shipId);
}

string Game::shipName(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipName(shipId);
}

Player* Game::play(Player* p1, Player* p2, bool shouldPause)
{
    if (p1 == nullptr  ||  p2 == nullptr  ||  nShips() == 0)
        return nullptr;
    Board b1(*this);
    Board b2(*this);
    return m_impl->play(p1, p2, b1, b2, shouldPause);
}

