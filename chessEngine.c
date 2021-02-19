
#include "chessEngine.h"

#define maxNum(a, b) (((a) > (b)) ? (a) : (b))




chessPiece *checkMatePiece; //will point piece who declare check

clock_t globClock;


int char2int(char c[1], int pos)
{
	// this functions converts to real lite checkerboard to boardPieces[8][8].
	// I could have used "atio" function, but I did not known that before.
	// if *c 65-72  -> return (c*-65)
	// if *c 97-104 -> return (c* - 97)
	// if *c 49-56	-> return (56 - *c)

	int k = *c;
	if (pos % 2 == 0) //if pos == even, it should be letter
	{

		if (k >= 65 && k <= 72)
			return (k - 65);
		else if (k >= 97 && k <= 104)
			return (k - 97);
	}

	if (pos % 2 == 1)
	{ //if pos == odd, it should be number
		if (k <= 56 && k >= 49)
			return (56 - k);
	}

	return 999; // error
}

int move(Game *game, char str[20])
{
	int y = char2int(&str[0], 0); //converting char into integer, xy cordinates
	int x = char2int(&str[1], 1);
	int ty = char2int(&str[2], 2); // targetY, targetX
	int tx = char2int(&str[3], 3);

	// wprintf(L"x:%d, y:%d, tx:%d ty:%d\n",x,y,tx,ty);
	if (x == tx && y == ty)
	{ //Played and Target are same!.
		printf("Invalid move!\n");
		return 0;
	}

	Board *boardPtr = &(game->board);  //get boardPtr

	chessPiece *atkP = boardPtr->boardPieces[x][y]; //get played piece!

	chessPiece *tarP = boardPtr->boardPieces[tx][ty]; //get target piece!

	if (atkP->side != game->currentPlayer || atkP->alive == 0) //Player can only move their pieces.
	{
		printf("Invalid move!\n");
		return 0;
	}

	if (tarP->side == 0)
	{ //there is only one piece for *(dummy piece), so we need to update xpos,ypos
		tarP->xPos = tx;
		tarP->yPos = ty;
	}

	// Pawn(1), Knight(2), Bishop(3), Rook(4), Queen(5), King(6)
	int checkFlag = 0;
	//If the desired move is valid then return 1, if it is castling return 2, if it is passant reuturn 4, if it is invalid move return 0
	//Control the move in pieceWise Condition. Ex: knight go "L" shape, Rook can go in one direction etc.
	checkFlag = decideCheckFunction(boardPtr, game, atkP, tarP);

	
	if (checkFlag == 1)
	{
		
		//When to move is valid in pieceWise condition.
		//Apply the move, then checks if it is valid in GameWise conditio.
		int endGame = makeMove(game, atkP, tarP);

		if (endGame)
			return 5; //gama ended
	}
	else if (checkFlag == 0)
	{
		//the move is invalid!
		printf("Invalid move!\n");
		return 0;
	}
	else if (checkFlag == 2)
	{
		//special move castling
		printf("---------------------\n");
		printf("CASTLING!\n");
	}
	else if (checkFlag == 4)
	{
		//special move passan!
		printf("----------------------\n");
		printf("Passant!\n");
	}

	return 1;
}

int makeMove(Game *game, chessPiece *atkP, chessPiece *tarP)
{

	//get pointer of Board
	Board *boardPtr = &(game->board);


	int x = atkP->xPos; //positions in checker board
	int y = atkP->yPos;
	int tx = tarP->xPos;
	int ty = tarP->yPos;

	int cc; // flag for checkCheck function.

	//update the move,be aware it is for testing! Last input is 1.
	updateMove(boardPtr, game, atkP, tarP, x, y, tx, ty, 1);

	

	//Control if the move trigger "chechk" for both side.
	//If enemy piece can attack your king then it is invalid move!   (cc = 1)
	//If your piece can attack enemy king then is is check! (cc = 2)
	cc = checkCheck(boardPtr, game, atkP->side, tarP->side, 0);

	if (cc != 1 && atkP->pieceType == 1)
	{ // if the move is not trigger check for enemy, and pawn at last position of board, then promote the pawn.
		if (tx == 0 || tx == 7)
		{
			//pawn can not go backward so we dont need to check that condition

			pawnPromote(game, atkP);
				//first undo test move, then update the move.
			undoMove(boardPtr, game, atkP, tarP, x, y, tx, ty, 1);
			updateMove(boardPtr, game, atkP, tarP, x, y, tx, ty, 0);

			//change current player. 1-White 2-Black
			game->currentPlayer = 3 - game->currentPlayer;
			game->currentRound++; //increment the current round.
			return 0;
		}
	}

	if (cc == 1)
	{ //undo last move,your move trigger "check" for enemy.
		// wprintf(L"Move triggers \"check\" for the enemy\n");
		printf("Invalid Move!\n");
		undoMove(boardPtr, game, atkP, tarP, x, y, tx, ty, 1);
		return 0;
	}
	else if (cc == 2)
	{ // declared check, control if it is checkmate!

		//control if it is "checkmate", if not it is "check".
		if (checkMate(boardPtr, game, atkP->side, tarP->side))
		{
			printf("Checkmate!\n");
			displayBoard(game);
			return 1;
			
		}
		else
		{
			printf("Check!\n");
		}
	}

	//first undo test move, then update the move.
	undoMove(boardPtr, game, atkP, tarP, x, y, tx, ty, 1);
	updateMove(boardPtr, game, atkP, tarP, x, y, tx, ty, 0);

	//change current player. 1-White 2-Black
	game->currentPlayer = 3 - game->currentPlayer;
	game->currentRound++; //increment the current round.
	return 0;
}

