#pragma comment(linker,"/STACK:256000000")
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <map>
#include <set>
#include <ctime>
#include <algorithm>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "system.h"

using namespace std;
using namespace PlatBox;

#define WR printf
#define RE scanf
#define PB push_back
#define Y second
#define X first

#define FOR(i,k,n) for(int i=(k); i<=(n); i++)
#define DFOR(i,k,n) for(int i=(k); i>=(n); i--)
#define SZ(a) (int)((a).size())
#define FA(i,v) FOR(i,0,SZ(v)-1)
#define RFA(i,v) DFOR(i,SZ(v)-1,0)
#define CLR(a) memset(a, 0, sizeof(a))
#define CLRm1(a) memset(a, -1, sizeof(a))

#define LL long long
#define VI  vector<int>
#define PAR pair<int ,int>
#define o_O 1000000000

void __never(int a){fprintf(stderr, "\nOPS %d", a);}
#define ass(s) {if (!(s)) {__never(__LINE__);cout.flush();cerr.flush();throw(42);}}

const int kNForCode = 100000;
const int kPivotShift = 20000;

int input_seed = -1;
int param1 = 0;
int param2 = 0;
int id_num = 0;

#define CHECK_NEW_WORD

string guten_tag = "merged_1";
int move_by_chr[255];
vector<string> powerphrases;
bool quiet = false;
int return_code = 0;

typedef unsigned int uint;
const int MAX_VERS = 2000;
int n_vers = 0;
map<vector<int>, int> vers;
const char letters[] = "p'!.03aghij4lmno 5bcefy2dqrvz1kstuwx";
const int n_letters = sizeof(letters);

int a_next[MAX_VERS][n_letters];
int a_mask[MAX_VERS][n_letters];
int potential[MAX_VERS];

int get_ver(const vector<int> lens, bool &added)
{
	added = false;
	if (vers.find(lens) != vers.end()) return vers[lens];
	ass(n_vers < MAX_VERS);
	vers[lens] = n_vers;
	int pot = 0;
	FA(a,lens) pot += lens[a];
	potential[n_vers] = pot;
	added = true;
	n_vers++;
	return vers[lens];
}

void automata_dfs(int prev, string s0)
{
	int n_words = powerphrases.size();
	for (uint i = 0; i < n_letters; i++)
	{
		string s = s0 + letters[i];
		int mask = 0;

		vector<int> lens(n_words);
		for (uint j = 0; j < powerphrases.size(); j++)
		{
			int plen = powerphrases[j].size();

			int maxp = 0;
			for (int le = 1; le <= (int)s.size() && le <= plen; le++)
			{
				if (powerphrases[j].substr(0, le) == s.substr(s.size()-le, le))
				{
					if (le < plen) lens[j] = le;
					maxp = le;
				}
			}
			if (maxp == plen) mask |= (1 << j);
		}
		bool added = false;
		int v = get_ver(lens, added);
		a_next[prev][i] = v;
		a_mask[prev][i] = mask;
		if (added) automata_dfs(v, s);
	}
}

void build_automata()
{
	ass( powerphrases.size() <= 31 );
	n_vers = 0;
	vers.clear();
	bool dummy;
	int v = get_ver(vector<int>(powerphrases.size()), dummy);
	automata_dfs(v, "");
}

void print_automata()
{
	printf("n_vers = %d\n", n_vers);
	for (int i = 0; i < n_vers; i++)
	{
		for (int j = 0; j < n_letters; j++)
			if (a_next[i][j] || a_mask[i][j])
				printf("%d -%c-> %d (%d)\n", i, letters[j], a_next[i][j], a_mask[i][j]);
	}
}

struct RANDOM
{
	vector< unsigned int > seed;
	int ind;
	void set_seed( unsigned int s )
	{
		seed.clear();
		seed.push_back( s );
		FOR(a,1,100000)
			seed.push_back( seed[SZ(seed)-1]*1103515245 + 12345 );
		ind = 0;
	}
	unsigned int cur( int i=0 )
	{
		return (seed[ind+i]>>16) & ((1<<15)-1);
	}
} rnd;

struct STATE
{
	vector< vector< int > > board;
	int height, width;

	PAR pivot;
	vector< PAR > unit;
	int rotate;

	int max_rotate;

	// constants for value
	int board_value;
	vector< int > filled;
	int holes;
	int score, ls_old;

	typedef  int (STATE::*Evaluator)();
	static Evaluator valf;

	void init( int w, int h )
	{
		//CLR( board );
		board = vector< vector< int > >( h, vector< int >( w, 0 ) );
		filled = vector< int >( h, 0 );
		height = h;
		width = w;
		score = 0;
		ls_old = 0;
	}

