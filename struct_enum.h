#pragma once

#include <string>

#define NameMutKKSInPut "Global\\mutex_KKSInPut"
#define NameMutKKSOutPut "Global\\mutex_KKSOutPut"
#define NameMemoryKKSInPut "Global\\MemoryListKKSInPut"
#define NameMemoryKKSOutPut "Global\\MemoryListKKSOutPut"

#define NameStatusMemoryGate "Global\\Status_gate_map"
#define NameMutStatusMemoryGate "Global\\Status_gate_mut"

#define NameMutMemoryInfoChannels "Global\\mutex_MemoryInfoChannels"
#define NameMemoryInfoChannels "Global\\MemoryInfoChannels"

#define NameMutexConcordKKSIn "Global\\Mutex_ConcordKKSIn"
#define NameMutexConcordKKSOut "Global\\Mutex_ConcordKKSOut"
#define NameMemoryConcordKKSIn "Global\\Memory_ConcordKKSIn"
#define NameMemoryConcordKKSOut "Global\\Memory_ConcordKKSOut"

#define SizeMapStatus 14

enum class TypeData
{
	Analog = 2,
	Discrete = 1,
	Binar = 0,
	Empty = -1
};

enum class TypeValue
{
	INPUT = 1,
	OUTPUT = 0
};


struct KKSDTS
{
	int channel = -1;
	TypeData typedata;
	int indexdts = -1;
	char KKS[10];
};

struct InfoChannels
{
	int channel = -1;

	int countAin = 0;
	int countDin = 0;
	int countBin = 0;
	int countAout = 0;
	int countDout = 0;
	int countBout = 0;

	char status = 0;
};

struct KKSConcord_RAEK
{
	char KKS[10];
	TypeData type;
	int channel;
	int indexdts;
	int indexraek;	
};


struct KKS_RAEK
{
	char KKS[10];
	TypeData type;
	int index;
};

enum class CommandEmt
{
	UpdateKKSListOut = 1,
	UpdateKKSListIn = 2,
	UpdateListChannel = 4
};



std::string CreateNameMutexMemory(TypeData TD, TypeValue TV, int channel);

std::string CreateNameMemory(TypeData TD, TypeValue TV, int channel);