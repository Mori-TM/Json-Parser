#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
namespace ms
{
#include "DynamicArray.h"
};

#define MALLOC_SUCKS_USUAL_BLOCK_SIZE 1 * 1024 * 1024

ms::DynamicArray s_Tracker;

typedef struct
{
	size_t Size;
	void* Address;

	void* UnderlyingBlockPTR;
	size_t UnderlyingBlockIndex;
} s_TrackerInfo;

ms::DynamicArray s_MemBlocks;

typedef struct
{
	uint8_t InUse;

	size_t Size;
	size_t PotentialSize;

	union
	{
		void* Data;
		intptr_t DataAddress;
		char* ByteAddress;
	};
} s_AllocatedElementInfo;

typedef struct
{
	size_t Size;
	size_t AllocatedSize;
	union
	{
		void* Data;
		intptr_t DataAddress;
		char* ByteAddress;
	};

	ms::DynamicArray AllocatedElements;
} s_MemBlockInfo;

//FIX - Function spelling, spelling in generall
size_t s_AddBlock(size_t Size)
{
	char Name[32];
	snprintf(Name, 32, "Malloc Sucks, Block: %zu", s_MemBlocks.Size);
	Name[31] = '\0';

	s_MemBlockInfo DefBlock;
	DefBlock.AllocatedSize = Size;
	DefBlock.Size = 0;
	DefBlock.Data = malloc(Size); //FIX - Check for Success
	if (!DefBlock.Data)
	{
		printf("[Malloc Sucks]: Failed to alloacte block: %zu, size: %zu!\n", s_MemBlocks.Size, Size);
		exit(3666);
	}
	DefBlock.AllocatedElements = ms::DynamicArrayCreate(sizeof(s_AllocatedElementInfo), Name);
	ms::DynamicArrayPush(&s_MemBlocks, &DefBlock);

	return s_MemBlocks.Size - 1;
}

void s_init()
{
	//	s_Tracker = ms::DynamicArrayCreate(sizeof(s_TrackerInfo), "Malloc Sucks, Tracker");
	s_MemBlocks = ms::DynamicArrayCreate(sizeof(s_MemBlockInfo), "Malloc Sucks, Mem Blocks");

	s_AddBlock(MALLOC_SUCKS_USUAL_BLOCK_SIZE);
}

void s_destroy()
{
	for (size_t i = 0; i < s_MemBlocks.Size; i++)
	{
		s_MemBlockInfo* Block = (s_MemBlockInfo*)ms::DynamicArrayGetAt(&s_MemBlocks, i);
		printf("\tAllocated Block size: %zu\n", Block->AllocatedSize);

		free(Block->Data);
		ms::DynamicArrayDestroy(&Block->AllocatedElements);
	}

	ms::DynamicArrayDestroy(&s_MemBlocks);
	//	ms::DynamicArrayDestroy(&s_Tracker);
}

void s_checkForLeaks()
{
	printf("[Malloc Sucks]: Congrats! You made it this far!\n");
	printf("[Malloc Sucks]: Here is your Application Status:\n");
	printf("\tUsed Blocks: %zu\n", s_MemBlocks.Size);
	size_t Errors = 0;
	size_t Allocations = 0;

	for (size_t i = 0; i < s_MemBlocks.Size; i++)
	{
		s_MemBlockInfo* Block = (s_MemBlockInfo*)ms::DynamicArrayGetAt(&s_MemBlocks, i);

		for (size_t j = 0; j < Block->AllocatedElements.Size; j++)
		{
			s_AllocatedElementInfo* Element = (s_AllocatedElementInfo*)ms::DynamicArrayGetAt(&Block->AllocatedElements, j);

			if (Element->InUse == 1)
			{
				printf("\tBlock: %zu: You failed to free: %p, size: %zu\n", i, Element->Data, Element->Size);
			//	Element->ByteAddress[Element->Size - 1] = 0;
			//	printf("\t\t%s\n\n", Element->ByteAddress);
				Errors++;
			}

			Allocations++;
		}

		//	printf("[Malloc Sucks]: Failed to free: %p, Free2: %td\n", Block->Data, Block->DataAddress);
	}

	if (Errors)
		printf("[Malloc Sucks]: Result: Your Coding Sucks, %zu/%zu missed frees, thats impressive (in a bad way)!\n", Errors, Allocations);
	else
		printf("[Malloc Sucks]: Result: 0 memory leaks, wow. Be proud of yourself!\n");

	printf("[Malloc Sucks]: Checked for Leaks!\n");
}

