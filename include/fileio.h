// Copyright (c) 2001-2015, SCALABLE Network Technologies, Inc.  All Rights Reserved.
//                          600 Corporate Pointe
//                          Suite 1200
//                          Culver City, CA 90230
//                          info@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the QualNet
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

/// \defgroup Package_FILEIO FILEIO

/// \file
/// \ingroup Package_FILEIO
///
/// This file describes data strucutres and functions used for reading from
/// input files and printing to output files.

#ifndef FILEIO_H
#define FILEIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "clock.h"

//-----------------------------------------------------------------------------
// DEFINES
//-----------------------------------------------------------------------------

///
/// Optional macro values to use when calling IO_Read...() APIs.
/// Defines any node id.
#define ANY_NODEID      0xffffffff

///
/// Optional macro values to use when calling IO_Read...() APIs.
/// Defines any node address.
#define ANY_ADDRESS     0xffffffff

///
/// Optional macro values to use when calling IO_Read...() APIs.
/// Defines any instance.
#define ANY_INSTANCE    0xffffffff

///
/// Maximum input file line length. Evaluates (6 * MAX_STRING_LENGTH)
// Original this was set to (6* MAX_STRING_LENGTH), but needs to be increased
// to accomodate the long lines
#define MAX_INPUT_FILE_LINE_LENGTH      (200 * MAX_STRING_LENGTH)

///
/// Maximum length of address string.
#define MAX_ADDRESS_STRING_LENGTH       80

///
/// Max number of -FILE references in an input file. (Restriction is
/// only for immediate children)
#define MAX_NUM_CACHED_FILES 128

///
/// Defines the matching at global level
#define MATCH_GLOBAL   2


///
/// Defines the matching by node id.
#define MATCH_NODE_ID   4

///
/// Defines the matching by network.
#define MATCH_NETWORK   6

///
/// Defines the matching by interface.
#define MATCH_INTERFACE 8

///
/// Defines input allocation unit.
#define INPUT_ALLOCATION_UNIT 500

//-----------------------------------------------------------------------------
// STRUCTS, ENUMS, AND TYPEDEFS
//-----------------------------------------------------------------------------

///
/// Used in dynamic changing of parameters.
enum {
    TYPE_INT32 = 0,
    TYPE_INT64 = 1,
    TYPE_UINT32 = 2,
    TYPE_FLOAT32 = 3,
    TYPE_FLOAT64 = 4,
    TYPE_NODEADDRESS = 5,
    TYPE_STRING = 6,
    TYPE_CLOCKTYPE = 7,
    TYPE_BOOL = 8,
    TYPE_INPUTFILE = 9
};

///
/// Definition of node input structure. typedef to NodeInput in
/// include/main.h.
struct NodeInput
{
    char* ourName;// just the name, not path
    int  numLines;
    int  maxLineLen;
    char **inputStrings;
    char **timeQualifiers;
    char **qualifiers;
    char **variableNames;
    int  *instanceIds;
    char **values;
    int  numFiles;
    char *cachedFilenames[MAX_NUM_CACHED_FILES];
    NodeInput *cached[MAX_NUM_CACHED_FILES];
    NodeInput *routerModelInput;
    int maxNumLines;
    clocktype startSimClock;

    // Needed for change param avlue
    PartitionData* partition;
};

//-------------------------------------------------------------------------
// INLINED FUNCTIONS
//-------------------------------------------------------------------------

/// Parses IPv4 address into a dotted-decimal string.
///
/// \param ipAddress  IPv4 address to be converted into
///    the string.
/// \param addressString  Storage for string.
///
/// \note This function does not distinguish between network- and
/// host-byte order.
// NOTE:        This function should be renamed to
//              IO_ConvertIpAddressToString().
inline void
IO_ConvertIpAddressToString(
    NodeAddress ipAddress,
    char *addressString)
{
    sprintf(addressString, "%u.%u.%u.%u",
            (ipAddress & 0xff000000) >> 24,
            (ipAddress & 0xff0000) >> 16,
            (ipAddress & 0xff00) >> 8,
            ipAddress & 0xff);
}

/// Returns the index of the first subString found in s.
///
/// \param s  Source string.
/// \param subString  Substring to earch for.
///
/// \return Index of the first subString found in s. -1, if not
/// found.
inline int
IO_FindStringPos(
    char s[],
    const char subString[])
{
    char *foundAt = strstr(s, subString);

    if (foundAt == NULL)
    {
        return -1;
    }
    else
    {
        return (int)(foundAt - &s[0]);
    }
}

/// Searches source buffer for the first %s-style token
/// encountered, and copies it to dst.
///
/// \param dst  Buffer to copy token too. If passed in as
///    NULL, this is not used.
/// \param src  Source string.
/// \param next  Storage for pointer to remainder of string.
///    If passed in as NULL, this is not used. If
///    a token was not found, this is not used.
///    If token was found and next is not NULL,
///    then pointer to remainder of string is stored
///    in *next.
///
/// \return
///    * dst, if string was found and dst was passed in as
///      non-NULL.
///    * Pointer to token in src, if string was found
///      and dst was passed in as NULL.
///    * NULL, otherwise.
///
/// \note The caller should make sure dst is big enough.  This
///    function does not perform "dumb" input validation, like
///    checking whether src is passed in as NULL.
///
/// \note This function was written as a replacement of strtok()
///    that's simpler and safer to use.  Although less
///    efficient than strtok(), it should be faster than
///    sscanf().
///
/// \sa IO_GetDelimitedToken
inline char *
IO_GetToken(char *dst, const char *src, char **next)
{
    // a is moving across src.
    // b is moving across dst.

    const char *a = src;
    char *b = dst;
    const char *tokenInSrc = NULL;

    // Scan past left-side white space.

    while (1)
    {
        if (*a == 0)
        {
            // Couldn't find a token.
            *dst = 0;
            *next = NULL;
            return NULL;
        }

        if (!isspace(*a))
        {
            break;
        }

        a++;
    }

    // Token was found.  Copy into dst until right-side white space is
    // encountered.

    if (dst == NULL)
    {
        tokenInSrc = a;
    }

    while (1)
    {
        if (dst)
        {
            *b = *a;
            b++;
        }

        a++;

        if (*a == 0 || isspace(*a))
        {
            // Found right-side white space.

            if (dst)
            {
                *b = 0;
            }

            if (next != NULL)
            {
                // Copy pointer to remainder of string into *next.

                *next = (char *) a;
            }

            if (dst)
            {
                return dst;
            }
            else
            {
                return (char *) tokenInSrc;
            }
        }
    }
}

