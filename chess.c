#include "util.h"

const int FPS = 60;
const int TICKS = 1000 / FPS;

SDL_Window *window;
SDL_Renderer *renderer;
Texture texture;

int main(int argc, const char *argv[])
{
    FC_Font *fontTexture = FC_CreateFont();

    if (initSdl() & initTextureMap(&texture, "chess-pieces_0.png"))
    {
        FC_LoadFont(fontTexture, renderer, "ASCII.ttf", 20, FC_MakeColor(0, 0, 0, 255), TTF_STYLE_NORMAL);

        char *joingame = "Join Game";

        int timer, delta;

        bool black = false,
             turn = WHITE;

        SDL_Rect buttons[3];
        SDL_Event e;
        SDL_Rect clips[12];

        SDL_Rect p_quad = {
            .w = 64, 
            .h = 64,
            .x = 400 - 256, 
            .y = 300 - 256
        };

        initButtons(buttons);
        initClips(clips);

        g_piece player1_set[16];
        g_piece player2_set[16];

        game GAME = {
            .p1_set = &player1_set[0],
            .p2_set = &player2_set[0],
            .b_quad = {
                .w = 512, 
                .h = 512, 
                .x = 400 - 256, 
                .y = 300 - 256
            },
            .s_quad = {
                .w = 64, 
                .h = 64, 
                .x = 400 - 256, 
                .y = 300 - 256
            },
            .h_quad = {
                .w = 64,
                .h = 64,
                .x = 400 - 256,
                .y = 300 - 256
            },
            .hosting = false,
            .m_pressed = false,
            .promotion = false,
            .quit = false,
            .selection = false,
            .enpassant = true,
            .state = G_MENU,
            .inplen = 0,
            .TURN = WHITE
        };

        while (!GAME.quit)
        {
            timer = SDL_GetTicks();

            // clear renderer
            SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
            SDL_RenderClear(renderer);

            switch (GAME.state)
            {
                case G_MENU:
                    //input read
                    menuInput(e, &GAME, buttons);

                    // draw buttons
                    SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0xff);
                    SDL_RenderFillRect(renderer, &buttons[B_JOIN]);

                    FC_Draw(
                        fontTexture, 
                        renderer, 
                        buttons[B_JOIN].x + (buttons[B_JOIN].w >> 2) + 8, 
                        buttons[B_JOIN].y + (buttons[B_JOIN].h >> 2) + 8, 
                        "Join Game");

                    SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0xff);
                    SDL_RenderFillRect(renderer, &buttons[B_HOST]);

                    FC_Draw(
                        fontTexture, 
                        renderer, 
                        buttons[B_HOST].x + (buttons[B_HOST].w >> 2) + 8, 
                        buttons[B_HOST].y + (buttons[B_HOST].h >> 2) + 8, 
                        "Host Game");

                    SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0xff);
                    SDL_RenderFillRect(renderer, &buttons[B_EXIT]);

                    FC_Draw(
                        fontTexture, 
                        renderer, 
                        buttons[B_EXIT].x + (buttons[B_EXIT].w >> 2) + 8, 
                        buttons[B_EXIT].y + (buttons[B_EXIT].h >> 2) + 8, 
                        "Quit Game");
                break;
                case G_JOIN:
                    joinInput(e, &GAME);

                    FC_Draw(
                        fontTexture, 
                        renderer, 
                        buttons[B_HOST].x + (buttons[B_HOST].w >> 2), 
                        buttons[B_HOST].y + (buttons[B_HOST].h >> 2), 
                        GAME.addr_input);
                break;
                case G_HOST:
                    hostInput(e, &GAME);

                    FC_Draw(
                        fontTexture, 
                        renderer, 
                        buttons[B_HOST].x + (buttons[B_HOST].w >> 2), 
                        buttons[B_HOST].y + (buttons[B_HOST].h >> 2), 
                        "Waiting for client ...");
                break;
                case G_PLAY:
                    //read input
                    playInput(e, &GAME, player1_set, player2_set);

                    // board
                    for (int i = 0; i < 64; i++)
                    {
                        if (black) 
                            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
                        else 
                            SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);

                        SDL_RenderFillRect(renderer, &p_quad);

                        black = !black;

                        p_quad.x += 64;

                        if (p_quad.x >= 656) 
                        {
                            black = !black;

                            p_quad.x = GAME.b_quad.x;
                            p_quad.y += 64;

                            if (p_quad.y >= 556) p_quad.y = GAME.b_quad.y;
                        }
                    }

                    // border
                    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
                    SDL_RenderDrawRect(renderer, &GAME.b_quad);

                    // draw hover quad
                    // draw selector
                    if (GAME.selection) drawSelectHover(renderer, texture, clips, GAME);
                    
                    // draw pieces
                    drawPieces(texture, clips, GAME, player1_set, player2_set);
                break;
            }

            // render the scene
            SDL_RenderPresent(renderer);

            // limit framerate to ~60 fps
            delta = SDL_GetTicks() - timer;
            if (delta < TICKS) SDL_Delay(TICKS - delta);
        }

        shutdown(GAME.sockfd, SHUT_RDWR);
        close(GAME.sockfd);
    }

    // free texture
    freeTexture(&texture);

    // free font
    FC_FreeFont(fontTexture);

    // quit sdl
    SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

    return 0;
}

