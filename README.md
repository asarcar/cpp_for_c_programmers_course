Author: Arijit Sarcar

HOW TO RUN:
-----------
Pre-requisite Packages: Please ensure that the following packages are already installed: 
cmake libgoogle-glob-dev libgflags-dev

Build the graph library and the test cases
> cd $PROJ_DIR/build
> cmake ../src
> make

RUN TEST CASE
cd $PROJ_DIR
> mkdir -p $PROJ_DIR/tmp
> export TEST_OUTPUT_DIR="./tmp"
> ./bin/unit_tests/utils/mst_test_d --input_file="./data/input2.txt" --output_file="./tmp/mst_output2.txt"
> ./bin/unit_tests/utils/spt_test_d --gen_random_graph_flag=true --gen_random_graph_op_file="./tmp/random_graph_output.txt" --output_file="./tmp/spt_output.txt" --num_vertices=100 --are_edges_directed=false --edge_density=0.75 --min_distance=50 --max_distance=100 --src_vertex_id=4 --dst_vertex_id=8
> ./bin/unit_tests/utils/find_merge_test_d --input_from_file=true --input_file="./data/find_merge_input.txt" --output_file="./tmp/find_merge_output.txt"
> ./bin/unit_tests/utils/bfs_dfs_test_d --input_file="./data/input3.txt" --output_file="./tmp/bfs_dfs_output.txt"
> ./bin/unit_tests/games/hex_test_d --dimension=11 --num_moves=4 --output_dir="./tmp"
> ./bin/unit_tests/games/mc_hex_test_d

VALIDATE OUTPUT
> less mst_output.txt  # shows output of MST Prim run on input.txt graph
> less mst_test_d.INFO # log information dumped from the MST Test application

======================================
ASSIGNMENT INSTRUCTIONS


Objective

Generate an expert Hex move on an 11x11 board. You are to implement a very strong AI that is primarily Monte Carlo based. You should be able to play against this program. This program can be computationally intensive. If you find your move evaluation is working correctly but is too slow, reduce the board size to 9 x 9 or even 7 x 7. You do not want a human playing the computer getting bored waiting for its move.

our Hex Program

Your program should use your Homework 4 work or an improvement on it. 

The player should be able to interact with the program  with blue (or X) going first and red (or O) going second. The program should have a convenient interface for entering a move, displaying the board, and then making its own move. The program should determine when the game is over and announce the winner. 
This program will evaluate a position using a Monte Carlo selection of moves until the board is filled up. Then using work of Homework 4 you determine who won.  The program takes turns. It inputs the human (or machine opponent if playing against another program) move. When s turn, it is to evaluate all legal available next moves and  move.  Each legal move will be evaluated using  ~1000 or more trials. Each trial winds the game forward by randomly selecting successive moves until there is a winner. The trial is counted as a win or loss. The ratio: wins/trials are s metric for picking which next move to make. 
A simple board display would be to have an 11 x 11 printout with B, R, or a blank (or X, O, b) in each position. A simple way to input a move would be to have the player enter an (i,j) coordinate corresponding to a currently empty hexagon and have the program check that this is legal, and if not, ask for another choice. 
You may want to combine this approach with the min-max algorithm (or the more efficient alpha-beta), as described in the videos.

Homework 5 expectations:

The computer should be able to play Hex intelligently against a human on an 11 by 11 board.
Efficiently determine within no more than 2 minutes what the computer's move is.
Some suggestions:

Use a specialized algorithm to determine who won (as opposed to Dijkstra).
Try your algorithm on a 5 x 5 board first to make sure it works. 
Be careful about computational cost To evaluate a specific trial, fill up the remaining empty squares on the board and evaluate the filled up board for who won. This avoids applying the algorithm for who wins each time a move is made. You should understand why this works.
If available, use the new C++11 library <random> to get experience with it.
Resources for computer hex: http://webdocs.cs.ualberta.ca/~hayward/hex/.
More Advanced:

If you have the interest, time and energy, the videos talk about using Minimax evaluation of moves (with Alpha/Beta pruning) as well as Monte Carlo. The two approaches can be combined. Here are some links to these algorithms.

Links for Minimax and Alpha_Beta evaluation:

http://en.wikipedia.org/wiki/Minimax
http://en.wikipedia.org/wiki/Alphabeta_pruning

----

BitBoard

Board representation should be as compact as possible, because it is copied many times during simulation to restore the current state. Use two bitsets to store state of each player.

