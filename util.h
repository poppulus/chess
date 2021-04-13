#include "defs.h"

bool initSdl();
void initButtons(SDL_Rect set[]);
void initTypes(g_piece set[], bool player);
void initPositions(g_piece p1[], g_piece p2[]);
void initRects(g_piece set[]);
void initClips(SDL_Rect clips[]);
void initColor(g_piece set[], bool player);
void initIds(g_piece p1[], g_piece p2[]);

void setupGame(g_piece p1[], g_piece p2[], bool p1color, bool p2color);

void drawPieces(Texture, SDL_Rect[], game, g_piece[], g_piece[]);
void drawSelectHover(SDL_Renderer*, Texture, SDL_Rect[], game);

bool initTextureMap(Texture *sheet, char *str);
void renderTexture(Texture *text, int x, int y, SDL_Rect *clip, const SDL_RendererFlip flip);
void freeTexture(Texture *text);
SDL_Texture *loadTexture(char path[16]);

bool checkDiagonal(g_piece *piece, g_piece p[], int x, int y, int dx, int dy);
bool checkHorizontal(g_piece *piece, g_piece p[], int x, int y, int dx);
bool checkVertical(g_piece *piece, g_piece p[], int x, int y, int dy);
bool checkMove(game *GAME, g_piece p1[], g_piece p2[]);
void checkOpponent(game GAME, g_piece p2[]);
void checkSelf(g_piece p1[], int x, int y);
bool checkNextMove(g_piece p1[], g_piece p2[], int x, int y);
bool checkOpCastle(game GAME, g_piece set[], g_piece *piece, int buf[6]);

void epOpponent(g_piece *piece, g_piece *p2);

bool movePiece(g_piece piece, int x, int y);
void promotePiece();
void castling(g_piece *s_piece, g_piece *king);
bool checkCastle(bool player, g_piece p1[], g_piece p2[], int x, int x2);

void hostInput(SDL_Event e, game *GAME);
void joinInput(SDL_Event e, game *GAME);
void menuInput(SDL_Event e, game *GAME, SDL_Rect buttons[]);
void playInput(SDL_Event e, game *GAME, g_piece p1_set[], g_piece p2_set[]);
void waitInput(SDL_Event e, game *GAME);

void setRect(g_piece *piece);

bool moveBishop(g_piece piece, int x, int y);
bool moveKnight(g_piece piece, int x, int y);
bool moveKing(g_piece piece, int x, int y);
bool movePawn(g_piece piece, int x, int y);
bool moveRook(g_piece piece, int x, int y);
bool moveQueen(g_piece piece, int x, int y);

bool hostServer(game *GAME);
bool connectClient(game *GAME);
void setSocketData(game *GAME);

int connect_thread(void *ptr);
int host_thread(void *ptr);
void wait_disconnect(game *GAME);

