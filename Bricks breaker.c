/* game.c - xmain, prntr */

#include <conf.h>
#include <kernel.h>
#include <io.h>
#include <bios.h>

void loginscreen();
void Sound( int hertz );
extern SYSCALL  sleept(int);

INTPROC Int70hHandler(int mdevno);
extern struct intmap far *sys_imp;
unsigned char far* ScreenPointer = 0xb8000000;
int intmap(int vec, int (*Handler)(), int mdevno);
void Change70hRate(int rate);
/*------------------------------------------------------------------------
*  xmain  --  example of 2 processes executing the same code concurrently
*------------------------------------------------------------------------
*/

//bricks color defines to set b800h attribute
 
#define GREYBRICK 120
#define WHITEBRICK 112
#define BLUEBRICK 24
#define GREENBRICK 40
#define REDBRICK 72
#define YELLOWBRICK 104
#define PURPLEBRICK 58
#define BLACKBRICK 0
//
#define BLINKGREY 140
#define BLINKWHITE 240
#define BLINKBLUE 152
#define BLINKGREEN 168
#define BLINKRED 200
#define BLINKYELLOW 232
#define BLINKPURPLE 186
//#define BLINKORANGE 

#define LAISERSHOTS 'L'
#define EXTRABALLS 'E'
#define LEVELUP 'U'
#define SLOWBALL 'S'
#define BALLCATCH 'C'
#define LONGBAT 'L'
#define NOFEATURE 'N'

#define NORMALSPEED 5
#define TARGET_NUMBER 4


#define ON (1)
#define OFF (0)
int receiver_pid;
INTPROC new_int9(int mdevno)
{
	char result = 0;
	int scan = 0;
	int ascii = 0;

	asm{
		MOV AH,1
		INT 16h
		JZ Skip1
		MOV AH,0
		INT 16h
		MOV BYTE PTR scan,AH
		MOV BYTE PTR ascii,AL
	} //asm
		if (scan == 75)
			result = 'a';
		else
			if (scan == 72)
				result = 'w';
			else
				if (scan == 77)
					result = 'd';
	if ((scan == 46) && (ascii == 3)) // Ctrl-C?
		asm INT 27; // terminate xinu

	send(receiver_pid, result);

Skip1:

} // new_int9

void set_new_int9_newisr()
{
	int i;
	for (i = 0; i < 32; i++)
		if (sys_imp[i].ivec == 9)
		{
			sys_imp[i].newisr = new_int9;
			return;
		}

} // set_new_int9_newisr



typedef struct Directions {
	int left;
	int right;
	int down;
	int up;
} DIRECTIONS;

typedef struct position
{
	int x;
	int y;

}  POSITION;

typedef struct Feature
{
	POSITION pos;
	char feature_type;
	int feature_color;
}FEATURE;

typedef struct Ball {
	POSITION pos;
	DIRECTIONS dir;
	int color;
	int speed;
	char ball_shape;
} BALL;

typedef struct Bat {
	int left_edge;
	int right_edge;
	int bat_length;
	int bat_color;
}BAT;


typedef struct Brick {
	POSITION pos;
	int right_side;
	int color;
	int strength;
	char feature;
	int feature_attr;
	int points;
	
} BRICK;

BALL Ball[3];
BAT Bat;
BRICK Bricks[18];
FEATURE Feature[2];
char display[2001];

char ch_arr[2048];
int front = -1;
int rear = -1;

char Points[4]="    ";
char Score[7]="Score: ";
char Life[9]="Life: ***";


int BallCatchFlag=0;
int LongBatFlag=0;
int LaiserShotsFlag=0;
int SlowBallFlag=0;
int counter=0;
int NumberOfBalls=1;
int FeaturesInGame=0;
int FeatureFlag=0;
int SumPoints=0;
int ballid2;
int ballid3;
int featurepid;
int scorepid;
int BricksCurrentNumber;
int StageFlag=1;
int LifeNumber=3;
int point_in_cycle;
int gcycle_length;
int gno_of_pids;


/*------------------------------------------------------------------------
*  prntr  --  print a character indefinitely
*------------------------------------------------------------------------
*/



void displayer(int ballmutex)
{
	int i, j = 0;
	while (1)
	{
		receive();
		//wait(ballmutex);
		//ScreenPointer[(160*Ball[0].pos.y+2*Ball[0].pos.x)]=Ball[0].ball_shape;
		//ScreenPointer[(160*Ball[0].pos.y+2*Ball[0].pos.x)+1]=9;
		//signal(ballmutex);
		if(FeatureFlag)
		{
			ScreenPointer[160*Feature[FeaturesInGame-1].pos.y+2*Feature[FeaturesInGame-1].pos.x]=Feature[FeaturesInGame-1].feature_type;
			ScreenPointer[(160*Feature[FeaturesInGame-1].pos.y+2*Feature[FeaturesInGame-1].pos.x)+1]=Feature[FeaturesInGame-1].feature_color;
		}
		for(i=130,j=0;i<147;i+=2,j++)
		{
			ScreenPointer[i]=Score[j];
			ScreenPointer[i+1]=12;
		}
		for(i=144,j=0;i<152;i+=2,j++)
		{
			ScreenPointer[i]=Points[j];
			ScreenPointer[i+1]=12;
		}
		// show life 
		for(i=290,j=0;i<308;i+=2,j++)
		{
			ScreenPointer[i]=Life[j];
			ScreenPointer[i+1]=12;
		}
		//sleept(18);
		/*for (i = 0; i < 4000; i += 2, j++) {
			ScreenPointer[i] = display[j];
			ScreenPointer[i + 1] = 4;
		}*/
	} //while
} // prntr

void receiver()
{
	while (1)
	{
		char temp;
		temp = receive();
		rear++;
		ch_arr[rear] = temp;
		if (front == -1)
			front = 0;
		//getc(CONSOLE);
	} // while

} //  receiver


//char display_draft[25][80];
//POSITION target_pos[TARGET_NUMBER];
//POSITION arrow_pos[ARROW_NUMBER];


