#include <algorithm>
#include <cstdio>

#include "maze.hpp"

Maze::Maze( size_t D )
	: m_dim( D )
{
	m_values = new int[ D * D ];

	reset();
}

void Maze::reset()
{
	size_t sz = m_dim*m_dim;
	std::fill( m_values, m_values + sz, 0 );
}

Maze::~Maze()
{
	delete [] m_values;
}

size_t Maze::getDim() const
{
	return m_dim;
}

int Maze::getValue( int x, int y ) const
{
	return m_values[ y * m_dim + x ];
}

void Maze::setValue( int x, int y, int h )
{
	m_values[ y * m_dim + x ] = h;
}

int perm[24][4] = {
	0,1,2,3,
	0,1,3,2,
	0,2,1,3,
	0,2,3,1,
	0,3,1,2,
	0,3,2,1,

	1,0,2,3,
	1,0,3,2,
	1,2,0,3,
	1,2,3,0,
	1,3,0,2,
	1,3,2,0,

	2,1,0,3,
	2,1,3,0,
	2,0,1,3,
	2,0,3,1,
	2,3,1,0,
	2,3,0,1,

	3,1,2,0,
	3,1,0,2,
	3,2,1,0,
	3,2,0,1,
	3,0,1,2,
	3,0,2,1,
};

int Maze::numNeighbors(int r, int c) {
	return getValue(r-1,c) + getValue(r+1,c) + getValue(r,c-1) + getValue(r,c+1);
}

// Print ASCII version of maze for debugging
void Maze::printMaze() {
	int i,j;
	for (i=0; i<m_dim; i++) {
		for (j=0; j<m_dim; j++) { 
			if ( getValue(i,j)==1 ) {
				printf("X");
			} else {
				printf(" ");
			}
		}
		printf("\n");
	}
}


void Maze::recDigMaze(int r, int c) {
	int* p;
	p = perm[random()%24];
	for (int i=0; i<4; i++) {
		switch (p[i]) {
		case 0:
			if ( r > 1 && getValue(r-1,c) && numNeighbors(r-1,c)==3 ) {
				setValue(r-1,c,0);
				recDigMaze(r-1,c);
			}
			break;
		case 1:
			if ( r < m_dim-2 && getValue(r+1,c) && numNeighbors(r+1,c)==3 ) {
				setValue(r+1,c,0);
				recDigMaze(r+1,c);
			}
			break;
		case 2:
			if ( c > 1 && getValue(r,c-1) && numNeighbors(r,c-1)==3 ) {
				setValue(r,c-1,0);
				recDigMaze(r,c-1);
			}
			break;
		case 3:
			if ( c < m_dim-2 && getValue(r,c+1) && numNeighbors(r,c+1)==3 ) {
				setValue(r,c+1,0);
				recDigMaze(r,c+1);
			}
			break;
		}
	} 
}

void Maze::digMaze()
{
	int i,j;
	// set all values to 1
	for (i=0;i<m_dim;i++) {
		for (j=0;j<m_dim;j++) {
			setValue(i,j,1);
		}
	}

	// pick random start location
	int s=random()%(m_dim-2)+1;
	setValue(0,s,0);
	setValue(1,s,0);
	recDigMaze(1,s);
	do {
		s=rand()%(m_dim-2)+1;
		if ( getValue(m_dim-2,s)==0 ) {
			setValue(m_dim-1,s,0);
		}
	} while (getValue(m_dim-1,s)==1);
	setValue(m_dim/2-2,m_dim/2-1,0);
        setValue(m_dim/2-2,m_dim/2,0);
        setValue(m_dim/2-2,m_dim/2+1,0);
        setValue(m_dim/2-1,m_dim/2-1,0);
        setValue(m_dim/2-1,m_dim/2,1);
        setValue(m_dim/2-1,m_dim/2+1,0);
        setValue(m_dim/2,m_dim/2-1,0);
        setValue(m_dim/2,m_dim/2,1);
        setValue(m_dim/2,m_dim/2+1,0);
        setValue(m_dim/2+1,m_dim/2-1,0);
        setValue(m_dim/2+1,m_dim/2,0);
        setValue(m_dim/2+1,m_dim/2+1,0);

}