	void move_pnt( PAR & p, int dir, int d )
	{
		if (d<0)
		{
			d *= -1;
			dir += 3;
			if (dir>=6) dir -= 6;
		}
		if ((p.Y & 1)==0)
		{
			if (dir==0) { p.X -= d; }
			else if (dir==1) { p.X -= ((d+1)>>1); p.Y += d; }
			else if (dir==2) { p.X += (d>>1); p.Y += d; }
			else if (dir==3) { p.X += d; }
			else if (dir==4) { p.X += (d>>1); p.Y -= d; }
			else if (dir==5) { p.X -= ((d+1)>>1); p.Y -= d; }
		}
		else
		{
			if (dir==0) { p.X -= d; }
			else if (dir==1) { p.X -= (d>>1); p.Y += d; }
			else if (dir==2) { p.X += ((d+1)>>1); p.Y += d; }
			else if (dir==3) { p.X += d; }
			else if (dir==4) { p.X += ((d+1)>>1); p.Y -= d; }
			else if (dir==5) { p.X -= (d>>1); p.Y -= d; }
		}
	}

	// cmd:
	// 0 - left
	// 1 - left-down
	// 2 - right-down
	// 3 - right
	// 4 - rotate cw
	// 5 - rotate ccw

	bool can_move( int move )
	{
		FA(a,unit)
		{
			PAR p = unit[a];
			if (move <= 3)
			{
				move_pnt( p, move, 1 );
			}
			else
			{
				PAR p_end = pivot;
				PAR p1 = pivot;
				move_pnt( p1, 5, pivot.Y-p.Y );
				move_pnt( p_end, (move==4 ? 4 : 0), pivot.Y-p.Y );
				move_pnt( p_end, (move==4 ? 5 : 1), p1.X-p.X );
                p = p_end;
			}
			if (!(0<=p.X && p.X<width && 0<=p.Y && p.Y<height)) return false;
			if (board[p.Y][p.X]) return false;
		}
		return true;
	}

	void do_move( int move )
	{
		for( int a=0; a<=(int)unit.size(); a++)
		{
			PAR p = pivot;
			if (a<(int)unit.size()) p = unit[a];
			if (move <= 3)
			{
				move_pnt( p, move, 1 );
			}
			else
			{
				PAR p_end = pivot;
				PAR p1 = pivot;
				move_pnt( p1, 5, pivot.Y-p.Y );
				move_pnt( p_end, (move==4 ? 4 : 0), pivot.Y-p.Y );
				move_pnt( p_end, (move==4 ? 5 : 1), p1.X-p.X );
				p = p_end;
			}
			if (a<(int)unit.size()) unit[a] = p;
			else pivot = p;
		}
		if (move==4)
		{
			rotate++;
			if (rotate==max_rotate) rotate = 0;
		}
		if (move==5)
		{
			rotate--;
			if (rotate<0) rotate = max_rotate-1;
		}
	}

	void undo_move( int move )
	{
		if (move <= 3)
		{
			int m2 = move+3;
			if (m2>=6) m2-=6;
			for( int a=0; a<(int)unit.size(); a++)
				move_pnt( unit[a], m2, 1 );
			move_pnt( pivot, m2, 1 );
		}
		else do_move( move==4 ? 5 : 4 );
	}

	void do_commands( string cmds )
	{
		FA(a,cmds)
		{
			int move = move_by_chr[(int)cmds[a]];
			if (can_move(move))
				do_move(move);
		}
	}

	void recalc_consts()
	{
		board_value = 0;
		FOR(a,0,height-1) FOR(b,0,width-1)
			if (board[a][b])
			{
				board_value++;
				if (a<3) board_value-=10;
			}
		FOR(a,0,height-1)
		{
			filled[a] = 0;
			FOR(b,0,width-1)
				if (board[a][b])
					filled[a]++;
		}
		holes = 0;
		FOR(a,0,height-1)
			FOR(b,0,width-1)
				if (!board[a][b])
					FOR(c,0,5)
					{
						PAR p = make_pair( b, a );
						move_pnt( p, c, 1 );
						if (!(0<=p.X && p.X<width && 0<=p.Y && p.Y<height))
							holes++;
						else if (board[p.Y][p.X])
							holes++;
					}
	}

	void lock_unit()
	{
		FA(a,unit)
		{
			ass( !board[unit[a].Y][unit[a].X] );
			board[unit[a].Y][unit[a].X] = true;
		}
		int lines = 0;

		DFOR(a,height-1,0)
		{
			bool flag = true;
			FOR(b,0,width-1)
				if (!board[a][b])
					flag = false;
			if (flag)
			{
				DFOR(b,a-1,0)
					FOR(c,0,width-1)
						board[b+1][c] = board[b][c];
				FOR(c,0,width-1)
					board[0][c] = false;
				a++;
				lines++;
			}
		}

		int points = (int)unit.size() + 100 * (1 + lines) * lines / 2;
		if (ls_old > 1)
			points += (ls_old - 1) * points / 10;
		ls_old = lines;
		score += points;

		// recalc constants for value
		recalc_consts();
	}

	bool valid(PAR p)
	{
		return p.first >= 0 && p.second >= 0 && p.first < width && p.second < height;
	}

