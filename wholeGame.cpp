#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <termios.h>

#define VKEY_UP         (256+'A')
#define VKEY_DOWN       (256+'B')
#define VKEY_RIGHT      (256+'C')
#define VKEY_LEFT       (256+'D')
#define VKEY_HOME       (256+'H')

#define TITLE_LINE    4
#define TOP           6
#define LEFT         20
#define BOTTOM       (TOP+3*4)
#define RIGHT        (LEFT+3*8)
#define CREDIT_LINE    21

#define DIGIT(id)              (board[id])
#define IS_EMPTY(id)           (0 == board[id])
#define IS_FIXED(id)           (temp[id]-'0' != 0)

#define ROW(id)                ((id)/9)
#define COLUMN(id)             ((id)%9)
#define BLOCK(id)              (3*(ROW(id)/3)+(COLUMN(id)/3))
#define INDEX(row,col)          (9*(row)+(col))

#define COL_ELEMENT(val, id)        (val*9 + id)
#define ROW_ELEMENT(val, id)    (id*9 + val)
#define BLOCK_ELEMENT(val, id)  (((id/3)*3 + (val/3)*9) + (id%3)*3 + (val%3))

#define GET_DIGIT(val)          (board[val])
#define MAKE_DIGIT(val)         (temp[val] - '0')

#ifndef TEMPLATE
#define TEMPLATE "./template/easy.txt"
#endif

//char filetemp[20] = "./template/easy.txt\0";
char filetemp[25];
char tempeasy[25] = "./template/easy.txt\0";
char tempmedium[25] = "./template/medium.txt\0";
char temphard[25] = "./template/hard.txt\0";

static char title[ 80 ] = "SUDOKU!\0";
int curx = 0, cury = 0;

static const char * program;        /* argv[0] */
static int board[ 81 ];

int solved;
int hint = -1;

FILE *ftemp;
char temp[82];

static
void
move_to( int x, int y )
{
    curx = x;
    cury = y;
    move( TOP + 1 + y + y / 3, LEFT + 2 + 2 * ( x + x / 3 ) );
    wrefresh( stdscr );
}

static
void
clear_all( void )
{
    memset( board, 0x00, sizeof( board ) );
    memset( temp, 0x00, sizeof( temp ) );
    move_to(0,0);
}

static
int
fill( int id, int digit )
{
    if( !IS_EMPTY( id ) )
        return ( DIGIT( id ) == digit ) ? 0 : -1;

    board[ id ] = digit;

    return 0;
}

static
void
reset( void )
{
    int i;
    char * p;

    p = temp;
    for(i = 0; i < 81; i++)
    {
            if( isdigit( *p ) )
            {
                board[i] = *p - '0';
            }
            p++;

    }
    move_to(0,0);
    solved = -1;
}

static
int
getkey( void )
{
    int ch = wgetch( stdscr );

    if( 0x1b == ch )
    {
        ch = wgetch( stdscr );
        if( '[' == ch )
        {
            ch = wgetch( stdscr );
            switch( ch )
            {
            case 'A':
                ch = VKEY_UP;
                break;
            case 'B':
                ch = VKEY_DOWN;
                break;
            case 'C':
                ch = VKEY_RIGHT;
                break;
            case 'D':
                ch = VKEY_LEFT;
                break;
            case 'H':
                ch = VKEY_HOME;
                break;
            }
        }
    }
    return ch;
}


static
void
cleanup( int ignored )
{
    move_to( 23, 0 );
    wrefresh( stdscr );
    endwin( );
    exit( 1 );
}