void* s_FindRecycableElement(size_t Size, size_t i, s_MemBlockInfo* Block)
{
	if (Size > Block->AllocatedSize)
	{
		//	printf("This can happen only once\n");
		return NULL;
	}


	if (Block->AllocatedElements.Size == 0)
	{
		s_AllocatedElementInfo Element;
		Element.InUse = 1;
		Element.PotentialSize = Size;
		Element.Size = Size;
		Element.Data = Block->Data;
		ms::DynamicArrayPush(&Block->AllocatedElements, &Element);
		//	printf("[Malloc Sucks]: Init the first element in block: %zu!\n", i);
		return Element.Data;
	}

	for (size_t j = 0; j < Block->AllocatedElements.Size; j++)
	{
		s_AllocatedElementInfo* Element = (s_AllocatedElementInfo*)ms::DynamicArrayGetAt(&Block->AllocatedElements, j);

		if (Element->InUse == 0 && Element->PotentialSize >= Size)
		{
			Element->Size = Size;
			Element->InUse = 1;

			//	printf("[Malloc Sucks]: Found one to recycle in block: %zu!\n", i);
			return Element->Data;
		}
	}

	return NULL;
}

void* s_AddNewElement(size_t Size, size_t i, s_MemBlockInfo* Block)
{
	if (Block->AllocatedElements.Size == 0 || Size > Block->AllocatedSize)
		return NULL;

	//FIX - does last elemnt always has the biggest memory address???
	s_AllocatedElementInfo* LastElement = (s_AllocatedElementInfo*)ms::DynamicArrayGetAt(&Block->AllocatedElements, Block->AllocatedElements.Size - 1);
	if (LastElement == NULL)
		return NULL;

	if (LastElement->InUse == 0 && LastElement->ByteAddress + Size < Block->ByteAddress + Block->AllocatedSize)
	{
		LastElement->InUse = 1;
		LastElement->Size = Size;
		LastElement->PotentialSize = Size;

		return LastElement->Data;
	}

	//	size_t TotalSize = 0;
	//
	//	for (size_t j = 0; j < Block->AllocatedElements.Size; j++)
	//	{
	//		s_AllocatedElementInfo* Element = (s_AllocatedElementInfo*)ms::DynamicArrayGetAt(&Block->AllocatedElements, j);
	//		TotalSize += Element->PotentialSize;
	//	}

	char* UseableAddress = LastElement->ByteAddress + LastElement->PotentialSize;
	//
	if (UseableAddress + Size < Block->ByteAddress + Block->AllocatedSize)
	{
		//Check if UseableAddress isnt overfloaw for block

		s_AllocatedElementInfo Element;
		Element.InUse = 1;
		Element.PotentialSize = Size;
		Element.Size = Size;
		//	Element.DataAddress = UseableAddress;
		Element.ByteAddress = LastElement->ByteAddress + LastElement->PotentialSize;

		void* Add = Element.ByteAddress;
		Element.Data = Add;

		ms::DynamicArrayPush(&Block->AllocatedElements, &Element);
	//	printf("[Malloc Sucks]: Added new element: %zu, in block: %zu!, Address: %p\n", Block->AllocatedElements.Size - 1, i, Add);

		return Element.Data;
	}

	return NULL;
}

void* s_malloc(size_t Size)
{
	if (Size == 0)
		return NULL;

	//Case 1. Find existing slot that fits
	for (size_t i = 0; i < s_MemBlocks.Size; i++)
	{
		s_MemBlockInfo* Block = (s_MemBlockInfo*)ms::DynamicArrayGetAt(&s_MemBlocks, i);

		void* Data = s_FindRecycableElement(Size, i, Block);
		if (Data != NULL)
			return Data;
	}

	//Case 2. Add new slot in current block that fits
	for (size_t i = 0; i < s_MemBlocks.Size; i++)
	{
		s_MemBlockInfo* Block = (s_MemBlockInfo*)ms::DynamicArrayGetAt(&s_MemBlocks, i);

		void* Data = s_AddNewElement(Size, i, Block);
		if (Data != NULL)
			return Data;

	}

	//	printf("[Malloc Sucks]: Damn it\n");

	//Case 3. Add new block, because previous ran out of space, also check if usual size of block is enough for allocation request
	size_t BlockIndex = s_AddBlock(MALLOC_SUCKS_USUAL_BLOCK_SIZE < Size ? Size : MALLOC_SUCKS_USUAL_BLOCK_SIZE);
	s_MemBlockInfo* Block = (s_MemBlockInfo*)ms::DynamicArrayGetAt(&s_MemBlocks, BlockIndex);

	//	printf("[Malloc Sucks]: Here we go: %zu\n", BlockIndex);
	//I guess this should be enough
	void* Data = s_FindRecycableElement(Size, BlockIndex, Block);
	if (Data != NULL)
		return Data;

	//How?
	printf("[Malloc Sucks]: Daa fuck did this happen?\n");
	Data = s_AddNewElement(Size, BlockIndex, Block);
	if (Data != NULL)
		return Data;

	printf("[Malloc Sucks]: How is that even possible?\n");


	return NULL;
}