void updateter(int mutex)
{
	int i, j;
	/*int gun_position;
	int no_of_arrows;
	int target_disp = 80 / TARGET_NUMBER;*/
	char ch;

	/*int no_of_targets;
	no_of_arrows = 0;
	no_of_targets = 4;
	gun_position = 39;
	target_pos[0].x = 3;
	target_pos[0].y = 0;


	for (i = 1; i < TARGET_NUMBER; i++)
	{
		target_pos[i].x = i*target_disp;
		target_pos[i].y = 0;

	} // for
	for (i = 0; i < ARROW_NUMBER; i++)
		arrow_pos[i].x = arrow_pos[i].y = -1;
*/
	while (1)
	{

		receive();

		while (front != -1)
		{
			ch = ch_arr[front];
			if (front != rear)
				front++;
			else
				front = rear = -1;

			if ((ch == 'a') || (ch == 'A'))
				if (Bat.left_edge >= 2)
				{
					Bat.left_edge--;
					ScreenPointer[(22*160+Bat.right_edge*2)+1]=0;
					Bat.right_edge--;
				}
				else;
			else if ((ch == 'd') || (ch == 'D'))
				if (Bat.right_edge <= 58)
				{
					Bat.right_edge++;
					ScreenPointer[(22*160+Bat.left_edge*2)+1]=0;
					Bat.left_edge++;
				}
				else;
			/*else if ((ch == 'w') || (ch == 'W'))
				if (no_of_arrows < ARROW_NUMBER)
				{
					arrow_pos[no_of_arrows].x = gun_position;
					arrow_pos[no_of_arrows].y = 23;
					no_of_arrows++;

				} // if*/
		} // while(front != -1)

		ch = 0;
		/*for (i = 0; i < 25; i++)
			for (j = 0; j < 80; j++)
				display_draft[i][j] = ' ';  // blank

		display_draft[23][gun_position + 2] = '_';
		display_draft[23][gun_position - 2] = '_';
		display_draft[23][gun_position - 1] = '_';
		display_draft[23][gun_position + 1] = '_';
		display_draft[23][gun_position] = '_';

		for (i = 0; i < ARROW_NUMBER; i++)
			if (arrow_pos[i].x != -1)
			{
				if (arrow_pos[i].y > 0)
					arrow_pos[i].y--;
				display_draft[arrow_pos[i].y][arrow_pos[i].x] = '^';
				display_draft[arrow_pos[i].y + 1][arrow_pos[i].x] = '|';

			} // if*/
			
		for(i=0,j=0;i<Bat.bat_length;i++,j+=2)
			ScreenPointer[(22*160+Bat.left_edge*2)+1+j]=GREYBRICK;
			
			
		wait(mutex);
		for (i = 0; i < 18; i++)
		{
			for(j=0;j<4;j++)
			{
				if (Bricks[i].strength == 2)
				{
					ScreenPointer[(Bricks[i].pos.x * 2 + Bricks[i].pos.y * 160)+ j*2] = '*';
					ScreenPointer[(Bricks[i].pos.x * 2 + Bricks[i].pos.y * 160)+j*2 + 1] = Bricks[i].color;
				}
				else if (Bricks[i].strength ==3)
				{
					ScreenPointer[(Bricks[i].pos.x * 2 + Bricks[i].pos.y * 160)+ j*2] = '#';
					ScreenPointer[(Bricks[i].pos.x * 2 + Bricks[i].pos.y * 160)+j*2 + 1] = Bricks[i].color;
				}
				else
				{
					ScreenPointer[(Bricks[i].pos.x * 2 + Bricks[i].pos.y * 160)+j*2] = ' ';
					ScreenPointer[(Bricks[i].pos.x * 2 + Bricks[i].pos.y * 160) +j*2+ 1] = Bricks[i].color;
				}
			}
			/*else
			{
				//Bricks[i].color=BLACKBRICK;
				Bricks[i].points=0;
				ScreenPointer[(Bricks[i].pos.x*2 + Bricks[i].pos.y*160)+1]=Bricks[i].color;
				//SumPoints+=Bricks[i].points;
				//itoa(SumPoints,Points,10);
				//send(scorepid,SumPoints);
			}*/
		}
		signal(mutex);

		/*for (i = 0; i < 25; i++)
			for (j = 0; j < 80; j++)
				display[i * 80 + j] = display_draft[i][j];
		display[2000] = '\0';*/

	} // while(1)

} // updater 

int sched_arr_pid[5] = { -1 };
int sched_arr_int[5] = { -1 };


SYSCALL schedule(int no_of_pids, int cycle_length, int pid1, ...)
{
	int i;
	int ps;
	int *iptr;

	disable(ps);

	gcycle_length = cycle_length;
	point_in_cycle = 0;
	gno_of_pids = no_of_pids;

	iptr = &pid1;
	for (i = 0; i < no_of_pids; i++)
	{
		sched_arr_pid[i] = *iptr;
		iptr++;
		sched_arr_int[i] = *iptr;
		iptr++;
	} // for
	restore(ps);

} // schedule 

