
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <fcntl.h>
#include <io.h>
#include <time.h>
#include <dirent.h>
#include <string.h>

typedef struct chessPiece
{
	int xPos, yPos;			// x,y position of piece, left-->right(y), up-->down(x)
	int pieceType;			// Pawn(1), Knight(2), Bishop(3), Rook(4), Queen(5), King(6)
	int side;				// White(1), Black(2)
	int alive;				// 1~alive - 0~dead
	int playTimes; 			// how many times played. Need this variable for lots of moves.For ex: "en passant"
	int playedTurn;         // Which turn it is played last time.
}chessPiece;

typedef struct Board {
	// 8x8 game board, will hold chessPieces

	chessPiece *boardPieces[8][8];

}Board;

typedef struct Game {
	chessPiece pieces[33]; //all pieces, last piece is dummp piece*
	int currentPlayer; // White:1  Black:2 
	double blackTime;  //remeaning time for blackplayer
	double whiteTime;  //remeaning time for whiteplayer
	Board board;	   //board structures 
	int currentRound;  //currentRound
}Game;



//initizalize chessPiece unicode with respect to it's type and side!.
void setCode(chessPiece* piece); //set uni code of "piece" respect to its side and type

void printChessPiece(chessPiece *piece);	

//initialize chessPiece, with xPosition, yPosition, side and pieceType(pawn,rook etc..)
int pieceInit(chessPiece* piece, int xPos, int yPos, int side, int pType); //piece initialization


//initialize every piece, initlization players' time, current round and current player.
void gameInit(Game* game); //initialize game1


// very important function block.
// first replace every chessboard tile with empy pieces, then after replace every piece to respective location if it is alive!
void boardInitialize(Game* game);

//display the board, also display the current player with it's remeaning time.
//if the times run out, congrats winner.
void displayBoard(Game* game);



//decide whether the given input is for play movement, go to menu or save!
void decide(Game* game,char str[20]); //decide what to do with input


//this function is checking the input (A2B2), then transforms it into (x,y,tx,ty). It is used for general movement checking.
//whether the played piece is yours or if there is a piece. After that it checks what type of piece is attack piece, then decide if it is legal move or not.
//!!! BE AWARE, THIS FUNCTION ONLY CHECKS PIECE BASE MOVEMENT, NOT GAME(WHETHER THE MOVE CAUSES CHECK FOR ENEMY). FOR EX, PAWN ATTACK CROSS, HORSE MOVE "L" SHAPE.
int move(Game* game,char str[20]); 

//this function update the movement input given in move function. Then decides whether it causes "check" for enemy or current player.
//this function also check if the last move causes "checkmate"
int makeMove(Game* game,chessPiece* atkP,chessPiece* tarP); //makeMove takes Game, atkP(attackPiece) and tarP(targetPiece-target position)
//this function is for general move checking! 


int pawnMoveCheck(Game* game,Board* board,chessPiece* atkP,chessPiece* tarP);  	 //check if pawn move is valid.

int knightMoveCheck(Board* board,chessPiece* atkP,chessPiece* tarP); 			 //check if knight move is valid.

int bishopMoveCheck(Board* board,chessPiece* atkP,chessPiece* tarP); 			 //check if bishop move is valid.

int rookMoveCheck(Board* board,chessPiece* atkP,chessPiece* tarP);   			 //check if rook move is valid.

int queenMoveCheck(Board* board,chessPiece* atkP,chessPiece* tarP);  			 //check if queen move is valid. 

int kingMoveCheck(Board* board,Game* game,chessPiece* atkP,chessPiece* tarP);    //check if king move is valid.

int checkCheck(Board* board,Game* game,int atkS,int tarS,int mateControl);	     //check "check"(sah) is exist, 
																				 //also check if the move causes "check" for enemy!

int checkMate(Board* boardPtr,Game*,int atkS,int tarS);							 //check if the move is "checkmate"

int decideCheckFunction(Board* boardPtr,Game* game,chessPiece* atkP, chessPiece* tarP); //attackPiece and TargetPiece

void congratsWinner(Game* game,int side);										//congrats Winner,return menu after!.



/* updateMove and undoMove function works together while updateMove update move from x,y to tx,ty. 
undoMove just undo it. If test=0, it means it is still testing, do not update piece's last played times(last played turn!).*/
void updateMove(Board* boardPtr,Game* game,chessPiece* atkP,chessPiece* tarP,int x,int y,int tx,int ty,int test);

void undoMove(Board* boardPtr,Game* game,chessPiece* atkP,chessPiece* tarP,int x,int y,int tx,int ty,int test);


int menu(Game* game,char str[20]); //menu, if str[0]==1,2,3 --> 1:start game, 2:load game, 3:exit game

void menuDisp(); //display menu


int char2int(char c[1],int pos); //input char to string conversion, for ex: A2B4 -> 6041 


int int2char(int a); //for creating string for saved game.


void time2str(struct tm*,char s[30]); //convert time value to string value, used for "save file name"


void playerTimeUpdate(Game* game);    //update player's remeaning time.


void pawnPromote(Game* game,chessPiece* pawn); //if pawn reaches end tile, promote pawn!