int checkMate(Board *boardPtr, Game *game, int atkS, int tarS)
{
	//this function control if it is checkMate.
	//Three different situation is checked!
	//1: king can move adjacent tile, if one of theese tiles is not "check" then return 0
	//2: Try to attack the checkMatePiece (it is global pointer which points the piece which declare check).
	//If one of those piece can kill checkMatePiece and if the move does not trigger check for enemy then return 0 (not checkmate!).
	//3: Try to block checkMatePiece projection towards to the tarKing. If it can block then return 0;
	//If one of those above does not return 0, then return 1(which is checkmate.)

	chessPiece *tarKing;	 // target king pointer
	chessPiece *tmpTarget;	 // tmpTarget piece, will be used for checking the move
	chessPiece *tmpAttacker; // tmpAttacker piece pointer, will be used for to attack checkMatePiece!.
	int innerFlag;			 // innerFlag will be used for output of checkCheck function.

	int x, y;	//x,y position of temporary Move piece 	-- from where
	int tx, ty; //x,y position of temporary Target position -- to where

	if (atkS == 1)
	{ //attackter white, defend black.
		tarKing = &(game->pieces[31]);
	}
	else if (atkS == 2)
	{
		tarKing = &(game->pieces[29]);
	}

	x = tarKing->xPos; //x and y position of targetking
	y = tarKing->yPos;

	// Trying to move king to adjacent title.
	for (int i = -1; i < 2; i++)
	{ //adjacent tile for x position
		tx = x + i;
		for (int j = -1; j < 2; j++)
		{ //ajdacent tile for y position
			ty = y + j;
			if (ty > 7 || tx > 7 || ty < 0 || tx < 0) //if the tile is out of checkerboard then continue
				continue;
			tmpTarget = boardPtr->boardPieces[tx][ty]; //
			if (tmpTarget->side == 0)
			{	//if the ajdacent tile is *(dummy piece)
				//since there is only one dummy piece we need to update its x and y position.
				tmpTarget->xPos = tx;
				tmpTarget->yPos = ty;
			}

			//if decideCheckFunction returns 1, then it is valid move!
			//update the move, then control if it is triggers "check" for enemy!
			//if it triggers, then undo the move, if not then return 0 (which is not checkmate!)
			if (decideCheckFunction(boardPtr, game, tarKing, tmpTarget))
			{

				updateMove(boardPtr, game, tarKing, tmpTarget, x, y, tx, ty, 1);
				innerFlag = checkCheck(boardPtr, game, tarKing->side, tmpTarget->side, 1);
				// if innerFlag == 1 then, move triggers check for enemy.
				// wprintf(L"inF:%d, kTx:%d kTy:%d\n",innerFlag,tx,ty);
				if (innerFlag != 1)
				{ //if not triggers checkmate for you
					undoMove(boardPtr, game, tarKing, tmpTarget, x, y, tx, ty, 1);
					return 0;
				}
				undoMove(boardPtr, game, tarKing, tmpTarget, x, y, tx, ty, 1);
			}
		}
	}

	tx = checkMatePiece->xPos; //global piece who's causing declaring "check"
	ty = checkMatePiece->yPos;

	// wprintf(L"x:%d y:%d side:%d type:%d\n",tx,ty,checkMatePiece->side,checkMatePiece->pieceType);
	//----------------------------------------------------------------
	//attack the "checkMatePiece", if there is at least one piece whick can kill checkmatepiece
	//and do not trigger "check" for enemy then return 0(not checkmate)
	for (int i = 0; i < 32; i++)
	{
		tmpAttacker = &(game->pieces[i]);
		if (tmpAttacker->side == tarKing->side && tmpAttacker->alive == 1)
		{
			x = tmpAttacker->xPos;
			y = tmpAttacker->yPos;
			// wprintf(L"Before Pass Attacker info: x:%d y:%d side:%d type:%d\n",x,y,tmpAttacker->side,tmpAttacker->pieceType);
			if (decideCheckFunction(boardPtr, game, tmpAttacker, checkMatePiece))
			{ //if there is a piece to attack "checkMatePiece"
				// wprintf(L"Checkmatepieceinfo: alive:%d x:%d y:%d\n",checkMatePiece->alive,checkMatePiece->xPos,checkMatePiece->yPos);
				updateMove(boardPtr, game, tmpAttacker, checkMatePiece, x, y, tx, ty, 1);
				// wprintf(L"Checkmatepieceinfo: alive:%d x:%d y:%d\n",checkMatePiece->alive,checkMatePiece->xPos,checkMatePiece->yPos);
				// wprintf(L"Pass Attacker info: x:%d y:%d side:%d type:%d\n",x,y,tmpAttacker->side,tmpAttacker->pieceType);
				innerFlag = checkCheck(boardPtr, game, tmpAttacker->side, checkMatePiece->side, 1);
				// wprintf(L"Checkmatepieceinfo: alive:%d x:%d y:%d\n",checkMatePiece->alive,checkMatePiece->xPos,checkMatePiece->yPos);

				// wprintf(L"Flag result:%d\n",innerFlag);
				if (innerFlag != 1)
				{
					undoMove(boardPtr, game, tmpAttacker, checkMatePiece, x, y, tx, ty, 1);
					return 0; //not checkmate
				}

				undoMove(boardPtr, game, tmpAttacker, checkMatePiece, x, y, tx, ty, 1);
				// wprintf(L"Checkmatepieceinfo: alive:%d x:%d y:%d\n",checkMatePiece->alive,checkMatePiece->xPos,checkMatePiece->yPos);
			}
		}
	}

	//----------------------------------------------------------------
	//trying to block checkMatePiece projection towards tarKing
	int kx = tarKing->xPos; //x,y position of tarking
	int ky = tarKing->yPos;
	tx = checkMatePiece->xPos;
	ty = checkMatePiece->yPos;

	int dx = tx - kx;
	int dy = ty - ky;

	int lim = maxNum(abs(dx), abs(dy)) - 1; //number of tiles between checkmatePiece and tarking
	if (lim > 0)
	{								 //if they are not next to each other.
		for (int k = 0; k < 32; k++) //go through each piece
		{
			tmpAttacker = &(game->pieces[k]);
			x = tmpAttacker->xPos;
			y = tmpAttacker->yPos;
			// if tmpAttacker and tarking in the same side and it is alive
			if (tmpAttacker->side == tarKing->side && tmpAttacker->alive == 1)
			{
				tx = checkMatePiece->xPos;
				ty = checkMatePiece->yPos;
				//for each tmpAtacker, try to move every tile between checkmatePiece and tarking
				for (int i = 0; i < lim; i++)
				{
					if (dx != 0)
						tx -= dx / abs(dx); // tx = tx - (tx - kx)/abs(tx-kx)
					if (dy != 0)
						ty -= dy / abs(dy);
					tmpTarget = boardPtr->boardPieces[tx][ty];
					if (tmpTarget->side == 0)
						tmpTarget->xPos = tx;
					tmpTarget->yPos = ty;

					//if tmpAttacker can move one of these tile then update the move and control if it triggers
					//check for enemy. If it does not triggers then return 0(not checkmate)
					if (decideCheckFunction(boardPtr, game, tmpAttacker, tmpTarget))
					{
						//Update the move
						updateMove(boardPtr, game, tmpAttacker, tmpTarget, x, y, tx, ty, 1);

						//control if it triggers check for enemy,
						// recall: checkCheck returns 1 if it triggers check for enemy
						innerFlag = checkCheck(boardPtr, game, tmpAttacker->side, tmpTarget->side, 1);
						if (innerFlag != 1)
						{
							// wprintf(L"NOT CHECKMATE x:%d y:%d tx:%d ty:%d\n",x,y,tx,ty);
							//undo the move, it is not checkmate
							undoMove(boardPtr, game, tmpAttacker, tmpTarget, x, y, tx, ty, 1);
							return 0; //not checkmate
						}
						undoMove(boardPtr, game, tmpAttacker, tmpTarget, x, y, tx, ty, 1);
					}
				}
			}
		}
	}

	// It is checkmate
	return 1; //checkmate
}