//FIX - maybe for (size_t i = 0; i < s_MemBlocks.Size; i++) should also be bakcwards ????
void s_free(void* Data)
{
	for (long long i = s_MemBlocks.Size - 1; i >= 0; i--)
	{
		s_MemBlockInfo* Block = (s_MemBlockInfo*)ms::DynamicArrayGetAt(&s_MemBlocks, i);
		//	printf("\tBlockItteration\n");
			//Trust, this makes sense
		if (Block->AllocatedElements.Size > 0)
		{
			for (long long j = Block->AllocatedElements.Size - 1; j >= 0; j--)
			{
				//	printf("\tItteration: %zu\n", j);

				s_AllocatedElementInfo* Element = (s_AllocatedElementInfo*)ms::DynamicArrayGetAt(&Block->AllocatedElements, j);

				if (Element->Data == Data)
				{
					//	printf("[Malloc Sucks]: Found element: %zu, in block: %zu!\n", j, i);

					Element->InUse = 0;
					Element->Size = 0;

					if (Block->AllocatedElements.Size - 1 == j)
					{
						ms::DynamicArrayPop(&Block->AllocatedElements, j);
						//	printf("[Malloc Sucks]: Poped element: %zu, in block: %zu!\n", j, i);
					}

					if (i > 0)
					{
						uint8_t UsedElements = 0;

						for (size_t j = 0; j < Block->AllocatedElements.Size; j++)
						{
							s_AllocatedElementInfo* Element = (s_AllocatedElementInfo*)ms::DynamicArrayGetAt(&Block->AllocatedElements, j);
							if (Element->InUse)
							{
								UsedElements = 1;
								break;
							}
						}

						if (!UsedElements)
						{
							ms::DynamicArrayDestroy(&Block->AllocatedElements);
							free(Block->Data);
							ms::DynamicArrayPop(&s_MemBlocks, i);
							//	printf("[Malloc Sucks]: Block: %zu, is empty and therefore was destroyed!\n", i);
						}
					}

					return;
				}
			}
		}
	}

	//FIX - print some useful infos like address
	printf("[Malloc Sucks]: Nothing to free, Address: %p\n", Data);
}

void* s_calloc(size_t Count, size_t Size)
{
	if (Size == 0 || Count == 0)
		return NULL;

	void* Data = s_malloc(Count * Size);
	if (!Data)
		return NULL;

	memset(Data, 0, Count * Size);

	return Data;
}

void* s_realloc(void* Data, size_t Size)
{
	if (Size == 0)
		return NULL;

	if (Data == NULL)
	{
		return s_malloc(Size);
	}

	size_t OldSize = 0;

	for (size_t i = 0; i < s_MemBlocks.Size; i++)
	{
		s_MemBlockInfo* Block = (s_MemBlockInfo*)ms::DynamicArrayGetAt(&s_MemBlocks, i);

		if (Block->AllocatedElements.Size > 0)
		{
			for (long long j = Block->AllocatedElements.Size - 1; j >= 0; j--)
			{
				s_AllocatedElementInfo* Element = (s_AllocatedElementInfo*)ms::DynamicArrayGetAt(&Block->AllocatedElements, j);

				if (Element->Data == Data)
				{
					OldSize = Element->Size;

					if (Element->PotentialSize >= Size)
					{
						Element->Size = Size;
						return Data;
					}

					goto EndLoop;
				}
			}
		}
	}

EndLoop:
	void* NewData = s_malloc(Size);
	if (!NewData)
		return NULL;

	memcpy(NewData, Data, OldSize < Size ? OldSize : Size);
	s_free(Data);

	return NewData;
}