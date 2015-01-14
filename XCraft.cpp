#include <SDL.h>
#include <string>   //FOR GETTING THE FILE NAMES
#include <stdlib.h> //FOR RANDOMIZER IN AI
#include <time.h>   //FOR RANDOMIZER IN AI
#include <fstream>  //EVENTUALLY FOR POWERUPS AND STORING SCORE DATA ETC...

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const int SCREEN_BPP = 32;
const int FRAMES_PER_SECOND = 20;
SDL_Surface *pallette = NULL;
SDL_Surface *screen = NULL;
SDL_Surface *player = NULL;
SDL_Surface *power = NULL;
SDL_Event event;
SDL_Rect block;
SDL_Rect plane;
SDL_Rect blank;
SDL_Rect shooter_image;
SDL_Rect blast_image;

SDL_Surface *load_image( std::string filename )
{
    SDL_Surface* loadedImage = NULL;
    SDL_Surface* optimizedImage = NULL;
    loadedImage = SDL_LoadBMP( filename.c_str() );
    if( loadedImage != NULL )
    {
        optimizedImage = SDL_DisplayFormat( loadedImage );
        SDL_FreeSurface( loadedImage );
    }
    return optimizedImage;
}

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL )
{
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;
    SDL_BlitSurface( source, clip, destination, &offset );
}

void set_clips()
{
    block.x = 0;
    block.y = 0;
    block.w = 80;
    block.h = 80;

    blank.x = 160;
    blank.y = 0;
    blank.w = 80;
    blank.h = 80;

    plane.x = 80;
    plane.y = 0;
    plane.w = 80;
    plane.h = 80;

    shooter_image.x = 0;
    shooter_image.y = 0;
    shooter_image.w = 120;
    shooter_image.h = 120;

    blast_image.x = 120;
    blast_image.y = 0;
    blast_image.w = 120;
    blast_image.h = 120;
}

bool init()
{
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        return false;
    }
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_FULLSCREEN );
    if( screen == NULL )
    {
        return false;
    }
    SDL_WM_SetCaption( "X-CRAFT: UNLEASHED", NULL );
    return true;
}

class Craft //ADD FUNCTION FOR GENERATING NEW BLOCKS AND LINK WITH RENDER_TILES. ALSO HAVE POWERUP FEATURES
{
    public:
    int xVel, yVel, tx, ty, x, y, ticker, second, power_shooter, power_blast, a[10][16];
    bool left, right;
    Craft();
    void collision_check();
    void render_tiles();
    void intel();
    void handle_input();
    void powerup();
    void tick();
    void show();
};

Craft::Craft()
{
    xVel = 0;
    yVel = 0;
    x = SCREEN_WIDTH/2-40;
    y = 0;
    tx = 0;
    ty = 0;
    left=0;
    right=0;
    ticker=0;
    second=0;
    power_shooter=1;
    power_blast=1;
    for (int i=0;i<10;i++) {
        for (int j=0;j<16;j++) {
            a[i][j]=0;
        }
    }
    player = load_image("pallette2.bmp");
    power = load_image("powerup.bmp");
}

void Craft::collision_check() //CHECKS WHETHER RIGHT OR LEFT IS BRICK AND THEN DESTROYS CRAFT BY RESETTING ALL THE VARIABLES
{
    if (a[8][(x+5)/80]==1 || a[8][(x-5)/80+1]==1)                 //EXTRA SPACE THAT WILL NOT DESTROY CRAFT
    {
        SDL_Delay(400);
        xVel = 0;
        yVel = 0;
        y = 0;
        tx = 0;
        ty = 0;
        left=0;
        right=0;
        ticker=0;
        second=0;
        power_shooter=1;
        power_blast=1;
        for (int i=0;i<10;i++) {
            for (int j=0;j<16;j++) {
                a[i][j]=0;
            }
        }
    }

    //COLLISION WITH SIDES
    if (x==SCREEN_WIDTH) {
        x = 0;
    }
    else if (x==0) {
        x = SCREEN_WIDTH;
    }
}

void Craft::powerup()
{
    if (second==2000) {
        power_shooter++;
    }
    if (second==2500) {
        power_blast++;
        second=0;
    }
}