static
void
signals( void )  //prepisana funkcija
{
    sigset_t sigset;
    struct sigaction sighandler;
    struct termios tp;

    sigemptyset( &sigset );
    sighandler.sa_handler = cleanup;
    sighandler.sa_mask = sigset;
    sighandler.sa_flags = 0;

    sigaction( SIGINT,  &sighandler, 0 );
    sigaction( SIGABRT, &sighandler, 0 );
    sigaction( SIGTERM, &sighandler, 0 );
#ifdef SIGHUP
    sigaction( SIGHUP,  &sighandler, 0 );
#endif
#ifdef SIGQUIT
    sigaction( SIGQUIT, &sighandler, 0 );
#endif
#ifdef SIGKILL
    sigaction( SIGKILL, &sighandler, 0 );
#endif

    /* Reenable signal processing */
    if( 0 == tcgetattr( 0, &tp ) )
    {
        tp.c_lflag |= ISIG;
        tcsetattr( 0, TCSANOW, &tp );
    }
}


void
static
move_next( void )
{
    do
    {
        if( curx < 8 )
            move_to( curx + 1, cury );
        else if( cury < 8 )
            move_to( 0, cury + 1 );
        else
            move_to( 0, 0 );
    }
    while( IS_FIXED( INDEX( cury, curx ) ) );
}

static
void
draw_screen( void )
{
    int i;

    wclear( stdscr );
    attron( A_BOLD );
    mvaddstr( 2, 35, "Sudoku!" );
    attroff( A_BOLD );

    for( i = 0 ; i < 3 ; ++i )
    {
        mvaddstr( TOP + 0 + 4 * i, LEFT, "+-------+-------+-------+" );
        mvaddstr( TOP + 1 + 4 * i, LEFT, "|       |       |       |" );
        mvaddstr( TOP + 2 + 4 * i, LEFT, "|       |       |       |" );
        mvaddstr( TOP + 3 + 4 * i, LEFT, "|       |       |       |" );
    }
    mvaddstr( TOP + 4 * 3, LEFT, "+-------+-------+-------+" );
    mvaddstr( TOP + 0, RIGHT + 8, "k" );
    mvaddstr( TOP + 1, RIGHT + 4, "  h   l pomjeranje kursora" );
    mvaddstr( TOP + 2, RIGHT + 8, "j" );
    mvaddstr( TOP + 3, RIGHT + 7, "1-9  upisivanje cifre" );
    mvaddstr( TOP + 4, RIGHT + 8, "0   ocisti polje" );
    mvaddstr( TOP + 5, RIGHT + 8, "c   ocisti sva polja" );
    mvaddstr( TOP + 6, RIGHT + 8, "n   nova igra" );
    mvaddstr( TOP + 7, RIGHT + 8, "q   odustani" );
    mvaddstr( TOP + 8, RIGHT + 8, "v   rjesenje" );
    mvaddstr( TOP + 9, RIGHT + 8, "?   pomoc" );
}

static
void
render( void )
{
    int i, x, y;

    for( i = 0 ; i < 81 ; ++i )
    {
        x = LEFT + 2 + 2 * ( COLUMN( i ) + COLUMN( i ) / 3 );
        y = TOP + 1 + ROW( i ) + ROW( i ) / 3;
        if( IS_FIXED( i ) )
            attron( A_BOLD );
        if( IS_EMPTY( i ) )
            mvaddch( y, x, '-' );
        else
            mvaddch( y, x, '0' + DIGIT( i ) );
        if( IS_FIXED( i ) )
            attroff( A_BOLD );
    }
}

static
int
read_board( FILE * f )
{
    char * p;
    int i, row, col, r;

    clear_all();
    r = (rand() %10 + 1);
    for( i = 0; i < r; i++)
    {
        if( 0 == fgets( temp, sizeof( temp ), f ) )
            return -1;
    }

    p = temp;
    for(row = 0; row < 9; row++)
    {
        for(col = 0; col < 9; col++)
        {
            if( isdigit( *p ) )
            {
                if( 0 != fill( INDEX( row, col ), *p - '0' ) )
                    return -1;
            }
            p++;
        }
    }
    solved = -1;

    return 0;
}


void
load_board( void )
{
    ftemp = fopen(filetemp, "r");
    read_board(ftemp);

    render();
    curx = cury = 8;
    move_next();
    fclose(ftemp);
}

int
check_row( int id )
{
    int i;
    int digits[10];

    for( i = 0; i < 9; i++)
        digits[board[ROW_ELEMENT(i, id)]] = 1;

    for( i = 1; i < 10; i++)
        if( digits[i] != 1) return -1;

    return 0;
}

