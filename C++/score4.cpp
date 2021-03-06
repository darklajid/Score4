#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

const int width = 7;
const int height = 6;
const int orangeWins = 1000000;
const int yellowWins = -orangeWins;

int g_maxDepth = 7;

enum Mycell {
    Orange=1,
    Yellow=2,
    Barren=0
};

struct Board {
    Mycell _slots[height][width];
    Board() {
	memset(_slots, 0, sizeof(_slots));
    }
};

bool inside(int y, int x)
{
    return y>=0 && y<height && x>=0 && x<width;
}

int ScoreBoard(const Board& board)
{
    int counters[9] = {0,0,0,0,0,0,0,0,0};
    int scores[height][width];

    for(int y=0; y<height; y++)
	for(int x=0; x<width; x++)
	    switch(board._slots[y][x]) {
	    case Orange:
		scores[y][x] = 1; break;
	    case Yellow:
		scores[y][x] = -1; break;
	    case Barren:
		scores[y][x] = 0; break;
	    }
    // Horizontal spans
    for(int y=0; y<height; y++) {
	int score = scores[y][0] + scores[y][1] + scores[y][2];
	for(int x=3; x<width; x++) {
	    assert(inside(y,x));
	    score += scores[y][x];
	    counters[score+4]++;
	    assert(inside(y,x-3));
	    score -= scores[y][x-3];
	}
    }
    // Vertical spans
    for(int x=0; x<width; x++) {
	int score = scores[0][x] + scores[1][x] + scores[2][x];
	for(int y=3; y<height; y++) {
	    assert(inside(y,x));
	    score += scores[y][x];
	    counters[score+4]++;
	    assert(inside(y-3,x));
	    score -= scores[y-3][x];
	}
    }
    // Down-right (and up-left) diagonals
    static int negativeSlope[4][2] = { {0,0}, {1,1},  {2,2},  {3,3}  };
    for(int y=0; y<height-3; y++) {
	for(int x=0; x<width-3; x++) {
	    int score = 0;
	    for(int idx=0; idx<4; idx++) {
		int yy = y + negativeSlope[idx][0];
		int xx = x + negativeSlope[idx][1];
		assert(inside(yy,xx));
		score += scores[yy][xx];
	    }
	    counters[score+4]++;
	}
    }
    // up-right (and down-left) diagonals
    static int positiveSlope[4][2] = { {0,0}, {-1,1}, {-2,2}, {-3,3} };
    for(int y=3; y<height; y++) {
	for(int x=0; x<width-3; x++) {
	    int score = 0;
	    for(int idx=0; idx<4; idx++) {
		int yy = y + positiveSlope[idx][0];
		int xx = x + positiveSlope[idx][1];
		assert(inside(yy,xx));
		score += scores[yy][xx];
	    }
	    counters[score+4]++;
	}
    }
    if (counters[0] != 0)
	return yellowWins;
    else if (counters[8] != 0)
	return orangeWins;
    else 
	return 
	    counters[5] + 2*counters[6] + 5*counters[7] + 10*counters[8] -
            counters[3] - 2*counters[2] - 5*counters[1] - 10*counters[0];
}

int dropDisk(Board& board, int column, Mycell color)
{
    for (int y=height-1; y>=0; y--)
	if (board._slots[y][column] == Barren) {
	    board._slots[y][column] = color;
	    return y;
	}
    return -1;
}

int g_debug = 0;

Board loadBoard(int argc, char *argv[]) 
{
    Board newBoard;
    for(int i=1; i<argc; i++)
	if (argv[i][0] == 'o' || argv[i][0] == 'y')
	    newBoard._slots[argv[i][1]-'0'][argv[i][2]-'0'] = (argv[i][0] == 'o')?Orange:Yellow;
	else if (!strcmp(argv[i], "-debug"))
	    g_debug = 1;
	else if (!strcmp(argv[i], "-level"))
	    g_maxDepth = atoi(argv[i+1]);
    return newBoard;
}

void abMinimax(bool maximizeOrMinimize, Mycell color, int depth, Board& board, int& move, int& score)
{
    if (0==depth) {
        move = -1;
        score = ScoreBoard(board);
    } else {
	int bestScore=maximizeOrMinimize?-10000000:10000000;
	int bestMove=-1;
	for (int column=0; column<width; column++) {
	    if (board._slots[0][column]!=Barren) continue;
	    int rowFilled = dropDisk(board, column, color);
	    if (rowFilled == -1)
		continue;
	    int s = ScoreBoard(board);
	    if (s == (maximizeOrMinimize?orangeWins:yellowWins)) {
		bestMove = column;
		bestScore = s;
		board._slots[rowFilled][column] = Barren;
		break;
	    }
	    int moveInner, scoreInner;
	    abMinimax(!maximizeOrMinimize, color==Orange?Yellow:Orange, depth-1, board, moveInner, scoreInner);
	    board._slots[rowFilled][column] = Barren;
	    if (depth == g_maxDepth && g_debug)
		printf("Depth %d, placing on %d, score:%d\n", depth, column, scoreInner);
	    if (maximizeOrMinimize) {
		if (scoreInner>=bestScore) {
		    bestScore = scoreInner;
		    bestMove = column;
		} 
	    } else {
		if (scoreInner<=bestScore) {
		    bestScore = scoreInner;
		    bestMove = column;
		}
	    }
	}
	move = bestMove;
	score = bestScore;
    }
}

int main(int argc, char *argv[])
{
    Board board = loadBoard(argc, argv);
    int scoreOrig = ScoreBoard(board);
    if (scoreOrig == orangeWins) { puts("I win\n"); exit(-1); }
    else if (scoreOrig == yellowWins) { puts("You win\n"); exit(-1); }
    else {
	int move, score;
	abMinimax(true,Orange,g_maxDepth,board,move,score);
	if (move != -1) {
	    printf("%d\n",move);
	    dropDisk(board, move, Orange);
	    scoreOrig = ScoreBoard(board);
	    if (scoreOrig == orangeWins) { puts("I win\n"); exit(-1); }
	    else if (scoreOrig == yellowWins) { puts("You win\n"); exit(-1); }
	    else exit(0);
	} else {
	    puts("No move possible");
	    exit(-1);
	}
    }
    return 0;
}

// vim: set expandtab ts=8 sts=4 shiftwidth=4 