	int get_value()
	{
		return (this->*valf)();
	}

	int get_value1()
	{
			int re = 0;
			FOR(a,0,height-1) FOR(b,0,width-1)
				if (board[a][b])
					re += a*3+1;
			FA(a,unit) re += unit[a].Y*3+1;
			int lines = 0;

			FOR(a,0,height-1)
			{
				bool flag = true;
				FOR(b,0,width-1)
					if (!board[a][b])
					{
						bool flag2 = false;
						FA(c,unit)
							if (unit[c] == make_pair( b, a ))
								flag2 = true;
						if (!flag2) flag = false;
					}
				if (flag) lines++;
			}
			re += lines*lines*width*100;

			FA(d, unit) board[unit[d].second][unit[d].first] = true;
			/*
			for (int i = 0; i < height; i++)
			{
				cnts[i] = 0;
				for (int j = 0; j < width; j++) if (board[i][j]) cnts[i]++;
			}
			*/
			for (int i = 1; i < height; i++)
			{
				for (int j = 0; j < width; j++) if (!board[i][j])
				{
					PAR p1(j, i);
					move_pnt(p1, 4, 1);
					bool f1 = !valid(p1) || board[p1.second][p1.first];
					if (f1)
					{
						PAR p2(j, i);
						move_pnt(p2, 5, 1);
						bool f2 = !valid(p2) || board[p2.second][p2.first];
						if (f2)
							re -= 200;
					}
				}
				// 1-holes
				for (int j = 1; j + 1 < width; j++) if (!board[i][j] && board[i][j-1] && board[i][j+1]) re -= 150;
				// 2-holes
				for (int j = 1; j + 2 < width; j++) if (!board[i][j] && !board[i][j+1] && board[i][j-1] && board[i][j+2]) re -= 150;

				if (!board[i][0] && board[i][1]) re -= 160;
				if (!board[i][width-1] && board[i][width-2]) re -= 160;

				if (!board[i][0] && !board[i][1] && board[i][2]) re -= 160;
				if (!board[i][width-1] && !board[i][width-2] && board[i][width-3]) re -= 160;
			}
			FA(d, unit) board[unit[d].second][unit[d].first] = false;

			// vertical placement is bad
			int miy = o_O, may = -o_O;
			for (u32 i = 0; i < unit.size(); i++) {
				miy = min(miy, unit[i].second);
				may = max(may, unit[i].second);
			}
			re -= (may - miy) * 500;

			// "singularities"
			int sings = 0;
			for (int i = 1; i < height; i++)
			{
				for (int j = 1; j + 1 < width; j++) if (board[i][j] && !board[i][j-1] && !board[i][j+1]) sings ++;
			}
			//if (sings > 2) re -= sings * 100;

			FOR(a,0,height-1)
				FOR(b,0,width-1)
					if (board[a][b])
						FOR(c,0,5)
						{
							PAR p = make_pair( b, a );
							move_pnt( p, c, 1 );
							bool flag = true;
							FA(d,unit)
								if (unit[d] == p)
									flag = false;
							if (flag)
								if (0<=p.X && p.X<width && 0<=p.Y && p.Y<height)
									if (!board[p.Y][p.X])
										re -= 2;
						}

			FA(a,unit) FOR(b,0,5)
			{
				PAR p = unit[a];
				move_pnt( p, b, 1 );
				bool flag = true;
				FA(c,unit)
					if (unit[c] == p)
						flag = false;
				if (flag)
					if (0<=p.X && p.X<width && 0<=p.Y && p.Y<height)
						if (!board[p.Y][p.X])
							re -= 2;
			}

			return re;
	}


	int get_value0()
	{
		int re = 0;
		re += board_value*3+1;
		FA(a,unit)
		{
			re += unit[a].Y*3+1;
		}

		int lines = 0;
		FA(a,unit)
		{
			filled[unit[a].Y]++;
			if (filled[unit[a].Y]==width) lines++;
		}
		FA(a,unit) filled[unit[a].Y]--;
		re += lines*lines*width*200;

		int holes_delta = 0;
		FA(a,unit) board[unit[a].Y][unit[a].X] = 2;
		FA(a,unit) FOR(b,0,5)
		{
			PAR p = unit[a];
			move_pnt( p, b, 1 );
			if (!(0<=p.X && p.X<width && 0<=p.Y && p.Y<height))
				holes_delta--;
			else if (board[p.Y][p.X]==0)
				holes_delta++;
			else if (board[p.Y][p.X]==1)
				holes_delta--;
		}
		FA(a,unit) board[unit[a].Y][unit[a].X] = false;
		re -= (holes + holes_delta)*2;

		return re;
	}

	int calc_max_rotate()
	{
		sort( unit.begin(), unit.end() );
		vector< PAR > v = unit;
		FOR(a,0,5)
		{
			do_move( 4 );
			sort( unit.begin(), unit.end() );
			if (unit==v) return a+1;
		}
		ass( false );
		return -1;
	}

