// chessProject.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include "chessEngine.c"

int main()
{
	//#pragma warning(disable:4996)

	system("chcp 65001");
	// _setmode(_fileno(stdout), 0x00040000); // 0x00040000 == _O_U8TEXT

	char *strInput;
	strInput = (char *)malloc(20 * sizeof(char));

	Game *game;
	game = (Game *)malloc(1 * sizeof(Game));

	menuDisp();
	fgets(strInput, 20, stdin);
	menu(game, strInput);
	printf("Please make your move.\n");


	while (1 == 1)
	{

		
		displayBoard(game);

		fgets(strInput, 20, stdin);

		playerTimeUpdate(game);
		globClock = clock();
		decide(game, strInput);
		globClock = clock();
	}

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started:
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
