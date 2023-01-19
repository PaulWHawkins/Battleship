#include "Player.h"
#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>
#include <queue>
#include <string>
#include <vector>
#include <algorithm>


using namespace std;


//========================================================================
// Timer t;                 // create a timer and start it
// t.start();               // start the timer
// double d = t.elapsed();  // milliseconds since timer was last started
//========================================================================

#include <chrono>

class Timer
{
public:
	Timer()
	{
		start();
	}
	void start()
	{
		m_time = std::chrono::high_resolution_clock::now();
	}
	double elapsed() const
	{
		std::chrono::duration<double, std::milli> diff =
			std::chrono::high_resolution_clock::now() - m_time;
		return diff.count();
	}
private:
	std::chrono::high_resolution_clock::time_point m_time;
};

//*********************************************************************
//  AwfulPlayer
//*********************************************************************

class AwfulPlayer : public Player
{
public:
	AwfulPlayer(string nm, const Game& g);
	virtual bool placeShips(Board& b);
	virtual Point recommendAttack();
	virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
		bool shipDestroyed, int shipId);
	virtual void recordAttackByOpponent(Point p);
private:
	Point m_lastCellAttacked;
};

AwfulPlayer::AwfulPlayer(string nm, const Game& g)
	: Player(nm, g), m_lastCellAttacked(0, 0)
{}

bool AwfulPlayer::placeShips(Board& b)
{
	// Clustering ships is bad strategy
	for (int k = 0; k < game().nShips(); k++)
		if (!b.placeShip(Point(k, 0), k, HORIZONTAL))
			return false;
	return true;
}

Point AwfulPlayer::recommendAttack()
{
	if (m_lastCellAttacked.c > 0)
		m_lastCellAttacked.c--;
	else
	{
		m_lastCellAttacked.c = game().cols() - 1;
		if (m_lastCellAttacked.r > 0)
			m_lastCellAttacked.r--;
		else
			m_lastCellAttacked.r = game().rows() - 1;
	}
	return m_lastCellAttacked;
}

void AwfulPlayer::recordAttackResult(Point /* p */, bool /* validShot */,
	bool /* shotHit */, bool /* shipDestroyed */,
	int /* shipId */)
{
	// AwfulPlayer completely ignores the result of any attack
}

void AwfulPlayer::recordAttackByOpponent(Point /* p */)
{
	// AwfulPlayer completely ignores what the opponent does
}

//*********************************************************************
//  HumanPlayer
//*********************************************************************

bool getLineWithTwoIntegers(int& r, int& c)
{
	bool result(cin >> r >> c);
	if (!result)
		cin.clear();  // clear error state so can do more input operations
	cin.ignore(10000, '\n');
	return result;
}

class HumanPlayer : public Player {
public:
	HumanPlayer(string nm, const Game& g);

	virtual bool isHuman() const { return true; }

	virtual bool placeShips(Board& b);
	virtual Point recommendAttack();
	virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
		bool shipDestroyed, int shipId);
	virtual void recordAttackByOpponent(Point p);
private:
	int nShips;
	string humanName;
	vector<string> mNames;
	vector<int> mLen;

};

HumanPlayer::HumanPlayer(string nm, const Game& g) : Player(nm, g) {
	nShips = g.nShips();
	humanName = nm;

	for (int i = 0; i < nShips; i++) {
		mNames.push_back(g.shipName(i));
		mLen.push_back(g.shipLength(i));
	}
};