/// Searches source buffer for the first delimited token
/// encountered, and copies it to dst.
///
/// \param dst  Buffer to copy token too. If passed in as
///    NULL, this is not used.
/// \param src  Source string.
/// \param delim  Delimiter string.
/// \param next  Storage for pointer to remainder of string.
///    If passed in as NULL, this is not used. If
///    a token was not found, this is not used.
///    If token was found and next is not NULL,
///    then pointer to remainder of string is stored
///    in *next.
///
/// \return
///    * dst, if string was found and dst was passed in as
///      non-NULL.
///    * Pointer to token in src, if string was found
///      and dst was passed in as NULL.
///    * NULL, otherwise.
///
/// \note The caller should make sure dst is big enough.  This
///    function does not perform "dumb" input validation, like
///    checking whether src is passed in as NULL.
///
/// \note This function was written as a replacement of strtok()
///    that's simpler and safer to use.  Although less
///    efficient than strtok(), it should be faster than
///    sscanf().
inline
char* IO_GetDelimitedToken(
    char* dst,
    const char* src,
    const char* delim,
    char** next)
{
    // a is moving across src.
    // b is moving across dst.
    const char *a = src;
    char *b = dst;
    const char *tokenInSrc = 0;

    const int numDelim = (int)strlen(delim);

    // Scan past left-side.
    while (1) {
        int i;

        if (*a == 0) {
            // Couldn't find a token.
            *dst = 0;
            *next = NULL;
            return NULL;
        }

        for (i = 0; i < numDelim; i++) {
            if (*a == delim[i]) {
                break;
            }
        }
        if (i == numDelim) {
            // *a is not in the delim string
            break;
        }

        a++;
    }

    // Token was found.  Copy into dst until right-side white space is
    // encountered.
    if (dst == NULL) {
        tokenInSrc = a;
    }

    while (1) {
        int i;

        if (dst) {
            *b = *a;
            b++;
        }

        a++;

        if (*a == 0) {
            break;
        }
        for (i = 0; i < numDelim; i++) {
            if (*a == delim[i]) {
                break;
            }
        }
        if (i != numDelim) {
            // *a is in the delim string
            // Found right-side white space.
            break;
        }
    }

    if (dst) {
        *b = 0;
    }

    if (next != NULL) {
        // Copy pointer to remainder of string into *next.
        *next = (char*)a;
    }

    if (dst) {
        return dst;
    }
    else {
        return (char*)tokenInSrc;
    }
}

/// Returns a pointer to the right side of the string of length
/// "count" characters.
///
/// \param s  String.
/// \param count  Number of characters on the right side.
///
/// \return A pointer to the right side of the string of
/// length "count" characters. If count is 0, then
/// a pointer to the string's terminating NULL is
/// returned. If count is equal to or greater than
/// the number of characters in the string, then
/// the whole string is returned. A "character"
/// is just a byte of char type that's not NULL.
/// So, '\n' counts as a character.
inline const char *
IO_Right(const char *s, unsigned count)
{
    unsigned len = (unsigned)strlen(s);
    const char *termNull = s + len;

    if (count == 0)
    {
        return termNull;
    }

    if (count >= len)
    {
        return s;
    }

    return termNull - count;
}

/// Removes the last character of string.
///
/// \param s  String.
///
/// \return s. If the string has a strlen() of zero, then the
/// string is returned unmodified. A "character" is
/// just a byte of char type that's not NULL. So, '\n'
/// counts as a character.
inline char *
IO_Chop(char *s)
{
    if (*s != 0)
    {
        s[strlen(s) - 1] = 0;
    }

    return s;
}

/// Changes nsbp charecters for UTF-8 encoding to spaces.
///
/// \param s  String.
///
/// \note Behavior is as expected for empty strings or strings
///     with only white space.  This function does not validate
///     that s is non-NULL.
inline void
IO_TrimNsbpSpaces(char *s)
{
    unsigned char *p = ( unsigned char* )s;
    size_t len = strlen(s);
    size_t count = 0;

    for (; count < len ; p++, count++)
    {
        if (*p == 160 || *p == 194){
            *p = ' ';
        }
    }
}

/// Strips leading white space from a string (by memmove()ing
/// string contents left).
///
/// \param s  String.
///
/// \note Behavior is as expected for empty strings or strings
///     with only white space.  This function does not validate
///     that s is non-NULL.
inline void
IO_TrimLeft(char *s)
{
    char *p = s;
    unsigned len = (unsigned)strlen(s);
    unsigned count = 0;

    for (; count < len && isspace(*p); p++, count++)
    {
        // Do nothing.
    }

    if (count > 0)
    {
        memmove(s, s + count, len - count + 1);
    }
}

/// Strips trailing white space from a string (by inserting
/// early NULL).
///
/// \param s  String.
///
/// \note Behavior is as expected for empty strings or strings
///     with only white space.  This function does not validate
///     that s is non-NULL.
inline void
IO_TrimRight(char *s)
{
    unsigned len = (unsigned)strlen(s);
    char *p = s + len;          // p points at terminating NULL

    if (p == s)
    {
        return;
    }

    while (1)
    {
        p--;

        if (!isspace(*p))
        {
            *(p + 1) = 0;
            return;
        }
    }
}

/// Compresses white space between words in the string to one
/// space in a string.
///
/// \param s  String.
///
/// \note        White space at the very beginning and very end of the
///              string is also compressed to one space -- not stripped
///              entirely.  Use IO_TrimLeft() and IO_TrimRight() as desired
///              before calling IO_CompressWhiteSpace().
///
/// \note        Behavior is as expected for empty strings or strings
///              with only white space.  This function does not validate
///              that s is non-NULL.
inline void
IO_CompressWhiteSpace(char *s)
{
    unsigned len = (unsigned)strlen(s);
    char *p = s + len;          // p points at terminating NULL

    if (p == s)
    {
        return;
    }

    // This function is not yet implemented.
}

/// Returns TRUE if every character in string is a digit. (Even
/// white space will cause return of FALSE)
///
/// \param s  String.
///
/// \return TRUE if every character is a digit. FALSE, otherwise.
inline BOOL
IO_IsStringNonNegativeInteger(const char *s)
{
    for (; *s; s++)
    {
        if (!isdigit(*s))
        {
            return FALSE;
        }
    }

    return TRUE;
}

/// Runs tolower() on each character in string and converts the
/// same to lowercase.
///
/// \param s  String.
inline void
IO_ConvertStringToLowerCase(char s[])
{
    int lineLength = (int)strlen(s);
    int i;

    for (i = 0; i < lineLength; i++)
    {
        s[i] = (char)tolower(s[i]);
    }
}

/// Runs toupper() on each character in string and converts the
/// same to uppercase.
///
/// \param s  String.
inline void
IO_ConvertStringToUpperCase(char s[])
{
    int lineLength = (int)strlen(s);
    int i;

    for (i = 0; i < lineLength; i++)
    {
        s[i] = (char)toupper(s[i]);
    }
}

/// Checks two strings are equal or not ignoring case.
///
/// \param s1  First string.
/// \param s2  Second string.
/// \param lengthToCompare  Length to compare.
///
/// \return Returns TRUE if strings are equal, FALSE otherwise.
inline BOOL
IO_CaseInsensitiveStringsAreEqual(
    const char s1[],
    const char s2[],
    const int lengthToCompare)
{
    int i;

    for (i = 0; i < lengthToCompare; i++)
    {
        if (tolower(s1[i]) != tolower(s2[i]))
        {
            return FALSE;
        }

        if (s1[i] == 0)
        {
            return TRUE;
        }
    }
    return TRUE;
}

/// Checks the blank line/string.
///
/// \param s  String.
///
/// \return Returns TRUE if the string is blank. FALSE, otherwise.
inline BOOL
IO_BlankLine(const char s[])
{
    unsigned i;

    for (i = 0; i < strlen(s); i++)
    {
        if ((int) s[i] > (int) (' '))
        {
            return FALSE;
        }
    }
    return TRUE;
}

/// Checks whether the line is a comment(i.e. starts with '#').
///
/// \param s  String.
///
/// \return Returns TRUE if the line is a comment. FALSE, otherwise.
inline BOOL
IO_CommentLine(const char s[])
{
    return (BOOL) (s[0] == '#' || IO_BlankLine(s));
}