bool initSdl()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        //Create window
        window = SDL_CreateWindow(
            "Chess", 
            SDL_WINDOWPOS_UNDEFINED, 
            SDL_WINDOWPOS_UNDEFINED, 
            800, 600, 
            SDL_WINDOW_SHOWN);
        if( window == NULL )
        {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if (renderer == NULL) 
            {
                printf("Could not create renderer! %s\n", SDL_GetError());
                success = false;
            }
            else
            {
                //Initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
                    success = false;
                }
            }
        }
    }
    return success;
}

void initTypes(g_piece set[], bool color)
{
    for (int i = 0; i < 16; i++)
    {
        set[i].dead = false;
        switch (set[i].id)
        {
            case 0 ... 7:
                set[i].type = PAWN;
                set[i].first = false;
                break;
            case 8:
            case 15:
                set[i].type = ROOK;
                set[i].first = false;
                break;
            case 9:
            case 14:
                set[i].type = KNIGHT;
                break;
            case 10:
            case 13:
                set[i].type = BISHOP;
                break;
            case 11:
                set[i].type = QUEEN;
                break;
            case 12:
                set[i].type = KING;
                set[i].first = false;
                break;
        }
    }
}

void initPositions(g_piece p1[], g_piece p2[], bool white)
{
    for (int i = 0; i < 16; i++)
    {
        p1[i].x = i % 8;

        if (i < 8) p1[i].y = 6;
        else p1[i].y = 7;
    }

    for (int j = 0; j < 16; j++)
    {
        p2[j].x = j % 8;

        if (j > 7) p2[j].y = 1;
        else p2[j].y = 0;
    }
    
    //temp fix for king positions
    if (!white)
    {
        p1[11].x = 4;
        p1[12].x = 3;
    }
    else
    {
        p2[3].x = 4;
        p2[4].x = 3;
    }
}

void initRects(g_piece set[])
{
    for (int i = 0; i < 16; i++)
    {
        set[i].rect.w = 64;
        set[i].rect.h = 64;
        setRect(&set[i]);
    }
}

void initClips(SDL_Rect clips[])
{
    for (int i = 0; i < 12; i++)
    {
        clips[i].w = 32;
        clips[i].h = 32;

        clips[i].x = (i % 6) << 5;

        if (i < 6) clips[i].y = 0;
        else clips[i].y = 32;
    }
}

SDL_Texture *loadTexture(char path[16])
{
    SDL_Texture *newTexture = NULL;

    SDL_Surface *loadedSurface = IMG_Load(path);

    if (loadedSurface == NULL) printf("could not load image! %s\n", IMG_GetError());
    else 
    {
        newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        
        if (newTexture == NULL) printf("could not load optimised surface! %s\n", IMG_GetError());

        SDL_FreeSurface(loadedSurface);
    }

    return newTexture;
}

void renderTexture(Texture *text, int x, int y, SDL_Rect *clip, const SDL_RendererFlip flip)
{
    SDL_Rect renderQuad = {x, y, text->mWidth, text->mHeight};

    if (clip != NULL) 
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }

    SDL_RenderCopyEx(renderer, text->mTexture, clip, &renderQuad, 0, NULL, flip);
}

bool initTextureMap(Texture *sheet, char *str)
{
    bool success = true;
    
    sheet->mTexture = loadTexture(str);

    if (sheet->mTexture == NULL) 
    { 
        success = false;
        printf("Image could not initialize! SDL Error: %s\n", SDL_GetError());
    }
    else 
    {
        sheet->mWidth = 192;
        sheet->mHeight = 64;
    }

    return success;
}