bool HumanPlayer::placeShips(Board& b) {
	cout << humanName << " must place " << nShips << " ships." << endl;

	for (int i = 0; i < nShips; i++) {

		b.display(false);

		//Set ship direction
		char shipDir; Direction dirFull;
		bool result(false); // shipDir == 'h' || shipDir == 'v'); //result must be h or v

		while (!result) { //loop until valid input
			cout << "Enter h or v for direction of " << mNames[i]
				<< " (length " << mLen[i] << "): "; //ask for ship location

			cin >> shipDir; //accept ship Direct

			if (shipDir == 'h') { //set values and break if h or v
				dirFull = HORIZONTAL;
				break;
			}
			else if (shipDir == 'v') {
				dirFull = VERTICAL;
				break;
			}

			cin.clear();  // clear error state so can do more input operations
			cin.ignore(10000, '\n');
			cout << "Direction must be h or v." << endl;
		}
		// Set ship location
		Point shipPoint;
		bool validLoc;
		cout << "Enter row and column of leftmost cell (e.g., 3 5): ";
		result = (getLineWithTwoIntegers(shipPoint.r, shipPoint.c));

		validLoc = (b.placeShip(shipPoint, i, dirFull));
		while (!result || !validLoc) {
			if (!result)
				cout << "You must enter two integers." << endl;
			else if (!validLoc)
				cout << "The ship can not be placed there." << endl;

			cout << "Enter row and column of leftmost cell (e.g., 3 5): ";
			result = (getLineWithTwoIntegers(shipPoint.r, shipPoint.c));
			validLoc = (b.placeShip(shipPoint, i, dirFull));
		}
	}
	return true;
};

Point HumanPlayer::recommendAttack() {
	Point newPoint;
	cout << "Enter the row and column to attack (e.g., 3 5): ";
	getLineWithTwoIntegers(newPoint.r, newPoint.c);
	return newPoint;

};

void HumanPlayer::recordAttackResult(Point p, bool validShot, bool shotHit,
	bool shipDestroyed, int shipId) {

};
void HumanPlayer::recordAttackByOpponent(Point p) {

};

//*********************************************************************
//  MediocrePlayer
//*********************************************************************
class MediocrePlayer : public Player
{
public:
	MediocrePlayer(string nm, const Game& g);
	virtual bool placeShips(Board& b);
	virtual Point recommendAttack();
	virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
		bool shipDestroyed, int shipId);
	virtual void recordAttackByOpponent(Point p);
private:
//	Point m_lastCellAttacked;
	bool isUnchosen(Point p);
	int state = 1;
	vector<Point> allPoints;
	vector<Direction> dirCallVec;
	vector<Point> attackVec; //points that need to be attacked (state 2)
	vector<int> shipLen;
	bool recursivePlace(int shipNum, Board& b);
	int nShips, mRows, mCols;

};

MediocrePlayer::MediocrePlayer(string nm, const Game& g) : Player(nm, g) {
	for (int i = 0; i < g.nShips(); i++) {
		// Creating a vector of ships with length information
		shipLen.push_back(g.shipLength(i));
	}

	nShips = g.nShips();
	mRows = g.rows();
	mCols = g.cols();

	//creating vector with all possible points for attacking
	for (int r = 0; r < mRows; r++) {
		for (int c = 0; c < mCols; c++) {
			Point newPoint(r, c);
			allPoints.push_back(newPoint);
		}
	}

};


bool MediocrePlayer::placeShips(Board& b) {
	int i = 0; bool flag = false; //i as iterator, flag as true or false for 
	while (i < 50 && flag == false) {
		b.block(); //block half of squares w/ #
		flag = recursivePlace(0, b); //function recusively places each ship
		b.unblock(); //unblock previously blocked squares
		i++;
	}
	return flag;

};

bool MediocrePlayer::recursivePlace(int shipNum, Board& b) {
	for (int i = 0; i < mRows; i++) {
		for (int j = 0; j < mCols; j++) {
			Point p(i, j);
			if (b.placeShip(p, shipNum, HORIZONTAL)) {
				//if this is the last ship, return true
				if (nShips - shipNum == 1) //placing last ship
					return true; //all ships have been placed

				//see if the rest of the ships can be placed
				if (recursivePlace(shipNum + 1, b) == true) //try to place remining ships
					return true;
				else
					//unplace ships if therest of the ships can't be placed
					b.unplaceShip(p, shipNum, HORIZONTAL); 
			}
			if (b.placeShip(p, shipNum, VERTICAL)) {
				if (nShips - shipNum == 1) //placing last ship
					return true; //all ships have been placed

				if (recursivePlace(shipNum + 1, b) == true) {
					return true;
				} //try to place remaining ships

				else
					b.unplaceShip(p, shipNum, VERTICAL);
			}
		}
	}
	return false; //unable to place ship at any location in the 
	//iteraate through of the spaces in the board, trying to put the first ship in each location. If a ship can be placed, make the recusrsive call

}

