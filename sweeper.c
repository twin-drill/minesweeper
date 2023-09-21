#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// todo implement config file
int SIZE = 16;
int MINES = 40;
const int DIRS[8][2] = {{1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}};
const int CDIRS[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

// state:
//      0 - unflipped
//      1 - flipped
//      2 - flagged
typedef struct space {
    char value, state;
}space;

char validCoordinate(int dim, int x, int y) {
    return (x < dim && x >= 0 && y < dim && y >= 0);
}

space** createBoard(int size, int mines, space **mine_locations) {

    if (mines > size * size) {
        return NULL;
    }

    // allocate board
    space **board = calloc(size, sizeof(space*));
    for (int i = 0; i < size; ++i) {
        board[i] = calloc(size, sizeof(space));
    }

    // fprintf(stderr, "\tboard allocated.\n");

    // generate mine positions
    int x, y, z;
    while (mines) {
        x = rand() % size;
        y = rand() % size;

        // fprintf(stderr, "\tgenerated mine %d at (%d, %d)... ", 21-mines, x, y);

        // if there is already a mine present
        if (board[x][y].value == -1) {
            // fprintf(stderr, "already present.\n");
            continue;
        }

        // place mine
        board[x][y].value = -1;
        // fprintf(stderr, "placed. incrementing numbers...\n");
        // generate numbers, for every direction in DIRS
        for (z = 0; z < 8; ++z) {
            // fprintf(stderr, "\t\tattempting (%d, %d)... ", x + DIRS[z][0], y + DIRS[z][1]);
            if (validCoordinate(size, x + DIRS[z][0], y + DIRS[z][1]) &&
                board[x + DIRS[z][0]][y + DIRS[z][1]].value != -1) {
                board[x + DIRS[z][0]][y + DIRS[z][1]].value += 1;
                // fprintf(stderr, "success.\n");
            }
            else {
                // fprintf(stderr, "failure.\n");
            }
        }
        --mines;
        mine_locations[mines] = &board[x][y];
    }

    return board;
}

void printBoard(space **board, int dim) {
    printf("\n");
    for (int i = dim-1; i >= 0; --i) {
        printf("%2d [", i);
        for (int j = 0; j < dim; ++j) {
            // fprintf(stderr, "iterating (%d, %d)\n", i, j);
            switch(board[i][j].state) {
                case 0 :
                    printf("   ");
                    break;
                case 1:
                    if (board[i][j].value == -1) {
                        printf(" X ");
                    }
                    else {
                        printf(" %d ", board[i][j].value);
                    }
                    break;
                case 2:
                    printf(" ⚑ ");
                    break;
            }
        }
        printf("]\n");
    }

    printf("    ");
    for (int i = 0; i < dim; ++i) {
        printf("%2d ", i);
    }

    printf("\n");
    return;
}

char isValid(space **locations, int mines) {
    for (int i = 0; i < mines; ++i) {
        if ((*locations[i]).state != 2) {
            return 1;
        }
    }
    return 0;
}

// for opening zeroes on flip
void open(space **board, int dim, int x, int y) {
    board[x][y].state = 1;
    for (int i = 0; i < 8; ++i) {
        if (validCoordinate(dim, x + DIRS[i][0], y + DIRS[i][1]) &&
                board[x + DIRS[i][0]][y + DIRS[i][1]].state == 0) {
                    board[x + DIRS[i][0]][y + DIRS[i][1]].state = 1;
                    if (board[x + DIRS[i][0]][y + DIRS[i][1]].value == 0) {
                        open(board, dim, x + DIRS[i][0], y + DIRS[i][1]);
                    }
                }
    }
}

int flip(space **board, int dim, int x, int y, char first) {
    // can't flip a flagged tile
    if (board[x][y].state == 2) {
        return 0;
    }
    // flip the tile
    board[x][y].state = 1;
    // if it is a bomb
    if (board[x][y].value == -1) {
        return 1;
    }
    // if auto-processing is needed for 0
    if (board[x][y].value == 0) {
        open(board, dim, x, y);
    }

    // zero-checking on sides
    for (int i = 0; i < 4; ++i) {
        if (validCoordinate(dim, x + CDIRS[i][0], y + CDIRS[i][1]) &&
        board[x + CDIRS[i][0]][y + CDIRS[i][1]].state == 0 &&
        board[x + CDIRS[i][0]][y + CDIRS[i][1]].value == 0) {
            open(board, dim, x + CDIRS[i][0], y + CDIRS[i][1]);
        }
    }

    // first flip gets bonus spaces
    if (first) {
        for (int i = 0; i < 8; ++i) {
            if (validCoordinate(dim, x + DIRS[i][0], y + DIRS[i][1]) &&
                board[x + DIRS[i][0]][y + DIRS[i][1]].value != -1 &&
                board[x + DIRS[i][0]][y + DIRS[i][1]].state == 0) {
                flip(board, dim, x + DIRS[i][0], y + DIRS[i][1], 0);
            }
        }
    }
    return 0;
}

void play(space **board, space **mine_locations, int dim) {
    // 0 - help
    // 1 - open tile
    // 2 - flag tile
    // 3 - display board
    // 4 - exit
    int inp, x, y;
    char first = 1;
    while (isValid(mine_locations, MINES)) {
        printBoard(board, dim);
        printf("> ");
        scanf("%d", &inp);
        switch(inp) {
            case 0:
                printf("\t0 - help\n\t1 - open tile\n\t2 - flag tile\n\t3 - display board\n\t4 - exit\n");
                break;
            case 1:
                printf("\t~: ");
                scanf("%d %d", &y, &x);
                if (validCoordinate(dim, x, y)) {
                    if (flip(board, dim, x, y, first)) {
                        // reveal all mines
                        for (int i = 0; i < MINES; ++i) {
                            (*mine_locations[i]).state = 1;
                        }
                        printBoard(board, dim);
                        return;
                    }
                    first = 0;
                }
                break;
            case 2:
                printf("\t]): ");
                scanf("%d %d", &y, &x);
                if (board[x][y].state == 0) {
                    board[x][y].state = 2;
                }
                else if (board[x][y].state == 2) {
                    board[x][y].state = 0;
                }
                break;
            case 3:
                printBoard(board, dim);
                break;
            case 4:
                return;
        }
    }
    printBoard(board, dim);
    printf("🎉\n");
}

int main() {
    srand(time(NULL));
    space **mine_locations = calloc(MINES, sizeof(space*));
    space **board = createBoard(SIZE, MINES, mine_locations);
    play(board, mine_locations, SIZE);
}