	bool spawn_unit( PAR n_pivot, vector< PAR > n_unit )
	{
		int mi = o_O, ma = -o_O;
		FA(a,n_unit) mi = min( mi, n_unit[a].Y );
		FA(a,n_unit) move_pnt( n_unit[a], 5, mi );
		move_pnt( n_pivot, 5, mi );

		mi = o_O;
		FA(a,n_unit)
		{
			mi = min( mi, n_unit[a].X );
			ma = max( ma, n_unit[a].X );
		}

		int d = (width - (ma-mi+1))/2 - mi;
		FA(a,n_unit) move_pnt( n_unit[a], 3, d );
		move_pnt( n_pivot, 3, d );

		FA(a,n_unit) if (board[n_unit[a].Y][n_unit[a].X]) return false;
		pivot = n_pivot;
		unit = n_unit;

		max_rotate = calc_max_rotate();
		rotate = 0;
		return true;
	}

	void render(LL x = -1)
	{
		cout << "raw: " << get_value() << " combined: " << x << "\n";
		FOR(a,0,height-1)
		{
			if (a&1) cout << " ";
			FOR(b,0,width-1)
			{
				bool flag = false;
				FA(c,unit) if (unit[c] == make_pair( b, a )) flag = true;
				if (pivot == make_pair( b, a )) cout << "* ";
				else if (flag) cout << "$ ";
				else if (board[a][b]) cout << "# ";
				else cout << ". ";
			}
			cout << "\n";
		}
		cout << "\n";
	}
};

STATE::Evaluator STATE::valf = &STATE::get_value0;

struct CELL
{
	int x, y;
	FLD_BEGIN FLD(x) FLD(y) FLD_END
};


struct UNIT
{
	vector< CELL > members;
	CELL pivot;
	FLD_BEGIN FLD(members) FLD(pivot) FLD_END
};

struct INPUT
{
	int id;
	vector< UNIT > units;
	int width;
	int height;
	vector< CELL > filled;
	int sourceLength;
	vector< unsigned int > sourceSeeds;

	FLD_BEGIN FLD(id) FLD(units) FLD(width) FLD(height)
		FLD(filled) FLD(sourceLength) FLD(sourceSeeds) FLD_END
};

struct OUTPUT
{
	int problemId;
	unsigned int seed;
	string tag;
	string solution;
	int my_score;

	FLD_BEGIN FLD(problemId) FLD(seed) FLD(tag) FLD(solution) FLD(my_score) FLD_END
};

INPUT inp;

void unit_to_unit( UNIT & u, PAR & pivot, vector< PAR > & unit )
{
	pivot = make_pair( u.pivot.x, u.pivot.y );
	FA(a,u.members) unit.push_back( make_pair( u.members[a].x, u.members[a].y ) );
}

#define MP make_pair

struct xpos
{
	int x, y, rot;
	xpos(int x, int y, int rot) : x(x), y(y), rot(rot) {}
	bool operator < (const xpos &other) const {
		return MP(x, MP(y, rot)) < MP(other.x, MP(other.y, other.rot));
	}
	static xpos Get(const STATE &sta)
	{
		return xpos(sta.pivot.first, sta.pivot.second, sta.rotate);
	}
};

bool operator== (const xpos & A, const xpos & B)
{
	return A.x == B.x && A.y == B.y && A.rot == B.rot;
}



vector<int> found_words;

struct DP_STATE
{
	PAR pivot;
	int rotate;
	PAR rot_range;
	int from; // 0 - ?, 1 - left, 2 - right
	int node;
	LL getcode(const STATE &st)
	{
		int res = 0;
		int t;
		res = res * 6 + rotate;
		t = rot_range.first;
		res = res * 6 + t;
		t = rot_range.second;
		res = res * 6 + t;
		res = res * 3 + from;
		res = res * n_vers + node;

		LL res2 = res;
		t = pivot.first + kPivotShift;
		ass( t >= 0 && t < kNForCode );
		res2 = res2 * kNForCode + t;
		t = pivot.second + kPivotShift;
		ass ( t >= 0 && t < kNForCode );
		res2 = res2 * kNForCode + t;
		return res2;
	}
};

// 1000 * 1000 * 6 * 36 * 3 * 50

struct DP_VALUE
{
	LL cost;
	int mx_len;
	int sum;
	int end_value;
	int potential;
	int nxt;
	DP_VALUE()
	{
		mx_len = 0;
		sum = 0;
		end_value = -o_O;
		potential = 0;
		nxt = -1;
		recalc_cost();
	}
	void recalc_cost()
	{
		cost = (LL)end_value*100000 + mx_len*1000 + sum*10 + potential;
	}
};

struct CanMove
{
	bool can[6];
};

#define HTABLE_LOG 22

int get_hash( xpos tt )
{
	return ( ((tt.x*(o_O+7) + tt.y)*(o_O+9) + tt.rot) & ((1<<HTABLE_LOG)-1) );
}