Point MediocrePlayer::recommendAttack() {
	if (state == 1 || attackVec.size() < 1) { //searching for ship
		int shotIdx = randInt(allPoints.size()); //pick an index from vector of unattacked points
		Point p = allPoints[shotIdx]; //record point
		allPoints.erase(allPoints.begin() + shotIdx); //remove point from list of poi
		return p;
	}
	else {//trying to sink the rest of a ship
		int shotIdx = randInt(attackVec.size()); //pick an index from vector of unattacked points
		Point p = attackVec[shotIdx]; //record point
		for (int i = 0; i < allPoints.size(); i++) {
			if (allPoints[i].r == p.r && allPoints[i].c == p.c) {
				allPoints.erase(allPoints.begin() + i); //remove point from list of point
			}
				
		}
		attackVec.erase(attackVec.begin()+shotIdx);
		return p;
	}
};


void MediocrePlayer::recordAttackResult(Point p, bool validShot, bool shotHit,
	bool shipDestroyed, int shipId) {
	if (shipDestroyed) {
		attackVec.clear();
		state = 1; //return to searching state
		return;
	}


	if (state == 1) {
		if (shotHit && !shipDestroyed) {//push all potential attack points
			for (int ii = 1; ii <= 4; ii++) {
				Point newPt(p.r + ii, p.c);
				if (isUnchosen(newPt))
					attackVec.push_back(newPt);
				newPt.r = p.r - ii;
				if (isUnchosen(newPt))
					attackVec.push_back(newPt);
				newPt.r = p.r;
				newPt.c = p.c + ii;
				if (isUnchosen(newPt))
					attackVec.push_back(newPt);
				newPt.c = p.c - ii;
				if (isUnchosen(newPt))
					attackVec.push_back(newPt);
			}
			state = 2;
		}
	}
	return;
};

void MediocrePlayer::recordAttackByOpponent(Point p) {

};

bool MediocrePlayer::isUnchosen(Point p) {
	for (int i = 0; i < allPoints.size(); i++) {
		if (allPoints[i].r == p.r && allPoints[i].c == p.c)
			return true;
	}
	return false;
};


//*********************************************************************
//  GoodPlayer
//*********************************************************************

class GoodPlayer : public Player
{
public:
	GoodPlayer(string nm, const Game& g);
	virtual bool placeShips(Board& b);
	virtual Point recommendAttack();
	virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
		bool shipDestroyed, int shipId);
	virtual void recordAttackByOpponent(Point p);
	virtual bool isHuman() const { return false; }
	~GoodPlayer();
private:
	bool place1(Board& b);
	int choiceStatus(Point p);
	int state = 1;
	vector<Point> allPointsVec;
	vector<Direction> dirCallVec;
	vector<Point> attackVec; //points that need to be attacked (state 2)
	vector<Point> foundShipPoints; //used if a different ship is found during searching
	vector<int> shipLen;
	bool recursivePlace(int shipNum, Board& b, int skipVal = 1);
	int oppoGrid[MAXROWS][MAXCOLS];
	int nShips, mRows, mCols;
	int arrayLoc = 0;
	vector<int> shipsFound;
	vector<int> alternatingRows;
	vector<int> alternatingCols; 
	int shipOrientation; //represent the direction of the ship being attacked
	Point minLoc, maxLoc; //point to each known end of the ship
	int state1counter = 0; 
	int initSpiralFill; 
	bool minBlocked = false; 
	bool maxBlocked = false;
	int currDepthSearch = 1; 
};

GoodPlayer::~GoodPlayer() {

};

