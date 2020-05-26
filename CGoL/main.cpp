#define OLC_PGE_APPLICATION

#include "olcPixelGameEngine.h"
#include <random>
#include <limits>

#define X_CELLS 32
#define Y_CELLS 32
#define CELLS X_CELLS * Y_CELLS
#define CELL_ARRAY_SIZE CELLS / (sizeof(int) * 8)
#define T_PERIOD 0.2

// Override base class with your custom functionality
class CGoL : public olc::PixelGameEngine
{
public:
	CGoL()
	{
		// Name you application
		sAppName = "Conway's Game of Life";

		std::random_device rd;     //Get a random seed from the OS entropy device, or whatever
		std::mt19937_64 eng(rd()); //Use the 64-bit Mersenne Twister 19937 generator
								   //and seed it with entropy.

		//Define the distribution, by default it goes from 0 to MAX(unsigned long long)
		//or what have you.
		std::uniform_int_distribution<unsigned int> distr;

		
	}

public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		Clear(olc::WHITE);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		switch (state)
		{
			case USER_SELECT_GRID:
			{
				Clear(olc::WHITE);

				if (olc::PixelGameEngine::GetMouse(0).bReleased)
				{
					toggleBit(cellMap, olc::PixelGameEngine::GetMouseX() + olc::PixelGameEngine::GetMouseY() * X_CELLS);
				}

				for (int x = 0; x < ScreenWidth(); x++)
					for (int y = 0; y < ScreenHeight(); y++)
					{
						if (testBit(cellMap, (y * X_CELLS) + x))
							Draw(x, y, olc::BLACK);
					}

				if (olc::PixelGameEngine::GetKey(olc::SPACE).bReleased)
					state = RUN;

				break;
			}

			case RUN:
			{
				lastUpdate += fElapsedTime;

				if (lastUpdate > T_PERIOD)
				{
					Clear(olc::WHITE);

					UpdateCells();

					for (int x = 0; x < ScreenWidth(); x++)
						for (int y = 0; y < ScreenHeight(); y++)
							if (testBit(cellMap, x + (y * X_CELLS)))
								Draw(x, y, olc::BLACK);

					if (olc::PixelGameEngine::GetKey(olc::SPACE).bReleased)
						state = USER_SELECT_GRID;

					lastUpdate = 0;
				}

				if (olc::PixelGameEngine::GetKey(olc::SPACE).bReleased)
					state = USER_SELECT_GRID;
			}
		}
		
		return true;
	}

private:

	unsigned int cellMap[CELL_ARRAY_SIZE];
	unsigned int newCellMap[CELL_ARRAY_SIZE];
	float lastUpdate = 0;

	typedef enum
	{
		USER_SELECT_GRID,
		RUN
	} GameState;

	GameState state = USER_SELECT_GRID;

	bool UpdateCells()
	{
		for (int i = 0; i < CELLS; i++)
		{
			int neighbourCount = 0;
			unsigned char bitsToCheck = 0xFF;
			
			//check for edge cases

			//is it on left edge
			if (i % X_CELLS == 0) //"== 0" instead of "!", purpose is clearer
			{
				bitsToCheck &= ~0x29;
			}

			//is it on right edge
			if (i % X_CELLS == X_CELLS - 1)
			{
				bitsToCheck &= ~0x94;
			}

			//is it on top edge
			if (i < X_CELLS)
			{
				bitsToCheck &= ~0x07;
			}

			//is it on bottom edge
			if (i >= CELLS - X_CELLS)
			{
				bitsToCheck &= ~0xE0;
			}

			for (char j = 0; j < 8; j++)
			{
				if (bitsToCheck & (1 << j)) //if we need to check the bit
				{
					int relativeIndex = 0;
					if ((1 << j) & 0x29)
						relativeIndex -= 1;
					if ((1 << j) & 0x94)
						relativeIndex += 1;
					if ((1 << j) & 0x07)
						relativeIndex -= X_CELLS;
					if ((1 << j) & 0xE0)
						relativeIndex += X_CELLS;
					neighbourCount += testBit(cellMap, i + relativeIndex);
				}
				
			}

			//check if cell is alive or not
			if (testBit(cellMap, i)) //alive
			{
				if (neighbourCount < 2 || neighbourCount > 3)
					clearBit(newCellMap, i);
				else
					setBit(newCellMap, i);
			}
			else //dead
			{
				if (neighbourCount == 3)
					setBit(newCellMap, i);
				else
					clearBit(newCellMap, i);
			}
		}

		std::copy(newCellMap, newCellMap + CELL_ARRAY_SIZE, cellMap);
		std::fill_n(newCellMap, CELL_ARRAY_SIZE, 0);


		return true;
	}

	bool setBit(unsigned int * arr, int k)
	{
		arr[k / 32] |= 1 << (k % 32);
		return true;
	}

	bool clearBit(unsigned int * arr, int k)
	{
		arr[k / 32] &= ~(1 << (k % 32));
		return true;
	}

	bool testBit(unsigned int * arr, int k)
	{
		if ((arr[k / 32] & (1 << (k % 32))) != 0)
			return true;
		else
			return false;
		
	}

	bool toggleBit(unsigned int * arr, int k)
	{
		if (testBit(arr, k))
			clearBit(arr, k);
		else
			setBit(arr, k);
		return true;
	}
};

int main()
{
	CGoL demo;
	if (demo.Construct(X_CELLS, Y_CELLS, 16, 16))
		demo.Start();
	return 0;
}