DFS works quite fast with this simple data structure to determine if the player wins.

Empty Positions List

Store all empty positions in an array/vector, with additional array/vector for indexing. Index array is only used when the actual move is played, and it isn't used in MC simulations.

Copy empty positions only once per simulation, so you can "shuffle" this array to get random positions. Iterate through all possible moves using original array and then just skip current position when it is randomly chosen from the copied array.

Check if the player won when the board is completely filled

Current Player State on the Board

To perform simulation, state of the current player is enough, so copy only half of the BitBoard. State of other player is ignored because when the board is completely filled, and algorithm to determine the winner returns false for the current player, it means that the other player won.

Random Shuffle

Don't shuffle the whole empty positions array, use a slight modification of the Fisher-Yates shuffling algorithm because half of this positions can be ignored.

Possible wins counter

On each MC iteration when the player loses, decrease possible wins counter, and when it is lower than max_wins then stop iterating and go to the next empty position.

----

Here's my HW5 solution. I believe the code quality is good, though there are plenty of optimization opportunities I haven't pursued. (Boris suggested many good ones in this thread.)

You can download a zip file with the code from here:

a5.zip -- http://www.gribble.org/tmp/a5.zip -- this zip file contains a directory with all of my source code in it. Fetch and expand this to make it convenient for you to compile and run my solution.

Or, you can look at the individual files through gist.github.com here:

gameplay.cc -- https://gist.github.com/gribble/d989a934eea4dbfc57e7#file-gameplay-cc -- this file contains "main", and it contains the logic for playing the game of Hex. It makes use of the classes and routines in the other source files in order to render a Hex board, track the state of a Hex board, calculate the next move of the AI, and so on.

HexBoard.h -- https://gist.github.com/gribble/d989a934eea4dbfc57e7#file-hexboard-h -- this file defines the interface to the HexBoard class, which tracks the state of a Hex game and offers an API to make moves, test to see if a player has won, and render a string formatted picture of the game configuration.

HexBoard.cc -- https://gist.github.com/gribble/d989a934eea4dbfc57e7#file-hexboard-cc -- this file contains the implementation of the HexBoard.h class defined above.

AI.h -- https://gist.github.com/gribble/d989a934eea4dbfc57e7#file-ai-h -- this file defines the interface to the AI class, which is used to find the best next move, given a current HexBoard state as an input.

AI.cc -- https://gist.github.com/gribble/d989a934eea4dbfc57e7#file-ai-cc -- the implementation of our AI class. It uses minimax with alpha-beta pruning, and Monte Carlo to estimate board value.

Graph.h -- https://gist.github.com/gribble/d989a934eea4dbfc57e7#file-graph-h -- the Graph class abstraction and implementaton from homeworks 2, 3, and 4.

UnionFindForest.h -- https://gist.github.com/gribble/d989a934eea4dbfc57e7#file-unionfindforest-h -- a Union-Find Forest implementation for tracking set membership efficiently, which Graph.h uses to detect when a player has won the game.

UnionFindForest.cc -- https://gist.github.com/gribble/d989a934eea4dbfc57e7#file-unionfindforest-cc -- the implementation of the Union-Find forest class.

boardsearch.cc -- https://gist.github.com/gribble/d989a934eea4dbfc57e7#file-boardsearch-cc -- an auxiliary program that calculates optimal opening moves and emits them to a cache file. gameplay.cc can use these cache files to short-circuit the calculation of the first few opening moves of a game, speeding up gameplay.

Compiling and running the game:

My code makes heavy use of C++11 features, so you'll need a relatively recent compiler. Using a recent g++, you can compile the game as follows:

   g++ -Wall -std=c++11 -O3 -o gameplay gameplay.cc HexBoard.cc AI.cc UnionFindForest.cc

To run the game, you provide the gameplay binary with the board size as follows:

  ./gameplay 5 5

Optionally, you can provide the gameplay binary with a board position cache file as follows:

  ./gameplay 5 5 5x5_cache.txtObjective

The objective of this assignment together with Homework 5  is to make a playable version of the game of HEX.  Homework 4 is about drawing a HEX board and determining a legal position and a winning position. Homework 5 will be to add an AI that can play HEX well. In Homework 5 we will use ideas from the video lectures that implement a Monte Carlo determined best move that will let your final program possibly exceed human ability.

The Game of Hex 
The game of Hex has been invented in 1942 by Piet Hein, reinvented in 1948 by John Nash, got its name in 1952 from a commercial distribution by Parker Brothers and has been popularized by Martin Gardner in 1957. It is similar to tic-tac-toe on a hexagonal 11x11 board. 