/// Finds the case insensitive sub string position in a string.
///
/// \param s  String.
/// \param subString  Sub string
///
/// \return Returns the position of case insensitive sub string if
/// found. -1, otherwise.
inline int
IO_FindCaseInsensitiveStringPos(
    const char s[],
    const char subString[])
{
    int i;
    int subStringLength = (int)strlen(subString);

    for (i = 0; i <= (int)(strlen(s) - subStringLength); i++)
    {
        int n;

        for (n = 0; n < subStringLength
                    && tolower(s[i + n]) == tolower(subString[n]); n++)
        {
            // Still comparing.
        }

        if (n == subStringLength)
        {
            return i;
        }
    }

    return -1;  // Not found.
}

/// Finds the case insensitive sub string position in a string.
///
/// \param s  String.
/// \param subString  Sub string
///
/// \return Returns the position of case insensitive sub string if
/// found. -1, otherwise.


/// skip the first n tokens.
///
/// \param token  pointer to the input string,
/// \param tokenSep  pointer to the token separators,
/// \param skip  number of skips.
///
/// \return pointer to the next token position.
inline char*
IO_SkipToken(char* token, const char* tokenSep, int skip)
{
    int idx;

    if (token == NULL)
    {
        return NULL;
    }

    // Eliminate preceding space
    idx = (int)strspn(token, tokenSep);
    token = (idx < (signed int)strlen(token)) ? token + idx : NULL;

    while (skip > 0 && token != NULL)
    {
        token = strpbrk(token, tokenSep);

        if (token != NULL)
        {
            idx = (int)strspn(token, tokenSep);
            token = (idx < (signed int)strlen(token)) ? token + idx : NULL;
        }
        skip--;
    }
    return token;
}


//--------------------------------------------------------------------------
// PROTOTYPES FOR FUNCTIONS WITH EXTERNAL LINKAGE
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Input files
//--------------------------------------------------------------------------
/// Allocates a NodeInput datastructure that can then be
/// passed to IO_ReadNodeInput
/// Called for each file variable in the config file.
///
/// \param nodeInput  Pointer to node input.
/// \param filename  Path to input file.
///
/// \note This function will excise comments, remove leading and
///     trailing white space.
NodeInput * IO_CreateNodeInput (bool allocateRouterModelInput = false);

/// Initializes a NodeInput structure
///
/// \param nodeInput  A pointer to NodeInput structure.
void IO_InitializeNodeInput(NodeInput *nodeInput,
    bool allocateRouterModelInput);

/// Reads an input file into a NodeInput struct.
/// Calls IO_ReadFileParameters to first read in -FILE paramters.
/// Then calls IO_ReadFileParameters to read the rest of the
/// parameters.
///
/// \param nodeInput  Pointer to node input.
/// \param filename  Path to input file.
void
IO_ReadNodeInput(NodeInput *nodeInput, const char *filename);
//BackwardCompatibility Fix Start
/// Reads an input file into a NodeInput struct.
/// The includeComment Flag facilitate whether to include
/// the commented line
/// lines in the nodeInput structure or not.
/// The commented lines should only
/// be included in Backward Compatibity for Old scenario exe.
/// This exe is responsible
/// for creating new config file and router model file.
/// These new files contains
/// changes according to current VERSION of QualNet.
/// Calls IO_ReadFileParameters to first read in -FILE paramters.
/// Then calls IO_ReadFileParameters to read the rest of the
/// parameters.
///
/// \param nodeInput  Pointer to node input.
/// \param filename  Path to input file.
/// \param includeComment  When this flag is true it
///    included the commented line in
///    nodeInput structure also.
void
IO_ReadNodeInputEx(NodeInput *nodeInput,
                   const char *filename,
                   BOOL includeComment = FALSE);
//BackwardCompatibility Fix End
void
IO_ReadFile(NodeInput *nodeInput, const char *filename);

/// Converts the contents of an old configuration file
/// to the latest version.
///
/// \param nodeInput  A pointer to node input.
/// \param nodeOutput  A pointer to node input. Goes through
///    database and print current values to
///    nodeOutput.
/// \param version  Not used.
///
/// \return Returns TRUE if able to convert. FALSE, otherwise.
/// Either couldn't load the database or something else bad
/// happened, so just copy the old into the new.
BOOL
IO_ConvertFile(NodeInput *nodeInput,
                  NodeInput *nodeOutput,
                  char *version,
                  BOOL includeComments);

//---------------------------------------------------------------------------
// Input files parameter retrieval
//---------------------------------------------------------------------------

/// This API is used to retrieve a whole line
/// from input files.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IP address of interface.
///    Can be ANY_IP.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.
void
IO_ReadLine(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    char *readVal);

/// This API is used to retrieve a string parameter
/// value from input files.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IP address of interface.
///    Can be ANY_IP.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.
void
IO_ReadString(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    char *readVal);

/// This API is used to retrieve a boolean parameter
/// value from input files.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IP address of interface.
///    Can be ANY_IP.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.
void
IO_ReadBool(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    BOOL *readVal);

/// This API is used to retrieve an integer parameter
/// value from input files.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IP address of interface.
///    Can be ANY_IP.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.
void
IO_ReadInt(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    int *readVal);

void
IO_ReadandCheckInt(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    int *parameterValue,
    BOOL required,
    BOOL minReq,
    int minVal,
    BOOL maxReq,
    int maxVal);

/// This API is used to retrieve a double parameter
/// value from input files.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IP address of interface.
///    Can be ANY_IP.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.
void
IO_ReadDouble(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    double *readVal);

void
IO_ReadInt64(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    Int64 *readVal);

/// This API is used to retrieve a float parameter
/// value from input files.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IP address of interface.
///    Can be ANY_IP.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.
void
IO_ReadFloat(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    float *readVal);

/// This API is used to retrieve time parameter
/// value from input files.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IP address of interface.
///    Can be ANY_IP.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.
void
IO_ReadTime(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    clocktype *readVal);

void
IO_ReadandCheckTime(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    clocktype *parameterValue,
    BOOL required,
    BOOL minReq,
    clocktype minVal,
    BOOL maxReq,
    clocktype maxVal);

/// This API is used to retrieve cached file parameter
/// value from input files.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IP address of interface.
///    Can be ANY_IP.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadCachedFile(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    NodeInput *parameterValue);

//---------------------------------------------------------------------------
// Input files parameter retrieval by instance qualifier
//---------------------------------------------------------------------------

/// This API is used to retrieve string parameter values
/// from input files for a specific instance.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IP address of interface.
///    Can be ANY_IP.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadStringInstance(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    char *parameterValue);

/// This API is used to retrieve boolean parameter values
/// from input files for a specific instance.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IP address of interface.
///    Can be ANY_IP.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadBoolInstance(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    BOOL *parameterValue);

/// This API is used to retrieve integer parameter values
/// from input files for a specific instance.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IP address of interface.
///    Can be ANY_IP.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadIntInstance(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    int *parameterValue);

/// This API is used to retrieve double parameter values
/// from input files for a specific instance.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IP address of interface.
///    Can be ANY_IP.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadDoubleInstance(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    double *parameterValue);

/// This API is used to retrieve float parameter values
/// from input files for a specific instance.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IP address of interface.
///    Can be ANY_IP.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadFloatInstance(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    float *parameterValue);