int
check_column( int id )
{
    int i;
    int digits[10];

    for( i = 0; i < 9; i++)
        digits[board[COL_ELEMENT(i, id)]] = 1;

    for( i = 1; i < 10; i++)
        if( digits[i] != 1) return -1;

    return 0;
}

int
check_block( int id )
{
    int i;
    int digits[10];

    for( i = 0; i < 9; i++)
        digits[board[BLOCK_ELEMENT(i, id)]] = 1;

    for( i = 1; i < 10; i++)
        if( digits[i] != 1) return -1;

    return 0;
}

void
check_solution( void )
{
    int i;
    for( i = 0; i < 9; i++)
        if( check_row(i) == -1 || check_column(i) == -1 || check_block(i) == -1)
        {
            solved = -1;
            return;
        }
    solved = 0;
}

int
provjeriKolonujednaD(int broj,int red,int kolona,int s[81])
{
    int i;
    for(i=0; i<9; i++)
    {
        if(s[i*9+kolona]==broj)
            return 1;
    }
    return 0;
}

int
provjeriRedjedanD(int broj,int red,int kolona,int s[81])
{
    int i;
    for(i=0; i<9; i++)
    {
        if(s[i+red*9]==broj)
            return 1;
    }
    return 0;
}

int
provjeriKockujedanD(int broj,int red, int kolona,int s[81])
{
    int i,j;
    //samo se jedan od uslova izvrsi prilikom poziva funkcije tako da je O( n na 2)
    //uradit cu ovu funkciju krace ovo mi je prvo palo napamet
    if(red<3 && kolona<3) //provjeri jel  celija u prvoj kocki
    {

        for(i=0; i<3; i++)
        {
            for(j=0; j<3; j++)
            {
                if(s[j+i*9]==broj)
                    return 1;
            }
        }
        return 0;
    }
    else if (red<3 && kolona>=3 && kolona<6) //provjeri jel u drugoj kocki
    {
        for(i=0; i<3; i++)
        {
            for(j=3; j<6; j++)
            {
                if(s[j+i*9]==broj)return 1;
            }
        }
        return 0;
    }
    else if( red<3 && kolona>=6 && kolona<9) //provjeri jel u trecoj kocki
    {
        for(i=0; i<3; i++)
        {
            for(j=6; j<9; j++)
            {
                if(s[j+i*9]==broj)
                    return 1;
            }
        }
        return 0;
    }
    else if( red>=3 && red<6 && kolona<3 ) //provjeri jel u cetvrtoj kocki
    {
        for(i=3; i<6; i++)
        {
            for(j=0; j<3; j++)
            {
                if(s[j+i*9]==broj)return 1;
            }
        }
        return 0;
    }
    else if( red>=3 && red<6 && kolona>=3 &&kolona<6) //provjeri jel u petoj kocki
    {
        for(i=3; i<6; i++)
        {
            for(j=3; j<6; j++)
            {
                if(s[j+i*9]==broj)return 1;
            }
        }
        return 0;
    }
    else if( red>=3 && red<6 && kolona>=6 &&kolona<9) //provjeri jel u sestoj kocki
    {
        for(i=3; i<6; i++)
        {
            for(j=6; j<9; j++)
            {
                if(s[j+i*9]==broj)return 1;
            }
        }
        return 0;
    }
    else if( red>=6 && red<9 && kolona<3) //provjeri jel u sedmoj kocki
    {
        for(i=6; i<9; i++)
        {
            for(j=0; j<3; j++)
            {
                if(s[j+i*9]==broj)return 1;
            }
        }
        return 0;
    }
    else if( red>=6 && red<9 && kolona>=3 &&kolona<6) //provjeri jel u osmoj kocki
    {
        for(i=6; i<9; i++)
        {
            for(j=3; j<6; j++)
            {
                if(s[j+i*9]==broj)return 1;
            }
        }
        return 0;
    }
    else if( red>=6 && red<9 && kolona>=6 &&kolona<9) //provjeri jel u posljednjoj devetoj kocki
    {
        for(i=6; i<9; i++)
        {
            for(j=6; j<9; j++)
            {
                if(s[j+i*9]==broj)return 1;
            }
        }
        return 0;
    }
}