The blue player must make a connected set of blue hexagons from east to west. The red player must do the same from north to south. At each turn a player chooses an unoccupied hexagon and gives it their color.  Unlike tic-tac-toe the game cannot end in a draw. Indeed it can be proven that by best play the first player wins.(John Nash). However there is no known optimal strategy.

Your Hex Program

Your program should use a graph representation and treat the game as a path finding problem. Each internal node (hexagon) has six neighbors â so each would have 6 edges. The corners and the edges are special. A corner has either two or three neighbors and a non-corner edge has 4 neighbors.

The player should be able to interact with the program and choose its âcolorâ with blue going first. The program should have a convenient interface for entering a move, displaying the board, and then making its own move. The program should determine when the game is over and announce the winner.

A simple strategy could be to extend your current longest path or to block your opponentâs longest path. A very dumb strategy would be to play on an empty hexagon at random.

A simple board display would be to have an 11 x 11 printout with B, R, or a blank in each position. A simple way to input a move would be to have the player enter an (i,j) coordinate corresponding to a currently empty hexagon and have the program check that this is legal and if not ask for another choice.

HW 4 expectations:

Be able to draw the board using ASCII symbols and a given size, such as 7 by 7 or 11 by 11.
Input a move and determine if a move is legal.
Determine who won.
Some suggestions:

X - . - . - . - .
 \ / \ / \ / \ / \
  . - . - . - . - .
   \ / \ / \ / \ / \
    . - . - . - . - .
     \ / \ / \ / \ / \
      . - . - . - . - .
       \ / \ / \ / \ / \
        . - . - . - . - O

The board can be drawn in ASCII as follows: (7 x 7 board)
Image: 7 by 7 Hex Board drawn in ASCII

Here X indicates a move in the corner. The dots are empty cells. A  O indicates the second player. The community TA will provide further tips and advice on this issue. Remember that Homework 4 does not need to actually play the game ; this is reserved for Homework 5.

-----
Here are 2 board designs I worked up. The first is one I did before seeing Dr. Pohl's version in the assignment writeup. The second combines my design with his slash-and-dash design.

Both are good. There are others that would also be good. For one you could choose to 'lean' yours the other way. I do find the slash-and-dash ones a little busy and in the wrong font and using X for one player is a bit hard to read so if you use slash-and-dash version you might want to play around with what characters to use for player 1 and player 2.

I am using letters for one coordinate and numbers for the other, but you can use numbers or letters for both if you prefer it that way.

BTW, these are both empty starting boards with the periods representing unoccupied spaces on the board.

                  NORTH

  A   B   C   D   E   F   G   H   I   J   K
1  .   .   .   .   .   .   .   .   .   .   .  1
  2  .   .   .   .   .   .   .   .   .   .   .  2
    3  .   .   .   .   .   .   .   .   .   .   .  3
      4  .   .   .   .   .   .   .   .   .   .   .  4
        5  .   .   .   .   .   .   .   .   .   .   .  5
  WEST    6  .   .   .   .   .   .   .   .   .   .   .  6    EAST
            7  .   .   .   .   .   .   .   .   .   .   .  7
              8  .   .   .   .   .   .   .   .   .   .   .  8
                9  .   .   .   .   .   .   .   .   .   .   .  9
                 10  .   .   .   .   .   .   .   .   .   .   .  10
                   11  .   .   .   .   .   .   .   .   .   .   .  11
                        A   B   C   D   E   F   G   H   I   J   K 

                                            SOUTH


                  NORTH

  A   B   C   D   E   F   G   H   I   J   K
 1 . - . - . - . - . - . - . - . - . - . - . 1
    \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ 
   2 . - . - . - . - . - . - . - . - . - . - . 2
      \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ 
     3 . - . - . - . - . - . - . - . - . - . - . 3
        \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ 
       4 . - . - . - . - . - . - . - . - . - . - . 4
          \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ 
         5 . - . - . - . - . - . - . - . - . - . - . 5
            \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ 
   WEST    6 . - . - . - . - . - . - . - . - . - . - . 6    EAST
              \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ 
             7 . - . - . - . - . - . - . - . - . - . - . 7
                \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ 
               8 . - . - . - . - . - . - . - . - . - . - . 8
                  \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ 
                 9 . - . - . - . - . - . - . - . - . - . - . 9
                    \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ 
                  10 . - . - . - . - . - . - . - . - . - . - . 10
                      \ / \ / \ / \ / \ / \ / \ / \ / \ / \ / \ 
                    11 . - . - . - . - . - . - . - . - . - . - . 11
                        A   B   C   D   E   F   G   H   I   J   K

                                            SOUTH