int checkCheck(Board *board, Game *game, int atkS, int tarS, int mateControl)
{

	// if mateControl==1, then do not control "check for you". Only control if the move triggers check for enemey!.
	//return 0 if there is no "check", return 1 if causes "check" for enemy, return 2 if triggers "check" for you.
	chessPiece *atkKing;
	chessPiece *tarKing;
	chessPiece *tmpTargetAttacker;
	if (atkS == 1)
	{ //attackter white, deffend black.
		atkKing = &(game->pieces[29]);
		tarKing = &(game->pieces[31]);
	}
	else if (atkS == 2)
	{
		atkKing = &(game->pieces[31]);
		tarKing = &(game->pieces[29]);
	}

	int tx = tarKing->xPos;
	int ty = tarKing->yPos;
	int x; //attackter x,y pos
	int y;

	int checkFlag = 0;

	//go through every pieces, if the one of the enemy pieces can
	//attack current player(your) king then it is invalid move (triggers check for enemy!)!

	for (int i = 0; i <= 32; i++)
	{
		tmpTargetAttacker = &(game->pieces[i]);
		if (tmpTargetAttacker->side == tarKing->side && tmpTargetAttacker->alive == 1)
		{
			x = tmpTargetAttacker->xPos;
			y = tmpTargetAttacker->yPos;
			// wprintf(L"INCHECK x:%d y:%d a:%d t:%d\n",x,y,tmpTargetAttacker->alive,tmpTargetAttacker->pieceType);
			checkFlag = decideCheckFunction(board, game, tmpTargetAttacker, atkKing);
			if (checkFlag == 1)
			{
				// wprintf(L"INNkEST x:%d y:%d a:%d t:%d\n",x,y,tmpTargetAttacker->alive,tmpTargetAttacker->pieceType);

				// wprintf(L"Causes enemy pieces to trigger check!!!\n");
				return 1;
			}
		}
	}

	//if mateControl==1 then to not enter this loop.
	//for every pieces, try to atack enemy king, if it can attack enemy king then return 2 (check!);
	if (!mateControl)
	{
		//player check control, return 2 if triggers "check" for you.
		for (int i = 0; i <= 32; i++)
		{
			tmpTargetAttacker = &(game->pieces[i]);
			if (tmpTargetAttacker->side == atkKing->side && tmpTargetAttacker->alive == 1)
			{
				x = tmpTargetAttacker->xPos;
				y = tmpTargetAttacker->yPos;
				checkFlag = decideCheckFunction(board, game, tmpTargetAttacker, tarKing);
				if (checkFlag == 1)
				{
					checkMatePiece = tmpTargetAttacker;
					// wprintf(L"Damnn mate!! I'll check!!!\n");
					return 2;
				}
			}
		}
	}

	return 0;
}

int decideCheckFunction(Board *boardPtr, Game *game, chessPiece *atkP, chessPiece *tarP)
{
	//call pieceMoveCheck function with respect to atkPs type;
	int tmpCheckVal = 0;
	int attackType = atkP->pieceType;

	switch (attackType)
	{
	case 1:
		// pawn call
		// wprintf(L"pawn call\n");
		tmpCheckVal = pawnMoveCheck(game, boardPtr, atkP, tarP);
		break;
	case 2:
		//knight call
		// wprintf(L"knight call\n");
		tmpCheckVal = knightMoveCheck(boardPtr, atkP, tarP);
		break;
	case 3:
		//bishop call
		// wprintf(L"bishop call\n");
		tmpCheckVal = bishopMoveCheck(boardPtr, atkP, tarP);
		break;
	case 4:
		//rook call
		// wprintf(L"rook call\n");
		tmpCheckVal = rookMoveCheck(boardPtr, atkP, tarP);
		break;
	case 5:
		//queen call
		// wprintf(L"queen call\n");
		tmpCheckVal = rookMoveCheck(boardPtr, atkP, tarP);
		tmpCheckVal += bishopMoveCheck(boardPtr, atkP, tarP);
		break;
	case 6:
		//king call
		// wprintf(L"king call\n");
		tmpCheckVal = kingMoveCheck(boardPtr, game, atkP, tarP);
		break;
	}

	return tmpCheckVal;
}