int solver(int s[81])  //primi niz od 81 element
{
    int r=99;//da zabiljezi kolnu u kojoj je 0
    int k=99;//da zabiljezi red u kojoj je 0
    int broj=99;//d zabiljezi redni broj u nizu gdje je nula tipa ako je nula naniz[43] onda je broj=43
    int i,j,m=0;
    for(i=0; i<9; i++) //petlja za trazenje nule
    {
        for(j=0; j<9; j++)
        {
            if(s[m]==0)
            {
                r=i;
                k=j;
                broj=m;
            }
            m++;
        }
    }
    if(r==99 && k==99 && broj==99)return 1;
    for(j=1; j<10; j++)
    {
        if(provjeriKockujedanD(j,r,k,s)==0 && provjeriKolonujednaD(j,r,k,s)==0 && provjeriRedjedanD(j,r,k,s)==0)  //za svih 9 brojeva provjerava mogul se ubacit
        {
            s[broj]=j;//prvi koji moze ubaci
            fill(broj, j);
            if(solver(s)==1)return 1;//rekurzivnim putem ako se ne moze ubacit vraca se nazad
            s[broj]=0;
            fill(broj, 0);
        }
    }
    return 0;
}

int
check_row_missing( int id, int num )
{
    int i;

    for( i = 0; i < 9; i++)
        if( board[ROW_ELEMENT(i, ROW(id))] == num ) return -1;
    return 0;
}

int
check_column_missing( int id, int num )
{
    int i;

    for( i = 0; i < 9; i++)
        if( board[COL_ELEMENT(i, COLUMN(id))] == num ) return -1;
    return 0;
}

int
check_block_missing( int id, int num )
{
    int i;

    for( i = 0; i < 9; i++)
        if( board[BLOCK_ELEMENT(i, BLOCK(id))] == num ) return -1;
    return 0;
}

int
find_missing( int id )
{
    int digits[10];
    int i, num = 0, dig;
    digits[0] = 0;
    for( i = 1; i < 10; i++ )
    {
    if(provjeriKockujedanD(i,ROW(id),COLUMN(id),board)==0 && provjeriKolonujednaD(i,ROW(id),COLUMN(id),board)==0 && provjeriRedjedanD(i,ROW(id),COLUMN(id),board)==0) digits[i] = 1;
    }

    for( i = 0; i < 10; i++)
        if(digits[i] == 1)
        {
            num++;
            dig = i;
        }

    if(num == 1) return dig;
    else return -1;
}


void
find_hint( void )
{
    int i, digit;

    for( i = 0; i < 81; i++ )
        if(board[i] == 0)
        {
            digit = find_missing(i);
            mvaddstr(23, LEFT, "Trazi...");
            if( digit != -1)
            {
                fill(i, digit);
                move_to(COLUMN(i), ROW(i));
                addch( '0' == (digit + '0') ? '-' : (digit + '0') );
                hint = 0;
                return;
            }

            else continue;
        }

    hint = -1;
}