----
In HW4, to achieve a natural progression from start to a winning condition I suggest making the game work for 2 human players, but keep in mind in HW5 one player (or maybe both) will be the computer.

To assist you seeing how a game might play I've written up the following example. This example is a human vs. computer game and you should adjust things accordingly for HW4.

The dashed separator lines can be thought of as part of the UI or you may think of them as merely separators in this example.

You do not have to make your game look like this, you have flexibility to design as you like.

Note I show some errors by the human in selecting a location to place their next piece. This design is allowing them to enter a position in letter-number or number-letter order.

Application start...
-----------------------------------------------------------------------
Enter number of hexes on 1 side: 11
-----------------------------------------------------------------------
Playing with 11x11 board.
-----------------------------------------------------------------------
Human, do you want to go first (Y or N)? Y
-----------------------------------------------------------------------
************************************************************
* KEY                                                      *
************************************************************
*      ITEM      * SYMBOL  *             NOTES             *
************************************************************
* Empty Location *    .    *                               *
*   Human Piece  *    X    * connects West-East, moves 1st *
* Computer Piece *    0    * connects North-South          *
************************************************************
-----------------------------------------------------------------------
BOARD (no moves)    
                  NORTH

  A   B   C   D   E   F   G   H   I   J   K
1  .   .   .   .   .   .   .   .   .   .   .  1
  2  .   .   .   .   .   .   .   .   .   .   .  2
    3  .   .   .   .   .   .   .   .   .   .   .  3
      4  .   .   .   .   .   .   .   .   .   .   .  4
        5  .   .   .   .   .   .   .   .   .   .   .  5
  WEST    6  .   .   .   .   .   .   .   .   .   .   .  6    EAST
            7  .   .   .   .   .   .   .   .   .   .   .  7
              8  .   .   .   .   .   .   .   .   .   .   .  8
                9  .   .   .   .   .   .   .   .   .   .   .  9
                 10  .   .   .   .   .   .   .   .   .   .   .  10
                   11  .   .   .   .   .   .   .   .   .   .   .  11
                        A   B   C   D   E   F   G   H   I   J   K 

                                            SOUTH
-----------------------------------------------------------------------
Move 1: Human(X)
Enter location: C6
-----------------------------------------------------------------------
BOARD (after Move 1)    
                  NORTH

  A   B   C   D   E   F   G   H   I   J   K
1  .   .   .   .   .   .   .   .   .   .   .  1
  2  .   .   .   .   .   .   .   .   .   .   .  2
    3  .   .   .   .   .   .   .   .   .   .   .  3
      4  .   .   .   .   .   .   .   .   .   .   .  4
        5  .   .   .   .   .   .   .   .   .   .   .  5
  WEST    6  .   .   X   .   .   .   .   .   .   .   .  6    EAST
            7  .   .   .   .   .   .   .   .   .   .   .  7
              8  .   .   .   .   .   .   .   .   .   .   .  8
                9  .   .   .   .   .   .   .   .   .   .   .  9
                 10  .   .   .   .   .   .   .   .   .   .   .  10
                   11  .   .   .   .   .   .   .   .   .   .   .  11
                        A   B   C   D   E   F   G   H   I   J   K 

                                            SOUTH
-----------------------------------------------------------------------
Move 2: Computer(O)
Computer selects: H4
-----------------------------------------------------------------------
BOARD (after Move 2)
                  NORTH

  A   B   C   D   E   F   G   H   I   J   K
1  .   .   .   .   .   .   .   .   .   .   .  1
  2  .   .   .   .   .   .   .   .   .   .   .  2
    3  .   .   .   .   .   .   .   .   .   .   .  3
      4  .   .   .   .   .   .   .   O   .   .   .  4
        5  .   .   .   .   .   .   .   .   .   .   .  5
  WEST    6  .   .   X   .   .   .   .   .   .   .   .  6    EAST
            7  .   .   .   .   .   .   .   .   .   .   .  7
              8  .   .   .   .   .   .   .   .   .   .   .  8
                9  .   .   .   .   .   .   .   .   .   .   .  9
                 10  .   .   .   .   .   .   .   .   .   .   .  10
                   11  .   .   .   .   .   .   .   .   .   .   .  11
                        A   B   C   D   E   F   G   H   I   J   K 

                                            SOUTH
