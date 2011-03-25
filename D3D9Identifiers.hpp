#ifndef	D3D9IDENTIFIERS_HPP
#define	D3D9IDENTIFIERS_HPP

#include <d3d9.h>
#include <d3dx9.h>

/* ------------------------------------------------------------------------------- */

const char *findShader(void *iface, UINT len, const DWORD* buf);
const char *findShader(void *iface);
const char *findFormat(D3DFORMAT fmt);
const char *findUsage(DWORD use);

#endif