GoodPlayer::GoodPlayer(string nm, const Game& g) : Player(nm, g) {
	nShips = g.nShips();
	mRows = g.rows();
	mCols = g.cols();


	for (int i = 0; i < g.nShips(); i++) {
		// Creating a vector of ships with length information
		shipLen.push_back(g.shipLength(i));
	}
	////creating vector with all possible points for attacking
	//for (int r = 0; r < mRows; r++) {
	//	for (int c = 0; c < mCols; c++) {
	//		Point newPoint(r, c);
	//	}
	//}

	//initializing all values to zero
	for (int ii = 0; ii < mRows; ii += 1) {
		for (int jj = 0; jj < mCols; jj += 1) {
			oppoGrid[ii][jj] = 0;
		}
	}

	queue<Point> spiralOut;
	int i = mRows / 2; int j = mCols / 2;
	int oldi = i - 1;
	int oldj = j;
	int fillVal = 16;
	int countIncr = 0;
	int offset = randInt(4);//add some randomness to the 
	vector<Point>tempPointHolder;

	//sprialing out from center to save the points nearest to the center first
	while (i >= 0 && i < mRows && j >= 0 && j < mCols) {
		Point newPt(i, j);
		spiralOut.push(newPt);
		if ((countIncr + offset) % 9 == 0 && countIncr < 36) {
			allPointsVec.push_back(newPt);
		}
		else if ((countIncr + offset) % 11 == 0 && countIncr < 64) {
			allPointsVec.push_back(newPt);
		}
		else if (((countIncr + offset) - 2) % 15 == 0 && countIncr >= 64){
			allPointsVec.push_back(newPt);
		}
		else
			tempPointHolder.push_back(newPt);
		oppoGrid[i][j] = fillVal; 
		if (i == oldi && j > oldj) { //moving right
			oldj = j;
			if (oppoGrid[i + 1][j] == 0) {
				i++;
			}
			else {
				j++;
			}
		}
		else if (i == oldi && j < oldj) { //moving left
			oldj = j;
			if (oppoGrid[i - 1][j] == 0) {
				i--;
			}
			else {
				j--;
			}
		}
		else if (i > oldi && j == oldj) {
			oldi = i;
			if (oppoGrid[i][j - 1] == 0) {
				j--;
			}
			else {
				i++;
			}
		}
		else if (i < oldi && j == oldj) {
			oldi = i;
			if (oppoGrid[i][j + 1] == 0) {
				j++;
			}
			else {
				i--;
			}
		}
		else {
			break;
		}


		countIncr++;
		if (countIncr == 36)
			fillVal -= 2;
		if (countIncr == 64)
			fillVal -= 2;

	}
	//catching any values not found in the spiral
	for (int ii = 0; ii < mRows; ii += 1) {
		for (int jj = 0; jj < mCols; jj += 1) {
			if (oppoGrid[ii][jj] == 0) {
				oppoGrid[ii][jj] = 2;
				Point newPt(ii, jj);
				tempPointHolder.push_back(newPt);
			}

		}
	}

	//setting the corners to have the lowest precedence level
	oppoGrid[0][0] = 8;
	oppoGrid[mRows-1][mCols-1] = 8;
	oppoGrid[0][mCols-1] = 8;
	oppoGrid[mRows-1][0] = 8;


	//Integrating the non-priority atttack squares
	initSpiralFill = allPointsVec.size(); 
	for (int i = 0; i < tempPointHolder.size(); i += 1) {
		allPointsVec.push_back(tempPointHolder[i]);
	}

	////generating a sequence of alternating rows (used to place ships)
	//vector<int> altRowTemp1;
	//for (int i = mRows-1; i >= 0; i--) {
	//	if (i % 2 == 0)
	//		alternatingRows.push_back(i);
	//	else
	//		altRowTemp1.push_back(i);
	//}

	//for (int i = 0; i < altRowTemp1.size(); i++) {
	//	alternatingRows.push_back(altRowTemp1[i]);
	//}

		//generating a sequence of alternating rows (used to place ships)
	vector<int> altRowTemp1, altRowTemp2;
	for (int i = mRows - 1; i >= mRows / 2; i--) {
		altRowTemp1.push_back(i);
	}
	for (int i = 0; i < mRows / 2; i++) {
		altRowTemp2.push_back(i);
	}
	if (altRowTemp1.size() > altRowTemp2.size()) {
		alternatingRows.push_back(altRowTemp1[0]);
		altRowTemp1.erase(altRowTemp1.begin());
	}
	for (int i = 0; i < altRowTemp1.size(); i++) {
		alternatingRows.push_back(altRowTemp2[i]);
		alternatingRows.push_back(altRowTemp1[i]);

	}

	vector<int> altColTemp1, altColTemp2;
	for (int i = mCols - 1; i >= mCols / 2; i--) {
		altColTemp1.push_back(i);
	}
	for (int i = 0; i < mCols / 2; i++) {
		altColTemp2.push_back(i);
	}
	if (altColTemp1.size() > altColTemp2.size()) {
		alternatingCols.push_back(altColTemp1[0]);
		altColTemp1.erase(altColTemp1.begin());
	}
	for (int i = 0; i < altColTemp1.size(); i++) {
		alternatingCols.push_back(altColTemp2[i]);
		alternatingCols.push_back(altColTemp1[i]);

	}


	//generating a sequence of alternating rows (used to place ships)
	//vector<int> altColTemp1, altColTemp2; 
	//for (int i =  mCols-1; i >= mCols / 2+1; i--) {
	//		altColTemp1.push_back(i);
	//}
	//for (int i = 0; i < mCols/2+1; i++) {
	//	altColTemp2.push_back(i);
	//}
	//if (altColTemp1.size() < altColTemp2.size()) {
	//	alternatingCols.push_back(altColTemp2[0]); 
	//	altColTemp2.erase(altColTemp2.begin());
	//}
	//for (int i = 0; i < altColTemp1.size(); i++) {
	//	alternatingCols.push_back(altColTemp1[i]);
	//	alternatingCols.push_back(altColTemp2[i]);
	//}

};

