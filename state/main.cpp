#pragma comment(linker,"/STACK:64000000")
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

using namespace std;

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

#define LL long long
#define VI  vector<int>
#define PAR pair<int ,int>
#define o_O 1000000000

void __never(int a){printf("\nOPS %d", a);}
#define ass(s) {if (!(s)) {__never(__LINE__);cout.flush();cerr.flush();abort();}}

#define NN 100

struct STATE
{
	bool board[NN][NN];
	int height, width;

	PAR pivot;
	vector< PAR > unit;

	void init( int w, int h )
	{
		CLR( board );
		height = h;
		width = w;
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
				move_pnt( p, 1, move );
			}
			else
			{
				PAR p_end = pivot;
				PAR p1 = pivot;
				move_pnt( p1, 5, pivot.Y-p.Y );
				move_pnt( p_end, (move==4 ? 4 : 0), pivot.Y-p.Y );
				move_pnt( p_end, (move==4 ? 5 : 1), p1.X-p.X );
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
				//cerr << pivot.Y-p.Y << " " << p1.X-p.X << "\n";
				move_pnt( p_end, (move==4 ? 4 : 0), pivot.Y-p.Y );
				move_pnt( p_end, (move==4 ? 5 : 1), p1.X-p.X );
				p = p_end;
			}
			if (a<(int)unit.size()) unit[a] = p;
			else pivot = p;
		}
	}

	void lock_unit()
	{
		FA(a,unit)
		{
			ass( !board[unit[a].Y][unit[a].X] );
			board[unit[a].Y][unit[a].X] = true;
		}
	}

	bool spawn_unit( PAR n_pivot, vector< PAR > n_unit )
	{
		int mi = o_O, ma = -o_O;
		FA(a,n_unit) mi = min( mi, n_unit[a].Y - n_pivot.Y );
		FA(a,n_unit) move_pnt( n_unit[a], 5, mi );
		move_pnt( n_pivot, 5, mi );
		mi = o_O;
		FA(a,n_unit)
		{
			mi = min( mi, n_unit[a].X );
			ma = max( ma, n_unit[a].X );
		}
		//cerr << mi << " " << ma << "\n";
		int d = (width - (ma-mi+1))/2 - mi;
		FA(a,n_unit) move_pnt( n_unit[a], 3, d );
		move_pnt( n_pivot, 3, d );

		FA(a,n_unit) if (board[n_unit[a].Y][n_unit[a].X]) return false;

		pivot = n_pivot;
		unit = n_unit;
		return true;
	}

	void render()
	{
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
} S;

void sol()
{
	S.init( 10, 10 );
	S.board[9][1] = true;
	S.board[8][6] = true;
	S.board[9][6] = true;

	PAR pi = make_pair( 0, 0 );
	vector< PAR > un;
	un.push_back( make_pair( 0, 1 ) );
	un.push_back( make_pair( 1, 1 ) );
	un.push_back( make_pair( 2, 2 ) );
	un.push_back( make_pair( 3, 2 ) );
	S.spawn_unit( pi, un );

	S.render();

	FOR(a,0,5)
	{
		S.do_move( a );
		S.render();
	}

	FOR(a,0,5)
	{
		S.do_move( 4 );
		S.render();
	}

	FOR(a,0,5)
	{
		S.do_move( 5 );
		S.render();
	}
}

int main()
{
	freopen("input.txt","r",stdin);
	freopen("output.txt","w",stdout);

	sol();
	return 0;
}