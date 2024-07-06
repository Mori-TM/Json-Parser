#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "Json.h"

int main(int argc, char** argv)
{
	Json Jsn;
	if (JsonParseFile("simple.json", &Jsn) == JSON_TRUE)
//	if (JsonParseFile("simpleComp.json", &Jsn) == JSON_TRUE)
//	if (JsonParseFile("Bin.glb", &Jsn) == JSON_TRUE)
//	if (JsonParseFile("Hell.json", &Jsn) == JSON_TRUE)
//	if (JsonParseFile("HellComp.json", &Jsn) == JSON_TRUE)
	{
		JsonDestroy(&Jsn);
	}

	return 0;
}