int pawnMoveCheck(Game *game, Board *board, chessPiece *atkP, chessPiece *tarP)
{


	int x = atkP->xPos; //current position
	int y = atkP->yPos;
	int tx = tarP->xPos; //target position
	int ty = tarP->yPos;
	int atkS = atkP->side; //pieces' side
	int tarS = tarP->side;
	int dire = x - tx; //movement direction
	int atkT = atkP->playTimes;


		/// general pawn control.
	if (atkS == 2 && dire < 0 || atkS == 1 && dire > 0)
	{ //directin checked.
		// wprintf(L"directin can not be reverse\n");
		return 0;
	}


	//passans control.
	if ((atkS == 1 && tx == 5 || atkS == 2 && tx == 2) && tarP->side == 0)
	{
		int px; //passant piece position.
		//passant piece possition
		if (atkS == 1)
			px = tx - 1;
		else
			px = tx + 1;

		chessPiece *passantPiece = board->boardPieces[px][ty]; // it a pawn piece which causes the passant.
		chessPiece *atkKing;								   //attacker King piece
		chessPiece *tarKing;								   //enemy(target) King piece
		chessPiece *tmpAttacker;							   //tmpAttacker to check "check"

		if (atkS == 1) //if attacker is white
		{
			atkKing = &(game->pieces[29]); //white
			tarKing = &(game->pieces[31]); //black
		}
		else if (atkS == 2)
		{
			atkKing = &(game->pieces[31]);
			tarKing = &(game->pieces[29]);
		}

			//if pawn moved diagonal and passants piece played last turn and its type is pawn
			if (abs(tx - x) == 1 && abs(ty - y) == 1 && passantPiece->playedTurn == game->currentRound - 1 && passantPiece->pieceType == 1)
			{

				//replace [x][y] to dummy piece(empty *)
				board->boardPieces[x][y] = &(game->pieces[32]);
				board->boardPieces[tx][ty] = atkP; //update atkP to tx,ty
				passantPiece->alive = 0;		   //kill passantpiece

				// wprintf(L"Tarking info: x:%d y:%d side:%d\n",atkKing->xPos,atkKing->yPos,atkKing->side);

				for (int k = 0; k < 32; k++)
				{
					tmpAttacker = &(game->pieces[k]);
					if (tmpAttacker->side != atkP->side && tmpAttacker->alive == 1)
					{
						//we need to check if there is a piece which can attack attackerKing after passant move

						// wprintf(L"TmpAttackerInfo x:%d y:%d tpye:%d\n",tmpAttacker->xPos,tmpAttacker->yPos,tmpAttacker->pieceType);
						int mm = decideCheckFunction(board, game, tmpAttacker, atkKing);
						// wprintf(L"mm %d\n",mm);

						//if there is at least one piece that can attack attackerKing, then the move is invalid!. undo the movements and return 0
						if (mm != 0)
						{

							passantPiece->alive = 1;
							board->boardPieces[x][y] = atkP;
							board->boardPieces[tx][ty] = &(game->pieces[32]);
							// wprintf(L"passant basarisiz!\n");
							return 0; //ınvalid, causes check for enemy
						}
					}
				}

				//passant basarlisi! tasi oyna. piyonu öldür. oyun turunu arttır!.

				atkP->xPos = tx;
				atkP->yPos = ty;
				atkP->playedTurn = game->currentRound;
				atkP->playTimes++;

				game->currentPlayer = 3 - game->currentPlayer; //??
				game->currentRound++;

				int checkFlag = 0;
				//after the passant move, if any of our piece can attack enemy piece, then it is "check"
				for (int m = 0; m < 32; m++)
				{
					tmpAttacker = &(game->pieces[m]);
					if (tmpAttacker->side != tarKing->side && tmpAttacker->alive == 1)
					{
						if (decideCheckFunction(board, game, tmpAttacker, tarKing))
						{
							checkFlag = 1;
							checkMatePiece = tmpAttacker;
						}
					}
				}
				if (checkFlag)
					printf("Check!\n");

				return 4; //passant basarili
			}
		
	}



	// initial movement control.
	if (tarS == 0 && atkT == 0 && abs(dire) <= 2)
	{
		if (abs(y - ty) == 0)
		{
			// wprintf(L"kotu boom\n");
			return 1;
		}
		else
			// wprintf(L"boom\n");
			return 0;
	}
	//attack check
	else if ((atkS + tarS) == 3 && (abs(y - ty) != 1 || abs(x - tx) != 1))
	{
		// wprintf(L"aS:%d tS:%d\n",atkS,tarS);
		// wprintf(L"Invalid attack!\n");

		return 0;
	}

	else if (abs(x - tx) != 1)
	{ //last possible move, only one forward.
		// wprintf(L"Invalid move!\n");
		return 0;
	}
	else if (atkS == tarS)
	{ //tas tas uste geliyor.
		// wprintf(L"Invalid move! Collision!\n");
		return 0;
	}

	if (abs(y - ty) == 1)
	{
		if ((atkS + tarS) == 3 && abs(tx - x) == 1)
			return 1;
		else
			return 0;
	}

	// wprintf(L"kotu boom v2\n");
	return 1;
}

int knightMoveCheck(Board *board, chessPiece *atkP, chessPiece *tarP)
{
	int x = atkP->xPos;
	int y = atkP->yPos;
	int tx = tarP->xPos;
	int ty = tarP->yPos;
	int attackSide = atkP->side;
	int targetSide = tarP->side;

	int totalMov = abs(x - tx) + abs(y - ty); //should be 3. sum of dx+dy should be 3!
	int xMov = abs(x - tx);					  // should be 1 or 2. It is enough to chech only x.
	//knight moves with "L" shape whether or not there is enemy.
	if (attackSide == targetSide)
	{ // friend piece collision.
		// wprintf(L"Friendly piece collision!\n");
		return 0;
	}
	else if (totalMov != 3 || xMov == 0 || xMov == 3)
	{
		// If the move is not "L" shape then return 0,Invalid move
		// wprintf(L"Invalid move! Knight moves as L shape! \n");
		return 0;
	}

	return 1;
}

int bishopMoveCheck(Board *board, chessPiece *atkP, chessPiece *tarP)
{

	int x = atkP->xPos;
	int y = atkP->yPos;
	int tx = tarP->xPos;
	int ty = tarP->yPos;
	int attackSide = atkP->side;
	int targetSide = tarP->side;

	//bishop moves as diagonal. Through moving path, should check collision with other pieces.
	int tmpx = x; // Temporary x,y for collision detection.
	int tmpy = y;
	int difx = tx - x;
	int dify = ty - y;
	int tmpSide;

	if (abs(x - tx) != abs(y - ty))
	{ //diagonal move check!
		// wprintf(L"Invalid move! Bishop moves diagonal!\n");
		return 0;
	}
	else if (attackSide == targetSide)
	{ //friendly collision check! Attack tile side should be different.
		// wprintf(L"Invalid bishop move! Collision!\n");
		return 0;
	}

	//there should be no piece between atkP and tarP,(if there is one, it should be dead(alive==0))
	for (int i = 0; i < abs(difx) - 1; i++)
	{
		tmpx += difx / abs(difx);
		tmpy += dify / abs(dify);

		tmpSide = board->boardPieces[tmpx][tmpy]->side;
		if (tmpSide != 0 && board->boardPieces[tmpx][tmpy]->alive != 0)
		{
			// wprintf(L"Invalid bishop move! Collision through path!!!\n");
			// wprintf(L"Collision cordinates %d %d\n",tmpx,tmpy);

			return 0;
		}
	}

	return 1;
}