void
read_activity( void )
{
    int ch;

    ch = ' ';
    while( 'q' != ch )
    {
    wrefresh( stdscr );
    //move_to(curx, cury);
        ch = getkey( );
        //mvaddstr(23, LEFT, "                             "); //clear status line

        switch( ch )
        {
        case '-':
            ch = '0';
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            if( !IS_FIXED( INDEX( cury, curx ) ) )
            {
                fill( INDEX( cury, curx ), ch - '0' );
                move_to(curx, cury);
                addch( '0' == ch ? '-' : ch );
                move_to( curx, cury );
            }
            else
                beep( );
            break;
        case ' ':
            move_next( );
            break;
        case 'h':
        case VKEY_LEFT:
            if( 0 < curx )
                move_to( curx - 1, cury );
            else
                move_to( 8, cury );
            break;
        case 'j':
        case VKEY_DOWN:
            if( cury < 8 )
                move_to( curx, cury + 1 );
            else
                move_to( curx, 0 );
            break;
        case 'k':
        case VKEY_UP:
            if( 0 < cury )
                move_to( curx, cury - 1 );
            else
                move_to( curx, 8 );
            break;
        case 'l':
        case VKEY_RIGHT:
            if( curx < 8 )
                move_to( curx + 1, cury );
            else
                move_to( 0, cury );
            break;
        case 'c': // clear board
            reset( );
            render( );
            wrefresh( stdscr );
            move_to( 0, 0 );
            break;
        case 'n': // load new board
            reset( );
            wrefresh( stdscr );
            load_board( );
            render( );
            break;
        case 'v': // show solution
            beep( );
            solver( board );
            render( );
            wrefresh( stdscr );
            move_to( curx, cury );
            break;
        case '?': //hint
            find_hint();
        mvaddstr(23, LEFT, "            ");
            if(hint == 0) render();
            else mvaddstr(23, LEFT, "Pomoc trenutno nije dostupna");
        mvaddstr(23, LEFT, "                             ");
            move_to( curx, cury );
            break;
        case 'q': // quit
            check_solution();
            if(solved == 0) mvaddstr(23, LEFT, "Odlicno, rijesili ste sudoku!");
            else mvaddstr(23, LEFT, "Rjesenje nije tacno :(");
            mvaddstr(24, LEFT+10, "Da li sigurno zelite izaci (Y/N)");
            ch = getkey();
            if(ch != 'y' && ch != 'Y')
            {
                ch = '-';
                mvaddstr(23, LEFT, "                             ");
                mvaddstr(24, LEFT+10, "                                ");
        curx = cury = 8;
        move_next();
            }
            else ch = 'q';
            break;
        default:
            beep( );
        }
    }
}

void
load_menu( void )
{
    mvaddstr(2, 30, "Odaberi tezinu:");
    mvaddstr(TOP, 35, "e   lagano");
    mvaddstr(TOP+2, 35, "m   srednje");
    mvaddstr(TOP+4, 35, "h   tesko");
    mvaddstr(TOP+6, 35, "q   izlaz");
}

void
mode_choice( void )
{
    int ch;

    ch = ' ';
    while( 'q' != ch )
    {
    wclear( stdscr );
        load_menu();
        wrefresh( stdscr );
    
        ch = getkey( );
        wrefresh( stdscr );

        switch( ch )
        {
        case 'e':
            strcpy (filetemp, tempeasy);
            wclear( stdscr );
            draw_screen( );
            noecho( );
            raw( );

            load_board( );
        move_to(curx, cury);
            read_activity();
            //fclose(ftemp);
            break;
        case 'm':
            strcpy( filetemp, tempmedium );
            wclear( stdscr );
            draw_screen( );
            noecho( );
            raw( );

            load_board( );
        move_to(curx, cury);
            read_activity();
            //fclose(ftemp);
            break;
        case 'h':
            strcpy( filetemp, temphard );
            wclear( stdscr );
            draw_screen( );
            noecho( );
            raw( );

            load_board( );
        move_to(curx, cury);
            read_activity();
            //fclose(ftemp);
            break;
        case 'q':
            //fclose(ftemp);
            break;
        default:
            beep();
        }


    }
}

int
main( int argc, char **argv )
{
    int i, id;

    program = argv[ 0 ];

    srand( time( 0 ) ^ getpid( ) );

    if( !isatty( 0 ) || !isatty( 1 ) )
    {
        fprintf( stderr, "Error: stdin/out cannot be redirected\n" );
        return 1;
    }

    if( 0 == initscr( ) )
    {
        fprintf( stderr,
                 "Error: failed to initialise curses screen library\n" );
        return 1;
    }

    signals( );

    //read_activity();
    noecho( );
    raw( );

    load_menu();
    mode_choice();

    move( 23, 0 );
    endwin( );

}



