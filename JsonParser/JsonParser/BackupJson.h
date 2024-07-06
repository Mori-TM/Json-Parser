#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <varargs.h>
#include <ctype.h>
#include "DynamicArray.h"

//typedef uint32_t JsonRet;

typedef enum : uint32_t
{
	JSON_FALSE = 0,
	JSON_TRUE,
	JSON_ERROR = UINT32_MAX
} JsonRet;


JsonRet JsonRuntimeError(const char* Msg, ...)
{
	char Buf[2048];
	snprintf(Buf, 2048, "\x1B[91m[Renderer Error]\033[0m\t%s\n", Msg);
	Buf[2047] = '\0';

	va_list ArgList;
	va_start(ArgList, Msg);
	vprintf(Buf, ArgList);
	va_end(ArgList);

	return JSON_ERROR;
}

typedef enum : uint32_t
{
	JSON_INT = 0,
	JSON_DUB,
	JSON_STR,
	JSON_BOOL,
	JSON_OBJ,
	JSON_ARY,
	JSON_NULL,
	JSON_TYPE_COUNT
} JsonType;

typedef struct
{
	char* Name;
	JsonType Type;

	union
	{
		char* Str;
		int64_t Int;
		double Double;
		bool Bool;
		//	JsonObject* Obj;
		//	JsonObject* Ary;
	} Data;
} JsonVariables;

typedef struct JsonObject_t
{
	char* Name;
	JsonType Type;
	JsonObject_t* PrevObj;
	DynamicArray Refrences;
	//	JsonVariables
} JsonObject;



typedef struct
{

} Json;

char* JsonSetName(const char* Name)
{
	size_t StrLen = strlen(Name);
	char* Str = (char*)malloc(StrLen + 2);
	if (!Str)
		return NULL;

	if (StrLen > 0)
		memcpy(Str, Name, StrLen);

	Str[StrLen] = 0;
	return Str;
}

void JsonPrintRefrences(JsonObject* pObj)
{
	// = (JsonObject*)DynamicArrayGetAt(&Objects, i);
	for (size_t i = 0; i < pObj->Refrences.Size; i++)
	{
		JsonObject* pObj1 = (JsonObject*)DynamicArrayGetAt(&pObj->Refrences, i);

	}
}

size_t JsonLevel = 0;
bool JsonReset = false;

void preOrderTraversal(JsonObject* root)
{
	if (root == NULL || root->Refrences.Size == 0) {
		return;
	}

	for (size_t j = 0; j < JsonLevel; j++)
		printf("\t");

	// Print the data of the node
	if (root->Type == JSON_OBJ ||
		root->Type == JSON_ARY)
	{
		if (root->Type == JSON_OBJ)
			printf("%s {%zu}:\n", root->Name, root->Refrences.Size);
		else
			printf("%s [%zu]:\n", root->Name, root->Refrences.Size);

		//	if (JsonReset)
		//	{
		//		if (JsonLevel > 0)
		//			JsonLevel--;
		//		JsonReset = false;
		//	}
		//	else
		JsonLevel++;
		//	printf("\t");
	}
	else
	{
		JsonVariables* pVar = (JsonVariables*)root;



		if (pVar->Type == JSON_STR)
			printf("Var[Str]: %s: %s\n", pVar->Name, pVar->Data.Str);
		else if (pVar->Type == JSON_DUB)
			printf("Var[Double]: %s: %f\n", pVar->Name, pVar->Data.Double);
		else if (pVar->Type == JSON_INT)
			printf("Var[Int]: %s: %lld\n", pVar->Name, pVar->Data.Int);
		else if (pVar->Type == JSON_BOOL)
			printf("Var[Bool]: %s: %s\n", pVar->Name, (pVar->Data.Bool ? "true" : "false"));
		else if (pVar->Type == JSON_NULL)
			printf("Var[Unknown]: %s\n", pVar->Name);

		/*if (JsonReset)
		{
			if (JsonLevel > 0)
				JsonLevel--;
			JsonReset = false;
		}*/
	}

	/*if (Obj->Refrences.Size == 0)
			{
				if (JsonLevel > 0)
					JsonLevel--;
				JsonReset = false;
			}
	*/

	// Recur on each child
	if (root->Type == JSON_OBJ ||
		root->Type == JSON_ARY)
	{
		//	JsonReset = true;

		for (size_t i = 0; i < root->Refrences.Size; i++) {



			JsonObject* Obj = (JsonObject*)DynamicArrayGetAt(&root->Refrences, i);

			preOrderTraversal(Obj);
		}


	}


}