int rookMoveCheck(Board *board, chessPiece *atkP, chessPiece *tarP)
{
	int x = atkP->xPos;
	int y = atkP->yPos;
	int tx = tarP->xPos;
	int ty = tarP->yPos;
	int attackSide = atkP->side;
	int targetSide = tarP->side;

	//rook moves only in one direction, difx or dify should be zero!
	int tmpx = x;
	int tmpy = y;
	int tmpSide; //temporary board id for collision check

	int difx = tx - x;
	int dify = ty - y;

	if (difx != 0 && dify != 0)
	{ //should move only in one direction
		// wprintf(L"Invalid move! Rook moves only in one direction\n");
		return 0;
	}
	else if (attackSide == targetSide)
	{ //friendly collision
		// wprintf(L"Invalid move! Collision!\n");
		return 0;
	}

	//for every pieces between atkP and tarP, check if there is piece.
	//If there is piece and its side != 0 and ->alive!=0 then return 0
	for (int i = 0; i < abs(difx + dify) - 1; i++)
	{
		if (difx != 0)
		{ //prevent divison by zero!
			tmpx += difx / abs(difx);
		}
		else
		{
			tmpy += dify / abs(dify);
		}

		tmpSide = board->boardPieces[tmpx][tmpy]->side;
		if (tmpSide != 0 && board->boardPieces[tmpx][tmpy]->alive != 0)
		{
			// wprintf(L"Invalid rook move! Collision through path!!!\n");
			// wprintf(L"Collision cordinates side:%d %d %d\n",tmpSide, tmpx, tmpy);
			return 0;
		}
	}

	return 1;
}

int kingMoveCheck(Board *board, Game *game, chessPiece *atkP, chessPiece *tarP)
{
	int x = atkP->xPos; //attacker x,y position
	int y = atkP->yPos;
	int tx = tarP->xPos; //target x,y position
	int ty = tarP->yPos;
	int attackSide = atkP->side; //attacker side
	int targetSide = tarP->side; //targget site
	int pT = atkP->playTimes;	 //attacker played timed, need this for castling

	int dx = tx - x;
	int dy = ty - y;
	int tmpSide; //collision checker for king-rook switch
	int tmpy = y;

	chessPiece otherKing; //white 29, black 31. King can not move next to other king.
	if (attackSide == 1)
	{
		otherKing = game->pieces[31];
	}
	else if (attackSide == 2)
	{
		otherKing = game->pieces[29];
	}

	int ox = otherKing.xPos; //other king x,y position
	int oy = otherKing.yPos;

	//// CASTLING CHECH PART
	int targetType = board->boardPieces[tx][ty]->pieceType; //rook=4
	int targetPT = board->boardPieces[tx][ty]->playTimes;	//rook can not be played before for castling.

	//IF THE TARGET IS ROOK, AND BOTH PIECE IS NOT PLAYED BEFORE
	if (dx == 0 && pT == 0 && targetSide == attackSide && targetType == 4 && targetPT == 0)
	{
		for (int i = 0; i < abs(dy) - 1; i++)
		{ // collision check for castling
			tmpy += dy / abs(dy);
			tmpSide = board->boardPieces[x][tmpy]->side;

			//CHECK IF THERE IS PIECE BETWEEN ROOK AND KING
			if (tmpSide != 0 && board->boardPieces[x][tmpy]->alive != 0)
			{
				// wprintf(L"Invalid move, collision occured! rook-king switch!\n");
				return 0;
			}
		}

		int sx;
		int sy; //saver for * piece x and y
		chessPiece *tmpAttack;
		chessPiece *tmpTarget;
		int innerCheck; // flag for output of "validattack"
		//king ve rook arası (king ve rook) dahil herhangi bir tasa dusman saldiramamali.
		for (int k = 0; k <= abs(dy); k++)
		{ //collision check.
			tmpy = y + k * dy / abs(dy);
			// wprintf(L"tmpy %d\n", tmpy);
			tmpTarget = board->boardPieces[x][tmpy];
			sx = tmpTarget->xPos; //save target pieces position
			sy = tmpTarget->yPos;
			tmpTarget->xPos = x;
			tmpTarget->yPos = tmpy;

			//IF THERE IS AT LEAST ONE PIECE WHICH CAN ATTACK TO (KING,ROOK OR THE PATH BETWEEN THEM) THEN RETURN 0
			//IF NOT RETURN 2 WHICH IS FOR CASTLING
			for (int j = 0; j < 32; j++)
			{
				tmpAttack = &(game->pieces[j]);
				//we checked the collision before so, we only need to check if there is valid attack for enemy pieces.
				if (tmpAttack->side == 3 - attackSide && tmpAttack->alive == 1)
				{
					innerCheck = decideCheckFunction(board, game, tmpAttack, tmpTarget);
					if (innerCheck == 1)
					{
						tmpTarget->xPos = sx;
						tmpTarget->yPos = sy;
						// wprintf(L"x:%d,y:%d tx:%d ty:%d", tmpAttack->xPos, tmpAttack->yPos,
						// 		tmpTarget->xPos, tmpTarget->yPos);
						// wprintf(L"side%d type:%d\n", tmpAttack->side, tmpAttack->pieceType);
						return 0;
					}
				}
			}

			tmpTarget->xPos = sx;
			tmpTarget->yPos = sy;
		}

		// wprintf(L"The length of castling : %d \n", abs(dy));
		//decide if its
		// if abs(dy) == 4 long castling, if abs(dy) == 3 sort castling
		if (abs(dy) == 4)
		{
			//apply long castling
			board->boardPieces[x][y]->yPos = 2;
			board->boardPieces[x][y]->playTimes++;

			board->boardPieces[x][ty]->yPos = 3;
			board->boardPieces[x][ty]->playTimes++;

			game->currentPlayer = 3 - game->currentPlayer;
			game->currentRound++;
		}
		else if (abs(dy) == 3)
		{
			//apply short castling
			board->boardPieces[x][y]->yPos = 6;
			board->boardPieces[x][y]->playTimes++;

			board->boardPieces[x][ty]->yPos = 5;
			board->boardPieces[x][ty]->playTimes++;

			game->currentPlayer = 3 - game->currentPlayer;
			game->currentRound++;
		}

		return 2;
	}

	///KING ROOK SWITCH CHECK END

	//GENERAL KING MOVE CHECK
	//KING CAN NOT MOVE MORE THAN 1 TILE
	if (abs(dx) > 1 || abs(dy) > 1 || targetSide == attackSide)
	{
		// wprintf(L"Invalid King move!\n");
		return 0;
	}

	//last rule for king move! King can not move next to other King!
	if (abs(tx - ox) == 1 && abs(ty - oy) == 1)
	{
		// wprintf(L"Invalid move! Kings can not move nex to other king.\n");
		return 0;
	}

	return 1;
}