/// This API is used to retrieve time parameter values
/// from input files for a specific instance.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IP address of interface.
///    Can be ANY_IP.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadTimeInstance(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    clocktype *parameterValue);

/// This API is used to retrieve file parameter values
/// from input files for a specific instance.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IP address of interface.
///    Can be ANY_IP.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadCachedFileInstance(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    NodeInput *parameterValue);

//---------------------------------------------------------------------------
// Address string parsing
//---------------------------------------------------------------------------

/// Parses a string for a nodeId, host address, or network
/// address.
///
/// \param s  String to parse.
/// \param outputNodeAddress  Storage for nodeId or IP address.
/// \param numHostBits  Storage for number of host bits
///    (32 - number of network bits).
///    Defaults to zero, unless it's an
///    actual network address.
/// \param isNodeId  Storage for whether the string is
///    a nodeId.
void
IO_ParseNodeIdHostOrNetworkAddress(
    const char s[],
    NodeAddress *outputNodeAddress,
    int *numHostBits,
    BOOL *isNodeId);



/// Parses a string for a nodeId or host address.
///
/// \param s  String to parse.
/// \param outputNodeAddress  Storage for nodeId or IP address.
/// \param isNodeId  Storage for whether the string is
///    a nodeId.
void
IO_ParseNodeIdOrHostAddress(
    const char s[],
    NodeAddress *outputNodeAddress,
    BOOL *isNodeId);

/// Parses a string for a network address.
///
/// \param s  String to parse.
/// \param outputNodeAddress  Storage for network address.
/// \param numHostBits  Storage for number of host bits
///    (32 - number of network bits).
void
IO_ParseNetworkAddress(
    const char s[],
    NodeAddress *outputNodeAddress,
    int *numHostBits);

//---------------------------------------------------------------------------
// NodeInput helper functions
//---------------------------------------------------------------------------

/// Frees a NodeInput struct. (Currently unused.)
///
/// \param nodeInput  Pointer to node input.
void
IO_FreeNodeInput(NodeInput *nodeInput);

//---------------------------------------------------------------------------
// Output functions
//---------------------------------------------------------------------------

/// Print out the relevant stat in "buf", along with the
/// node id and the layer type generating this stat.
///
/// \param node  The node generating the stat.
/// \param layer  The layer generating the stat.
/// \param protocol  The protocol generating the stat.
/// \param interfaceAddress  Interface address.
/// \param instanceId  Instance id.
/// \param buf  String which has the statistic to
///    be printed out
void
IO_PrintStat(
    Node *node,
    const char *layer,
    const char *protocol,
    NodeAddress interfaceAddress,
    int instanceId,
    const char *fmt,
    ...);


//---------------------------------------------------------------------------
// Application input parsing functions.
//---------------------------------------------------------------------------

/// Application input parsing API. Parses the source and
/// destination strings.
///
/// \param node  A pointer to Node.
/// \param inputString  The input string.
/// \param sourceString  The source string.
/// \param sourceNodeId  A pointer to NodeAddress.
/// \param sourceAddr  A pointer to NodeAddress.
/// \param destString  Const char pointer.
/// \param destNodeId  A pointer to NodeAddress.
/// \param destAddr  A pointer to NodeAddress.
void IO_AppParseSourceAndDestStrings(
    Node *node,
    const char *inputString,
    const char *sourceString,
    NodeAddress *sourceNodeId,
    NodeAddress *sourceAddr,
    const char *destString,
    NodeAddress *destNodeId,
    NodeAddress *destAddr);

/// Application input parsing API. Parses the source
/// string.
///
/// \param node  A pointer to Node.
/// \param inputString  The input string.
/// \param sourceString  The source string.
/// \param sourceNodeId  A pointer to NodeAddress.
/// \param sourceAddr  A pointer to NodeAddress.
void IO_AppParseSourceString(
    Node *node,
    const char *inputString,
    const char *sourceString,
    NodeAddress *sourceNodeId,
    NodeAddress *sourceAddr);

/// Application input parsing API. Parses the
/// destination string.
///
/// \param node  A pointer to Node.
/// \param inputString  The input string.
/// \param destString  Const char pointer.
/// \param destNodeId  A pointer to NodeAddress.
/// \param destAddr  A pointer to NodeAddress.
void IO_AppParseDestString(
    Node *node,
    const char *inputString,
    const char *destString,
    NodeAddress *destNodeId,
    NodeAddress *destAddr);

/// Application input parsing API. Parses the
/// host string.
///
/// \param node  A pointer to Node.
/// \param inputString  The input string.
/// \param destString  Const char pointer.
/// \param destNodeId  A pointer to NodeAddress.
/// \param destAddr  A pointer to NodeAddress.
void IO_AppParseHostString(
    Node *node,
    const char *inputString,
    const char *destString,
    NodeAddress *destNodeId,
    NodeAddress *destAddr);

/// Application input checking API. Checks for the same
/// source and destination node id. Calls abort() for same
/// source and destination.
///
/// \param inputString  The input string.
/// \param sourceNodeId  Source node id, read from the
///    application input file.
/// \param destNodeId  Destination node id, read from the
///    application input file.
void IO_AppForbidSameSourceAndDest(
    const char *inputString,
    NodeAddress sourceNodeId,
    NodeAddress destNodeId);

// Used by OSPF also.

/// This is an auxiliary API used by the IO_Read...()
/// set of APIs.
///
/// \param nodeId  nodeId to select for.
///    ANY_NODEID matches any nodeId.
/// \param interfaceAddress  IP address to select for.
///    ANY_IP matches any IP address.
/// \param qualifier  String containing the
///    qualifier.
/// \param matchType  Stores the type of the match,
///    if any, e.g. byIP, byNodeId
///
/// \return Returns TRUE if match found. FALSE, otherwise.
BOOL
QualifierMatches(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    char *qualifier,
    int  *matchType);

// ------------------------------------------------------------------------
// Overloaded API for Ipv6 compatibility
//
// NOTE:
// Have to declare Overloaded APIs separately below this __cplusplus brace
// block instead of declaring it just below the original one. If it is
// declared just below the original one inside the  __cplusplus brace
// it is been treated as c function and showing compilation error.
// ------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Input files parameter retrieval - Ipv6 compatibility
//--------------------------------------------------------------------------


/// This API is used to retrieve boolean parameter values
/// from input files.  Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IPv6 address of interface.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadBool(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    BOOL *parameterValue);

/// Reads boolean value for specified ATM address.
///
/// \param nodeId  NodeId for which parameter has
///    has to be searched.
/// \param interfaceAddress  ATM Interface address
/// \param nodeInput  pointer to configuration inputs
/// \param parameterName  Parameter to be read
/// \param wasFound  Parameter found or not
/// \param parameterValue  Parameter's value if found.
void
IO_ReadBool(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    BOOL *parameterValue);

/// This API is used to retrieve boolean parameter values
/// from input files.  Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param address  Address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadBool(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    BOOL *parameterValue);

/// This API is used to retrieve a string parameter
/// value from input files. Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IP address of interface.
///    Can be NULL  .
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.
void
IO_ReadString(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    char *readVal);

/// Reads string value for specified ATM address.
///
/// \param nodeId  NodeId for which parameter has
///    has to be searched.
/// \param interfaceAddress  ATM Interface address
/// \param nodeInput  pointer to configuration inputs
/// \param parameterName  Parameter to be read
/// \param wasFound  Parameter found or not
/// \param parameterValue  Parameter's value if found.
void
IO_ReadString(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    char *parameterValue);