int get_hash( LL tt )
{
	return (int)((tt*(o_O+7) + tt) & ((1<<HTABLE_LOG)-1));
}

int get_hash( vector< PAR > & v )
{
	int re = 0;
	FA(a,v) re = re*(o_O+7) + v[a].first*10007 + v[a].second + 1;
	return ( re & ((1<<HTABLE_LOG)-1) );
}

template< typename KEY, typename VAL >
struct HASHTABLE
{
	vector< pair< KEY, pair< VAL, int > > > H;
	int hTable[1<<HTABLE_LOG];
	int visited[1<<HTABLE_LOG];
	int v_sz;
	int sz;

	HASHTABLE()
	{
		sz = 0;
		v_sz = 0;
		FOR(a,0,(1<<HTABLE_LOG)-1) hTable[a] = -1;
	}

	void clear()
	{
		FOR(a,0,v_sz-1) hTable[visited[a]] = -1;
		v_sz = 0;
		sz = 0;
	}

	// note: repeating the add operation may broke the pointer
	VAL * add( KEY key, VAL val = VAL() )
	{
		int ha = get_hash( key );
		if (hTable[ha]==-1) visited[v_sz++] = ha;

		pair< KEY, pair< VAL, int > > node = MP( key, MP( val, hTable[ha] ) );
		if (sz == SZ(H)) H.push_back( node );
		else H[sz] = node;
		hTable[ha] = sz;
		sz++;

		return &(H[sz-1].second.first);
	}

	// note: add operation may broke the pointer
	VAL * find( KEY key )
	{
		int ha = get_hash( key );

		for ( int ind = hTable[ha]; ind != -1; ind = H[ind].second.second )
			if ( H[ind].first == key )
				return &H[ind].second.first;

		return 0;
	}
};

HASHTABLE< xpos, CanMove > memo_canmove;
HASHTABLE< xpos, LL > vals_memo;
HASHTABLE< LL, DP_VALUE > mmemo;
HASHTABLE< xpos, int > memo_turn2;
HASHTABLE< xpos, int > memo_turn3;
HASHTABLE< vector< PAR >, LL > memo_end_turn1;
HASHTABLE< vector< PAR >, LL > memo_end_turn2;

bool cut_move3;

inline DP_STATE do_dp_move( STATE & sta, DP_STATE dps, int move )
{
	DP_STATE dps2 = dps;
	// sta is not valid here (intial position!)
	if (move==4)
	{
		dps2.rotate++;
		if (dps2.rotate==sta.max_rotate) dps2.rotate = 0;
	}
	if (move==5)
	{
		dps2.rotate--;
		if (dps2.rotate<0) dps2.rotate = sta.max_rotate-1;
	}
	if (move <= 3)
		sta.move_pnt( dps2.pivot, move, 1 );

	if (move==0) dps2.from = 2;
	if (move==1 || move==2)
	{
		dps2.from = 0;
		dps2.rot_range = make_pair( dps2.rotate, dps2.rotate );
	}
	if (move==3) dps2.from = 1;
	if (move==4)
	{
		dps2.rot_range.second = dps2.rotate;
		dps2.from = 0;
	}
	if (move==5)
	{
		dps2.rot_range.first = dps2.rotate;
		dps2.from = 0;
	}

	return dps2;
}

LL get_next2_move_value( STATE & sta )
{
	xpos xp = xpos::Get(sta);
	memo_turn3.add( xp );

	bool flag = false;
	LL re = -o_O;
	FOR(a,0,5)
	{
		if (sta.can_move( a ))
		{
			int tmp = sta.rotate;
			sta.do_move( a );
			if (!memo_turn3.find( xpos::Get(sta) ))
				re = max( re, get_next2_move_value( sta ) );
			sta.undo_move( a );
			ass( tmp == sta.rotate );
		}
		else flag = true;
	}
	if (flag)
		re = max( re, (LL)sta.get_value() );

	return re;
}

vector< LL > move2_values;

void estimate_next_move_value( STATE & sta )
{
	xpos xp = xpos::Get(sta);
	memo_turn2.add( xp );

	bool flag = false;
	FOR(a,0,5)
	{
		if (sta.can_move( a ))
		{
			int tmp = sta.rotate;
			sta.do_move( a );
			if (!memo_turn2.find( xpos::Get(sta) ))
				estimate_next_move_value( sta );
			sta.undo_move( a );
			ass( tmp == sta.rotate );
		}
		else flag = true;
	}
	if (flag)
	{
		if (sta.width * sta.height <= 160)
		{
			vector< PAR > uni = sta.unit;
			sort( uni.begin(), uni.end() );
			LL * vava = memo_end_turn2.find( uni );
			if (!vava)
			{
				LL value = sta.get_value();
				move2_values.push_back( value );
				memo_end_turn2.add( uni, value );
			}
		}
	}
}