void InitialBricks(){
	int i;
	if(StageFlag==1)
	{
		BricksCurrentNumber=18;
		
		Bricks[0].pos.x=10;
		Bricks[0].pos.y=3;
		Bricks[0].color=YELLOWBRICK;
		Bricks[0].strength=2;
		Bricks[0].feature=LONGBAT;
		Bricks[0].points=50;
		Bricks[0].feature_attr=BLINKGREY;
	
		Bricks[1].pos.x=16;
		Bricks[1].pos.y=3;
		Bricks[1].color=GREYBRICK;
		Bricks[1].strength=1;
		Bricks[1].feature=NOFEATURE;
		Bricks[1].points=50;
	
		Bricks[2].pos.x=22;
		Bricks[2].pos.y=3;
		Bricks[2].color=GREYBRICK;
		Bricks[2].strength=1;
		Bricks[2].feature=NOFEATURE;
		Bricks[2].points=50;
		
		Bricks[3].pos.x=28;
		Bricks[3].pos.y=3;
		Bricks[3].color=BLUEBRICK;
		Bricks[3].strength=1;
		Bricks[3].feature=SLOWBALL;
		Bricks[3].points=120;
		Bricks[3].feature_attr=BLINKYELLOW;
	
		Bricks[4].pos.x=34;
		Bricks[4].pos.y=3;
		Bricks[4].color=YELLOWBRICK;
		Bricks[4].strength=2;
		Bricks[4].feature=LONGBAT;
		Bricks[4].points=120;
	
		Bricks[5].pos.x=40;
		Bricks[5].pos.y=3;
		Bricks[5].color=YELLOWBRICK;
		Bricks[5].strength=2;
		Bricks[5].feature=NOFEATURE;
		Bricks[5].points=120;
	
		Bricks[6].pos.x=10;
		Bricks[6].pos.y=6;
		Bricks[6].color=REDBRICK;
		Bricks[6].strength=1;
		Bricks[6].feature=LONGBAT;
		Bricks[6].points=90;
		Bricks[6].feature_attr=BLINKGREY;
	
		Bricks[7].pos.x=16;
		Bricks[7].pos.y=6;
		Bricks[7].color=REDBRICK;
		Bricks[7].strength=1;
		Bricks[7].feature=NOFEATURE;
		Bricks[7].points=90;
	
		Bricks[8].pos.x=22;
		Bricks[8].pos.y=6;
		Bricks[8].color=REDBRICK;
		Bricks[8].strength=1;
		Bricks[8].feature=NOFEATURE;
		Bricks[8].points=90;
	
		Bricks[9].pos.x=28;
		Bricks[9].pos.y=6;
		Bricks[9].color=WHITEBRICK;
		Bricks[9].strength=1;
		Bricks[9].feature=NOFEATURE;
		Bricks[9].points=70;
	
		Bricks[10].pos.x=34;
		Bricks[10].pos.y=6;
		Bricks[10].color=WHITEBRICK;
		Bricks[10].strength=1;
		Bricks[10].feature=SLOWBALL;
		Bricks[10].points=70;
		Bricks[10].feature_attr=BLINKYELLOW;
	
		Bricks[11].pos.x=40;
		Bricks[11].pos.y=6;
		Bricks[11].color=WHITEBRICK;
		Bricks[11].strength=1;
		Bricks[11].feature=BALLCATCH;
		Bricks[11].points=70;
		Bricks[11].feature_attr=BLINKGREEN;
	
		Bricks[12].pos.x=10;
		Bricks[12].pos.y=9;
		Bricks[12].color=GREENBRICK;
		Bricks[12].strength=2;
		Bricks[12].feature=NOFEATURE;
		Bricks[12].points=80;
	
		Bricks[13].pos.x=16;
		Bricks[13].pos.y=9;
		Bricks[13].color=GREENBRICK;
		Bricks[13].strength=2;
		Bricks[13].feature=NOFEATURE;
		Bricks[13].points=80;
	
		Bricks[14].pos.x=22;
		Bricks[14].pos.y=9;
		Bricks[14].color=GREENBRICK;
		Bricks[14].strength=2;
		Bricks[14].feature=NOFEATURE;
		Bricks[14].points=80;
	
		Bricks[15].pos.x=28;
		Bricks[15].pos.y=9;
		Bricks[15].color=BLUEBRICK;
		Bricks[15].strength=1;
		Bricks[15].feature=BALLCATCH;
		Bricks[15].points=70;
		Bricks[15].feature_attr=BLINKGREEN;
	
		Bricks[16].pos.x=34;
		Bricks[16].pos.y=9;
		Bricks[16].color=BLUEBRICK;
		Bricks[16].strength=1;
		Bricks[16].feature=SLOWBALL;
		Bricks[16].points=70;
		Bricks[16].feature_attr=BLINKYELLOW;
		
	
		Bricks[17].pos.x=40;
		Bricks[17].pos.y=9;
		Bricks[17].color=BLUEBRICK;
		Bricks[17].strength=1;
		Bricks[17].feature=BALLCATCH;
		Bricks[17].points=70;
		Bricks[17].feature_attr=BLINKGREEN;
		
		for(i=0;i<18;i++)
			Bricks[i].right_side=Bricks[i].pos.x + 3;
	}
	else if(StageFlag==2)
	{
		BricksCurrentNumber=18;
		
		Bricks[0].pos.x=3;
		Bricks[0].pos.y=3;
		Bricks[0].color=GREENBRICK;
		Bricks[0].strength=1;
		Bricks[0].feature=LONGBAT;
		Bricks[0].points=80;
		Bricks[0].feature_attr=BLINKGREY;
	
		Bricks[1].pos.x=8;
		Bricks[1].pos.y=3;
		Bricks[1].color=GREENBRICK;
		Bricks[1].strength=1;
		Bricks[1].feature=NOFEATURE;
		Bricks[1].points=80;
	
		Bricks[2].pos.x=13;
		Bricks[2].pos.y=3;
		Bricks[2].color=GREYBRICK;
		Bricks[2].strength=2;
		Bricks[2].feature=NOFEATURE;
		Bricks[2].points=50;
		
		Bricks[3].pos.x=18;
		Bricks[3].pos.y=3;
		Bricks[3].color=YELLOWBRICK;
		Bricks[3].strength=2;
		Bricks[3].feature=SLOWBALL;
		Bricks[3].points=120;
		Bricks[3].feature_attr=BLINKBLUE;
	
		Bricks[4].pos.x=23;
		Bricks[4].pos.y=3;
		Bricks[4].color=GREYBRICK;
		Bricks[4].strength=2;
		Bricks[4].feature=NOFEATURE;
		Bricks[4].points=50;
	
		Bricks[5].pos.x=28;
		Bricks[5].pos.y=3;
		Bricks[5].color=GREENBRICK;
		Bricks[5].strength=1;
		Bricks[5].feature=NOFEATURE;
		Bricks[5].points=80;
	
		Bricks[6].pos.x=6;
		Bricks[6].pos.y=6;
		Bricks[6].color=REDBRICK;
		Bricks[6].strength=3;
		Bricks[6].feature=LONGBAT;
		Bricks[6].points=90;
		Bricks[6].feature_attr=BLINKGREY;
	
		Bricks[7].pos.x=11;
		Bricks[7].pos.y=6;
		Bricks[7].color=WHITEBRICK;
		Bricks[7].strength=1;
		Bricks[7].feature=NOFEATURE;
		Bricks[7].points=60;
	
		Bricks[8].pos.x=16;
		Bricks[8].pos.y=6;
		Bricks[8].color=REDBRICK;
		Bricks[8].strength=3;
		Bricks[8].feature=NOFEATURE;
		Bricks[8].points=90;
	
		Bricks[9].pos.x=21;
		Bricks[9].pos.y=6;
		Bricks[9].color=BLUEBRICK;
		Bricks[9].strength=1;
		Bricks[9].feature=NOFEATURE;
		Bricks[9].points=70;
	
		Bricks[10].pos.x=26;
		Bricks[10].pos.y=6;
		Bricks[10].color=WHITEBRICK;
		Bricks[10].strength=1;
		Bricks[10].feature=SLOWBALL;
		Bricks[10].points=60;
		Bricks[10].feature_attr=BLINKBLUE;
	
		Bricks[11].pos.x=31;
		Bricks[11].pos.y=6;
		Bricks[11].color=YELLOWBRICK;
		Bricks[11].strength=2;
		Bricks[11].feature=BALLCATCH;
		Bricks[11].points=120;
		Bricks[11].feature_attr=BLINKGREEN;
	
		Bricks[12].pos.x=8;
		Bricks[12].pos.y=9;
		Bricks[12].color=GREYBRICK;
		Bricks[12].strength=2;
		Bricks[12].feature=LONGBAT;
		Bricks[12].points=50;
		Bricks[12].feature_attr=BLINKGREY;
	
		Bricks[13].pos.x=14;
		Bricks[13].pos.y=9;
		Bricks[13].color=BLUEBRICK;
		Bricks[13].strength=1;
		Bricks[13].feature=NOFEATURE;
		Bricks[13].points=70;
	
		Bricks[14].pos.x=20;
		Bricks[14].pos.y=9;
		Bricks[14].color=REDBRICK;
		Bricks[14].strength=3;
		Bricks[14].feature=NOFEATURE;
		Bricks[14].points=90;
		
		Bricks[15].pos.x=26;
		Bricks[15].pos.y=9;
		Bricks[15].color=BLUEBRICK;
		Bricks[15].strength=1;
		Bricks[15].feature=BALLCATCH;
		Bricks[15].points=70;
		Bricks[15].feature_attr=BLINKGREEN;
	
		Bricks[16].pos.x=32;
		Bricks[16].pos.y=9;
		Bricks[16].color=GREENBRICK;
		Bricks[16].strength=1;
		Bricks[16].feature=NOFEATURE;
		Bricks[16].points=80;
	
		Bricks[17].pos.x=38;
		Bricks[17].pos.y=9;
		Bricks[17].color=YELLOWBRICK;
		Bricks[17].strength=2;
		Bricks[17].feature=BALLCATCH;
		Bricks[17].points=120;
		Bricks[17].feature_attr=BLINKGREEN;
		for(i=0;i<18;i++)
			Bricks[i].right_side=Bricks[i].pos.x + 3;
	}
	else if(StageFlag==3)
	{
		
	}
}

void InitialScreen() {
	int i,j=0;
	// paint first row
	for (i = 0; i < 120; i+=2)
	{
		ScreenPointer[i] = ' ';
		ScreenPointer[i+1] = 120;

	}
	// paint last row
	for (i =24*160; i <24*160+120; i+=2)
	{
		ScreenPointer[i] = ' ';
		ScreenPointer[i+1] = 120;

	}
	// paint first col
	for (i =0; i<24*160; i+=160)
	{
		ScreenPointer[i] = ' ';
		ScreenPointer[i+1] = 120;

	}
	// paint last col
	for (i =120; i<120*24*2; i+=160)
	{
		ScreenPointer[i] = ' ';
		ScreenPointer[i+1] = 120;

	}
	// show score
	for(i=130,j=0;i<147;i+=2,j++)
	{
		ScreenPointer[i]=Score[j];
		ScreenPointer[i+1]=12;
	}
	// show life 
	for(i=290,j=0;i<308;i+=2,j++)
	{
		ScreenPointer[i]=Life[j];
		ScreenPointer[i+1]=12;
	}
	// show bricks
	for(i=0;i<18;i++)		
		for(j=0;j<4;j++)
			ScreenPointer[(Bricks[i].pos.x*2 + Bricks[i].pos.y*160)+j*2+1]=Bricks[i].color;	
	// show bat
	for(i=0,j=22*160+25*2+1;i<10;i++,j+=2)
		ScreenPointer[j]=120;
	
}

void ChangeBatLong(int len)
{
	int ActualGrew,i,j,k;
	if(len>Bat.bat_length)
	{
		ActualGrew=(len-Bat.bat_length)/2;
		Bat.left_edge-=ActualGrew;
		Bat.right_edge+=ActualGrew;
	}
	else if(len<Bat.bat_length)
	{
		for(i=0,j=0,k=Bat.right_edge;i<10;i++,j+=2,k-=2)
		  {
		   ScreenPointer[2*Bat.left_edge+160*22+j+1]=0;
		   ScreenPointer[2*Bat.right_edge+160*22+k+1]=0;
		  }
		ActualGrew=(Bat.bat_length-len)/2;
		Bat.left_edge+=ActualGrew;
		Bat.right_edge-=ActualGrew;
	}
	Bat.bat_length=len;
}

void InitialBall(int ball)
{
	Ball[ball].pos.x=28;
	Ball[ball].pos.y=21;//שונה
	Ball[ball].dir.left=0;
	Ball[ball].dir.right=1;
	Ball[ball].dir.up=0;
	Ball[ball].dir.down=1;
	Ball[ball].color=GREENBRICK;
	Ball[ball].speed=NORMALSPEED;
	Ball[ball].ball_shape=9;
}

void InitialBat()
{
	Bat.left_edge=25;
	Bat.right_edge=34;
	Bat.bat_color=GREYBRICK;
	Bat.bat_length=10;
}

void BallPositionUpdater(int ball,int mutex,int ballmutex)
{
	int i,j;
	char GameOver[9]="GAME OVER";
	
	while(1)
	{
		
		sleep(1);
		ScreenPointer[(160*Ball[ball].pos.y+2*Ball[ball].pos.x)+1]=0;
		//wait(ballmutex);
		if(Ball[ball].pos.x==1)
		{
			if(Ball[ball].dir.up==1)
					Ball[ball].pos.y--;
			else
				Ball[ball].pos.y++;
			
			Ball[ball].dir.left=0;
			Ball[ball].dir.right=1;
			Ball[ball].pos.x++;
		}
		else if(Ball[ball].pos.x==59)
		{
			if(Ball[ball].dir.up==1)
				Ball[ball].pos.y--;
			else
				Ball[ball].pos.y++;
			Ball[ball].dir.left=1;
			Ball[ball].dir.right=0;
			Ball[ball].pos.x--;

		}
		else if(Ball[ball].pos.y==1)
		{
			if(Ball[ball].dir.left==1)
				Ball[ball].pos.x--;
			else
				Ball[ball].pos.x++;
			Ball[ball].dir.down=1;
			Ball[ball].dir.up=0;
			Ball[ball].pos.y++;
		}
		else if(Ball[ball].pos.y==21)
		{
			if(Ball[ball].pos.x>=Bat.left_edge && Ball[ball].pos.x<=Bat.right_edge)
			{
				
				if(Ball[ball].pos.x<=(Bat.left_edge+Bat.right_edge)/2)
				{ if(Ball[ball].pos.x<=(Bat.left_edge+Bat.right_edge)/4+1)
						Ball[ball].pos.x--;
					if(Ball[ball].dir.left==1)
					{
							Ball[ball].pos.x--;
						Ball[ball].dir.down=0;
						Ball[ball].dir.up=1;
						Ball[ball].pos.y--;
					}
					else
					{
							Ball[ball].pos.x--;
						Ball[ball].dir.left=1;
						Ball[ball].dir.right=0;
						Ball[ball].dir.down=0;
						Ball[ball].dir.up=1;
						Ball[ball].pos.y--;
					}
					//if(Ball[ball].dir.left==1)
					}		//Ball[ball].pos.x--;
			   else
				{ 
					if(Ball[ball].pos.x>=(((Bat.left_edge+Bat.right_edge)/2 + Bat.right_edge)/2 -1))
						Ball[ball].pos.x++;   
					if(Ball[ball].dir.left==1)
					{
							Ball[ball].pos.x++;
						Ball[ball].dir.left=0;
						Ball[ball].dir.right=1;
						Ball[ball].dir.down=0;
						Ball[ball].dir.up=1;
							Ball[ball].pos.y--;
					}
					else
					{
							Ball[ball].pos.x++;
						Ball[ball].dir.down=0;
						Ball[ball].dir.up=1;
							Ball[ball].pos.y--;
					}
					
				}
				/*if(Ball[ball].dir.left==1)
					Ball[ball].pos.x--;
				else
					Ball[ball].pos.x++;
				Ball[ball].dir.down=0;
				Ball[ball].dir.up=1;
				Ball[ball].pos.y--;*/
			}
			else
			{
				if(LifeNumber>1)
				{
					int i,j;
						for(i=0,j=0;i<Bat.bat_length;i++,j+=2)
							ScreenPointer[(22*160+Bat.left_edge*2)+j+1]=0;
					InitialBat();
						
					/*if(NumberOfBalls>1)
						NumberOfBalls--;*/
					//else
					//{
					LifeNumber--;
					InitialBall(0);
					//}
					if(LifeNumber==2)
						Life[8]=' ';
					if(LifeNumber==1)
						Life[7]=' ';
					
				}
				else
				{
					Life[6]=' ';
					//wait(dispid);
					for(i=0;i<18;i++)
					{
						Bricks[i].pos.x=0;
						Bricks[i].pos.y=0;
						Bricks[i].color=BLACKBRICK;
					}
					Ball[ball].ball_shape=' ';
					Bat.left_edge=0;
					Bat.right_edge=0;
					Bat.bat_length=0;
					for(i=0;i<25*80*2;i+=2)
					{	
						ScreenPointer[i]= ' ';
						ScreenPointer[i+1]=BLACKBRICK;
					}
					for(i=0,j=0;i<18;i+=2,j++)
					{
						ScreenPointer[11*160+74+i]=GameOver[j];
						ScreenPointer[11*160+74+i+1]=REDBRICK;// red color
					}
					//signal(dispid);
					//exit(0);//צריך עוד לעבוד על סגירת התכנית		
				}
				
			}
		}
		else if(StageFlag==1)
		{
			if(Ball[ball].pos.y==3)
			{
				for(i=0;i<6;i++)
				{
					if(((Ball[ball].pos.x>=Bricks[i].pos.x && Ball[ball].pos.x<=Bricks[i].right_side)||Ball[ball].pos.x==Bricks[i].pos.x-1 || Ball[ball].pos.x==Bricks[i].right_side+1) && Bricks[i].strength>0 && Bricks[i].color!=BLACKBRICK)
									{
									  
									  Sound(733);
									  ScreenPointer[(160*Ball[ball].pos.y+2*Ball[ball].pos.x)+1]=0;
									  Bricks[i].strength--;
									  wait(mutex);
									  if(Bricks[i].strength==0)
									  {
									   //ScreenPointer[174]=i;
									   //ScreenPointer[175]=14;
									   Bricks[i].color=BLACKBRICK;
									   send(scorepid,Bricks[i].points);
									   Bricks[i].points=0;
									   BricksCurrentNumber--;
									  }
									  if(Bricks[i].feature!=NOFEATURE && FeaturesInGame==0)
									  {
									   FeaturesInGame=1;
									   FeatureFlag=1;
									   send(featurepid,i);
									  }
									  signal(mutex);
									  if(BricksCurrentNumber==0)
									  {
									   //Stage1Flag=0;
									   //Stage2Flag=1;
									   StageFlag++;
									   BricksCurrentNumber=18;
							
											
									    InitialBricks();
									    InitialBat();
									    InitialScreen();
									    InitialBall(0);
									  }
									  if(Ball[ball].pos.x==Bricks[i].pos.x-1&&Ball[ball].dir.right==1)
									  {
									   if(Ball[ball].dir.up==1)
											Ball[ball].pos.y--;
									   else
										Ball[ball].pos.y++;
											Ball[ball].pos.x--;
									   Ball[ball].dir.left=1;
									   Ball[ball].dir.right=0;
									  }
									  else if(Ball[ball].pos.x==Bricks[i].right_side+1&&Ball[ball].dir.left==1)
									  {
									   if(Ball[ball].dir.up==1)
												Ball[ball].pos.y--;
									   else
											Ball[ball].pos.y++;
									   Ball[ball].pos.x++;
									   Ball[ball].dir.left=0;
									   Ball[ball].dir.right=1;
									  }
									  else if(Ball[ball].dir.up==1)
									  {
									   if(Ball[ball].dir.left==1)
												Ball[ball].pos.x--;
									   else
												Ball[ball].pos.x++;
									   Ball[ball].dir.down=1;
									   Ball[ball].dir.up=0;
											Ball[ball].pos.y++;
									   
									  }
									  else
									  {
									   if(Ball[ball].dir.left==1)
											Ball[ball].pos.x--;
									   else
											Ball[ball].pos.x++;
									   Ball[ball].dir.down=0;
									   Ball[ball].dir.up=1;
									  
											Ball[ball].pos.y--;
									   
									  }
						 }
						
				}
				
				if(Ball[ball].dir.up==1)
					Ball[ball].pos.y--;
				else
					Ball[ball].pos.y++;
				if(Ball[ball].dir.left==1)
					Ball[ball].pos.x--;
				else
					Ball[ball].pos.x++;
				
			}
			else if(Ball[ball].pos.y==6)
			{
				for(i=6;i<12;i++)
				{
					if(((Ball[ball].pos.x>=Bricks[i].pos.x && Ball[ball].pos.x<=Bricks[i].right_side)||Ball[ball].pos.x==Bricks[i].pos.x-1 || Ball[ball].pos.x==Bricks[i].right_side+1) && Bricks[i].strength>0 && Bricks[i].color!=BLACKBRICK)
									{
									  
									  Sound(733);
									  ScreenPointer[(160*Ball[ball].pos.y+2*Ball[ball].pos.x)+1]=0;
									  Bricks[i].strength--;
									  wait(mutex);
									  if(Bricks[i].strength==0)
									  {
									   //ScreenPointer[174]=i;
									   //ScreenPointer[175]=14;
									   Bricks[i].color=BLACKBRICK;
									   send(scorepid,Bricks[i].points);
									   Bricks[i].points=0;
									   BricksCurrentNumber--;
									  }
									  if(Bricks[i].feature!=NOFEATURE && FeaturesInGame==0)
									  {
									   FeaturesInGame=1;
									   FeatureFlag=1;
									   send(featurepid,i);
									  }
									  signal(mutex);
									  if(BricksCurrentNumber==0)
									  {
									   //Stage1Flag=0;
									   //Stage2Flag=1;
									   StageFlag++;
									   BricksCurrentNumber=18;
									   
									   InitialBricks();
									   InitialBat();
									   InitialScreen();
									   InitialBall(0);
									  }
									  if(Ball[ball].pos.x==Bricks[i].pos.x-1&&Ball[ball].dir.right==1)
									  {
									   if(Ball[ball].dir.up==1)
										Ball[ball].pos.y--;
									   else
										Ball[ball].pos.y++;
									   
									   Ball[ball].pos.x--;
									   Ball[ball].dir.left=1;
									   Ball[ball].dir.right=0;
									  }
									  else if(Ball[ball].pos.x==Bricks[i].right_side+1&&Ball[ball].dir.left==1)
									  {
									   if(Ball[ball].dir.up==1)
										Ball[ball].pos.y--;
									   else
										Ball[ball].pos.y++;
									   
									   Ball[ball].pos.x++;
									   Ball[ball].dir.left=0;
									   Ball[ball].dir.right=1;
									  }
									  else if(Ball[ball].dir.up==1)
									  {
									   if(Ball[ball].dir.left==1)
										Ball[ball].pos.x--;
									   else
										Ball[ball].pos.x++;
									   Ball[ball].dir.down=1;
									   Ball[ball].dir.up=0;
									   Ball[ball].pos.y++;
									   
									  }
									  else
									  {
									   if(Ball[ball].dir.left==1)
										Ball[ball].pos.x--;
									   else
										Ball[ball].pos.x++;
									   Ball[ball].dir.down=0;
									   Ball[ball].dir.up=1;
									   Ball[ball].pos.y--;
									   
									  }
						 }
						
				}
				
				if(Ball[ball].dir.up==1)
					Ball[ball].pos.y--;
				else
					Ball[ball].pos.y++;
				if(Ball[ball].dir.left==1)
					Ball[ball].pos.x--;
				else
					Ball[ball].pos.x++;
			}
			else if(Ball[ball].pos.y==9)
			{
				for(i=12;i<18;i++)
				{
					if(((Ball[ball].pos.x>=Bricks[i].pos.x && Ball[ball].pos.x<=Bricks[i].right_side)||Ball[ball].pos.x==Bricks[i].pos.x-1 || Ball[ball].pos.x==Bricks[i].right_side+1) && Bricks[i].strength>0 && Bricks[i].color!=BLACKBRICK)
									{
									  
									  Sound(733);
									  ScreenPointer[(160*Ball[ball].pos.y+2*Ball[ball].pos.x)+1]=0;
									  Bricks[i].strength--;
									  wait(mutex);
									  if(Bricks[i].strength==0)
									  {
									   //ScreenPointer[174]=i;
									   //ScreenPointer[175]=14;
									   Bricks[i].color=BLACKBRICK;
									   send(scorepid,Bricks[i].points);
									   Bricks[i].points=0;
									   BricksCurrentNumber--;
									   
									  }
									  
									  if(Bricks[i].feature!=NOFEATURE && FeaturesInGame==0)
									  {
									   FeaturesInGame=1;
									   FeatureFlag=1;
									   send(featurepid,i);
									  }
									  signal(mutex);
									  if(BricksCurrentNumber==0)
									  {
									   //Stage1Flag=0;
									   //Stage2Flag=1;
									   StageFlag++;
									   BricksCurrentNumber=18;
									;
									   InitialBricks();
									   InitialBat();
									   InitialScreen();
									   InitialBall(0);
									  }
									  if(Ball[ball].pos.x==Bricks[i].pos.x-1&&Ball[ball].dir.right==1)
									  {
									   if(Ball[ball].dir.up==1)
											Ball[ball].pos.y--;
										else
											Ball[ball].pos.y++;
									    Ball[ball].pos.x--;
									    Ball[ball].dir.left=1;
									    Ball[ball].dir.right=0;
									   
									  }
									  else if(Ball[ball].pos.x==Bricks[i].right_side+1&&Ball[ball].dir.left==1)
									  {
									   if(Ball[ball].dir.up==1)
										Ball[ball].pos.y--;
									   else
										Ball[ball].pos.y++;
									   
									   Ball[ball].pos.x++;
									   Ball[ball].dir.left=0;
									   Ball[ball].dir.right=1;
									  }
									  else if(Ball[ball].dir.up==1)
									  {
									   if(Ball[ball].dir.left==1)
										Ball[ball].pos.x--;
									   else
										Ball[ball].pos.x++;
									   Ball[ball].dir.down=1;
									   Ball[ball].dir.up=0;
									   Ball[ball].pos.y++;
									   
									  }
									  else
									  {
									   if(Ball[ball].dir.left==1)
										Ball[ball].pos.x--;
									   else
										Ball[ball].pos.x++;
									   Ball[ball].dir.down=0;
									   Ball[ball].dir.up=1;
									   Ball[ball].pos.y--;
									   
									  }
						 }
						
				}
				
				if(Ball[ball].dir.up==1)
					Ball[ball].pos.y--;
				else
					Ball[ball].pos.y++;
				if(Ball[ball].dir.left==1)
					Ball[ball].pos.x--;
				else
					Ball[ball].pos.x++;
			}
			else
			{
				if(Ball[ball].dir.up==1)
					Ball[ball].pos.y--;
				else
					Ball[ball].pos.y++;
				if(Ball[ball].dir.left==1)
					Ball[ball].pos.x--;
				else
					Ball[ball].pos.x++;
			}
			
		}
		else if(StageFlag==2)
		{
			asm{
				push ax
				push bx
				mov al,36h
				out 43h,al
				mov bx,4800
				mov al,bl
				out 40h,al
				mov al,bh		
				out 40h,al
				pop ax
				pop bx				
			}
			
			if(Ball[ball].pos.y==3)
			{
				
				for(i=0;i<6;i++)
				{
					if(((Ball[ball].pos.x>=Bricks[i].pos.x && Ball[ball].pos.x<=Bricks[i].right_side)||Ball[ball].pos.x==Bricks[i].pos.x-1 || Ball[ball].pos.x==Bricks[i].right_side+1) && Bricks[i].strength>0 && Bricks[i].color!=BLACKBRICK)
									{
									  
									  Sound(733);
									  ScreenPointer[(160*Ball[ball].pos.y+2*Ball[ball].pos.x)+1]=0;
									  Bricks[i].strength--;
									  wait(mutex);
									  if(Bricks[i].strength==0)
									  {
									   //ScreenPointer[174]=i;
									   //ScreenPointer[175]=14;
									   Bricks[i].color=BLACKBRICK;
									   send(scorepid,Bricks[i].points);
									   Bricks[i].points=0;
									   BricksCurrentNumber--;
									   
									  }
									  if(Bricks[i].feature!=NOFEATURE && FeaturesInGame==0)
									  {
									   FeaturesInGame=1;
									   FeatureFlag=1;
									   send(featurepid,i);
									  }
									  signal(mutex);
									  if(BricksCurrentNumber==0)
									  {
									   //Stage1Flag=0;
									   //Stage2Flag=1;
									   StageFlag++;
									   BricksCurrentNumber=18;
								
									   InitialBricks();
									   InitialBat();
									   InitialScreen();
									   InitialBall(0);
									  }
									  if(Ball[ball].pos.x==Bricks[i].pos.x-1&&Ball[ball].dir.right==1)
									  {
									   if(Ball[ball].dir.up==1)
										Ball[ball].pos.y--;
									   else
										Ball[ball].pos.y++;
									   
									   Ball[ball].pos.x--;
									   Ball[ball].dir.left=1;
									   Ball[ball].dir.right=0;
									  }
									  else if(Ball[ball].pos.x==Bricks[i].right_side+1&&Ball[ball].dir.left==1)
									  {
									   if(Ball[ball].dir.up==1)
										Ball[ball].pos.y--;
									   else
										Ball[ball].pos.y++;
									   
									   Ball[ball].pos.x++;
									   Ball[ball].dir.left=0;
									   Ball[ball].dir.right=1;
									  }
									  else if(Ball[ball].dir.up==1)
									  {
									   if(Ball[ball].dir.left==1)
										Ball[ball].pos.x--;
									   else
										Ball[ball].pos.x++;
									   Ball[ball].dir.down=1;
									   Ball[ball].dir.up=0;
									   Ball[ball].pos.y++;
									  }
									  else
									  {
									   if(Ball[ball].dir.left==1)
										Ball[ball].pos.x--;
									   else
										Ball[ball].pos.x++;
									   Ball[ball].dir.down=0;
									   Ball[ball].dir.up=1;
									   Ball[ball].pos.y--;
									   
									  }
						 }
						
				}
				
				if(Ball[ball].dir.up==1)
					Ball[ball].pos.y--;
				else
					Ball[ball].pos.y++;
				if(Ball[ball].dir.left==1)
					Ball[ball].pos.x--;
				else
					Ball[ball].pos.x++;
				
			}
			else if(Ball[ball].pos.y==6)
			{
				for(i=6;i<12;i++)
				{
					if(((Ball[ball].pos.x>=Bricks[i].pos.x && Ball[ball].pos.x<=Bricks[i].right_side)||Ball[ball].pos.x==Bricks[i].pos.x-1 || Ball[ball].pos.x==Bricks[i].right_side+1) && Bricks[i].strength>0 && Bricks[i].color!=BLACKBRICK)
							 {
							  Sound(733);
							  ScreenPointer[(160*Ball[ball].pos.y+2*Ball[ball].pos.x)+1]=0;
							  Bricks[i].strength--;
							  wait(mutex);
							  if(Bricks[i].strength==0)
							  {
							   //ScreenPointer[174]=i;
							   //ScreenPointer[175]=14;
							   Bricks[i].color=BLACKBRICK;
							   send(scorepid,Bricks[i].points);
							   Bricks[i].points=0;
							   BricksCurrentNumber--;

							  }
							  if(Bricks[i].feature!=NOFEATURE && FeaturesInGame==0)
							  {
							   FeaturesInGame=1;
							   FeatureFlag=1;
							   send(featurepid,i);
							  }
							  signal(mutex);
							  if(BricksCurrentNumber==0)
							  {
							   //Stage1Flag=0;
							   //Stage2Flag=1;
								StageFlag++;
								BricksCurrentNumber=18;
							   InitialBricks();
							   InitialBat();
							   InitialScreen();
							   InitialBall(0);
							  }
							  if(Ball[ball].pos.x==Bricks[i].pos.x-1&&Ball[ball].dir.right==1)
							  {
							   if(Ball[ball].dir.up==1)
								Ball[ball].pos.y--;
							   else
								Ball[ball].pos.y++;
							   
							   Ball[ball].pos.x--;
							   Ball[ball].dir.left=1;
							   Ball[ball].dir.right=0;
							  }
							  else if(Ball[ball].pos.x==Bricks[i].right_side+1&&Ball[ball].dir.left==1)
							  {
							   if(Ball[ball].dir.up==1)
								Ball[ball].pos.y--;
							   else
								Ball[ball].pos.y++;
							   
							   Ball[ball].pos.x++;
							   Ball[ball].dir.left=0;
							   Ball[ball].dir.right=1;
							  }
							  else if(Ball[ball].dir.up==1)
							  {
							   if(Ball[ball].dir.left==1)
								Ball[ball].pos.x--;
							   else
								Ball[ball].pos.x++;
							   Ball[ball].dir.down=1;
							   Ball[ball].dir.up=0;
							   Ball[ball].pos.y++;
							  }
							  else
							  {
							   if(Ball[ball].dir.left==1)
								Ball[ball].pos.x--;
							   else
								Ball[ball].pos.x++;
							   Ball[ball].dir.down=0;
							   Ball[ball].dir.up=1;
							   Ball[ball].pos.y--;
							   
							  }
							 }
				}
				
				if(Ball[ball].dir.up==1)
					Ball[ball].pos.y--;
				else
					Ball[ball].pos.y++;
				if(Ball[ball].dir.left==1)
					Ball[ball].pos.x--;
				else
					Ball[ball].pos.x++;
			}
			else if(Ball[ball].pos.y==9)
			{
				for(i=12;i<18;i++)
				{
					if(((Ball[ball].pos.x>=Bricks[i].pos.x && Ball[ball].pos.x<=Bricks[i].right_side)||Ball[ball].pos.x==Bricks[i].pos.x-1 || Ball[ball].pos.x==Bricks[i].right_side+1) && Bricks[i].strength>0 && Bricks[i].color!=BLACKBRICK)
							 {
							  Sound(733);
							  ScreenPointer[(160*Ball[ball].pos.y+2*Ball[ball].pos.x)+1]=0;
							  Bricks[i].strength--;
							  wait(mutex);
							  if(Bricks[i].strength==0)
							  {
							   //ScreenPointer[174]=i;
							   //ScreenPointer[175]=14;
							   Bricks[i].color=BLACKBRICK;
							   send(scorepid,Bricks[i].points);
							   Bricks[i].points=0;
							   BricksCurrentNumber--;
							  }
							  if(Bricks[i].feature!=NOFEATURE && FeaturesInGame==0)
							  {
							   FeaturesInGame=1;
							   FeatureFlag=1;
							   send(featurepid,i);
							  }
							  signal(mutex);
							  if(BricksCurrentNumber==0)
							  {
							   //Stage1Flag=0;
							   //Stage2Flag=1;
							   StageFlag++;
							   BricksCurrentNumber=18;
							   InitialBricks();
							   InitialBat();
							   InitialScreen();
							   InitialBall(0);
							  }
							  if(Ball[ball].pos.x==Bricks[i].pos.x-1&&Ball[ball].dir.right==1)
							  {
							   if(Ball[ball].dir.up==1)
								Ball[ball].pos.y--;
							   else
								Ball[ball].pos.y++;
							   
							   Ball[ball].pos.x--;
							   Ball[ball].dir.left=1;
							   Ball[ball].dir.right=0;
							  }
							  else if(Ball[ball].pos.x==Bricks[i].right_side+1&&Ball[ball].dir.left==1)
							  {
							   if(Ball[ball].dir.up==1)
								Ball[ball].pos.y--;
							   else
								Ball[ball].pos.y++;
							   
							   Ball[ball].pos.x++;
							   Ball[ball].dir.left=0;
							   Ball[ball].dir.right=1;
							  }
							  else if(Ball[ball].dir.up==1)
							  {
							   if(Ball[ball].dir.left==1)
								Ball[ball].pos.x--;
							   else
								Ball[ball].pos.x++;
							   Ball[ball].dir.down=1;
							   Ball[ball].dir.up=0;
							   Ball[ball].pos.y++;
							   
							  }
							  else
							  {
							   if(Ball[ball].dir.left==1)
								Ball[ball].pos.x--;
							   else
								Ball[ball].pos.x++;
							   Ball[ball].dir.down=0;
							   Ball[ball].dir.up=1;
							   Ball[ball].pos.y--;
							   
							  }
							 }
				}
				
				if(Ball[ball].dir.up==1)
					Ball[ball].pos.y--;
				else
					Ball[ball].pos.y++;
				if(Ball[ball].dir.left==1)
					Ball[ball].pos.x--;
				else
					Ball[ball].pos.x++;
			}
			else
			{
				if(Ball[ball].dir.up==1)
					Ball[ball].pos.y--;
				else
					Ball[ball].pos.y++;
				if(Ball[ball].dir.left==1)
					Ball[ball].pos.x--;
				else
					Ball[ball].pos.x++;
			}
		}
		else if(StageFlag==3)
		{
			
		}
		//signal(ballmutex);

	}
}

void FeaturePositionUpdater()
{
	int i;
	i=receive();
	Feature[FeaturesInGame-1].pos.x=Bricks[i].pos.x;
	Feature[FeaturesInGame-1].pos.y=Bricks[i].pos.y+1;
	Feature[FeaturesInGame-1].feature_type=Bricks[i].feature;
	Feature[FeaturesInGame-1].feature_color=Bricks[i].feature_attr;
	while(FeatureFlag)
	{
		sleep(1);
		ScreenPointer[160*Feature[FeaturesInGame-1].pos.y+2*Feature[FeaturesInGame-1].pos.x]=' ';
		ScreenPointer[(160*Feature[FeaturesInGame-1].pos.y+2*Feature[FeaturesInGame-1].pos.x)+1]=0;
		Feature[FeaturesInGame-1].pos.y++;
		
		if(Feature[FeaturesInGame-1].pos.y==21)
		{		
				FeatureFlag=0;

			if(Feature[FeaturesInGame-1].pos.x>=Bat.left_edge && Feature[FeaturesInGame-1].pos.x<=Bat.right_edge)
			{
				
				if(Feature[FeaturesInGame-1].feature_type==LONGBAT)
					ChangeBatLong(20);
				else if(Feature[FeaturesInGame-1].feature_type==SLOWBALL)
				{
				}
				else if (Feature[FeaturesInGame-1].feature_type==BALLCATCH)
				{
				}
				else if(Feature[FeaturesInGame-1].feature_type==LAISERSHOTS)
				{
				}
				else if(Feature[FeaturesInGame-1].feature_type==LEVELUP)
				{
				}
				else if(Feature[FeaturesInGame-1].feature_type==EXTRABALLS)
				{
					/*InitialBall(1);
					NumberOfBalls++;
					InitialBall(2);
					NumberOfBalls++;
					resume(ballid2);
					resume(ballid3);*/
				}
				
			}
			
		}
	}
	//ScreenPointer[160*Feature[FeaturesInGame-1].pos.y+2*Feature[FeaturesInGame-1].pos.x]=' ';
	//ScreenPointer[(160*Feature[FeaturesInGame-1].pos.y+2*Feature[FeaturesInGame-1].pos.x)+1]=0;
		Feature[FeaturesInGame-1].pos.y=0;
		Feature[FeaturesInGame-1].pos.x=0;
	FeatureFlag=0;
}
void ScoreUpdater()
{
	int sum=0;
	while(1)
	{
		sum+=receive();
		itoa(sum,Points,10);
	}
}


void loginscreen(){
  int i,j=0;

  // paint first row
  for (i = 200; i < 208; i+=2)
  {
    ScreenPointer[i] = ' ';
    ScreenPointer[i+1] = 120;
  }
  for (i = 260; i < 268; i+=2)
  {
    ScreenPointer[i] = ' ';
    ScreenPointer[i+1] = 120;
  }
  for (i = 420; i < 428; i+=2)
  {
    ScreenPointer[i] = ' ';
    ScreenPointer[i+1] = 120;
  }
  for (i = 572; i < 580; i+=2)
  {
    ScreenPointer[i] = ' ';
    ScreenPointer[i+1] = 120;
  }
  for (i = 732; i < 740; i+=2)
  {
    ScreenPointer[i] = ' ';
    ScreenPointer[i+1] = 120;
  }
  for (i = 360; i < 368; i+=2)
  {
    ScreenPointer[i] = ' ';
    ScreenPointer[i+1] = 120;
  }
  for (i = 528; i < 536; i+=2)
  {
    ScreenPointer[i] = ' ';
    ScreenPointer[i+1] = 120;
  }
  for (i = 688; i < 696; i+=2)
  {
    ScreenPointer[i] = ' ';
    ScreenPointer[i+1] = 120;
  }
  for (i = 840; i < 908; i+=2)
  {
    ScreenPointer[i] = ' ';
    ScreenPointer[i+1] = 120;
  }
  for (i = 1000; i < 1068; i+=2)
  {
    ScreenPointer[i] = ' ';
    ScreenPointer[i+1] = 120;
  }
  for (i = 1152; i < 1236; i+=2)
  {
    ScreenPointer[i] = ' ';
    ScreenPointer[i+1] = 120;
  }
  for (i = 1312; i < 1396; i+=2)
  {
    ScreenPointer[i] = ' ';
    ScreenPointer[i+1] = 120;
  }
  for (i = 1464; i < 1564; i+=2)
  {
    ScreenPointer[i] = ' ';
    ScreenPointer[i+1] = 120;
  }
  for (i = 1624; i < 1724; i+=2)
  {
    ScreenPointer[i] = ' ';
    ScreenPointer[i+1] = 120;
  }
  for (i = 1784; i < 1884; i+=2)
  {
    ScreenPointer[i] = ' ';
    ScreenPointer[i+1] = 120;
  }
  for (i = 1944; i < 1952; i+=2)
  {
    ScreenPointer[i] = ' ';
    ScreenPointer[i+1] = 120;
  }

for (i = 2104; i < 2112; i+=2)
{
  ScreenPointer[i] = ' ';
  ScreenPointer[i+1] = 120;
}
for (i = 2448; i < 2464; i+=2)
{
  ScreenPointer[i] = ' ';
  ScreenPointer[i+1] = 120;
}
for (i = 2608; i < 2624; i+=2)
{
  ScreenPointer[i] = ' ';
  ScreenPointer[i+1] = 120;
}
for (i = 2264; i < 2272; i+=2)
{
  ScreenPointer[i] = ' ';
  ScreenPointer[i+1] = 120;
}
for (i = 1960; i < 2028; i+=2)
{
  ScreenPointer[i] = ' ';
  ScreenPointer[i+1] = 120;
}
for (i = 2180; i < 2188; i+=2)
{
  ScreenPointer[i] = ' ';
  ScreenPointer[i+1] = 120;
}

for (i = 2340; i < 2348; i+=2)
{
  ScreenPointer[i] = ' ';
  ScreenPointer[i+1] = 120;
}
for (i = 2484; i < 2500; i+=2)
{
  ScreenPointer[i] = ' ';
  ScreenPointer[i+1] = 120;
}
for (i = 2644; i < 2660; i+=2)
{
  ScreenPointer[i] = ' ';
  ScreenPointer[i+1] = 120;
}
for (i = 2036; i < 2044; i+=2)
{
  ScreenPointer[i] = ' ';
  ScreenPointer[i+1] = 120;
}
for (i = 2196; i < 2204; i+=2)
{
  ScreenPointer[i] = ' ';
  ScreenPointer[i+1] = 120;
}
for (i = 2356; i < 2364; i+=2)
{
  ScreenPointer[i] = ' ';
  ScreenPointer[i+1] = 120;
}
for (i = 2120; i < 2128; i+=2)
{
  ScreenPointer[i] = ' ';
  ScreenPointer[i+1] = 120;
}
for (i = 2280; i < 2288; i+=2)
	{
	  ScreenPointer[i] = ' ';
	  ScreenPointer[i+1] = 120;
	}
	ScreenPointer[3580]='P';
	ScreenPointer[3582]='r';
	ScreenPointer[3584]='e';
	ScreenPointer[3586]='s';
	ScreenPointer[3588]='s';
	ScreenPointer[3590]=' ';
	ScreenPointer[3592]='E';
	ScreenPointer[3594]='n';
	ScreenPointer[3596]='t';
	ScreenPointer[3598]='e';
	ScreenPointer[3600]='r';
	ScreenPointer[3602]='.';
	ScreenPointer[3604]='.';
	}
void ChangeSpeaker( int status )
{
 int portval;
//   portval = inportb( 0x61 );

     portval = 0;
  asm {
       PUSH AX
       MOV AL,61h
       MOV byte ptr portval,AL
       POP AX
      }

   if ( status==ON )
    portval |= 0x03;
     else
      portval &=~ 0x03;
       // outportb( 0x61, portval );
       asm {
         PUSH AX
         MOV AX,portval
         OUT 61h,AL
         POP AX
       } // asm

 } /*--ChangeSpeaker( )----------*/

 void Sound( int hertz )
 {
  unsigned divisor = 1193180L / hertz;

   ChangeSpeaker( ON );

  //        outportb( 0x43, 0xB6 );
 asm{
mov     al, 182
out     43h, al
mov     ax, 733
out     42h, al
mov     al, ah
out     42h, al
in      al, 61h
or      al, 00000011b
out     61h, al
mov     bx, 25
}
pause1:
asm{
mov     cx, 2048
}
pause2:
asm{
dec     cx
jne     pause2
dec     bx
jne     pause1
in      al, 61h
and     al, 11111100b
out     61h, al
}
  ChangeSpeaker( OFF );
      } /*--Sound( )-----*/


void Change70hRate(int rate) 
{
 
 asm{
   // PROGRAMING MASTER PIC TO SET INT 8,9 AND SLAVE PIC
   CLI
   MOV AL,11111000b
   OUT 21h,AL
   // PROGRAMING SLAVE PIC TO SET INT 70h
   MOV AL,11111110b
   OUT 0A1h,AL
   // PROGRAMING A REGISTER TO WORK WITH THE RATE 2 AND BASE 5
   IN AL,70h
   MOV AL,0Ah
   OUT 70h,AL
   MOV AL,8Ah
   OUT 70h,AL
   XOR AX,AX
   MOV AL,BYTE PTR rate
   OR AL,01010000b
   OUT 71h,AL
   IN AL,71h
   // PROGRAMING B REGISTER TO WORK IN PERIODIC MODE
   MOV AL,0Bh
   OUT 70h,AL
   MOV AL,8Bh
   OUT 70h,AL
   XOR AX,AX
   OR AL,01000000b
   OUT 71h,AL
   IN AL,71h
   // READING C REGISTER VALUES
   MOV AL,0Ch
   OUT 70h,AL
   IN AL,71h
   // READING D REGISTER VALUES
   MOV AL,0Dh
   OUT 70h,AL
   IN AL,71h
   STI
 }
}