/// This API is used to retrieve a string parameter
/// value from input files. Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param address  IP address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.
void
IO_ReadString(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    char *readVal);

/// This API is used to retrieve an integer parameter
/// value from input files. Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IPv6 address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.
void
IO_ReadInt(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL* wasFound,
    int *parameterValue);

/// This API is used to retrieve an integer parameter
/// value from input files. Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param address  Address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.
void
IO_ReadInt(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL* wasFound,
    int *parameterValue);

void
IO_ReadInt(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL* wasFound,
    int *parameterValue,
    BOOL required,
    BOOL minReq,
    int minVal,
    BOOL maxReq,
    int maxVal);

/// Reads int value for specified ATM address.
///
/// \param nodeId  NodeId for which parameter has
///    has to be searched.
/// \param interfaceAddress  ATM Interface address
/// \param nodeInput  pointer to configuration inputs
/// \param parameterName  Parameter to be read
/// \param wasFound  Parameter found or not
/// \param parameterValue  Parameter's value if found.
void
IO_ReadInt(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL* wasFound,
    int *parameterValue);

/// This API is used to retrieve a double parameter
/// value from input files. Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IPv6 address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.

void
IO_ReadDouble(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    double *parameterValue);

/// Reads double value for specified ATM address.
///
/// \param nodeId  NodeId for which parameter has
///    has to be searched.
/// \param interfaceAddress  ATM Interface address
/// \param nodeInput  pointer to configuration inputs
/// \param parameterName  Parameter to be read
/// \param wasFound  Parameter found or not
/// \param parameterValue  Parameter's value if found.
void
IO_ReadDouble(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    double *parameterValue);

void
IO_ReadInt64(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    Int64 *parameterValue);




/// This API is used to retrieve a double parameter
/// value from input files. Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param address  Address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.
void
IO_ReadDouble(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    double *parameterValue);

void
IO_ReadInt64(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    Int64 *parameterValue);




/// This API is used to retrieve a float parameter
/// value from input files. Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IPv6 address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.
void
IO_ReadFloat(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    float *readVal);

/// Reads float value for specified ATM address.
///
/// \param nodeId  NodeId for which parameter has
///    has to be searched.
/// \param interfaceAddress  ATM Interface address
/// \param nodeInput  pointer to configuration inputs
/// \param parameterName  Parameter to be read
/// \param wasFound  Parameter found or not
/// \param parameterValue  Parameter's value if found.
void
IO_ReadFloat(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    float *parameterValue);

/// This API is used to retrieve a float parameter
/// value from input files. Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param address  Address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.
void
IO_ReadFloat(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    float *readVal);

void
IO_ReadFloat(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *index,
    BOOL *wasFound,
    float *readVal,
    BOOL required,
    BOOL minReq,
    float minVal,
    BOOL maxReq,
    float maxVal);

/// This API is used to retrieve time parameter
/// value from input files. Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IPv6 address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.

void
IO_ReadTime(
    const NodeAddress nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    clocktype *parameterValue);

/// Reads time value for specified ATM address.
///
/// \param nodeId  NodeId for which parameter has
///    has to be searched.
/// \param interfaceAddress  ATM Interface address
/// \param nodeInput  pointer to configuration inputs
/// \param parameterName  Parameter to be read
/// \param wasFound  Parameter found or not
/// \param parameterValue  Parameter's value if found.
void
IO_ReadTime(
    const NodeAddress nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    clocktype *parameterValue);

/// This API is used to retrieve time parameter
/// value from input files. Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param address  Address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param readVal  Storage for parameter value.
void
IO_ReadTime(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    clocktype *parameterValue);

/// Reads BOOL value for specified ATM address.
///
/// \param nodeId  NodeId for which parameter has
///    has to be searched.
/// \param interfaceAddress  ATM Interface address
/// \param nodeInput  pointer to configuration inputs
/// \param parameterName  Parameter to be read
/// \param parameterInstanceNumber  parameter instance number
/// \param fallbackIfNoInstanceMatch  get the
///    previous match if no match is found.
/// \param wasFound  Parameter found or not
/// \param parameterValue  Parameter's value if found.
void
IO_ReadBoolInstance(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    BOOL *parameterValue);

/// Reads BOOL value for specified IPv6 address.
///
/// \param nodeId  NodeId for which parameter has
///     has to be searched.
/// \param interfaceAddress  ipv6 Interface address
/// \param nodeInput  pointer to configuration inputs
/// \param parameterName  Parameter to be read
/// \param parameterInstanceNumber  parameter instance number
/// \param fallbackIfNoInstanceMatch  get the
///     previous match if no match is found.
/// \param wasFound  Parameter found or not
/// \param parameterValue  Parameter's value if found.
void
IO_ReadBoolInstance(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    BOOL *parameterValue);


//---------------------------------------------------------------------------
// Input files parameter retrieval by instance qualifier - Ipv6 compatibility
//---------------------------------------------------------------------------

/// This API is used to retrieve string parameter values
/// from input files for a specific instance.
/// Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IPv6 address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.

void
IO_ReadStringInstance(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    char *parameterValue);

/// Reads string value for specified ATM address.
///
/// \param nodeId  NodeId for which parameter has
///    has to be searched.
/// \param interfaceAddress  ATM Interface address
/// \param nodeInput  pointer to configuration inputs
/// \param parameterName  Parameter to be read
/// \param parameterInstanceNumber  parameter instance number
/// \param fallbackIfNoInstanceMatch  get the
///    previous match if no match is found.
/// \param wasFound  Parameter found or not
/// \param parameterValue  Parameter's value if found.
void
IO_ReadStringInstance(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    char *parameterValue);

/// This API is used to retrieve string parameter values
/// from input files for a specific instance.
/// Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param address  Address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadStringInstance(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    char *parameterValue);

/// This API is used to retrieve integer parameter values
/// from input files for a specific instance.
/// Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IPv6 address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.

void
IO_ReadIntInstance(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    int *parameterValue);

/// Reads int value for specified ATM address.
///
/// \param nodeId  NodeId for which parameter has
///    has to be searched.
/// \param interfaceAddress  ATM Interface address
/// \param nodeInput  pointer to configuration inputs
/// \param parameterName  Parameter to be read
/// \param parameterInstanceNumber  parameter instance number
/// \param fallbackIfNoInstanceMatch  get the
///    previous match if no match is found.
/// \param wasFound  Parameter found or not
/// \param parameterValue  Parameter's value if found.
void
IO_ReadIntInstance(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    int *parameterValue);

/// This API is used to retrieve integer parameter values
/// from input files for a specific instance.
/// Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param address  Address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadIntInstance(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    int *parameterValue);

/// This API is used to retrieve double parameter values
/// from input files for a specific instance.
/// Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IPv6 address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadDoubleInstance(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    double *parameterValue);


/// Reads double value for specified ATM address.
///
/// \param nodeId  NodeId for which parameter has
///    has to be searched.
/// \param interfaceAddress  ATM Interface address
/// \param nodeInput  pointer to configuration inputs
/// \param parameterName  Parameter to be read
/// \param parameterInstanceNumber  parameter instance number
/// \param fallbackIfNoInstanceMatch  get the
///    previous match if no match is found.
/// \param wasFound  Parameter found or not
/// \param parameterValue  Parameter's value if found.
void
IO_ReadDoubleInstance(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    double *parameterValue);