LL get_next_move_value( STATE & sta, LL score )
{
	xpos xp = xpos::Get(sta);
	memo_turn2.add( xp );

	bool flag = false;
	LL re = -o_O;
	FOR(a,0,5)
	{
		if (sta.can_move( a ))
		{
			int tmp = sta.rotate;
			sta.do_move( a );
			if (!memo_turn2.find( xpos::Get(sta) ))
				re = max( re, get_next_move_value( sta, score ) );
			sta.undo_move( a );
			ass( tmp == sta.rotate );
		}
		else flag = true;
	}
	if (flag)
	{
		if (sta.width * sta.height <= 160 && !cut_move3)
		{
			vector< PAR > uni = sta.unit;
			sort( uni.begin(), uni.end() );
			LL * vava = memo_end_turn2.find( uni );
			if (!vava)
			{
				LL value = sta.get_value() + 40;
				if (value > score + 40)
				{
					STATE S2 = sta;
					S2.lock_unit();
					int ind = (rnd.cur(2)) % SZ(inp.units);
					PAR pivot;
					vector< PAR > unit;
					unit_to_unit( inp.units[ind], pivot, unit );

					if (S2.spawn_unit( pivot, unit ))
					{
						memo_turn3.clear();
						value = max( value, get_next2_move_value( S2 ) );
					}
					else value = -o_O;

					re = max( re, value );
				}
				memo_end_turn2.add( uni, value );
			}
			else re = max( re, *vava );
		}
		else
		{
			re = max( re, (LL)sta.get_value() );
		}
	}

	return re;
}

void dfs_canmove( STATE & sta )
{
	xpos xp = xpos::Get(sta);
	memo_canmove.add( xp ); // mark as visited
	CanMove res;

	FOR(a,0,5)
	{
		res.can[a] = sta.can_move(a);
		if (res.can[a])
		{
			int tmp = sta.rotate;
			sta.do_move( a );
			if (!memo_canmove.find( xpos::Get(sta) ))
				dfs_canmove( sta );
			sta.undo_move( a );
			ass( tmp == sta.rotate );
		}
	}
	FOR(a,0,5)
		if (!res.can[a])
		{
			if (sta.width * sta.height <= 250)
			{
				vector< PAR > uni = sta.unit;
				sort( uni.begin(), uni.end() );
				LL * vava = memo_end_turn1.find( uni );
				if (!vava)
				{
					STATE S2 = sta;
					S2.lock_unit();
					int ind = (rnd.cur(1)) % SZ(inp.units);
					PAR pivot;
					vector< PAR > unit;
					unit_to_unit( inp.units[ind], pivot, unit );
					LL value = sta.get_value() + 80;
					if (S2.spawn_unit( pivot, unit ))
					{
						memo_turn2.clear();
						memo_end_turn2.clear();
						move2_values.clear();
						estimate_next_move_value( S2 );
						sort( move2_values.begin(), move2_values.end() );
						memo_turn2.clear();
						memo_end_turn2.clear();
						LL limit = -o_O;
						if ( SZ(move2_values)>0 )
							limit = move2_values[ max(0,SZ(move2_values)-10) ];
						value = max( value, get_next_move_value( S2, limit ) );
					}
					else value = -o_O;
					vals_memo.add( xp, value );
					memo_end_turn1.add( uni, value );
				}
				else vals_memo.add( xp, *vava );
			}
			else vals_memo.add( xp, sta.get_value() );
			break;
		}
	*memo_canmove.find(xp) = res;
}

DP_VALUE get_dp( STATE & sta, DP_STATE dps )
{
	LL dpscode = dps.getcode(sta);
	DP_VALUE * it = mmemo.find( dpscode );
	if (it) return *it;

	DP_VALUE v;
	int nxt = -1;

	xpos xp(dps.pivot.first, dps.pivot.second, dps.rotate);
	CanMove can = *memo_canmove.find( xp );

	FOR(move,0,5)
	{
		bool flag = true;
		if (move==0 && dps.from==1) flag = false;
		if (move==3 && dps.from==2) flag = false;
		if (move==4)
		{
			int r = dps.rotate+1;
			if (r==sta.max_rotate) r=0;
			if (r==dps.rot_range.first || r==dps.rot_range.second)
				flag = false;
			if (dps.rotate==dps.rot_range.first && dps.rot_range.first!=dps.rot_range.second)
				flag = false;
			if (sta.max_rotate==1)
				flag = false;
		}
		if (move==5)
		{
			int r = dps.rotate-1;
			if (r<0) r=sta.max_rotate-1;
			if (r==dps.rot_range.first || r==dps.rot_range.second)
				flag = false;
			if (dps.rotate==dps.rot_range.second && dps.rot_range.first!=dps.rot_range.second)
				flag = false;
			if (sta.max_rotate==1)
				flag = false;
		}

		if (flag && can.can[ move ])
		{
			DP_STATE dps2 = do_dp_move( sta, dps, move );

			FOR(c,0,5)
			{
				dps2.node = a_next[dps.node][move*6+c];

				DP_VALUE tmp = get_dp( sta, dps2 );

				int msk = a_mask[dps.node][move*6+c];
				FA(d,powerphrases) if ((msk>>d)&1)
				{
#ifdef CHECK_NEW_WORD
					if (found_words[d] == 0)
#endif
						tmp.mx_len = max( tmp.mx_len, SZ(powerphrases[d]) );
					tmp.sum += SZ(powerphrases[d]);
				}
				tmp.nxt = move*6+c;
				tmp.recalc_cost();
				if (v.cost < tmp.cost)
					v = tmp;
			}
		}
	}

	LL xval = 0;
	FOR(move,0,5)
		if (!can.can[ move ])
		{
			xval = *vals_memo.find( xp );
			break;
		}

	FOR(move,0,5)
		if (!can.can[ move ])
			FOR(c,0,5)
			{
				DP_VALUE tmp;
				tmp.potential = potential[a_next[dps.node][move*6+c]];
				int msk = a_mask[dps.node][move*6+c];
				FA(d,powerphrases) if ((msk>>d)&1)
				{
#ifdef CHECK_NEW_WORD
					if (found_words[d] == 0)
#endif
						tmp.mx_len = max( tmp.mx_len, SZ(powerphrases[d]) );
					tmp.sum += SZ(powerphrases[d]);
				}
				tmp.end_value = (int)xval;
				tmp.nxt = move*6+c;
				tmp.recalc_cost();
				if (v.cost < tmp.cost)
					v = tmp;
			}
	mmemo.add( dpscode, v );
	return v;
}