void freeTexture(Texture *text)
{
    if (text->mTexture != NULL)
    {
        SDL_DestroyTexture(text->mTexture);
        text->mTexture = NULL;

        text->mWidth = 0;
        text->mHeight = 0;
    }
}

bool checkDiagonal(g_piece *piece, g_piece p[], int x, int y, int dx, int dy)
{
    bool success = true;

    int n = piece->x;

    for (int i = piece->y; 
    (dy > 0) ? i < y : i > y; 
    (dy > 0) ? i++ : i--)
    {
        for (int k = 0; k < 16; k++)
        {
            if (piece != &p[k])
            {
                if (!p[k].dead && (p[k].y == i && p[k].x == n))
                {
                    success = false;
                    break;
                }
            } 
        }

        if (dx > 0) n++;
        else if (dx < 0) n--;
    }

    return success;
}

bool checkHorizontal(g_piece *piece, g_piece p[], int x, int y, int dx)
{
    bool success = true;

    for (int j = piece->x; 
    (dx > 0) ? j < x : j > x; 
    (dx > 0) ? j++ : j--)
    {
        for (int k = 0; k < 16; k++)
        {
            if (piece != &p[k])
            {
                if ((p[k].x == j && p[k].y == y) && !p[k].dead) 
                {
                    success = false; 
                    break;
                }
            }
        }
    }

    return success;
}

bool checkVertical(g_piece *piece, g_piece p[], int x, int y, int dy)
{
    bool success = true;

    for (int i = piece->y; 
    (dy > 0) ? i < y : i > y; 
    (dy > 0) ? i++ : i--)
    {
        for (int k = 0; k < 16; k++)
        {
            if (piece != &p[k])
            {
                if ((p[k].y == i && p[k].x == x) && !p[k].dead) 
                {
                    success = false; 
                    break;
                }
            }
        }
    }

    return success;
}

bool checkMove(game *GAME, g_piece p1[], g_piece p2[])
{
    bool success = true, found = false;

    int dx = GAME->cellx - GAME->selected_piece->x, 
        dy = GAME->celly - GAME->selected_piece->y;

    switch (GAME->selected_piece->type)
    {
        case PAWN:
            for (int i = 0; i < 16; i++)
            {
                if (!p2[i].dead)
                {
                    if (GAME->enpassant && p2[i].type == PAWN 
                    && (GAME->selected_piece->y == 3 && p2[i].y == 3))
                    {
                        if (GAME->celly == 2 && GAME->cellx == p2[i].x
                        && (GAME->selected_piece->x + 1 == p2[i].x 
                        || GAME->selected_piece->x - 1 == p2[i].x))
                        {
                            printf("enpassant?\n");
                            found = true;
                            epOpponent(GAME->selected_piece, &p2[i]);
                            GAME->enpassant = false;
                            break;
                        }
                    }
                    if (p2[i].x == GAME->cellx && p2[i].y == GAME->celly)
                    {
                        if (p2[i].x == GAME->selected_piece->x 
                        && p2[i].y == GAME->celly 
                        && GAME->celly == GAME->selected_piece->y - 1)
                        {
                            success = false;
                            break;
                        }
                        else if (p2[i].x == GAME->cellx && p2[i].y == GAME->celly
                        && ((GAME->selected_piece->x - 1 == GAME->cellx) 
                        || (GAME->selected_piece->x + 1 == GAME->cellx)))
                        {
                            found = true;
                            break;
                        }
                    }
                }
            }

            if (!found) 
            {
                if (GAME->cellx != GAME->selected_piece->x) success = false;
            }
        break;
        case ROOK:
            if (GAME->cellx == p1[GAME->PLAYER ? 12 : 11].x 
            && GAME->celly == p1[GAME->PLAYER ? 12 : 11].y)
            {
                if (!GAME->selected_piece->first && !p1[GAME->PLAYER ? 12 : 11].first)
                {
                    if (!checkHorizontal(GAME->selected_piece, p1, GAME->cellx, GAME->celly, dx) 
                    || !checkHorizontal(GAME->selected_piece, p1, GAME->cellx, GAME->celly, dx)) success = false;
                    else
                    {
                        if (GAME->selected_piece->x < p1[GAME->PLAYER ? 12 : 11].x) 
                            GAME->castling = checkCastle(GAME->PLAYER, p1, p2, p1[GAME->PLAYER ? 12 : 11].x - 1, p1[GAME->PLAYER ? 12 : 11].x - 2);
                        else if (GAME->selected_piece->x > p1[GAME->PLAYER ? 12 : 11].x) 
                            GAME->castling = checkCastle(GAME->PLAYER, p1, p2, p1[GAME->PLAYER ? 12 : 11].x + 1, p1[GAME->PLAYER ? 12 : 11].x + 2);

                        if (!GAME->castling)
                        {
                            success = false;
                            break;
                        } 
                        else break;
                    }
                }
            }

            if (GAME->celly != GAME->selected_piece->y)
            {
                if (!checkVertical(GAME->selected_piece, p1, GAME->cellx, GAME->celly, dy) 
                || !checkVertical(GAME->selected_piece, p1, GAME->cellx, GAME->celly, dy)) success = false;
                 
            }
            else if (GAME->cellx != GAME->selected_piece->x)
            {
                if (!checkHorizontal(GAME->selected_piece, p1, GAME->cellx, GAME->celly, dx) 
                || !checkHorizontal(GAME->selected_piece, p2, GAME->cellx, GAME->celly, dx)) success = false;
            }
        break;
        case BISHOP:
            if (!checkDiagonal(GAME->selected_piece, p1, GAME->cellx, GAME->celly, dx, dy) 
            || !checkDiagonal(GAME->selected_piece, p2, GAME->cellx, GAME->celly, dx, dy)) success = false;
        break;
        case QUEEN:
            if ((GAME->celly != GAME->selected_piece->y) 
            && (GAME->cellx == GAME->selected_piece->x))
            {
                if (!checkVertical(GAME->selected_piece, p1, GAME->cellx, GAME->celly, dy) 
                || !checkVertical(GAME->selected_piece, p2, GAME->cellx, GAME->celly, dy)) success = false;
            }
            else if ((GAME->cellx != GAME->selected_piece->x) 
            && (GAME->celly == GAME->selected_piece->y))
            {
                if (!checkHorizontal(GAME->selected_piece, p1, GAME->cellx, GAME->celly, dx) 
                || !checkHorizontal(GAME->selected_piece, p2, GAME->cellx, GAME->celly, dx)) success = false;
            }
            else
            {
                if (!checkDiagonal(GAME->selected_piece, p1, GAME->cellx, GAME->celly, dx, dy) 
                || !checkDiagonal(GAME->selected_piece, p2, GAME->cellx, GAME->celly, dx, dy)) success = false;
            } 
        break;
    }

    return success;
}