void printChessPiece(chessPiece *piece){
//set code respect to pieces type and side
	int s = piece->side;	  //piece side
	int t = piece->pieceType; //piece type
	//Pawn(1), Knight(2), Bishop(3), Rook(4), Queen(5), King(6)
	if (s == 2)
	{
		if (t == 1)
			printf("\u2659");
		else if (t == 2) //knight
			printf("\u2658");
		else if (t == 3)
			printf("\u2657");
		else if (t == 4)
			printf("\u2656");
		else if (t == 5)
			printf("\u2655");	
		else if (t == 6) //king
			printf("\u2654");
	return;
	}
	else if (s == 1)
	{

		if (t == 1)
			printf("\u265F");
		else if (t == 2)
			printf("\u265E");
		else if (t == 3)
			printf("\u265D");
		else if (t == 4)
			printf("\u265C");
		else if (t == 5)
			printf("\u265B");
		else if (t == 6)
			printf("\u265A");
	return;
	}
	else
	{	
		printf("\u00A0");//0x00A0;//0x002B;
		return;
	}
}

int pieceInit(chessPiece *piece, int xPos, int yPos, int side, int pType)
{
	//piece initializing function
	piece->side = side;
	piece->pieceType = pType;
	piece->xPos = xPos;
	piece->yPos = yPos;
	piece->alive = 1;
	piece->playTimes = 0;
	piece->playedTurn = 0;

	if (side == 0 && pType == 0)
	{ //dumpy piece
		piece->alive = 0;
	}

	return 0;
}

void gameInit(Game *game)
{
	/* Reminder
		// Pawn(1), Knight(2), Bishop(3), Rook(4), Queen(5), King(6) ~~pieceType
		// White(1), Black(2) ~~side
		*/
	pieceInit(&game->pieces[32], 0, 0, 0, 0); //last dummy piece, in order to show * at boards
	for (int i = 0; i <= 15; i++)
	{ //initializing pawns, first 16 piece are pawn
		if (i <= 7)
		{
			pieceInit(&(game->pieces[i]), 1, i % 8, 1, 1); //white Pawns top
		}
		else
		{
			pieceInit(&(game->pieces[i]), 6, i % 8, 2, 1); //black Pawns bottom
		}
	}
	// pieceInit(&(game->pieces[0]),1,0,1,1);

#pragma region pieceInitialization

	pieceInit(&game->pieces[16], 0, 0, 1, 4); //white left  Rook
	pieceInit(&game->pieces[17], 0, 7, 1, 4); //white right Rook

	pieceInit(&game->pieces[18], 7, 0, 2, 4); //black left  Rook
	pieceInit(&game->pieces[19], 7, 7, 2, 4); //black right Rook

	pieceInit(&game->pieces[20], 0, 1, 1, 2); //white left  knight
	pieceInit(&game->pieces[21], 0, 6, 1, 2); //white right knight

	pieceInit(&game->pieces[22], 7, 1, 2, 2); //black left  knight
	pieceInit(&game->pieces[23], 7, 6, 2, 2); //black right knight

	pieceInit(&game->pieces[24], 0, 2, 1, 3); //white left  bishop
	pieceInit(&game->pieces[25], 0, 5, 1, 3); //white right bishop

	pieceInit(&game->pieces[26], 7, 2, 2, 3); //black left  bishop
	pieceInit(&game->pieces[27], 7, 5, 2, 3); //black right bishop

	pieceInit(&game->pieces[28], 0, 3, 1, 5); //white queen
	pieceInit(&game->pieces[29], 0, 4, 1, 6); //white king

	pieceInit(&game->pieces[30], 7, 3, 2, 5); //black queen
	pieceInit(&game->pieces[31], 7, 4, 2, 6); //black king
#pragma endregion

	game->currentPlayer = 1;

	game->whiteTime = 600; //10min???
	game->blackTime = 600;
	game->currentRound = 0;

	boardInitialize(game);
}

void boardInitialize(Game *game)
{
	//struct Board* ptrBoard = game->board;

	int tmpX;
	int tmpY;
	Board *boardPtr = &(game->board);
	chessPiece *tmpPiece;
	// clear all board before updating
	for (int i = 0; i <= 7; i++)
	{ //vertical
		for (int j = 0; j <= 7; j++)
		{ //horizantal
			boardPtr->boardPieces[i][j] = &(game->pieces[32]);
			// boardPtr->boardCheckPieces[i][j] = (game->pieces[32]);
		}
	}

	//updating board with chess pieces's Code.
	for (int i = 0; i < 32; i++)
	{ //for every pieces.

		tmpX = game->pieces[i].xPos;
		tmpY = game->pieces[i].yPos;

		tmpPiece = &(game->pieces[i]);
		if (tmpPiece->alive == 1)
			boardPtr->boardPieces[tmpX][tmpY] = tmpPiece;

		// boardPtr->boardCheckPieces[tmpX][tmpY] = (game->pieces[i]);
	}
}