/// This API is used to retrieve double parameter values
/// from input files for a specific instance.
/// Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param address  IPv6 address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadDoubleInstance(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    double *parameterValue);

/// This API is used to retrieve float parameter values
/// from input files for a specific instance.
/// Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IPv6 address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadFloatInstance(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    float *parameterValue);

/// Reads float value for specified ATM address.
///
/// \param nodeId  NodeId for which parameter has
///    has to be searched.
/// \param interfaceAddress  ATM Interface address
/// \param nodeInput  pointer to configuration inputs
/// \param parameterName  Parameter to be read
/// \param parameterInstanceNumber  parameter instance number
/// \param fallbackIfNoInstanceMatch  get the
///    previous match if no match is found.
/// \param wasFound  Parameter found or not
/// \param parameterValue  Parameter's value if found.
void
IO_ReadFloatInstance(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    float *parameterValue);

/// This API is used to retrieve float parameter values
/// from input files for a specific instance.
/// Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param address  Address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadFloatInstance(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    float *parameterValue);

/// This API is used to retrieve time parameter values
/// from input files for a specific instance.
/// Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceAddress  IPv6 address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadTimeInstance(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    clocktype *parameterValue);

/// Reads clocktype value for specified ATM address.
///
/// \param nodeId  NodeId for which parameter has
///    has to be searched.
/// \param interfaceAddress  ATM Interface address
/// \param nodeInput  pointer to configuration inputs
/// \param parameterName  Parameter to be read
/// \param parameterInstanceNumber  parameter instance number
/// \param fallbackIfNoInstanceMatch  get the
///    previous match if no match is found.
/// \param wasFound  Parameter found or not
/// \param parameterValue  Parameter's value if found.
void
IO_ReadTimeInstance(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    clocktype *parameterValue);



/// This API is used to retrieve time parameter values
/// from input files for a specific instance.
/// Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param address  Address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadTimeInstance(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    clocktype *parameterValue);


/// This API is used to retrieve cached file parameter
/// value from input files.  Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param address  Address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadCachedFile(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    NodeInput *parameterValue);

/// This API is used to retrieve file parameter values
/// from input files for a specific instance.
/// Overloaded API for Ipv6 compatibility.
///
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param address  Address of interface.
///    Can be NULL.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadCachedFileInstance(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    NodeInput *parameterValue);


//---------------------------------------------------------------------------
// Output functions
//---------------------------------------------------------------------------

/// Print out the relevant stat in "buf", along with the
/// node id and the layer type generating this stat.
/// Overloaded API for Ipv6 compatibility.
///
/// \param node  The node generating the stat.
/// \param layer  The layer generating the stat.
/// \param protocol  The protocol generating the stat.
/// \param interfaceAddress  The Interface address the stat.
/// \param instanceId  Instance id.
/// \param buf  String which has the statistic to
///    be printed out

void
IO_PrintStat(
    Node *node,
    const char *layer,
    const char *protocol,
    const char *ipAddrString,
    int instanceId,
    const char *fmt,
    ...);

//--------------------------------------------------------------------------
// Address string parsing - Ipv6 compatibility
//--------------------------------------------------------------------------
/// Parses a string for a nodeId, host address, or network
/// address. Overloaded API for Ipv6 compatibility.
///
/// \param s  String to parse.
/// \param ipAddress  Storage for ipv6address.
/// \param isIpAddr  Storage for whether the string is
///    an ipv6address.
/// \param nodeId  Storage for nodeId.
void
IO_ParseNodeIdHostOrNetworkAddress(
    const char s[],
    in6_addr* ipAddress,
    BOOL* isIpAddr,
    NodeId* nodeId);

void
IO_ParseNodeIdHostOrNetworkAddress(
    const char inputString[],
    in6_addr* ipAddress,
    BOOL* isIpAddr,
    NodeId* nodeId,
    unsigned int* prefixLenth);


//For ATM
//--------------------------------------------------------------------------
// Address string parsing - ATM compatibility
//--------------------------------------------------------------------------
/// Parses a string for a nodeId, host address, or network
/// address. Overloaded API for Ipv6 compatibility.
///
/// \param s  String to parse.
/// \param atmAddress  Storage for ATMaddress.
/// \param isAtmAddr  Storage for whether the string is
///    an ATMaddress.
/// \param nodeId  Storage for nodeId.
void
IO_ParseNodeIdHostOrNetworkAddress(
   const char addressString[],
   AtmAddress* atmAddr,
   BOOL* isAtmAddr,
   NodeId* nodeId);

/// Parses a string for a nodeId, host address, or network
///    address. Overloaded API for Ipv6 compatibility.
///    When address is given in the form of TLA:NLA:SLA, then
///    value of TLA, NLA and SLA is stored at upper three unsigned
///    block of in6_addr. To get the corresponding ip address
///    we need to create it by using tla, nla, sla value from it
/// \param addressString  String to parse.
/// \param isIpAddr  Storage for whether the string is an Ipv6 address.
/// \param nodeId  Storage for node Identifier if present in the string
///    * zero - the string does not cointain a nodeId
///    * > zero - the string cointain a valid nodeId
void
IO_ParseNodeIdHostOrNetworkAddress(
    const char inputString[],
    Address* ipAddress,
    BOOL* isNodeId);


/// Parses a string for a nodeId or host address.
///
/// \param s  String to parse.
/// \param outputNodeAddress  Storage for ipv6address.
/// \param isNodeId  Storage for whether the string is
///    a nodeId.
void
IO_ParseNodeIdOrHostAddress(
    const char s[],
    in6_addr* outputNodeAddress,
    NodeId* nodeId);

/// Parses a string for a network address.
/// Overloaded API for Ipv6 compatibility.
///
/// \param s  String to parse.
/// \param tla  Storage for tla
/// \param nla  Storage for nla.
/// \param sla  Storage for sla.
void
IO_ParseNetworkAddress(
    const char s[],
    unsigned int *tla,
    unsigned int *nla,
    unsigned int *sla);

void
IO_ParseNetworkAddress(
    const char inputString[],
    in6_addr* ipAddress,
    unsigned int* PrefixLenth);

//---------------------------------------------------------------------------
// Application input parsing functions.
//---------------------------------------------------------------------------

/// Application input parsing API. Parses the source and
/// destination strings. Overloaded for Ipv6 compatibility.
///
/// \param node  A pointer to Node.
/// \param inputString  The input string.
/// \param sourceString  The source string.
/// \param sourceNodeId  A pointer to NodeId.
/// \param sourceAddr  A pointer to Address.
/// \param destString  Const char pointer.
/// \param destNodeId  A pointer to NodeId.
/// \param destAddr  A pointer to Address.

void
IO_AppParseSourceAndDestStrings(
    Node* node,
    const char* inputString,
    const char* sourceString,
    NodeId* sourceNodeId,
    Address* sourceAddr,
    const char* destString,
    NodeId* destNodeId,
    Address* destAddr);


/// Application input parsing API. Parses the source
/// string.  Overloaded for Ipv6 compatibility.
///
/// \param node  A pointer to Node.
/// \param inputString  The input string.
/// \param sourceString  The source string.
/// \param sourceNodeId  A pointer to NodeAddress.
/// \param sourceAddr  A pointer to Address.
/// \param networkType  used when sourceString
///    represents node id.
void
IO_AppParseSourceString(
    Node* node,
    const char* inputString,
    const char* sourceString,
    NodeAddress* sourceNodeId,
    Address* sourceAddr,
    NetworkType networkType = NETWORK_IPV4);