void checkOpponent(game GAME, g_piece p2[])
{
    for (int i = 0; i < 16; i++)
    {
        if ((p2[i].x == GAME.cellx && p2[i].y == GAME.celly) 
        && !p2[i].dead) 
        {
            p2[i].dead = true;
            break;
        }
    }
}

void checkSelf(g_piece p1[], int x, int y)
{
    for (int i = 0; i < 16; i++)
    {
        if ((p1[i].x == x && p1[i].y == y) 
        && !p1[i].dead) 
        {
            p1[i].dead = true;
            break;
        }
    }
}

bool checkNextMove(g_piece p1[], g_piece p2[], int x, int y)
{
    bool success = true;

    int dx , dy;

    for (int i = 0; i < 16; i++)
    {
        if (!p2[i].dead)
        {
            dx = x - p2[i].x, 
            dy = x - p2[i].y;

            switch (p2[i].type)
            {
                case PAWN:
                    if ((p2[i].x == x - 1 || p2[i].x == x + 1)
                    && p2[i].y == y + 1) success = false;
                    break;
                case ROOK:
                    if (moveRook(p2[i], x, y)) 
                    {
                        if (y != p2[i].y)
                        {
                            if (checkVertical(&p2[i], p1, x, y, dy)
                            && checkVertical(&p2[i], p2, x, y, dy)) success = false;
                        }
                        else if (x != p2[i].x)
                        {
                            if (checkHorizontal(&p2[i], p1, x, y, dx)
                            && checkHorizontal(&p2[i], p2, x, y, dx)) success = false;
                        }
                    }
                    break;
                case KNIGHT:
                    if (moveKnight(p2[i], x, y)) success = false;
                    break;
                case BISHOP:
                    if (moveBishop(p2[i], x, y)) 
                    {
                        if (checkDiagonal(&p2[i], p1, dx, x, y, dy) 
                        && checkDiagonal(&p2[i], p2, dx, x, y, dy)) success = false;
                    }
                    break;
                case QUEEN:
                    if (moveQueen(p2[i], x, y)) 
                    {
                        if ((y != p2[i].y) && (x == p2[i].x))
                        {
                            if (checkVertical(&p2[i], p1, x, y, dy) 
                            && checkVertical(&p2[i], p2, x, y, dy)) success = false;
                        }
                        else if ((x != p2[i].x) && (y == p2[i].y))
                        {
                            if (checkHorizontal(&p2[i], p1, x, y, dx) 
                            && checkHorizontal(&p2[i], p2, x, y, dx)) success = false;
                        }
                        else 
                        {
                            if (checkDiagonal(&p2[i], p1, x, y, dx, dy) 
                            && checkDiagonal(&p2[i], p2, x, y, dx, dy)) success = false;
                        }                        
                    }
                    break;
                case KING:
                    if (moveKing(p2[i], x, y)) success = false;
                    break;
            }
        }
        if (!success) break;
    }
    
    return success;
}

