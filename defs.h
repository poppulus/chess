#include "SDL_FontCache.h"
#include <unistd.h>
#include <stdbool.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <threads.h>
#include <time.h>

#define BLACK 0
#define WHITE 1

#define PLAYER1 0
#define PLAYER2 1

enum g_state
{
    G_MENU,
    G_JOIN,
    G_HOST,
    G_PLAY
};

enum g_buttons
{
    B_JOIN,
    B_HOST,
    B_EXIT
};

enum g_pieces
{
    PAWN,
    ROOK,
    KNIGHT,
    BISHOP,
    QUEEN,
    KING
};

enum s_data
{
    S_FILED,
    S_PIECEID,
    S_DELTAX,
    S_DELTAY,
    S_PIECEX,
    S_PIECEY,
    S_PROMOTE
};

typedef struct texture
{
    SDL_Texture *mTexture;
    short mWidth, mHeight;
} Texture; 

typedef struct g_piece
{
    SDL_Rect rect;
    int x, y, type, id;
    bool dead, first, color;
} g_piece;

typedef struct game
{
    thrd_t thread;
    
    SDL_Rect b_quad, s_quad, h_quad;
    g_piece *selected_piece;
    g_piece *p1_set, *p2_set;

    char addr_input[16];

	struct sockaddr_in servaddr, cli;

    int cellx, celly, 
        mousex, mousey, inplen,
        sockfd, connfd, addrlen,
        state, WINNER;

    bool quit, 
         hosting, 
         m_pressed, 
         selection,
         CHECKMATE,
         promotion,
         enpassant,
         castling,
         TURN, 
         PLAYER;
} game;