/// Application input parsing API. Parses the
/// destination string. Overloaded for Ipv6 compatibility.
///
/// \param node  A pointer to Node.
/// \param inputString  The input string.
/// \param destString  Const char pointer.
/// \param destNodeId  A pointer to NodeAddress.
/// \param destAddr  A pointer to Address.
/// \param networkType  used when sourceString
///    represents node id.

void
IO_AppParseDestString(
    Node *node,
    const char *inputString,
    const char *destString,
    NodeAddress *destNodeId,
    Address *destAddr,
    NetworkType networkType = NETWORK_IPV4);

/// This is an auxiliary API used by the IO_Read...()
/// set of APIs. Overloaded for Ipv6 compatibility
///
/// \param nodeId  nodeId to select for.
///    ANY_NODEID matches any nodeId.
/// \param interfaceAddress  IPv6 address to select for.
///    NULL matches any IP address.
/// \param qualifier  String containing the
///    qualifier.
/// \param matchType  Stores the type of the match,
///    if any, e.g. byIP, byNodeId
///
/// \return Returns TRUE if match found. FALSE, otherwise.
BOOL
QualifierMatches(
    const NodeId nodeId,
    const in6_addr* interfaceAddress,
    char *qualifier,
    int  *matchType);


/// This is an auxiliary API used by the IO_Read...()
/// set of APIs. Overloaded for Ipv6 compatibility
///
/// \param nodeId  nodeId to select for.
///    ANY_NODEID matches any nodeId.
/// \param interfaceAddress  ATM address to select for.
///    NULL matches any IP address.
/// \param qualifier  String containing the
///    qualifier.
/// \param matchType  Stores the type of the match,
///    if any, e.g. byIP, byNodeId
///
/// \return Returns TRUE if match found. FALSE, otherwise.
// -----------------------------------------------------------------------------
// NOTE:
// Overloaded Function QualifierMatches()

BOOL
QualifierMatches(
    const NodeId nodeId,
    const AtmAddress* interfaceAddress,
    char *qualifier,
    int  *matchType);

//--------------------------------------------------------------------------
// Utility Functions - Ipv6 compatibility
//--------------------------------------------------------------------------

/// Convert IPv6 address string to in6_addr structure.
/// API for Ipv6 compatibility.
///
/// \param interfaceAddr  Storage for ipv6address string
/// \param ipAddress  Storage for ipv6address.

void IO_ConvertStringToAddress(
    char* interfaceAddr,
    in6_addr* ipv6Address);

/// Parses IPv6 address into a formatted string.
/// Overloaded API for Ipv6 compatibility.
///
/// \param ipAddress  Storage for ipv6address.
/// \param interfaceAddr  Storage for ipv6address string

void IO_ConvertIpAddressToString(
        in6_addr* ipv6Address,
        char* interfaceAddr,
        BOOL ipv4EmbeddeAddr = FALSE);

void IO_ConvertIpAddressToString(
        char* ipv6Address,
        char* interfaceAddr,
        BOOL ipv4EmbeddeAddr = FALSE);

/// Parses IPv6 address into a formatted string.
/// Overloaded API for Ipv6 compatibility.
///
/// \param ipAddress  IP address info
/// \param interfaceAddr  Storage for ipv6address string

void IO_ConvertIpAddressToString(
        Address* ipv6Address,
        char* interfaceAddr);

/// This API is used to covert a string parameter
/// to NodeAdress.
///
/// \param addressString  IP address string info
/// \param outputNodeAddress  Storage for IP address
void IO_ConvertStringToNodeAddress(
        const char addressString[],
        NodeAddress* outputNodeAddress);


/// Compares IPv4 | IPv6 address.
/// API for Ipv6 compatibility.
///
/// \param addr1  Storage for IPv4 | IPv6 address
///    with network information.
/// \param addr2  Storage for IPv4 | IPv6 address
///    with network information.

BOOL IO_CheckIsSameAddress(
    Address addr1,
    Address addr2);


/// This API is used to retrieve a string parameter
/// value from input files.
///
/// \param node  Node pointer for which string is
///    to be  read.
/// \param nodeInput  Pointer to node input.
/// \param index  Parameter name.
/// \param wasFound  Storage for success of search.
/// \param readVal  Storage for parameter value.

void
IO_ReadString(
    Node* node,
    const NodeAddress nodeId,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    char *parameterValue);

/// This API is used to read an auxiliary input file into a
/// NodeInput struct Called for each file variable in the
/// config file.
///
/// \param nodeInput  Pointer to node input.
/// \param filename  Path to input file.
///

void
IO_CacheFile(NodeInput *nodeInput, const char *filename);

/// This API is used to get the maximun length of a line
/// in the file.
///
/// \param    char*  Pointer to the name of the file.
///
/// \return Interger with the largest line length.
int
IO_GetMaxLen(const char *fileName);

/// This API is used to get the maximun length of a line
/// in the file.
///
/// \param FILE*  Pointer to a file stream.
///
/// \return Interger with the largest line length.
int
IO_GetMaxLen(FILE *fp);

/// This API is used to get the maximun length of a line
/// in nodeInput.
///
/// \param NodeInput*  Pointer to a node input.
///
/// \return Interger with the largest line length.
int NI_GetMaxLen( NodeInput *nodeInput );

/// This API is used to get the maximun length of a line
/// in nodeInput.
///
/// \param const NodeInput*  Pointer to a node input.
///
/// \return Interger with the largest line length.
int NI_GetMaxLen( const NodeInput *nodeInput );


//-------------------------------------------------------------------------//
//----------------------- OverLoaded --------------------------------------//


/// Parses a string for a network address.
/// Overloaded API for ATM compatibility.
///
/// \param s  String to parse.
/// \param u_atmVal  Storage for icd, aid, ptp

void IO_ParseNetworkAddress(
         const char s[],
         unsigned int* u_atmVal);

/// Convert generic address to appropriate network type
/// address string format.
///
/// \param address  generic address
/// \param addrStr  address string
///

void IO_ConvertAddrToString(
         Address* address,
         char* addrStr);

/// Convert Atm address to address string format.
///
/// \param addr  Atm address
/// \param addrStr  address string
///

void IO_ConvertAtmAddressToString(AtmAddress addr,
                                  char* addrStr);

/// Insert integer value for specific string in case of ATM
///
/// \param s  character array
/// \param val  value to be inserted
/// \param u_atmVal  atm_value need to be checked
///
void IO_InsertIntValue(
    const char s[],
    const unsigned int val,
    unsigned int* u_atmVal);


/// Return Cached file index for the given parameter name
///
/// \param nodeId  node Id
/// \param interfaceAddress  Interface Address for the given node
/// \param nodeInput  atm_value need to be checked
/// \param parameterName  name use to match the file index
int IO_ReadCachedFileIndex(
    const NodeAddress nodeId,
    const NodeAddress interfaceAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound);

/// This API is used to retrieve a string parameter
/// value from input files.
///
/// \param node  node structure pointer.
/// \param nodeId  nodeId.
/// \param interfaceIndex  interface Index.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadString(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    char *parameterValue);

void
IO_ReadString(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    char *parameterValue,
    BOOL requiredVal);