void playInput(SDL_Event e, game *GAME, g_piece p1_set[], g_piece p2_set[])
{
    while (SDL_PollEvent(&e) != 0)
    {
        if (GAME->promotion) promotePiece(GAME, e);
        else
        {
            switch (e.type)
            {
                case SDL_MOUSEMOTION:
                    if ((e.motion.x > GAME->b_quad.x 
                    && e.motion.x - GAME->b_quad.x < GAME->b_quad.w) 
                    && (e.motion.y > GAME->b_quad.y 
                    && e.motion.y - GAME->b_quad.y < GAME->b_quad.h))
                    {
                        GAME->cellx = (e.motion.x - GAME->b_quad.x) >> 6;
                        GAME->celly = (e.motion.y - GAME->b_quad.y) >> 6;

                        GAME->mousex = e.motion.x;
                        GAME->mousey = e.motion.y;

                        if (GAME->selection)
                        {
                            GAME->h_quad.x = (GAME->cellx << 6) + GAME->b_quad.x;
                            GAME->h_quad.y = (GAME->celly << 6) + GAME->b_quad.y;
                        }
                    }
                break;
                case SDL_MOUSEBUTTONDOWN:
                    if (GAME->TURN == GAME->PLAYER)
                    {
                        if (e.button.button == SDL_BUTTON_RIGHT)
                        {
                            GAME->selection = false;
                            break;
                        }

                        if (!GAME->m_pressed)
                        {
                            if (e.button.button == SDL_BUTTON_LEFT) 
                            {
                                if ((e.motion.x > GAME->b_quad.x 
                                && e.motion.x - GAME->b_quad.x < GAME->b_quad.w) 
                                && (e.motion.y > GAME->b_quad.y 
                                && e.motion.y - GAME->b_quad.y < GAME->b_quad.h))
                                {
                                    if (!GAME->selection)
                                    {
                                        for (int i = 0; i < 16; i++)
                                        {
                                            if (p1_set[i].x == GAME->cellx && p1_set[i].y == GAME->celly)
                                            {
                                                GAME->selection = true;

                                                GAME->selected_piece = &p1_set[i];

                                                GAME->s_quad.x = (GAME->cellx << 6) + GAME->b_quad.x;
                                                GAME->s_quad.y = (GAME->celly << 6) + GAME->b_quad.y;

                                                GAME->h_quad.x = (GAME->cellx << 6) + GAME->b_quad.x;
                                                GAME->h_quad.y = (GAME->celly << 6) + GAME->b_quad.y;

                                                break;
                                            }
                                        }
                                    }
                                    else 
                                    {
                                        bool ok = true;
                                        
                                        for (int j = 0; j < 16; j++)
                                        {
                                            if (GAME->selected_piece->type == ROOK 
                                            && (!p1_set[GAME->PLAYER ? 12 : 11].first && !GAME->selected_piece->first))
                                            {
                                                if (p1_set[GAME->PLAYER ? 12 : 11].x == GAME->cellx 
                                                && p1_set[GAME->PLAYER ? 12 : 11].y == GAME->celly) continue;
                                            }
                                            
                                            if (p1_set[j].x == GAME->cellx && p1_set[j].y == GAME->celly)
                                            {
                                                ok = false;
                                                break;
                                            }
                                        }

                                        if (ok & movePiece(*GAME->selected_piece, GAME->cellx, GAME->celly)) 
                                        {
                                            if (checkMove(GAME, p1_set, p2_set))
                                            {
                                                GAME->selection = false;

                                                if (GAME->castling)
                                                {
                                                    // real bad !!!
                                                    setSocketData(GAME);

                                                    GAME->TURN = !GAME->PLAYER;

                                                    GAME->castling = false;
                                                    castling(GAME->selected_piece, &p1_set[GAME->PLAYER ? 12 : 11]);
                                                }
                                                else
                                                {
                                                    switch (GAME->selected_piece->type)
                                                    {
                                                        case PAWN:
                                                        case ROOK:
                                                        case KING:
                                                            GAME->selected_piece->first = true;
                                                        break;
                                                    }
                                                    checkOpponent(*GAME, p2_set);

                                                    if (GAME->celly == 0 && GAME->selected_piece->type == PAWN)
                                                    {
                                                        GAME->promotion = true;

                                                    }

                                                    /// test online functionality
                                                    setSocketData(GAME);
                                                    ///

                                                    GAME->TURN = !GAME->PLAYER;

                                                    GAME->selected_piece->x = GAME->cellx;
                                                    GAME->selected_piece->y = GAME->celly;
                                                    
                                                    setRect(GAME->selected_piece);                       
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            GAME->m_pressed = true;
                        }
                    }
                break;
                case SDL_MOUSEBUTTONUP:
                    if (e.button.button == SDL_BUTTON_LEFT)
                        GAME->m_pressed = false;
                break;
                case SDL_QUIT: GAME->quit = true; break;
                case SDL_KEYDOWN:
                    if (e.key.keysym.sym == SDLK_ESCAPE) 
                    {
                        shutdown(GAME->connfd, SHUT_RDWR);
                        close(GAME->connfd);
                        GAME->state = G_MENU;
                    }
                break;
            }
        }
    }
}

bool movePiece(g_piece piece, int x, int y)
{
    bool success = true;

    if (y < 0 || y > 7 || x < 0 || x > 7) 
    {
        return false;
    }

    switch (piece.type)
    {
        case PAWN:
            if (!movePawn(piece, x, y)) success = false;
            break;
        case ROOK:
            if (!moveRook(piece, x, y)) success = false;
            break;
        case KNIGHT:
            if (!moveKnight(piece, x, y)) success = false;
            break;
        case BISHOP:
            if (!moveBishop(piece, x, y)) success = false;
            break;
        case QUEEN:
            if (!moveQueen(piece, x, y)) success = false;
            break;
        case KING:
            if (!moveKing(piece, x, y)) success = false;
            break;
    }

    return success;
}

void promotePiece(game *GAME, SDL_Event e)
{
    if (e.type == SDL_QUIT) GAME->quit = true;
    else if (e.type == SDL_KEYDOWN)
    {
        switch (e.key.keysym.sym)
        {
            case 'q':
                GAME->selected_piece->type = QUEEN;
                GAME->promotion = false;
                GAME->m_pressed = false;
            break;
            case 'r':
                GAME->selected_piece->type = ROOK;
                GAME->promotion = false;
                GAME->m_pressed = false;
            break;
            case 'b':
                GAME->selected_piece->type = BISHOP;
                GAME->promotion = false;
                GAME->m_pressed = false;
            break;
            case 'k':
                GAME->selected_piece->type = KNIGHT;
                GAME->promotion = false;
                GAME->m_pressed = false;
            break;
            case SDLK_ESCAPE:
                GAME->quit = true;
            break;
        }
    }
}

void castling(g_piece *s_piece, g_piece *king)
{
    if (s_piece->x < king->x)
    {
        s_piece->x = king->x - 1;
        king->x = king->x - 2;
    }
    else if (s_piece->x > king->x)
    {
        s_piece->x = king->x + 1;
        king->x = king->x + 2;
    }

    setRect(s_piece);
    setRect(king);

    s_piece->first = true;
    king->first = true;
}

bool checkCastle(bool player, g_piece p1[], g_piece p2[], int x, int x2)
{
    if (checkNextMove(p1, p2, p1[player ? 12 : 11].x, p1[player == WHITE ? 12 : 11].y) 
    && checkNextMove(p1, p2, x, p1[player ? 12 : 11].y)
    && checkNextMove(p1, p2, x2, p1[player ? 12 : 11].y)) return true;
    return false;
}

void epOpponent(g_piece *piece, g_piece *p2)
{
    p2->dead = true;

    piece->y = piece->y - 1;
    piece->x = p2->x;

    setRect(piece);
}