JsonRet JsonParseBuffer(const size_t Length, const char* Buffer, Json* Jsn)
{
	printf(Buffer);

	DynamicArray Objects = DynamicArrayCreate(sizeof(JsonObject), "Json Objects");
	JsonObject Obj;
	Obj.Name = JsonSetName("Default");
	Obj.Type = JSON_OBJ;
	Obj.PrevObj = NULL;
	memset(&Obj.Refrences, 0, sizeof(DynamicArray));
	//	DynamicArrayPush(&Objects, &Obj);

	DynamicArray Variables = DynamicArrayCreate(sizeof(JsonVariables), "Json Variables");
	JsonVariables Var;
	Var.Type = JSON_TYPE_COUNT;
	Var.Data.Str = NULL;
	//	DynamicArrayPush(&Variables, &Var);

	JsonObject* CurObj = NULL;
	JsonObject* PrevObj = NULL;

	char StrLen = 0;
	char String[32];

	DynamicArray Names = DynamicArrayCreate(sizeof(char**), "Json Names");
	char* pNameDefault = JsonSetName("Default");
	//	DynamicArrayPush(&Names, (char**)&pNameDefault);

	for (size_t i = 0; i < Length; i++)
	{
		JsonObject Obj;
		Obj.Name = NULL;//Mem leak
		Obj.Type = JSON_OBJ;
		Obj.PrevObj = NULL;
		memset(&Obj.Refrences, 0, sizeof(DynamicArray));

		JsonVariables Var;
		Var.Type = JSON_TYPE_COUNT;
		Var.Data.Str = NULL;

		//	if (Buffer[i] == '}')
		//	{
		//		if (Names.Size > 0)
		//		{
		//			char* Name = *(char**)DynamicArrayGetAt(&Names, Names.Size - 1);
		//			Obj.Name = (Name);
		//			DynamicArrayPop(&Names, Names.Size - 1);
		//		//	free(Name);
		//			DynamicArrayPush(&Objects, &Obj);
		//		}
		//		
		//	}

		if (Buffer[i] == ':')
		{
			while (Buffer[++i] == ' ');
			char Type = Buffer[i];
			switch (Type)
			{
			case '{':
			case '[':
				if (Names.Size > 0)
				{
					char* Name = *(char**)DynamicArrayGetAt(&Names, Names.Size - 1);
					Obj.Name = (Name);
					if (Type == '{')
						Obj.Type = JSON_OBJ;
					else
						Obj.Type = JSON_ARY;

					Obj.Refrences = DynamicArrayCreate(sizeof(JsonObject), "JS Obj in Obj");
					Obj.PrevObj = CurObj;
					DynamicArrayPop(&Names, Names.Size - 1);
					//	DynamicArrayPush(&Objects, &Obj);
					DynamicArrayPush(&CurObj->Refrences, &Obj);


					CurObj = (JsonObject*)DynamicArrayGetAt(&CurObj->Refrences, CurObj->Refrences.Size - 1);
					//	CurObj->Refrences = DynamicArrayCreate(sizeof(JsonObject), "JS Obj in Obj");
				}
				else
					printf("Yeah\n");
				i++;
				break;
			case '\"':
				if (Names.Size > 0)
				{
					char* Name = *(char**)DynamicArrayGetAt(&Names, Names.Size - 1);
					Var.Type = JSON_STR;
					Var.Name = Name;
					DynamicArrayPop(&Names, Names.Size - 1);


					StrLen = 0;
					while (Buffer[++i] != '\"' && StrLen < 31)
					{
						if (Buffer[i] == '\\')
						{
							i++;
						}
						String[StrLen++] = Buffer[i];
					}
					while (StrLen == 31 && Buffer[i] != '\"') i++;
					String[StrLen] = 0;
					if (StrLen > 0)
					{
						char* p = JsonSetName(String);
						Var.Data.Str = p;
					}

					DynamicArrayPush(&Variables, &Var);
					DynamicArrayPush(&CurObj->Refrences, &Var);
					i++;
				}


				//	DynamicArrayPop(&Names, Names.Size - 1);
				break;
			case '-':
			case '+':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (Names.Size > 0)
				{
					char* Name = *(char**)DynamicArrayGetAt(&Names, Names.Size - 1);

					Var.Name = Name;
					DynamicArrayPop(&Names, Names.Size - 1);

					bool IsFloatingPointNum = false;

					StrLen = 0;
					while ((Buffer[i] != ' ' && Buffer[i] != '\n' && Buffer[i] != '\r' && Buffer[i] != ',') && StrLen < 31)
					{
						String[StrLen++] = Buffer[i++];
						if (Buffer[i] == '.')
							IsFloatingPointNum = true;
					}
					while (StrLen == 31 && Buffer[i] != ' ' && Buffer[i] != '\n' && Buffer[i] != '\r' && Buffer[i] != ',') i++;
					String[StrLen] = 0;

					char* p;
					if (IsFloatingPointNum)
					{
						Var.Type = JSON_DUB;
						Var.Data.Double = strtod(String, &p);
					}
					else
					{
						Var.Type = JSON_INT;
						Var.Data.Int = atoll(String);

					}


					DynamicArrayPush(&Variables, &Var);
					DynamicArrayPush(&CurObj->Refrences, &Var);
				}
				break;
			case 't':
			case 'f':
				if (Buffer[i + 1] == 'r' && Buffer[i + 2] == 'u' && Buffer[i + 3] == 'e')
				{
					if (Names.Size > 0)
					{
						char* Name = *(char**)DynamicArrayGetAt(&Names, Names.Size - 1);
						Var.Name = Name;
						DynamicArrayPop(&Names, Names.Size - 1);

						Var.Type = JSON_BOOL;
						Var.Data.Bool = true;

						DynamicArrayPush(&Variables, &Var);
						DynamicArrayPush(&CurObj->Refrences, &Var);
						i += 4;
					}
				}
				else if (Buffer[i + 1] == 'a' && Buffer[i + 2] == 'l' && Buffer[i + 3] == 's')
				{
					if (Names.Size > 0)
					{
						char* Name = *(char**)DynamicArrayGetAt(&Names, Names.Size - 1);
						Var.Name = Name;
						DynamicArrayPop(&Names, Names.Size - 1);

						Var.Type = JSON_BOOL;
						Var.Data.Bool = false;

						DynamicArrayPush(&Variables, &Var);
						DynamicArrayPush(&CurObj->Refrences, &Var);
						i += 5;
					}
				}

				break;

			case 'n':
				if (Buffer[i + 1] == 'u' && Buffer[i + 2] == 'l' && Buffer[i + 3] == 'l')
				{

					if (Names.Size > 0)
					{
						char* Name = *(char**)DynamicArrayGetAt(&Names, Names.Size - 1);
						Var.Name = Name;
						DynamicArrayPop(&Names, Names.Size - 1);

						Var.Type = JSON_NULL;
						Var.Data.Str = NULL;

						DynamicArrayPush(&Variables, &Var);

						DynamicArrayPush(&CurObj->Refrences, &Var);
						i += 4;
					}
				}
				break;

			default:
				break;
			}
		}

		if (Buffer[i] == '\"')
		{
			StrLen = 0;

			while (Buffer[++i] != '\"' && StrLen < 31)
			{
				if (Buffer[i] == '\\')
				{
					i++;
				}
				String[StrLen++] = Buffer[i];
			}
			while (StrLen == 31 && Buffer[i] != '\"') i++;
			//	i--;
			char CheckUp = Buffer[i];
			String[StrLen] = 0;
			//	if (StrLen > 0)
			{
				char* p = JsonSetName(String);
				DynamicArrayPush(&Names, (char**)&p);
			}
		}

		if (Buffer[i] == '{' ||
			Buffer[i] == '[')
		{
			//	JsonObject Obj;
			Obj.Name = JsonSetName("Unamed");
			if (Buffer[i] == '{')
				Obj.Type = JSON_OBJ;
			else
				Obj.Type = JSON_ARY;
			Obj.PrevObj = NULL;

			Obj.Refrences = DynamicArrayCreate(sizeof(JsonObject), "JS Obj in Obj");
			Obj.PrevObj = CurObj;

			if (Objects.Size == 0 || CurObj == NULL)
			{
				DynamicArrayPush(&Objects, &Obj);
				CurObj = (JsonObject*)DynamicArrayGetAt(&Objects, Objects.Size - 1);
			}
			else
			{
				DynamicArrayPush(&CurObj->Refrences, &Obj);
				CurObj = (JsonObject*)DynamicArrayGetAt(&CurObj->Refrences, CurObj->Refrences.Size - 1);
			}
			//	CurObj = (JsonObject*)DynamicArrayGetAt(&CurObj->Refrences, CurObj->Refrences.Size - 1);

		//	CurObj->Refrences = DynamicArrayCreate(sizeof(JsonObject), "JS Obj in Obj");

		//	printf("Unamed Object\n");
		}

		if (Buffer[i] == '}' ||
			Buffer[i] == ']')
		{
			CurObj = CurObj->PrevObj;
		}
	}
	/*
	for (size_t i = 0; i < Objects.Size; i++)
	{
		JsonObject* pObj = NULL;
		pObj = (JsonObject*)DynamicArrayGetAt(&Objects, i);

		if (pObj->Type == JSON_OBJ)
			printf("Obj: %zu, %s\n", i, pObj->Name);
		else
			printf("Ary: %zu, %s\n", i, pObj->Name);

		size_t Level = 1;

		JsonObject* LastObj = NULL;

	Recuresive:

		if (pObj->Refrences.Size > 0)
		{
			for (size_t j = 0; j < pObj->Refrences.Size; j++)
			{
				for (size_t k = 0; k < Level; k++)
					printf("\t");

				JsonObject* pObj1 = (JsonObject*)DynamicArrayGetAt(&pObj->Refrences, j);


				printf("Sub: %zu, %s\n", j, pObj1->Name);
				if (pObj1->Refrences.Size > 0)
				{
					JsonObject* pObj2 = (JsonObject*)DynamicArrayGetAt(&pObj1->Refrences, );
					pObj = pObj2;

					Level++;
					goto Recuresive;
				}



			}
		}
		else
		{
			pObj = LastObj;
			Level--;
			if (LastObj == NULL)
				break;
			goto Recuresive;
		}




	}
	*/
	JsonObject* pObj = NULL;
	pObj = (JsonObject*)DynamicArrayGetAt(&Objects, 0);
	preOrderTraversal(pObj);

	/*
	for (size_t i = 0; i < Variables.Size; i++)
	{
		JsonVariables* pVar = (JsonVariables*)DynamicArrayGetAt(&Variables, i);
		if (pVar->Type == JSON_STR)
			printf("Var[Str]: %zu, %s: %s\n", i, pVar->Name, pVar->Data.Str);
		else if (pVar->Type == JSON_DUB)
			printf("Var[Double]: %zu, %s: %f\n", i, pVar->Name, pVar->Data.Double);
		else if (pVar->Type == JSON_INT)
			printf("Var[Int]: %zu, %s: %lld\n", i, pVar->Name, pVar->Data.Int);
		else if (pVar->Type == JSON_BOOL)
			printf("Var[Bool]: %zu, %s: %s\n", i, pVar->Name, (pVar->Data.Bool ? "true" : "false"));
		else if (pVar->Type == JSON_NULL)
			printf("Var[Unknown]: %zu, %s\n", i, pVar->Name);
		//else

	}

	for (size_t i = 0; i < Names.Size; i++)
	{
		char* Str = *(char**)DynamicArrayGetAt(&Names, i);
		printf("Str: %zu, %s\n", i, Str);
	}
	*/
	return JSON_TRUE;
}

JsonRet JsonParseFile(const char* FileName, Json* Jsn)
{
	FILE* File = fopen(FileName, "rb");
	if (!File)
		return JsonRuntimeError("Failed to open: %s", FileName);

	fseek(File, 0, SEEK_END);
	size_t Length = ftell(File);
	fseek(File, 0, SEEK_SET);

	char* Buffer = (char*)malloc(Length + 1);
	if (!Buffer)
		return JsonRuntimeError("Failed to allocate buffer for: %s", FileName);

	fread(Buffer, 1, Length, File);
	Buffer[Length] = '\0';

	fclose(File);


	JsonRet Ret = JsonParseBuffer(Length, Buffer, Jsn);
	free(Buffer);
	return Ret;
}

void JsonDestroy(Json* Jsn)
{

}