bool GoodPlayer::placeShips(Board& b) {
	int i = 0; bool flag = false; //i as iterator, flag as true or false for 
	int skipVal = 3; //how many values are skipped in the iteration for the recursive place
	Timer timer;
	while (i < 50000 && flag == false) {
		if (i % 10 == 0) { //check timer status every 10th iteration
			//increase the number f points that the function tries to pplace at
			if (timer.elapsed() > 400)
				skipVal = 2; 
			if (timer.elapsed() > 700)
				skipVal = 1;
			//if time is running out, just place ships anywhere that they fit
			if (timer.elapsed() > 800)
				return recursivePlace(0, b, 1);
		}
		
		b.block(); //block half of squares w/ #
	flag = recursivePlace(0, b, skipVal); //function recusively places each ship
			b.unblock(); //unblock previously blocked squares
		i++;
	}
	if (!flag)
		return recursivePlace(0, b, 1);
	return flag;
};

bool GoodPlayer::recursivePlace(int shipNum, Board& b, int skipVal) {
	int i, j;
	for (int ii = 0; ii < mRows; ii += skipVal) {
		for (int jj = 0; jj < mCols; jj += skipVal) {
			i = alternatingRows[ii]; j = alternatingCols[jj];
			Point p(i, j);
			if (b.placeShip(p, shipNum, HORIZONTAL)) {
				if (nShips - shipNum == 1) { //placing last ship
					return true; //all ships have been placed
				}
				if (recursivePlace(shipNum + 1, b) == true) //try to place remining ships
					return true;
				else
					b.unplaceShip(p, shipNum, HORIZONTAL);
			}
			if (b.placeShip(p, shipNum, VERTICAL)) {
				if (nShips - shipNum == 1) { //placing last ship
					return true; //all ships have been placed

				}
				if (recursivePlace(shipNum + 1, b) == true) {
					return true;
				}//try to place remining ships

				else
					b.unplaceShip(p, shipNum, VERTICAL);
			}
		}
	}

	return false; //unable to place ship at any location in the 
	//iteraate through of the spaces in the board, trying to put the first ship in each location. If a ship can be placed, make the recusrsive call
}

