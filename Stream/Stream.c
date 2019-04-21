#include "Stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <assert.h>

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
#define stat _stat
#define strdup _strdup
#endif


void Stream_Attach(struct Stream* stream, char* text)
{
    Stream_Close(stream);
    stream->data = text;        
}

bool Stream_Set(struct Stream* stream, const char* text)
{
    char* data = strdup(text);
    if (data)
    {
        Stream_Attach(stream, data);
    }
    return data != NULL;
}

bool Stream_Open(struct Stream* stream, const char* path)
{
    bool result = false;
    struct stat info;
    int r = stat(
        path,
        &info);
    if (r == 0)
    {
        char* data = (char*)malloc(sizeof(char) * info.st_size + 1);
        if (data != NULL)
        {
            FILE* file = fopen(path, "r");
            if (file != NULL)
            {
                //SKIP BOM
                if (info.st_size >= 3)
                {
                    fread(data, 1, 3, file);
                    if (data[0] == (char)0xEF &&
                        data[1] == (char)0xBB &&
                        data[2] == (char)0xBF)
                    {
                        size_t n = fread(data, 1, info.st_size - 3, file);
                        data[n] = 0;
                    }
                    else
                    {
                        size_t n = fread(data + 3, 1, info.st_size - 3, file);
                        data[3 + n] = 0;
                    }
                }
                else
                {
                    size_t n = fread(data, 1, info.st_size, file);
                    data[n] = 0;
                }

                fclose(file);
                result = true;
                Stream_Attach(stream, data);
            }
        }
    }
    return result;
}

static wchar_t Stream_ReadNextChar(const struct Stream* stream, int* bytes)
{
    //https://www.ietf.org/rfc/rfc3629.txt
    //https://www.fileformat.info/info/unicode/utf8.htm

    unsigned u = EOF;
    int currentPos = stream->NextBytePos;
    if (stream->data != NULL)
    {
        int c = stream->data[currentPos];

        if (c == '\0' /*EOF*/)
        {
            u = EOF;
        }
        else if ((c & 0x80) == 0)
        {
            currentPos++;
            u = c;
        }
        else if ((c & 0xC0) == 0x80)
        {
            u = EOF;
        }
        else
        {
            currentPos++;
            u = (c & 0xE0) == 0xC0 ? (c & 0x1F)
                : (c & 0xF0) == 0xE0 ? (c & 0x0F)
                : (c & 0xF8) == 0xF0 ? (c & 0x07)
                : 0;

            if (u == 0)
            {
                u = EOF;
            }
            else
            {
                for (;;)
                {
                    c = stream->data[currentPos];
                    currentPos++;

                    if (c == EOF)
                    {
                        break;
                    }
                    else if ((c & 0xC0) == 0x80)
                    {
                        u = (u << 6) | (c & 0x3F);
                    }
                    else
                    {
                        currentPos--;
                        break;
                    }
                }
            }
        }
    }

    *bytes = currentPos - stream->NextBytePos;
    return u;
}

wchar_t Stream_LookAhead(const struct Stream* stream)
{
    int bytes = 0;
    wchar_t ch =
        Stream_ReadNextChar(stream, &bytes);

    return ch;
}

wchar_t Stream_Match(struct Stream* stream)
{
    //assert(stream->data != NULL);

    int bytes = 0;
    wchar_t ch =
        Stream_ReadNextChar(stream, &bytes);

    if (stream->CurrentLine == 0)
    {
        stream->CurrentLine = 1;
    }

    if (bytes > 0)
    {
        stream->CurrentBytePos = stream->NextBytePos;
        stream->NextBytePos += bytes;
        stream->CurrentCol++;
        
        if (ch == '\n')
        {
            stream->CurrentLine++;
            stream->CurrentCol = 0;
        }
    }
    stream->CurrentChar = ch;
    return ch;
}

void Stream_Close(struct Stream* stream)
{
    free(stream->data);
    stream->CurrentCol = 0;
    stream->CurrentLine = 0;
    stream->NextBytePos = 0;
    stream->CurrentBytePos = 0;
}


