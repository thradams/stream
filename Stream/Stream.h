#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

struct Stream
{
    //utf8 encoded text
    char* data;

    wchar_t CurrentChar;
    int CurrentLine;
    int CurrentCol;
    int CurrentBytePos;
    int NextBytePos;
};

#define STREAM_INIT {0}

wchar_t Stream_Match(struct Stream* stream);
void Stream_Close(struct Stream* stream);
void Stream_Attach(struct Stream* stream, char* text);
bool Stream_Set(struct Stream* stream, const char* text);
bool Stream_Open(struct Stream* stream, const char* path);
wchar_t Stream_LookAhead(const struct Stream* stream);