void calc_crazy_dp( STATE & sta, int starting_node )
{
	mmemo.clear();
	vals_memo.clear();
	memo_canmove.clear();
	memo_end_turn1.clear();
	dfs_canmove(sta);

	DP_STATE dps;
	dps.pivot = sta.pivot;
	dps.rotate = sta.rotate;
	dps.rot_range = make_pair( sta.rotate, sta.rotate );
	dps.from = 0;
	dps.node = starting_node;

	get_dp( sta, dps );
}

set< pair< PAR, int > > Set;
vector< pair< PAR, int > > end_pos;
vector< int > values;
vector< string > cmds;
string cmd = "palbdk";
string seq;

void dfs( STATE & sta )
{
	Set.insert( make_pair( sta.pivot, sta.rotate ) );
	int end_cmd = -1;
	FOR(a,0,5)
		if (sta.can_move(a))
		{
			int tmp = sta.rotate;
			sta.do_move( a );
			seq.push_back( cmd[a] );
			if (Set.find( make_pair( sta.pivot, sta.rotate ) ) == Set.end())
				dfs( sta );
			seq.resize( SZ(seq)-1 );
			sta.undo_move( a );
			ass( tmp == sta.rotate );
		}
		else end_cmd = a;
	if (end_cmd>=0)
	{
		end_pos.push_back( make_pair( sta.pivot, sta.rotate ) );
		values.push_back( sta.get_value() );
		seq.push_back( cmd[end_cmd] );
		cmds.push_back( seq );
		seq.resize( SZ(seq)-1 );
	}
}

void calc_all_end_positions( STATE & sta )
{
	Set.clear();
	seq.clear();
	end_pos.clear();
	values.clear();
	cmds.clear();
	dfs( sta );
}

OUTPUT solve_with_evaluator( STATE::Evaluator ev, int z )
{
	STATE::valf = ev;
	STATE S;
	S.init( inp.width, inp.height );
	FA(a,inp.filled) S.board[inp.filled[a].y][inp.filled[a].x] = true;
	S.recalc_consts();

	rnd.set_seed( inp.sourceSeeds[z] );

	OUTPUT out;
	out.problemId = inp.id;
	out.seed = inp.sourceSeeds[z];
	out.tag = guten_tag;
	out.solution = "";

	found_words = vector<int>(powerphrases.size());

	int prev_node = 0;

	FOR(a,0,inp.sourceLength-1)
	{
		int ind = (rnd.cur()) % SZ(inp.units);
		PAR pivot;
		vector< PAR > unit;
		unit_to_unit( inp.units[ind], pivot, unit );
		if (!S.spawn_unit( pivot, unit )) break;
#ifdef RENDERX
		cout << a << " ================\n"; // renderx
#endif
		calc_crazy_dp( S, prev_node );
		DP_STATE dps;
		dps.pivot = S.pivot;
		dps.rotate = S.rotate;
		dps.rot_range = make_pair( S.rotate, S.rotate );
		dps.from = 0;
		dps.node = prev_node;

		while(1)
		{
			int nxt = mmemo.find(dps.getcode(S))->nxt;
			out.solution.push_back( letters[nxt] );

			// checking powerwords
			for (uint j = 0; j < powerphrases.size(); j++)
			{
				u32 plen = powerphrases[j].size();
				if (out.solution.size() < plen) continue;
				if (powerphrases[j] == out.solution.substr(out.solution.size()-plen, plen)) found_words[j]++;
			}

			int move = nxt/6;
			if (S.can_move( move ))
				S.do_move( move );
			else
			{
				prev_node = a_next[dps.node][nxt];
				break;
			}
			DP_STATE dps2 = do_dp_move( S, dps, nxt/6 );
			dps2.node = a_next[dps.node][nxt];
			dps = dps2;
		}

		S.lock_unit();
		fprintf(stderr, "prob=%d, idx=%d, seed=%d, score=%d, %d states\n", inp.id, z, out.seed, a, (int)mmemo.sz);
		rnd.ind++;
	}

	int score = S.score;

	for (u32 i = 0; i < powerphrases.size(); i++) {
		int k = found_words[i];
		if (k) score += 300 + 2 * k * powerphrases[i].size();
	}
	out.my_score = score;
	return out;
}

