// TODO
// make sure the imput contains all the numbers in the range of 1-9 and every is used only and at most once
// make sure the input is not too long or too short (9 characters)

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>



#define SIZE 9
#define PLAYER 1
#define ALIEN 2

//---------------------------------------------------- methods ----------------------------------------------------

    //method to print the board
    void printBoard(int board[SIZE]){
        for (int i = 0; i < SIZE; i++){
            if (board[i] == 0){
                std::cout << " ";
            } else if (board[i] == 1){
                std::cout << "X";
            } else {
                std::cout << "O";
            }
            if (i % 3 == 2){
                std::cout << std::endl;
            } else {
                std::cout << "|";
            }
        }
    }

  //method to play the move
  //gets a pointer to the board, the player number, and the move to play
  //returns true if the move was made and false if the move was invalid
  bool makeMove(int board[SIZE], int player, int move){
    //check if in bounds
    if (move < 1 || move > 9){
        return false;
    }
    //check if the move is valid
        if (board[move-1] == 0){
            board[move-1] = player;
            printBoard(board);
            return true;
        }
        return false;

    }



    //method to check if the game is over that returns 0 if the game is not over, 1 if the player won, 2 if the alien won, 3 if it is a tie
    int checkGameOver(int board[SIZE]){
        //check rows
        for (int i = 0; i < 3; i++){
            if (board[i] == board[i + 3] && board[i] == board[i + 6] && board[i] != 0){
                return board[i];
            }
        }
        //check columns
        for (int i = 0; i < 9; i += 3){
            if (board[i] == board[i + 1] && board[i] == board[i + 2] && board[i] != 0){
                return board[i];
            }
        }
        //check diagonals
        if (board[0] == board[4] && board[0] == board[8] && board[0] != 0){
            return board[0];
        }
        if (board[2] == board[4] && board[2] == board[6] && board[2] != 0){
            return board[2];
        }
        //check if the board is full
        for (int i = 0; i < SIZE; i++){
            if (board[i] == 0){
                return 0;
            }
        }
        return 3;
    }

    //method to print the result of the game
    void printResult(int result){
        if (result == 1){
            std::cout << "Player won" << std::endl;
        } else if (result == 2){
            std::cout << "Alien won init Lunar deportation!!!" << std::endl;
        } else {
            std::cout << "It is a DRAW " << std::endl;
        }
    }

//---------------------------------------------------- main ----------------------------------------------------

int main(int argc, char const *argv[]){

    //make sure only one argument is passed
    if (argc != 2){
        std::cout << "Please enter the input and just the input" << std::endl;
        return 1;
    }
    
    //get the input copy to new int array 
    int input[SIZE];
    for (int i = 0; i < SIZE; i++){
        input[i] = argv[1][i] - '0';
    }
    ///////////////////////////////////////////////const char *input = argv[1];
    // check if the input is the right size
    if (strlen(argv[1]) != SIZE){
        std::cout << "The input is not the right size" << std::endl;
        return 1;
    }

    // check if the input contains all the numbers in the range of 1-9 and every is used only and at most once
    // the check is done by using a boolean array of size 9 so making sure that the input contais all the range of 1-9 is made by the pigeonhole principle
    bool numbers[SIZE] = {false};
    for (int i = 0; i < SIZE; i++){
    
        if (argv[1][i] < '1' || argv[1][i] > '9'){
            std::cout << "The input contains a digite not in the range of 1-9" << std::endl;
            return 1;
        }

            int num = argv[1][i] - '0';
        if (numbers[num - 1]){
            std::cout << "The input contains a number more than once" << std::endl;
            return 1;
        }
        numbers[num - 1] = true;
    }



    //TTT TODO
    //method to check if the game is over that returns 0 if the game is not over, 1 if the player won, 2 if the alien won, 3 if it is a tie
    //method to print the result of the game

    //initialize the game variables: in board 0 is empty, 1 is player, 2 is alien
    int board[SIZE] = {0};
    int gameStatus = 0;
    int alienIndex = 0;

    //play the game until it is over
    while (gameStatus == 0){
        //alien move (2)
        //print the move the alien is about to make
        std::cout << "Alien move: " << argv[1][alienIndex] << std::endl;

        int moveMade = makeMove(board, ALIEN, input[alienIndex]);
        while(!moveMade){
            alienIndex++;
            moveMade = makeMove(board, ALIEN, input[alienIndex]);
        }
        alienIndex++;
        moveMade = false;

        //check if the game is over
        gameStatus = checkGameOver(board);
        if (gameStatus != 0){
            break;
        }


        //player move (1)
        std::cout << "Enter your move: ";
        int playerMove;
        std::cin >> playerMove;
        moveMade = makeMove(board, PLAYER, playerMove);
        while(!moveMade){
            // if(strcmp(std::to_string(playerMove),"") != 0){               // check if the input is not empty
            // std::cout << "invalid move, enter a valid move: ";
            std::cin >> playerMove;
            moveMade = makeMove(board, PLAYER, playerMove);
            // }
        }
        //check if the game is over
        gameStatus = checkGameOver(board);
    
    }
    printResult(gameStatus);

        


    return 0;
}