void displayBoard(Game *game)
{	
	
	Board *boardPtr = &(game->board);
	boardInitialize(game); //cok onemli!.
	//I took the display idea from https://www.vanheusden.com/pos/.
	//but code it by myself.
	for (int i = 0; i < 8; i++)
	{ //vertical print (up to down)
		printf("--+---+---+---+---+---+---+---+---+\n");
		printf(" %d|", 8 - i);

		for (int j = 0; j < 8; j++)
		{ //horizantal (left to right)

			// wchar_t tmpBoardCode = boardPtr->boardCode[i][j];
			printf(" ");
			printChessPiece(boardPtr->boardPieces[i][j]);
			printf(" |");
		}

		printf("\n");
	}
	printf("--+---+---+---+---+---+---+---+---+\n  |");
	for (int j = 0; j < 8; j++)
	{
		printf(" %c |", 65 + j);
	}
	double remTime;
	char currPlayer[7];

	if (game->currentPlayer == 1)
	{
		remTime = game->whiteTime;
		//currPlayer = "WHITE\0";
		strcpy(currPlayer, "WHITE\0");
	}
	else
	{
		remTime = game->blackTime;
		// currPlayer = "BLACK\0";
		strcpy(currPlayer, "BLACK\0");
	}

	printf("\n%s  Time:%d min %.2d sec. \n", currPlayer, (int)remTime / 60, maxNum((int)remTime % 60, 0));
	printf("-----------------------------------\n");

	//if the time is run out, end the game!! congrats winner.

	if (remTime < 0)
	{
		printf("The time is up!!! \n");
		congratsWinner(game, 3 - game->currentPlayer);
	}

	globClock = clock();
}

void congratsWinner(Game *game, int side)
{
	printf("----------------------------------\n");
	if (side == 1)
		printf("Winner is white. Congrats!\n");
	else if (side == 2)
		printf("Winner is black. Congrats!\n");

	printf("Going to menu!\n");
	// wprintf(L"----------------------------------\n");

	char strInput[20];
	menuDisp();
	fgets(strInput, 20, stdin);
	menu(game, strInput);
}

void updateMove(Board *boardPtr, Game *game, chessPiece *atkP,
				chessPiece *tarP, int x, int y, int tx, int ty, int test)
{

	//update the move from x,y to tx,ty
	boardPtr->boardPieces[tx][ty] = atkP;
	atkP->xPos = tx;
	atkP->yPos = ty;
	atkP->playTimes++;

	boardPtr->boardPieces[x][y] = &(game->pieces[32]);
	boardPtr->boardPieces[x][y]->xPos = x;
	boardPtr->boardPieces[x][y]->yPos = y;

	//if it is not dummy piece
	if (tarP->side != 0)
		tarP->alive = 0; //dead

	//if it is not testing
	if (!test)
	{
		
		atkP->playedTurn = game->currentRound;//save last played turn
	}
}

void undoMove(Board *boardPtr, Game *game, chessPiece *atkP, chessPiece *tarP, int x, int y, int tx, int ty, int test)
{
	//undo the move from tx,ty to x,y
	if (tarP->side != 0)
		tarP->alive = 1; //revive if dead

	boardPtr->boardPieces[tx][ty] = tarP;
	tarP->xPos = tx;
	tarP->yPos = ty;

	boardPtr->boardPieces[x][y] = atkP;
	atkP->xPos = x;
	atkP->yPos = y;
	atkP->playTimes--;
}

void decide(Game *game, char str[20])
{
	//decide what to do with given input

	//if it is "m" then go to menu
	if ((str[0] == 'm' || str[0] == 'M') && str[2] == '\0')
	{ //menu
		printf("Do you want to see the menu? 1:Yes 2:No!\n");
		// playerTimeUpdate(game);
		fgets(str, 20, stdin);
		if (str[0] == '1' && str[2] == '\0')
		{

			printf("Saving the game!\n");

			time_t t = time(NULL);
			struct tm tm = *localtime(&t);
			char timeS[30] = "saved\\\\"; // two "\\" needed.
			time2str(&tm, timeS);
			FILE *outfile;
			// wprintf(L"%S string\n",timeS);
			outfile = fopen(timeS, "w");
			if (outfile == NULL)
			{ //if could not create.
				fprintf(stderr, "\nError while saving the game!.\n");
				return;
			}
			else
			{
				fwrite(game, sizeof(struct Game), 1, outfile);
				fclose(outfile);
				printf("Game Saved!\n");
			}

			// globClock = clock(); //save the game, then go to menu!

			menuDisp();
			fgets(str, 20, stdin);
			menu(game, str);
			// playerTimeUpdate(game); //menuye gecmeden once suresı guncellenıyor.
			// globClock = clock();
			return;
		}
		else if (str[0] == '2' && str[2] == '\0')
		{

			printf("Continue the game\n");

			return; //
		}
		else
		{
			printf("Invalid input.\n"); //if the input is not correct!
			str[0] = 'm';				  //send quit request again
			str[2] = '\0';
			decide(game, str);
			return;
		}

	} //if want to save!
	else if ((str[0] == 's' || str[0] == 'S') && str[2] == '\0')
	{ //save

		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		char timeS[30] = "saved\\\\"; // two \\ needed.
		time2str(&tm, timeS);
		FILE *outfile;

		printf("Saving the game!\n");

		outfile = fopen(timeS, "w");

		// outfile = fopen(timeS, "w");
		if (outfile == NULL)
		{ //if could not create.
			fprintf(stderr, "\nError opend file\n");
			// exit(1); //
			return;
		}
		else
		{
			fwrite(game, sizeof(struct Game), 1, outfile);
			fclose(outfile);
			printf("Game Saved!\n");
		}
	}
	else if (str[5] == '\0')
	{
		// wprintf(L"MoveKey!\n");
		//control if the given inpus is corrent for move!(A2B4)
		for (int i = 0; i < 4; i++)
		{
			int a;
			if ((a = char2int(&str[i], i)) > 7)
			{
				printf("Invalid Input!!\n");

				return;
			}
		}

		int c = move(game, str); //move returns 5 if the game is ended(checkmate)
		if (c == 5)
		{
			congratsWinner(game, game->currentPlayer);
		}

	}
	else
	{
		printf("Invalid Input\n");
	}

	return;
}