/// This API is used to retrieve a Int64 parameter
/// value from input files.
///
/// \param node  node structure pointer.
/// \param nodeId  nodeId.
/// \param interfaceIndex  interface Index.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadInt64(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    Int64 *parameterValue);

/// This API is used to retrieve a clocktype parameter
/// value from input files.
///
/// \param node  node structure pointer.
/// \param nodeId  nodeId.
/// \param interfaceIndex  interface Index.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadTime(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    clocktype *parameterValue);

/// This API is used to retrieve a Int parameter
/// value from input files.
///
/// \param node  node structure pointer.
/// \param nodeId  nodeId.
/// \param interfaceIndex  interface Index.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadInt(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    int *parameterValue);

/// This API is used to retrieve a double parameter
/// value from input files.
///
/// \param node  node structure pointer.
/// \param nodeId  nodeId.
/// \param interfaceIndex  interface Index.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadDouble(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    double *parameterValue);

void
IO_ReadDouble(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    double *parameterValue,
    BOOL required,
    BOOL minReq,
    double minVal,
    BOOL maxReq,
    double maxVal);

/// This API is used to retrieve a cached file parameter
/// value from input files.
///
/// \param node  node structure pointer.
/// \param nodeId  nodeId.
/// \param interfaceIndex  interface Index.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadCachedFile(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    NodeInput *parameterValue);

/// This API is used to retrieve a whole line from input files.
///
/// \param node  node structure pointer.
/// \param nodeId  nodeId.
/// \param interfaceIndex  interface Index.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadLine(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    char *parameterValue);

/// This API is used to retrieve string parameter values
/// from input files for a specific instance.
///
/// \param node  node structure pointer.
/// \param nodeId  nodeId.
/// \param interfaceIndex  interface Index.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadStringInstance(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    char *parameterValue);

/// This API is used to retrieve double parameter values
/// from input files for a specific instance.
///
/// \param node  node structure pointer.
/// \param nodeId  nodeId.
/// \param interfaceIndex  interface Index.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadDoubleInstance(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    double *parameterValue);

/// This API is used to retrieve int parameter values
/// from input files for a specific instance.
///
/// \param node  node structure pointer.
/// \param nodeId  nodeId.
/// \param interfaceIndex  interface Index.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadIntInstance(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    int *parameterValue);

/// This API is used to retrieve clocktype parameter values
/// from input files for a specific instance.
///
/// \param node  node structure pointer.
/// \param nodeId  nodeId.
/// \param interfaceIndex  interface Index.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadTimeInstance(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    clocktype *parameterValue);

/// This API is used to retrieve cached file parameter values
/// from input files for a specific instance.
///
/// \param node  node structure pointer.
/// \param nodeId  nodeId.
/// \param interfaceIndex  interface Index.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadCachedFileInstance(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL *wasFound,
    NodeInput *parameterValue);

/// This API is used to retrieve a string parameter
/// value from input files using the ip-address.
///
/// \param node  node structure pointer.
/// \param interfaceIndex  interface Index.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void IO_ReadStringUsingIpAddress(
    Node *node,
    int interfaceIndex,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    char *parameterValue);

int
IO_FindStringInFile(const char* compareLine, const char *filename);

/// This API is used to retrieve a string parameter
/// value from input files.
///
/// \param nodeId  nodeId.
/// \param ipv4Address  IP address of an interface
/// \param ipv6Address  IPv6 address of an interface
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadString(
    const NodeAddress nodeId,
    NodeAddress ipv4address,
    in6_addr* ipv6Address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    char *parameterValue);

/// This API is used to retrieve a string parameter
/// value from input files.
///
/// \param nodeId  nodeId.
/// \param interfaceIndex  interface index
/// \param ipv4SubnetAddress  IPv4 subnet address
/// \param ipv6SubnetAddress  IPv6 subnet address
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterValue  Storage for parameter value.
/// \param wasFound  Storage for success of search.
/// \param matchType  Storage for matchType.
void
IO_ReadString(
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeAddress ipv4SubnetAddress,
    const in6_addr* ipv6SubnetAddress,
    const NodeInput *nodeInput,
    const char *parameterName,
    char *parameterValue,
    BOOL& wasFound,
    int& matchType);

/// Reads string value for specified IPv6 address.
///
/// \param nodeId  NodeId for which parameter has
///    has to be searched.
/// \param address  Pointer to address structure.
///    address can have ip v4 or v6 address
/// \param nodeInput  pointer to configuration inputs
/// \param parameterName  Parameter to be read
/// \param wasFound  Parameter found or not
/// \param parameterValue  Parameter's value if found.

void
IO_ReadString(
    const NodeId nodeId,
    const Address* address,
    const NodeInput *nodeInput,
    const char *parameterName,
    BOOL *wasFound,
    char *parameterValue,
    int &matchType);

/// This API is used to retrieve the value channel mask
/// for a specific instance.
///
/// \param node  node structure pointer.
/// \param nodeId  nodeId.
/// \param address  Pointer to address
///    structure.
/// \param interfaceIndex  interface Index.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param parameterInstanceNumber  Instance number.
/// \param fallbackIfNoInstanceMatch  Selects parameter without
///    instance number if
///    parameterInstanceNumber
///    cannot be matched.
/// \param wasFound  Storage for success of search.
/// \param parseChannelList  Storage for identifying if
///    parameterValue is a channel
///    list or channel mask
/// \param parameterValue  Storage for parameter value.
void
IO_ReadChannelMask(
    Node* node,
    const NodeAddress nodeId,
    const Address* networkAddress,
    int interfaceIndex,
    const NodeInput* nodeInput,
    const char* parameterName,
    const int parameterInstanceNumber,
    const BOOL fallbackIfNoInstanceMatch,
    BOOL* wasFound,
    BOOL* parseChannelList,
    char* parameterValue);

/// Sets matchType to the value of the best match
///
/// \param nodeId  NodeId for
///    which parameter has to be searched.
/// \param address  Pointer to address
///    structure. address can have ip v4 or v6 address
/// \param nodeInput  pointer to
///    configuration inputs
/// \param parameterName  Parameter to be read
/// \param parameterFound   Parameter found or not
/// \param parameterInstanceNumber  Instance number.
/// \param matchType  Stores the type of the match if
///    parameterFound else sets to -1

void
IO_GetMatchType(
    const NodeAddress nodeId,
    const Address* address,
    const NodeInput* nodeInput,
    const char* parameterName,
    BOOL* parameterFound,
    const int parameterInstanceNumber,
    int& matchType);

/// This API is used to retrieve a boolean parameter
/// value from input files.
///
/// \param node  node structure pointer.
/// \param nodeId  nodeId.
/// \param interfaceIndex  interface Index.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadBool(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput* nodeInput,
    const char* parameterName,
    BOOL* wasFound,
    BOOL* parameterValue);

/// This API is used to retrieve a float parameter
/// value from input files.
///
/// \param node  node structure pointer.
/// \param nodeId  nodeId. Can be ANY_NODEID.
/// \param interfaceIndex  interface Index.
/// \param nodeInput  Pointer to node input.
/// \param parameterName  Parameter name.
/// \param wasFound  Storage for success of seach.
/// \param parameterValue  Storage for parameter value.
void
IO_ReadFloat(
    Node* node,
    const NodeAddress nodeId,
    int interfaceIndex,
    const NodeInput* nodeInput,
    const char* parameterName,
    BOOL* wasFound,
    float* parameterValue);
#endif // _FILEIO_H_