Point GoodPlayer::recommendAttack() {
	//if the player is trying to find the orientation of a ship but the attack vector is empty: 
	//check if there are any spaces open three spots away
	
	if (attackVec.size() == 0 && state == 2 ) {
		if (foundShipPoints.empty())
			state = 1;
		else {
			Point p = foundShipPoints[0];
			Point newPt(p.r + 2, p.c);
			if (choiceStatus(newPt) == 1)
				attackVec.push_back(newPt);
			newPt.r = p.r - 2;
			if (choiceStatus(newPt) == 1)
				attackVec.push_back(newPt);
			newPt.r = p.r;
			newPt.c = p.c + 2;
			if (choiceStatus(newPt) == 1)
				attackVec.push_back(newPt);
			newPt.c = p.c - 2;
			if (choiceStatus(newPt) == 1)
				attackVec.push_back(newPt);
		}
	}

	//check if there are any spaces availible three spots away
	if (state==2 && attackVec.empty()) {
		if (foundShipPoints.empty())
			state = 1;
		else {
			Point p = foundShipPoints[0];
			//see if there's any unshot spaces 
			Point newPt(p.r + 3, p.c);
			if (choiceStatus(newPt) == 1)
				attackVec.push_back(newPt);
			newPt.r = p.r - 3;
			if (choiceStatus(newPt) == 1)
				attackVec.push_back(newPt);
			newPt.r = p.r;
			newPt.c = p.c + 3;
			if (choiceStatus(newPt) == 1)
				attackVec.push_back(newPt);
			newPt.c = p.c - 3;
			if (choiceStatus(newPt) == 1)
				attackVec.push_back(newPt);
		}
	}

	if (state == 2 && attackVec.empty()) {
		state = 1; 
	}


	//Searching for next ship
	if (state == 1) { //searching for ship
		state1counter++; 
		Point p = allPointsVec[0]; //choosing default point if all others fail
		if (state1counter < mRows*mCols/10) {
			while (choiceStatus(p) != 1) {
				arrayLoc++;
				p = allPointsVec[arrayLoc];
			}
			return p;
		}
		else {
			Timer timer; 
			int maxVal = -5; int callcount = 0; 
			Point bestPoint; 
			for (int k = 0; k < 4 * mRows * mCols; k++) {
				//choosing randomly from unchosen points
				p = allPointsVec[randInt(allPointsVec.size())];

				//comparing the value at that point to the best observed value
				if (oppoGrid[p.r][p.c] > maxVal) {
					bestPoint = p; 
					maxVal = oppoGrid[p.r][p.c]; 
				}

				//checking if 1 second has elapsed
				if (++callcount == 10)
				{
					if (timer.elapsed() > 1000) 
					{
						return bestPoint; 
					}
					callcount = 0;
				}
					
			}
			return bestPoint;
			
		}
		
	}
	else if (state == 2) {//trying to sink the rest of a ship

		int maxVal = -100; //
		Point bestPt(-1, -1); //using invalid point for later checks
		vector<Point>::iterator it = attackVec.begin(); 
		vector<Point>::iterator bestIt = attackVec.begin();

		while (it != attackVec.end()) {

			//checking if this is the best point to attack
			if (choiceStatus( (*it) )==1 && oppoGrid [(*it).r] [ (*it).c ] > maxVal) {
				bestPt = (*it); //saving the location
				bestIt = it;  //sving the iterator value
				maxVal = oppoGrid[(*it).r][(*it).c];   //saving the best value
			}
			it++; 
		}
		//if a point was found
		if (bestPt.r > -1) {
			attackVec.erase(bestIt); 
			return bestPt;
		}
		//otherwise, use random at the end of the function

	}
	if (state == 3) {
		//shipOrientation = 0 means aligned horizontally
		if (shipOrientation == 0) { 
			Point leftEdge(minLoc.r, minLoc.c - 1);
			Point rightEdge(maxLoc.r, maxLoc.c + 1);

			// If either edge has been hit, move the edge of the ship to that point
			if (choiceStatus(leftEdge) == 0)
				minLoc = leftEdge;
			if (choiceStatus(rightEdge) == 0)
				maxLoc = rightEdge;


			if (choiceStatus(leftEdge)==1) {
				if (choiceStatus(rightEdge)==1 && oppoGrid[rightEdge.r][rightEdge.c] > oppoGrid[leftEdge.r][leftEdge.c])
					return rightEdge;
				return leftEdge;
			}
			else if (choiceStatus(rightEdge)==1)
				return rightEdge;

		}
		//shipOrientation = 1 means aligned vertically
		if (shipOrientation == 1) {
			Point topEdge(minLoc.r - 1, minLoc.c);
			Point botEdge(maxLoc.r + 1, minLoc.c);

			// If either edge has been hit, move the edge of the ship to that point
			if (choiceStatus(topEdge) == 0)
				minLoc = topEdge;
			if (choiceStatus(botEdge) == 0)
				maxLoc = botEdge;

			//check which points are valid and more likely to have a ship
			if (choiceStatus(topEdge) == 1) {
				if (choiceStatus(botEdge) == 1 && oppoGrid[botEdge.r][botEdge.c] > oppoGrid[topEdge.r][topEdge.c])
					return botEdge; 
				return topEdge;
			}
			else if (choiceStatus(botEdge) == 1)
				return botEdge;
		}
	}

	return(allPointsVec[randInt(allPointsVec.size())]); 
	//attack grid: create a grid with importance of attack -- corners lower, center higher
	//If an attack fails to hit a ship, decrease surrounding cardinal direction by some number
	//If an attack hits, create a vector with the surrouding four positions
	//


};