int menu(Game *game, char str[20])
{
	//MENU, 1:start game, 2:load game, 3:exit
	if (str[0] == '1' && str[2] == '\0')
	{
		printf("The game startes!\n");
		globClock = clock(); //update clock then initialize game
		gameInit(game);
		globClock = clock();
		return 1;
	}
	else if (str[0] == '2' && str[2] == '\0')
	{
		// I took this part from the internet, but I understood the code.
		// create DIR object then search all the files in it then print all files in "saved" folder
		DIR *d;
		struct dirent *dir;
		d = opendir("saved");
		if (d)
		{
			printf("-----------------------------\n");
			printf("The load files listed below.\n");
			while ((dir = readdir(d)) != NULL)
			{

				printf("%s\n", dir->d_name);
				//https://docs.microsoft.com/en-us/previous-versions/hf4y5e3w(v=vs.140)?redirectedfrom=MSDN
				// S for string in wprintf
			}
			closedir(d);
			printf("-----------------------------\n");
		}

		char strFile[100] = "saved\\\\";

		printf("Enter the name of the save file!!\n");
		fgets(strFile + 7, 92, stdin); //first 7 element == "saved\\"
		strFile[27] = '\0'; //tum save dosyalarının uzunlugu 26. save\\28122020_11.25pm.dat
		// wprintf(L"given input is: %S\n",strFile);
		if ((strFile[7] == 'm' || strFile[7] == 'M') && strFile[9] == '\0')
		{//go to menu if 'm' or 'M' is played
			{
				menu(game, strFile);
				// wprintf(L"menu returned\n");
				return 0;
			}
		}
		FILE *inFile;
		inFile = fopen(strFile, "r");
		if (inFile == NULL)
		{	//if cannot open the saved file
			printf("Could not open the file for load game!");
			printf("Be sure to write savedFile name correctly!\n");
			printf("-----------------------------------\n");
			menu(game, str);
			return 0;
			
		}

		while (fread(game, sizeof(struct Game), 1, inFile)){}

		//initizalize the board
		boardInitialize(game);
		fclose(inFile);

		printf("Black Time:%ds, White Time:%ds\n", (int)game->blackTime,(int)game->whiteTime);
		// wprintf(L"menu returned\n");
		return 0;
	}
	else if (str[0] == '3' && str[2] == '\0') //quit game
	{	
		//quit the game
		printf("quit game triggered\n");
		printf("Are you sure to exit? Yes:1, No:2\n");
		fgets(str, 20, stdin);
		if (str[0] == '1' && str[2] == '\0')
			{
			free(game);
			free(str);
			exit(0);}
		else if (str[0] == '2' && str[2] == '\0')
		{
			menuDisp();
			fgets(str, 20, stdin);
			menu(game, str);
		}
		else
		{
			printf("Invalid input.\n");
			str[0] = '3'; //send quit request again
			str[2] = '\0';
			menu(game, str);
		}
	}
	else //wrong input format
	{
		// wprintf(L"#########################\n");
		printf("Invalid Request!\n");
		// wprintf(L"#########################\n");
		menuDisp();
		fgets(str, 20, stdin);
		menu(game, str);
	}

	// wprintf(L"menu returned\n");
	return 0;
}

void menuDisp()
{
	printf("|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|\n");
	printf("|Welcome to the chess!           |\n");
	printf("|1: START NEW GAME!	         |\n");
	printf("|2: LOAD PREVIOUS GAME!          |\n");
	printf("|3: QUIT THE GAME!	         |\n");
	printf("|--------------------------------|\n");
}

int int2char(int a)
{
	// 0 ->> 48, 1-->>49
	return a + 48;
}

void playerTimeUpdate(Game *game)
{	
	//update player time 
	globClock = clock() - globClock; //how many clocks passed
	double time_taken = ((double)globClock) / CLOCKS_PER_SEC;

	if (game->currentPlayer == 1)
	{
		// wprintf(L"white##\n");
		game->whiteTime -= time_taken;
	}
	else
	{
		// wprintf(L"black##\n");
		game->blackTime -= time_taken;
	}
}

void time2str(struct tm *tm, char s[30])
{	
	//create string for saved file name
	// "saved\\" fill first 7 space

	s[7] = int2char((tm->tm_mday) / 10);
	s[8] = int2char((tm->tm_mday) % 10);

	//month
	s[9] = int2char((tm->tm_mon + 1) / 10);
	s[10] = int2char((tm->tm_mon + 1) % 10);

	//Year
	int year = tm->tm_year + 1900;
	s[14] = int2char(year % 10);
	year /= 10;

	s[13] = int2char(year % 10);
	year /= 10;

	s[12] = int2char(year % 10);
	year /= 10;

	s[11] = int2char(year % 10);
	year /= 10;

	// '_'
	s[15] = '_';

	//hours
	int hour = tm->tm_hour;
	int tmph;
	if (hour > 12)
		tmph = hour - 12;
	else
		tmph = hour;
	
	// wprintf(L"h %d\n",tmph);
	s[16] = int2char(tmph/10);
	s[17] = int2char(tmph%10);

	s[18] = '.';
	s[19] = int2char(tm->tm_min / 10);
	s[20] = int2char(tm->tm_min % 10);

	// printf("hour %d\n", hour);
	if (hour >= 12)
	{
		s[21] = 'p';
		s[22] = 'm';
		// printf("122hour %d\n", hour);
	}
	else
	{
		s[21] = 'a';
		s[22] = 'm';
		// printf("kucukkhour %d\n", hour);
	}

	s[23] = '.';
	s[24] = 'd';
	s[25] = 'a';
	s[26] = 't';
	s[27] = '\0';
}

void pawnPromote(Game *game, chessPiece *pawn)
{	
	//promote pawn, change its type and set it code after!
	printf("Choose your pawn promotion\n");
	printf("1: Knight\n");
	printf("2: Bishop\n");
	printf("3: Rook\n");
	printf("4: Queen\n");
	printf("-------------\n");
	//'1' ->49 4-->52
	char strPro[20];
	strPro[0] = '0';
	strPro[2] = '0';

	while (strPro[0] > 52 || strPro[0] < 49 || strPro[2] != '\0')
	{
		fgets(strPro, 20, stdin);
		if (strPro[2] == '\0')
		{
			pawn->pieceType = strPro[0] - 47; //49->2 52->5

		}
		else
		{
			printf("Please enter only 1 digit.\n");
		}
	}

	if(checkCheck(&(game->board),game,pawn->side,3-pawn->side,0)==2){
		printf("Check!\n");
	}
	printf("-------------\n");

}
