/* 046267 Computer Architecture - Winter 20/21 - HW #1                  */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include "math.h"
#include <stdio.h> 
using namespace std;
class BTB;
BTB* BP_Table;
int calls = 0;
int flushes = 0;


int bitExtracted(int number, int k, int p)
{
	return (((1 << k) - 1) & (number >> (p - 1)));
}

class BTBrow {
public:
	BTBrow() :validbit(0) {};

	unsigned int validbit;
	unsigned int tag;
	unsigned int target;
	unsigned int history;
};

class BTB {
public:
	BTB(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
		bool isGlobalHist, bool isGlobalTable, int Shared) :
		btbSize_(btbSize), historySize_(historySize), tagSize_(tagSize),
		fsmState_(fsmState), isGlobalHist_(isGlobalHist), isGlobalTable_(isGlobalTable), Shared_(Shared)
	{
		BTBrow* BTBTable = new BTBrow[btbSize_];
		for (int k = 0; k < btbSize_; k++)
		{

		}
		
		int** fsm = new int* [btbSize_];
		for (int j = 0; j < btbSize_; j++)
		{
			fsm[j] = new int[(int)pow(2, historySize_)];
			for (int i = 0; i < pow(2, btbSize_); i++)
			{
				fsm[j][i] = fsmState_;
			}
		}


	}; // BTB c'tor

	~BTB()
	{
		delete[] BTBTable;
		delete[] fsm;
	} // d'tor

	void updatefsm(int mappingplace, bool taken)
	{
		
		int state = (*BP_Table).fsm[mappingplace][(*BP_Table).BTBTable[mappingplace].history];
		if (state == 3 && taken) return;
		else if (state == 0 && !taken) return;
		else
		{
			if (taken) state++;
			else state--;
		}
	}

	unsigned btbSize_;
	unsigned historySize_;
	unsigned tagSize_;
	unsigned fsmState_;
	bool isGlobalHist_;
	bool isGlobalTable_;
	int Shared_;
	BTBrow* BTBTable;
	int** fsm;
};



int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
	bool isGlobalHist, bool isGlobalTable, int Shared)
{
	try
	{
		// Check
		BP_Table= new BTB(btbSize, historySize, tagSize, fsmState, isGlobalHist, isGlobalTable, Shared);
	}
	catch (...)
	{
		return -1;
	}

	return 0;
}

bool BP_predict(uint32_t pc, uint32_t* dst)
{
	int mappingplace = bitExtracted(pc, (int)log2((*BP_Table).btbSize_), 3);
	if (!(*BP_Table).BTBTable[mappingplace].validbit)
	{
		*dst = pc + 4; //check
		return false;
	}

	int offset = (*BP_Table).BTBTable[mappingplace].history;
	bool prediction = ((*BP_Table).fsm[mappingplace] + offset);

	if (prediction)
	{
		*dst = (*BP_Table).BTBTable[mappingplace].target;
		return true;
	}
	return false;
	
}


void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst)
{
	calls++;
	int size = (int)log2((*BP_Table).btbSize_);
	int mappingplace = bitExtracted(pc, size, 3);
	if (!(*BP_Table).BTBTable[mappingplace].validbit)
	{
		(*BP_Table).BTBTable[mappingplace].validbit = 1;
		if (taken) (*BP_Table).BTBTable[mappingplace].history++;
		else (*BP_Table).BTBTable[mappingplace].history = 0;
		(*BP_Table).BTBTable[mappingplace].tag = bitExtracted(pc, (*BP_Table).tagSize_, 3 + size);
		(*BP_Table).BTBTable[mappingplace].target = targetPc;
	}
	else
	{
		if ((*BP_Table).BTBTable[mappingplace].tag == bitExtracted(pc, (*BP_Table).tagSize_, 3 + size))
		{
			if (BP_predict(pc, &pred_dst) == taken)
			{
				flushes++;
			}
			(*BP_Table).updatefsm(mappingplace, taken);
			if (taken)
			{
				(*BP_Table).BTBTable[mappingplace].history = (*BP_Table).BTBTable[mappingplace].history * 2 + 1;
			}
			else
			{
				(*BP_Table).BTBTable[mappingplace].history = (*BP_Table).BTBTable[mappingplace].history * 2;
			}
			(*BP_Table).BTBTable[mappingplace].history = (*BP_Table).BTBTable[mappingplace].history % (int)(pow(2, (*BP_Table).historySize_));
		}
		else
		{
			(*BP_Table).BTBTable[mappingplace].history = taken ? 1 : 0;
			for (int i = 0; i < (int)pow(2, (*BP_Table).historySize_); i++)
			{
				(*BP_Table).fsm[mappingplace][i] = (*BP_Table).fsmState_;
			}
			(*BP_Table).BTBTable[mappingplace].tag = bitExtracted(pc, (*BP_Table).tagSize_, 3 + size);
			(*BP_Table).BTBTable[mappingplace].target = targetPc;
		}
	}
	return;
}

void BP_GetStats(SIM_stats* curStats) 
{
	curStats->br_num = calls;
	curStats->flush_num = flushes;
	curStats->size = 0;
	delete(BP_Table);


	return;
}

/*
1. should BP needs to be global?
2. about the valid bit.
3.

*/