void Craft::intel()
{
    y=ty*3;                                                                 //TIMING OF CREATING NEW BLOCKS
    if (ty==80/3)
    {
        srand(time(NULL)*rand());
        ty=0;
        y=0;
        for (int i=9;i>=0;i--) {
            for (int j=0;j<16;j++) {
                a[i][j]=a[i-1][j];
            }
        }
        ticker++;
        if (ticker==3)                                                      //NUMBER OF SPACES BETWEEN NEW ROWS
        {
            for (int j=0;j<16;j++)
            {
                //RANDOMIZE SO THAT MORE BLOCKS HAS LESS PROBABILITY
                srand(rand());
                int tile_value = rand()%4+1;
                for (;tile_value>0;tile_value--, j++)
                {
                    a[0][j] = 1;
                }
                for (tile_value=2;tile_value>0;tile_value--)                //NUMBER OF BLANK BLOCKS TO CREATE
                {
                    a[0][j] = 0;
                    j++;
                }
            }
            ticker=0;
        } else
        {
            for (int j=0;j<16;j++)
            {
                a[0][j] = 0;
            }
        }
    }
}

void Craft::render_tiles()
{
    powerup();
    handle_input();
    intel();
    for (int i=0;i<10;i++) {
        for (int j=0;j<16;j++) {
            if (a[i][j]==1) {
                apply_surface(80*j,80*(i-1)+y,player,screen,&block);
            } else if (a[i][j]==0) {
                apply_surface(80*j,80*(i-1)+y,player,screen,&blank);  //MAYBE ADD FEATURE THAT WILL CLEAR ALL BLOCKS AFTER DEATH SO THAT CRAFT WILL LEAVE PATH BEHIND IT
            }
        }
    }
    if (power_shooter) apply_surface(0,0,power,screen,&shooter_image);
    if (power_blast) apply_surface(120,0,power,screen,&blast_image);
    collision_check();
    apply_surface(x,SCREEN_HEIGHT-80,player,screen,&plane);
}

void Craft::handle_input() //FIX GLITCH WHERE PRESSING ANY OTHER KEY WHILE MOVING STOPS PLANE (MULTI-THREAD? OR GIVE THE MOVEMENT MEMORY TO GO BACK TO ORIGINAL STATE OR MOVEMENT AFTER KEY IS PRESSED.)
{
    if( event.type == SDL_KEYDOWN )
    {
        switch( event.key.keysym.sym )
        {
            case SDLK_LEFT: left=true; if(left) {
                    x += xVel*tx; tx = 0; xVel = 4;
                } break;
            case SDLK_RIGHT:right=true;if(right) {
                    x += xVel*tx; tx = 0; xVel = 4;
                } break;
            case SDLK_x:if (power_shooter) {
                    for(int i=10;i>0;i--) {
                        if(a[i][(x)/80]==1||a[i][(x)/80+1]==1) {
                            a[i][(x)/80]=0; a[i][(x)/80+1]=0;
                            power_shooter--;
                            second=0;
                            break;      //ONLY DESTROY FIRST BLOCK SHOULD BE FIXED
                        }
                    }
                } break;
            case SDLK_z: if (power_blast) {
                    for (int i=0;i<10;i++) {
                        for (int j=0;j<16;j++) {
                            a[i][j]=0;
                        }
                    }
                    power_blast--;
                    second=0;
            }
            default:tx=0;right=false;left=false; //TEMPORARY FIX FOR STOP GLITCH
        }
    }
    if ( event.type == SDL_KEYUP )
    {
        switch ( event.key.keysym.sym )
        {
            case SDLK_LEFT:  left=false; x += xVel*tx; tx = 0; break;
            case SDLK_RIGHT: right=false; x += xVel*tx; tx = 0; break;
            default:tx=0;right=false;left=false;
        }
    }
}

void Craft::tick()
{
    if (left) tx--;
    if (right)tx++;
    ty++;
    second++;
}

void Craft::show()
{
    render_tiles();
}

int main( int argc, char* args[] )
{
    bool quit = false;
    if ( init() == false )
    {
        return 1;
    }
    pallette = load_image( "pallette2.bmp" );
    set_clips();

    Craft craft;
    craft.show();

    while ( quit == false)
    {
        while( SDL_PollEvent( &event ) )
        {
            if( event.type == SDL_QUIT )
                quit = true;
            if ( event.type == SDL_MOUSEBUTTONDOWN ) {
                int x, y;
                SDL_GetMouseState( &x, &y );
                if(x>=SCREEN_WIDTH-50 && y >= SCREEN_HEIGHT-100)
                    quit = true;
            }
        }
        if( SDL_Flip( screen ) == -1 )
        {
            return 1;
        }
        if( SDL_GetTicks() < 1000 / FRAMES_PER_SECOND )
        {
            SDL_Delay( ( 1000 / FRAMES_PER_SECOND ) - SDL_GetTicks() );
        }
        craft.show();
        craft.tick();
    }
    SDL_FreeSurface( pallette );
    SDL_Quit();
    return 0;
}
