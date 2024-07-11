#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "MallocSucks.h"

#define malloc s_malloc
#define realloc s_realloc
#define free s_free

#include "Json.h"

int main(int argc, char** argv)
{
	//Malloc sucks aka s_function is used to check for memory leaks in the json parser
	s_init();

	Json Jsn;
//	if (JsonParseFile("simple.json", &Jsn) == JSON_TRUE)
//	if (JsonParseFile("simpleComp.json", &Jsn) == JSON_TRUE)
//	if (JsonParseFile("Bin.glb", &Jsn) == JSON_TRUE)
	if (JsonParseFile("Emb.gltf", &Jsn) == JSON_TRUE)
//	if (JsonParseFile("Hell.json", &Jsn) == JSON_TRUE)
//	if (JsonParseFile("HellComp.json", &Jsn) == JSON_TRUE)
	{
		JsonDestroy(&Jsn);
	}

	s_checkForLeaks();
	
	s_destroy();

	return 0;
}