void update_sol(STATE::Evaluator valf, int z, int &best_score, OUTPUT &best)
{
	try
	{
		OUTPUT cur = solve_with_evaluator(valf, z);

		fprintf(stderr, "Power:");
		for (u32 i = 0; i < powerphrases.size(); i++) fprintf(stderr, " %d", found_words[i]);
		fprintf(stderr, "\n");
		fprintf(stderr, "Score: %d\n", cur.my_score);
		fprintf(stderr, "\n");

		if (cur.my_score > best_score)
		{
			best_score = cur.my_score;
			best = cur;
		}
	}
	catch (...)
	{
		fprintf(stderr, "Crash!\n");
		return_code++;
		if (!quiet) abort();
	}
}

vector<OUTPUT> sol_internal( const char *path )
{

	FILE * file_in = fopen( path, "r" );
	ass( file_in );
	char buf[100];
	string str;
	while ( fgets( buf, 96, file_in ) ) str += string( buf );
	fclose( file_in );

	Json::Reader rdr;
	Json::Value data;
	if (!rdr.parse(str, data, false))
		ass( false );
	if (!deserializeJson( inp, data ))
		ass( false );


	vector< OUTPUT > answer;
	int area = inp.height * inp.width;

	FA(z,inp.sourceSeeds) if (input_seed == -1 || input_seed == z)
	{
		int best_score = -1;
		OUTPUT best;
		if (area <= 250)
		{
			cut_move3 = true;
			update_sol(&STATE::get_value1, z, best_score, best);
		}

		cut_move3 = false;
		update_sol(&STATE::get_value0, z, best_score, best);

		answer.push_back( best );
	}

	return answer;
}

void sol (int problem)
{
	cerr << "problem " << problem << "\n";
	char path[1000];
	sprintf( path, "../data/problems/problem_%d.json", problem );
	vector<OUTPUT> answer = sol_internal(path);
	Json::FastWriter fw;
	Json::Value data;
	data = serializeJson( answer );
	string res = fw.write( data );
	sprintf( path, "../data/solutions/solution_%d_%s.json", problem, guten_tag.c_str() );
	FILE * file_out = fopen( path, "w" );
	fprintf( file_out, "%s", res.c_str() );
	fclose( file_out );
}

int main(int argc, char** argv)
{
	System::ParseArgs(argc, argv);
	powerphrases = System::GetArgValues("p");
	vector<string> files = System::GetArgValues("f");
	if (System::HasArg("seed"))
	{
		input_seed = atoi(System::GetArgValue("seed").c_str());
	}
	if (System::HasArg("p1"))
	{
		param1 = atoi(System::GetArgValue("p1").c_str());
	}
	if (System::HasArg("p2"))
	{
		param2 = atoi(System::GetArgValue("p2").c_str());
	}
	if (System::HasArg("id"))
	{
		id_num = atoi(System::GetArgValue("id").c_str());
	}

	FOR(a,0,35) move_by_chr[(int)letters[a]] = a/6;

	if (files.empty())
	{
		powerphrases.push_back("ei!");
		powerphrases.push_back("ia! ia!");
		powerphrases.push_back("r'lyeh");
		powerphrases.push_back("yuggoth");
		powerphrases.push_back("planet 10");
		powerphrases.push_back("monkeyboy");
		powerphrases.push_back("yoyodyne");
		powerphrases.push_back("john bigboote");
		powerphrases.push_back("necronomicon");
		powerphrases.push_back("in his house at r'lyeh dead cthulhu waits dreaming.");

		build_automata();

		freopen("input.txt","r",stdin);
		freopen("output.txt","w",stdout);

		sol( id_num );

		cerr << clock() << "\n";
	} else {

		build_automata();
		quiet = true;
		vector<OUTPUT> res, tmp;

		for (int i = 0; i < (int)files.size(); i++)
		{
			tmp = sol_internal(files[i].c_str());
			res.insert(res.end(), tmp.begin(), tmp.end());
		}
		Json::FastWriter fw;
		Json::Value data;
		data = serializeJson( res );
		string sres = fw.write( data );
		printf("%s", sres.c_str());

	}

	return return_code;
}
