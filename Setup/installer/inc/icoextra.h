#if defined(_MSC_VER) && (_MSC_VER > 1000)
#pragma once
#endif

#ifndef _ICON_EXTRACT_H_
#define _ICON_EXTRACT_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// ExtractIconResource
//
BOOL CALLBACK ExtractIconResource(
	DWORD resId,
	LPCTSTR pszFile
);

// The following structures are taken from iconpro sdk example

#pragma pack(push)
#pragma pack(2)

#pragma warning(disable:4200)

typedef struct {
	BYTE  bWidth;       // Width of the image
	BYTE  bHeight;      // Height of the image (times 2)
	BYTE  bColorCount;  // Number of colors in image (0 if >=8bpp)
	BYTE  bReserved;    // Reserved
	WORD  wPlanes;      // Color Planes
	WORD  wBitCount;    // Bits per pixel
	DWORD dwBytesInRes; // how many bytes in this resource?
	WORD  wID;          // the ID
} MEMICONDIRENTRY, *LPMEMICONDIRENTRY;

typedef struct {
	WORD  idReserved;   // Reserved
	WORD  idType;       // resource type (1 for icons)
	WORD  idCount;      // how many images?
	MEMICONDIRENTRY idEntries[]; // the entries for each image
} MEMICONDIR, *LPMEMICONDIR;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif