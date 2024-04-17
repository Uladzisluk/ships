# Ships.
## This is a console game written in C++.
### Description
A simple game Battleship. Consult https://en.wikipedia.org/wiki/Battleship_(game) for an overview.
This game have 2 players: "A" and "B". The classes of ships are denoted by [CAR]RIER, [BAT]TLESHIP, [CRU]ISER, [DES]TROYER.

### How to Install and Run the Project
- Clone this repository
- Create C++ Console Application
- Add ships.cpp to source files
- Run

### How to Use the Project
#### Details of the interface
The commands are grouped into two groups: the description of the state of the game, the description of a players move in a given turn. E.g. a group [playerA] [playerA] specifies the commands given by A in a single turn. The players are doing the turns alternately.

The group of **state** commands starts with [state] and the commands from this group are provided until another [state] is encountered. There are
1. Basic Logic:
	- ''' PRINT x '''
	Print the state of the game. For x = 0 it is the basic version, for x = 1 it is the advanced version.
	- ''' SET_FLEET P a1 a2 a3 a4 '''
	The number of ships of consecutive classes (CARRIER ... DESTROYER) for the player P are a1 ... a4. If the command is not specified the default numbers 1, 2, 3, 4 apply.
	- ''' NEXT_PLAYER P '''
	Sets an information that P should make move next.
2. Extended Logic:
	- ''' BOARD_SIZE y x '''
	Set the board size to y times x.
	- ''' INIT_POSITION P y1 x1 y2 x2 '''
	The player P can place ships in rectangle given by (y1, x1) (left upper) and (y2, x2) (right lower). It means that the player can place ships in [y1, y2] in Y dimension and in [x1, x2] in X dimension - the edges are included.
	- ''' REEF y x '''
	There is a reef at position (y,x). It should be printed as #.
	- ''' SHIP P y x D i C a1...al '''
	There is a ship of player P at position (y,x) in the direction D(N/W/S/E) i-th of class C (CAR/BAT/CRU/DES). The state of the segments are a1 … al, where l is the length of the ship and 1 means that the part is not destroyed, 0 that it was destroyed. This command is unaffected by starting positions requirements. Keep in mind that there are no spaces between a1 and a2, …, al-1 and al.
	- ''' EXTENDED_SHIPS '''
	Turn the extended logic of ships. I.e. the ships are composed of the 2 sections, etc. Turning this on disables the default SHOOT operations by players.

The group of **player** commands starts and ends with [playerX], where X is either A or B. Commands are give until another corresponding [playerX] is given. In the [player] group you have:
1. Basic Logic:
	- ''' PLACE_SHIP y x D i C '''
	Place the i-th ship of class C at (y,x) with direction equal to D.
	- ''' SHOOT y x '''
	Shoot at the field (y,x). Shooting can only start if all the ships were placed.
2. Extended Logic:
	- ''' MOVE i C x '''
	Move the i-th ship of class C x ([F]orward, [L]eft, [R]ight).
	- ''' SHOOT i C y x '''
	Let one of the shoots of i-th class ship be at (y,x).
	
#### A sample input and corresponding output for basic mode:
'''
 [state]

SET_FLEET A 0 1 0 0

SET_FLEET B 0 0 1 1

[state]

[playerA]

PLACE_SHIP 6 0 N 0 BAT

[playerA]

[playerB]

PLACE_SHIP 16 0 N 0 CRU

PLACE_SHIP 16 2 N 0 DES

[playerB]


[state]

PRINT 0

[state]


[playerA]

SHOOT 17 0

[playerA]

[playerB]

SHOOT 7 0

[playerB]


[state]

PRINT 0

[state]

          
          
          
          
          
          
+         
+         
+         
+         
          
          
          
          
          
          
+ +       
+ +       
+         
          
          
PARTS REMAINING:: A : 4 B : 5
          
          
          
          
          
          
+         
x         
+         
+         
          
          
          
          
          
          
+ +       
x +       
+         
          

PARTS REMAINING:: A : 3 B : 4
'''


