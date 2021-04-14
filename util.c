#include "util.h"

void drawPieces(Texture texture, SDL_Rect clips[], game GAME, g_piece p1[], g_piece p2[])
{
    for (int i = 0; i < 16; i++)
    {
        if (p1[i].dead) continue;
        if (GAME.selection && &p1[i] == GAME.selected_piece)
            continue;

        if (GAME.PLAYER == WHITE)
            renderTexture(&texture, p1[i].rect.x + 16, p1[i].rect.y + 16, &clips[6 + p1[i].type], false);
        else 
            renderTexture(&texture, p1[i].rect.x + 16, p1[i].rect.y + 16, &clips[p1[i].type], false);
    }
    for (int j = 0; j < 16; j++)
    {
        if (p2[j].dead) continue;

        if (GAME.PLAYER == WHITE)
            renderTexture(&texture, p2[j].rect.x + 16, p2[j].rect.y + 16, &clips[p2[j].type], false);
        else
            renderTexture(&texture, p2[j].rect.x + 16, p2[j].rect.y + 16, &clips[6 + p2[j].type], false);
    }
}

void drawSelectHover(SDL_Renderer *renderer, Texture texture, SDL_Rect clips[], game GAME)
{
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0x80, 0xff);
    SDL_RenderFillRect(renderer, &GAME.h_quad);

    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0x40, 0xff);
    SDL_RenderFillRect(renderer, &GAME.s_quad);

    renderTexture(&texture, GAME.mousex, GAME.mousey, &clips[GAME.selected_piece->type], false);
}

void hostInput(SDL_Event e, game *GAME)
{
    while (SDL_PollEvent(&e) != 0)
    {
        switch (e.type)
        {
            case SDL_QUIT: GAME->quit = true; break;
            case SDL_KEYDOWN:
                if (e.key.keysym.sym == SDLK_ESCAPE)
                {
                    shutdown(GAME->sockfd, SHUT_RDWR);
                    close(GAME->sockfd);
                    GAME->state = G_MENU;
                }
            break;
        }
    }
}

void joinInput(SDL_Event e, game *GAME)
{
    while (SDL_PollEvent(&e) != 0)
    {
        switch (e.type)
        {
            case SDL_QUIT: GAME->quit = true; break;
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym)
                {
                    case 8: // backspace
                        if (GAME->inplen > 0)
                        {
                            GAME->addr_input[GAME->inplen - 1] = ' ';
                            GAME->inplen--;
                        }
                    break;
                    case SDLK_RETURN: 
                        if (connectClient(GAME)) 
                        {
                            GAME->PLAYER = BLACK;
                            GAME->state = G_PLAY;
                            setupGame(GAME->p1_set, GAME->p2_set, BLACK, WHITE);
                            thrd_create(&GAME->thread, connect_thread, GAME);
                        } 
                    break;
                    case SDLK_ESCAPE: GAME->state = G_MENU; break;
                    case '.':
                        if (GAME->addr_input[GAME->inplen - 1] == '.') break;
                    case '0' ... '9':
                        if (GAME->inplen < 15) 
                        {
                            GAME->addr_input[GAME->inplen] = e.key.keysym.sym;
                            GAME->inplen++;
                        }
                    break;                    
                }
            break;
        }
    }
}

void menuInput(SDL_Event e, game *GAME, SDL_Rect buttons[])
{
    while (SDL_PollEvent(&e) != 0)
    {
        switch (e.type)
        {
            case SDL_QUIT: GAME->quit = true; break;
            case SDL_KEYDOWN: 
                if (e.key.keysym.sym == SDLK_ESCAPE) GAME->quit = true; 
                else if (e.key.keysym.sym == SDLK_RETURN) 
                {
                    // testing purposes
                    GAME->PLAYER = WHITE;
                    GAME->state = G_PLAY;
                    setupGame(GAME->p1_set, GAME->p2_set, WHITE, BLACK);
                }
            break;
            case SDL_MOUSEMOTION:
            break;
            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    if ((e.motion.x > buttons[B_JOIN].x 
                    && e.motion.x < buttons[B_JOIN].x + buttons[B_JOIN].w)
                    && (e.motion.y > buttons[B_JOIN].y 
                    && e.motion.y < buttons[B_JOIN].y + buttons[B_JOIN].h))
                    {
                        // join
                        GAME->state = G_JOIN;
                    }
                    else if ((e.motion.x > buttons[B_HOST].x 
                    && e.motion.x < buttons[B_HOST].x + buttons[B_HOST].w)
                    && (e.motion.y > buttons[B_HOST].y 
                    && e.motion.y < buttons[B_HOST].y + buttons[B_HOST].h))
                    {
                        // host
                        GAME->state = G_HOST;
                        thrd_create(&GAME->thread, host_thread, GAME);
                    }
                    else if ((e.motion.x > buttons[B_EXIT].x 
                    && e.motion.x < buttons[B_EXIT].x + buttons[B_EXIT].w)
                    && (e.motion.y > buttons[B_EXIT].y 
                    && e.motion.y < buttons[B_EXIT].y + buttons[B_EXIT].h))
                    {
                        // quit
                        GAME->quit = true;
                    }
                }
            break;
        }
    }
}

void waitInput(SDL_Event e, game *GAME)
{
    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT) GAME->quit = true;
        else if (e.type == SDL_KEYDOWN)
        {
            if (e.key.keysym.sym == SDLK_ESCAPE) 
            {
                shutdown(GAME->connfd, SHUT_RDWR);
                close(GAME->connfd);
                GAME->state = G_MENU;
            }
        }
    }
}

void initButtons(SDL_Rect set[])
{
    set[B_JOIN].w = 256;
    set[B_JOIN].h = 64;
    set[B_JOIN].x = 272;
    set[B_JOIN].y = 128;

    set[B_HOST].w = 256;
    set[B_HOST].h = 64;
    set[B_HOST].x = 272;
    set[B_HOST].y = 256;

    set[B_EXIT].w = 256;
    set[B_EXIT].h = 64;
    set[B_EXIT].x = 272;
    set[B_EXIT].y = 384;
}

void initColor(g_piece set[], bool player)
{
    for (int i = 0; i < 16; i++)
    {
        if (player == PLAYER1) set[i].color = WHITE;
        else set[i].color = BLACK;
    }
}

void initIds(g_piece p1[], g_piece p2[])
{
    for (int i = 0; i < 16; i++) 
        p1[i].id = i;

    for (int j = 15; j > -1; j--) 
        p2[j].id = 15 - j;
}

void setupGame(g_piece p1[], g_piece p2[], bool p1color, bool p2color)
{
    initIds(p1, p2);
  
    initTypes(p1, p1color);
    initTypes(p2, p2color);

    initPositions(p1, p2, p1color);

    initRects(p1);
    initRects(p2);
}

void setRect(g_piece *piece)
{
    piece->rect.x = (piece->x << 6) + 144;
    piece->rect.y = (piece->y << 6) + 44;
}

bool moveBishop(g_piece piece, int x, int y)
{
    bool success = true;

    int deltax = abs(x - piece.x), 
        deltay = abs(y - piece.y);

    if (!((y < piece.y || y > piece.y) 
    && (x < piece.x || x > piece.x) 
    && deltax == deltay)) success = false;

    return success;
}

bool moveKnight(g_piece piece, int x, int y)
{
    bool success = true;

    if (!((y == piece.y - 2 || y == piece.y + 2) 
    && (x == piece.x - 1 || x == piece.x + 1))
    && !((y == piece.y - 1 || y == piece.y + 1) 
    && (x == piece.x - 2 || x == piece.x + 2))) success = false;

    return success;
}

bool moveKing(g_piece piece, int x, int y)
{
    bool success = true;

    if (!((y == piece.y - 1 || y == piece.y + 1) 
    && (x == piece.x - 1 || x == piece.x + 1))
    && !((y == piece.y - 1 || y == piece.y + 1) && x == piece.x) 
    && !(y == piece.y && (x == piece.x - 1 || x == piece.x + 1))) success = false;

    return success;
}

bool movePawn(g_piece piece, int x, int y)
{
    bool success = true;

    if (piece.y == 6) 
    {
        if (!(y == piece.y - 2 || y == piece.y - 1)) success = false;
    }
    else 
    {
        if (!(y == piece.y - 1)) success = false;
    }

    return success;
}

bool moveRook(g_piece piece, int x, int y)
{
    bool success = true;

    if (!((y < piece.y || y > piece.y) && x == piece.x) 
    && !(y == piece.y && (x < piece.x || x > piece.x))) success = false;

    return success;
}

bool moveQueen(g_piece piece, int x, int y)
{
    bool success = true;

    int deltax = abs(x - piece.x), 
        deltay = abs(y - piece.y);

    if (!((y < piece.y || y > piece.y) && x == piece.x)
    && !(y == piece.y && (x < piece.x || x > piece.x))
    && !((y < piece.y || y > piece.y) 
    && (x < piece.x || x > piece.x) && deltax == deltay)) success = false;

    return success;
}

bool connectClient(game *GAME)
{
    bool success = true;
    int buf[6];

    // socket create and varification
	GAME->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (GAME->sockfd == -1) {
		printf("socket creation failed...\n");
		success = false;
	}
	else
		printf("Socket successfully created..\n");
	bzero(&GAME->servaddr, sizeof(GAME->servaddr));

	// assign IP, PORT
	GAME->servaddr.sin_family = AF_INET;
	GAME->servaddr.sin_addr.s_addr = inet_addr(GAME->addr_input); 
	GAME->servaddr.sin_port = htons(8008);

	// connect the client socket to server socket
	if (connect(GAME->sockfd, (struct sockaddr*)&GAME->servaddr, sizeof(GAME->servaddr)) != 0) {
		printf("connection with the server failed...\n");
		success = false;
	}
	else
    { 
        printf("connected to the server..\n");
        bzero(buf, sizeof(buf));
        if (recv(GAME->sockfd, buf, sizeof(buf), 0)) GAME->connfd = buf[0];
        else success = false;
    }

    return success;
}

void setSocketData(game *GAME)
{
    int buf[6];

    buf[S_FILED] = GAME->sockfd;
    buf[S_PIECEID] = GAME->selected_piece->id;
    buf[S_DELTAX] = GAME->selected_piece->x - GAME->cellx;
    buf[S_DELTAY] = GAME->selected_piece->y - GAME->celly;
    buf[S_PIECEX] = GAME->cellx;
    buf[S_PIECEY] = GAME->celly;

    send(GAME->connfd, buf, sizeof(buf), 0); 
}

int connect_thread(void *ptr)
{
    game *GAME = ptr;
    wait_disconnect(GAME);
    thrd_exit(0);
    return 0;
}

int host_thread(void *ptr)
{
    game *GAME = ptr;
    int buf[6];

    printf("hello from thread!\n");

    // socket create and verification
	GAME->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (GAME->sockfd == -1) {
		printf("socket creation failed...\n");
        thrd_exit(1);
	}
	else
		printf("Socket successfully created..\n");

    if (setsockopt(GAME->sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        thrd_exit(1);
    }

	bzero(&GAME->servaddr, sizeof(GAME->servaddr));

	// assign IP, PORT
	GAME->servaddr.sin_family = AF_INET;
	GAME->servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	GAME->servaddr.sin_port = htons(8008);

	// Binding newly created socket to given IP and verification
	if ((bind(GAME->sockfd, (struct sockaddr*)&GAME->servaddr, sizeof(GAME->servaddr))) != 0) {
		printf("socket bind failed...\n");
        thrd_exit(1);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(GAME->sockfd, 1)) != 0) {
		printf("Listen failed...\n");
        thrd_exit(1);
	}
	else
		printf("Server listening..\n");
	GAME->addrlen = sizeof(GAME->cli);

	// Accept the data packet from client and verification
	GAME->connfd = accept(GAME->sockfd, (struct sockaddr*)&GAME->cli, &GAME->addrlen);
	if (GAME->connfd < 0) {
		printf("server acccept failed...\n");
        thrd_exit(1);
	}
	else
    {
	    printf("server acccept the client...\n");

        bzero(buf, sizeof(buf));
        buf[0] = GAME->sockfd;
        buf[1] = -1;
        
        if (send(GAME->connfd, buf, sizeof(buf), 0) > 0)
        {
            GAME->hosting = true;
            GAME->state = G_PLAY;
            GAME->PLAYER = WHITE;
            setupGame(GAME->p1_set, GAME->p2_set, WHITE, BLACK);
            wait_disconnect(GAME);
        }
    }
    
    printf("thread terminated\n");

    thrd_exit(0);

    return 0;
}

bool checkOpCastle(game GAME, g_piece set[], g_piece *piece, int buf[6])
{
    bool success = false;

    for (int i = 15; i > -1; i--)
    {
        if (set[i].type == KING)
        {
            if ((GAME.PLAYER && set[i].x == buf[S_PIECEX]) 
            || (!GAME.PLAYER && set[i].x == buf[S_PIECEX - 1]))
            {
                castling(piece, &set[i]);
                success = true;
            }
        }
    }

    return success;
}

void wait_disconnect(game *GAME)
{
    int buf[6];
    int nbytes;
    bool quit = false;

    printf("thread: waiting on disconnect\n");

    while (!quit)
    {
        nbytes = recv(GAME->connfd, buf, sizeof(buf), 0);

        if (nbytes <= 0)
        {
            printf("socket %d hung up\n", GAME->connfd);
            close(GAME->connfd);
            GAME->state = G_MENU;
            quit = true;
        }
        else
        {
            if (buf[S_PIECEID] != -1)
            {
                for (int i = 0; i < 16; i++)
                {
                    if (GAME->p2_set[i].id == buf[S_PIECEID])
                    {
                        /*
                        if (GAME->p2_set[i].type == ROOK
                        && checkOpCastle(*GAME, GAME->p2_set, &GAME->p2_set[i], buf)) 
                        {
                            GAME->TURN = GAME->PLAYER;
                            break;
                        }
                        else
                        */
                        //{
                            GAME->p2_set[i].x += buf[S_DELTAX];
                            GAME->p2_set[i].y += buf[S_DELTAY];

                            checkSelf(GAME->p1_set, GAME->p2_set[i].x, GAME->p2_set[i].y);
                            setRect(&GAME->p2_set[i]);

                            GAME->TURN = GAME->PLAYER;
                        //}
                    }
                }
            }
        }
        bzero(buf, sizeof(buf));
    }

    printf("thread: client disconnected\n thread terminated\n");
}