int Int70hHandler(int mdevno)
{  
 counter++;
 ScreenPointer[(160*Ball[0].pos.y+2*Ball[0].pos.x)]=Ball[0].ball_shape;
 ScreenPointer[(160*Ball[0].pos.y+2*Ball[0].pos.x)+1]=9;
 if(FeatureFlag==1)
 {
  if(SlowBallFlag==1)
  {
   if(counter==315)// Original 5 second time, based on 63 interrupts per second
   {
    counter=0;
    Change70hRate(2);// 134 interrupts per second
    FeatureFlag=0;
    FeaturesInGame=0;
   }
  }
  else if(LongBatFlag==1)
  {
   if(counter==670)
   {
    ChangeBatLong(10);
    FeatureFlag=0;
    FeaturesInGame=0;
   }
  }
  else if(BallCatchFlag==1)
  {
   
  }
  else if(LaiserShotsFlag==1)
  {
   
  }
 }
 
 
	 asm{
	  IN AL,70h
	  MOV BX,AX
	  MOV AL,0Ch
	  OUT 70h,AL
	  MOV AL,8Ch
	  OUT 70h,AL
	  IN AL,71h
	  MOV AX,BX
	  OUT 70h,AL
	  MOV AL,20h
	  OUT 0A0h,AL
	  OUT 020h,AL
	 }
}	  
	  
	  
	  
void SetInt70hValues(unsigned int *masterOldMask,unsigned int *slaveOldMask,unsigned int *rtcOldAMask,unsigned int *rtcOldBMask)
{
 asm{
   // PROGRAMING MASTER PIC TO SET INT 8,9 AND SLAVE PIC
   CLI
   IN AL,21h
   MOV BYTE PTR masterOldMask,AL
   MOV AL,11111000b
   OUT 21h,AL
   // PROGRAMING SLAVE PIC TO SET INT 70h
   IN AL,0A1h
   MOV BYTE PTR slaveOldMask,AL
   MOV AL,11111110b
   OUT 0A1h,AL
   // PROGRAMING A REGISTER TO WORK WITH THE RATE 2 AND BASE 5
   IN AL,70h
   MOV AL,0Ah
   OUT 70h,AL
   MOV AL,8Ah
   OUT 70h,AL
   IN AL,71h
   MOV BYTE PTR rtcOldAMask,AL
   XOR AX,AX
   OR AL,0010b
   OR AL,01010000b
   OUT 71h,AL
   IN AL,71h
   // PROGRAMING B REGISTER TO WORK IN PERIODIC MODE
   MOV AL,0Bh
   OUT 70h,AL
   MOV AL,8Bh
   OUT 70h,AL
   IN AL,71h
   MOV BYTE PTR rtcOldBMask,AL
   XOR AX,AX
   OR AL,01000000b
   OUT 71h,AL
   IN AL,71h
   // READING C REGISTER VALUES
   MOV AL,0Ch
   OUT 70h,AL
   IN AL,71h
   // READING D REGISTER VALUES
   MOV AL,0Dh
   OUT 70h,AL
   IN AL,71h
   STI
 }
 mapinit(0x70,Int70hHandler,0x70);
}	  

 

	  
	  
xmain()
{
	int uppid,featureid,scoreid,dispid,recvpid,ballpid,ballpid2,ballpid3,xmainpid=getpid();
	int mutex= screate(1);
	int ballmutex=screate(1);
	unsigned int* masterOldMask;
	unsigned int* slaveOldMask;
	unsigned int* rtcOldAMask;
	unsigned int* rtcOldBMask;
	//mapinit(INTERRUPT70H,int70hHandler,INTERRUPT70H);
	asm{
		mov al,36h
		out 43h,al
		mov bx,6600
		mov al,bl
		out 40h,al
		mov al,bh		
		out 40h,al	
	}
	loginscreen();
	getc();
	clrscr();
	SetInt70hValues(masterOldMask,slaveOldMask,rtcOldAMask,rtcOldBMask);
	InitialBat();
	InitialBricks();
	InitialScreen();
	InitialBall(0);
	resume(featureid = create(FeaturePositionUpdater, INITSTK, INITPRIO, "FEATURE", 0));
	featurepid=featureid;
	resume(scoreid = create(ScoreUpdater, INITSTK, INITPRIO, "SCOREUPDATER", 0));
	scorepid=scoreid;
	resume(dispid = create(displayer, INITSTK, INITPRIO, "DISPLAYER", 1,ballmutex));
	resume(recvpid = create(receiver, INITSTK, INITPRIO + 3, "RECIVEVER", 0));
	resume(uppid = create(updateter, INITSTK, INITPRIO, "UPDATER", 1,mutex));
	resume(ballpid = create(BallPositionUpdater, INITSTK, INITPRIO, "BALL", 3,0,mutex,ballmutex));
	//ballpid2 = create(BallPositionUpdater, INITSTK, INITPRIO, "BALL2", 1,1);
	//ballpid3 = create(BallPositionUpdater, INITSTK, INITPRIO, "BALL3", 1,2);
	receiver_pid = recvpid;
	//ballid2=ballpid2;
	//ballid3=ballpid3;
	set_new_int9_newisr();
	schedule(2, 2, dispid, 0, uppid, 1);

} // xmain