void GoodPlayer::recordAttackResult(Point p, bool validShot, bool shotHit,
	bool shipDestroyed, int shipId) {
	if(!shotHit)
		oppoGrid[p.r][p.c] = -300; 
	else
		oppoGrid[p.r][p.c] = -100;

	if (!shotHit) { //suggests that there likely is not a ship at any adjacent points
		Point p2(p.r, p.c + 1);
		if (choiceStatus(p2)==1)
			oppoGrid[p2.r][p2.c] -= 4;
		p2.c = p.c - 1; 
		if (choiceStatus(p2) == 1)
			oppoGrid[p2.r][p2.c] -= 4;
		p2.r = p.r - 1; p2.c = p.c; 
		if (choiceStatus(p2) == 1)
			oppoGrid[p2.r][p2.c] -= 4;
		p2.r = p.r + 1; 
		if (choiceStatus(p2) == 1)
			oppoGrid[p2.r][p2.c] -= 4;

		p2 = p; 
		p2.c += 2; 
		if (choiceStatus(p2) == 1)
			oppoGrid[p2.r][p.c + 2] -= 2;
		p2.c -= 4;
		if (choiceStatus(p2) == 1)
			oppoGrid[p.r][p.c - 2] -= 2;
		p2.c += 2;
		p2.r += 2;
		if (choiceStatus(p2) == 1)
			oppoGrid[p.r + 2][p.c] -= 2;
		p2.r -= 4;
		if (choiceStatus(p2) == 1)
			oppoGrid[p.r - 2][p.c] -= 2;
	}

	//erasing the point from the possible points to attack
	vector<Point>::iterator it = allPointsVec.begin();
	while (it != allPointsVec.end()) {
		if (p.c == (*it).c && p.r == (*it).r)
			break;
		it++;
	}
	allPointsVec.erase(it);

	if (shipDestroyed) {
		vector<Point> emptV;
		attackVec = emptV; //clear the attack vector

		//remove ship from the ship vector
		vector<int>::iterator it = shipsFound.begin(); int counter1 = 0;
		while (it != shipsFound.end()) {
			if ((*it) == shipId) {
				shipsFound.erase(it);
				foundShipPoints.erase(foundShipPoints.begin() + counter1);
				break;
			}
			it++; counter1++;
		}

		if (shipsFound.size() == 0) //no other ships have been found
			state = 1; //return to searching state
		else {
			state = 2; //stay in state 2
			Point addPt(foundShipPoints[0]);
			minLoc = addPt; maxLoc = addPt; //setting the ends of the ship

			Point newPt(p.r + 1, p.c);
			if (choiceStatus(newPt)==1)
				attackVec.push_back(newPt);
			newPt.r = p.r - 1;
			if (choiceStatus(newPt) == 1)
				attackVec.push_back(newPt);
			newPt.r = p.r;
			newPt.c = p.c + 1;
			if (choiceStatus(newPt) == 1)
				attackVec.push_back(newPt);
			newPt.c = p.c - 1;
			if (choiceStatus(newPt) == 1)
				attackVec.push_back(newPt);

		}
			
		return;
	}

	if (state == 1) {
		if (shotHit && !shipDestroyed) {//push all potential attack points
			if (shotHit && (find(shipsFound.begin(), shipsFound.end(), shipId) == shipsFound.end())) { //if shot hits and the ship has not yet been found
				shipsFound.push_back(shipId);
				foundShipPoints.push_back(p); 
			}


			Point addPt(p.r, p.c);
			minLoc = addPt; maxLoc = addPt; //setting the ends of the ship

			Point newPt(p.r + 1, p.c);
			if (choiceStatus(newPt) == 1)
				attackVec.push_back(newPt);
			newPt.r = p.r - 1;
			if (choiceStatus(newPt) == 1)
				attackVec.push_back(newPt);
			newPt.r = p.r;
			newPt.c = p.c + 1;
			if (choiceStatus(newPt) == 1)
				attackVec.push_back(newPt);
			newPt.c = p.c - 1;
			if (choiceStatus(newPt) == 1)
				attackVec.push_back(newPt);
			state = 2;
		}
	}
	else if (state == 2) {
		vector<int>::iterator currShip; 
		currShip = find(shipsFound.begin(), shipsFound.end(), shipId);

		if (shotHit && currShip != shipsFound.end()) {
			state = 3;
			if (p.r > maxLoc.r) {
				shipOrientation = 1; 
				maxLoc = p; 
			}else if (p.r < minLoc.r) {
				shipOrientation = 1;
				minLoc = p;
			}
			else if (p.c < minLoc.c) {
				shipOrientation = 0;
				minLoc = p;
			}
			else if (p.c > maxLoc.c) {
				shipOrientation = 0;
				maxLoc = p;
			}

		}
		else if (shotHit) {
			shipsFound.push_back(shipId);
			foundShipPoints.push_back(p);
		}
			

	}
	else if (state == 3) {

		if (shotHit) {

			if (p.r > maxLoc.r) {
				shipOrientation = 1;
				maxLoc = p;
			}
			else if (p.r < minLoc.r) {
				shipOrientation = 1;
				minLoc = p;
			}
			else if (p.c < minLoc.c) {
				shipOrientation = 0;
				minLoc = p;
			}
			else if (p.c > maxLoc.c) {
				shipOrientation = 0;
				maxLoc = p;
			}

		}

	}
	return;
};

void GoodPlayer::recordAttackByOpponent(Point p) {

};

//virtual bool GoodPlayer::isHuman() {
//    return true; 
//}

int GoodPlayer::choiceStatus(Point p) {
	if (p.r < 0 || p.r >= mRows || p.c < 0 || p.c >= mCols) {
		return -2;
	}

	if (oppoGrid[p.r][p.c] > -5)
		return 1;//point
	else if (oppoGrid[p.r][p.c] > -130)
		return 0; //point was chosen and hit
	return -1; //point was chosen but missed
};

//*********************************************************************
//  createPlayer
//*********************************************************************

Player* createPlayer(string type, string nm, const Game& g)
{
	static string types[] = {
		"human", "awful", "mediocre", "good"
	};

	int pos;
	for (pos = 0; pos != sizeof(types) / sizeof(types[0]) &&
		type != types[pos]; pos++)
		;
	switch (pos)
	{
	case 0:  return new HumanPlayer(nm, g);
	case 1:  return new AwfulPlayer(nm, g);
	case 2:  return new MediocrePlayer(nm, g);
	case 3:  return new GoodPlayer(nm, g);
	default: return nullptr;
	}
}