-----------------------------------------------------------------------
Move 3: Human(X)
Enter location: C6
ERROR - Location already occupied!
Enter location: Z1
ERROR - Invalid position!
Enter location: 6B
-----------------------------------------------------------------------
BOARD (after Move 3)
                  NORTH

  A   B   C   D   E   F   G   H   I   J   K
1  .   .   .   .   .   .   .   .   .   .   .  1
  2  .   .   .   .   .   .   .   .   .   .   .  2
    3  .   .   .   .   .   .   .   .   .   .   .  3
      4  .   .   .   .   .   .   .   O   .   .   .  4
        5  .   .   .   .   .   .   .   .   .   .   .  5
  WEST    6  .   X   X   .   .   .   .   .   .   .   .  6    EAST
            7  .   .   .   .   .   .   .   .   .   .   .  7
              8  .   .   .   .   .   .   .   .   .   .   .  8
                9  .   .   .   .   .   .   .   .   .   .   .  9
                 10  .   .   .   .   .   .   .   .   .   .   .  10
                   11  .   .   .   .   .   .   .   .   .   .   .  11
                        A   B   C   D   E   F   G   H   I   J   K 

                                            SOUTH
-----------------------------------------------------------------------
.
. ... a bunch of moves I'm not showing ...
.
-----------------------------------------------------------------------
Move 21: Human(X)
Enter next location: K5
-----------------------------------------------------------------------
BOARD (after move 21)
                  NORTH

  A   B   C   D   E   F   G   H   I   J   K
1  .   .   .   .   .   .   .   O   .   .   .  1
  2  .   .   .   .   .   .   .   .   .   .   .  2
    3  .   .   .   O   .   .   .   .   .   .   .  3
      4  .   .   .   .   .   .   .   O   .   .   .  4
        5  .   .   .   .   X   X   X   X   X   X   X  5
  WEST    6  X   X   X   X   O   .   .   .   .   .   .  6    EAST
            7  .   .   .   .   .   .   .   .   O   .   .  7
              8  .   .   .   .   .   .   .   O   .   .   .  8
                9  .   O   .   .   O   .   O   .   .   .   .  9
                 10  .   .   .   .   .   O   .   .   .   .   .  10
                   11  .   .   .   .   .   .   .   .   .   .   .  11
                        A   B   C   D   E   F   G   H   I   J   K 

                                            SOUTH
-----------------------------------------------------------------------
Argggh! You have bested me, Human!
-----------------------------------------------------------------------
Play again (Y or N)? N
...application end.

-----
Here are some links to my HW4 solution.

gameplay.cc -- https://gist.github.com/gribble/c1da1cbcd89084d0fe41#file-gameplay-cc -- the actual game playing program itself, including main(). This contains logic for prompting the user for moves, displaying messages when a user wins, and so on.

HexBoard.h -- https://gist.github.com/gribble/c1da1cbcd89084d0fe41#file-hexboard-h -- the interface to my HexBoard class, which includes functionality for constructing a Hex board, modifying the occupancy of cells, testing to see whether a player has won the game, and producing a string representation of the current board layout and state.

HexBoard.cc -- https://gist.github.com/gribble/c1da1cbcd89084d0fe41#file-hexboard-cc -- the implementation of the HexBoard class

Graph.h -- https://gist.github.com/gribble/c1da1cbcd89084d0fe41#file-graph-h -- my Graph class from HW2/HW3

ShortestPath.h -- https://gist.github.com/gribble/c1da1cbcd89084d0fe41#file-shortestpath-h -- my ShortestPath Dijkstra implementation from HW2.

UnionFindForest.h -- https://gist.github.com/gribble/c1da1cbcd89084d0fe41#file-unionfindforest-h -- the interface to my UnionFindForest class, which is used by Kruskal's algorithm in Graph.h's Graph::ComputeMST().

UnionFindForest.cc -- https://gist.github.com/gribble/c1da1cbcd89084d0fe41#file-unionfindforest-cc -- the implementation of my UnionFindForest class.

Note that my code uses features from C++11. So, if you want to try compiling it, you'll need access to a recent compiler with support for those features. On a Mac, you'll need to install the latest Xcode, and use the following command line to compile:

  clang++ -Wall -std=c++11 -stdlib=libc++ -o gameplay gameplay.cc HexBoard.cc UnionFindForest.cc
