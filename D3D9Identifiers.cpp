#include <assert.h>

#include <d3d9.h>
#include <d3dx9.h>

#include "D3D9Identifiers.hpp"

/* ------------------------------------------------------------------------------- */
#ifndef	OBGE_NOSHADER

const char *passNames[OBGEPASS_NUM] = {
  "0. No particular pass",
  "1. Reflection pass (off-screen)",
  "2. Water aux pass (off-screen)",
  "3. Water heightmap pass (off-screen)",
  "4. Water displacement pass (off-screen)",
  "5. Shadow pass (off-screen)",
  "6. Main pass (screen-space)",
  "7. Effects pass (screen-space)",
  "8. HDR pass (screen-space)",
  "9. Post pass (screen-space)",

  "-. Video pass",
  "-. Unknown pass",
};

const char *passScens[OBGEPASS_NUM][16] = {
  /* OBGEPASS_ANY		*/
  {},

  /* OBGEPASS_REFLECTION	*/
  {},
  /* OBGEPASS_WATER	 	*/
  {},
  /* OBGEPASS_WATERHEIGHTMAP	*/
  {},
  /* OBGEPASS_WATERDISPLACEMENT */
  {},
  /* OBGEPASS_SHADOW		*/
  {},
  /* OBGEPASS_MAIN		*/
  {},
  /* OBGEPASS_EFFECTS		*/
  {},
  /* OBGEPASS_HDR		*/
  {
  	"",
  },
  /* OBGEPASS_POST		*/
  {},

  /* OBGEPASS_VIDEO		*/
  {},
  /* OBGEPASS_UNKNOWN		*/
  {},
};
#endif

/* Shader-tandems:
 *
 * STLEAF000.vso + STLEAF2000.pso
 * unknown + PAR2018.pso
 * unknown + PAR2002.pso
 * PAR2028.vso + PAR2022.pso
 * STB2005.vso + SLS2003.pso
 * SLS1006.vso + SLS1004.pso
 * SLS2000.vso + SLS2000.pso
 * SLS2000.vso + SLS2002.pso
 * SLS2003.vso + SLS2003.pso
 * SLS2005.vso + SLS2003.pso
 * SLS2025.vso + SLS2033.pso
 * SLS2033.vso + SLS2039.pso
 * SLS2034.vso + SLS2039.pso
 * SLS2042.vso + SLS2048.pso
 * SLS2043.vso + SLS2049.pso
 * SLS2044.vso + SLS2050.pso
 * SLS2047.vso + SLS2053.pso
 * SLS2001.vso + SLS2001.pso
 * SLS2064.vso + SLS2068.pso
 * SM3006.vso + SM3LL010.pso
 * unknown + PAR2000.pso
 * DISTLOD2003.vso + unknown
 * WATER000.vso + WATER012.pso
 * SKY.vso + SKY.pso
 * SKYCLOUDS.vso + SKYTEX.pso+
 * GRASS2030.vso + GRASS2004.pso
 * WATER000.vso + WATER000.pso
 * WATER000.vso + WATER001.pso
 * SKYOCC.vso + SKYSUNOCCL.pso
 */

/* Observations:
 *
 * - STLEAF-permutations:
 *   - STLEAF == PS1.1, STLEAF2 == PS2.0
 *   - STLEAF?000.vso == no fog, STLEAF?001.vso == fog
 *   - STLEAF?000.pso == no fog, STLEAF?001.pso == fog
 *   - STLEAF?000|1.vso == parallel directional light, STLEAF?002|3.vso == point/spot directional light
 *
 * - STFROND-permutations identical to above
 */

/* Reconstructions:
 *
 * STLEAF: -------------------------------------------------
 *
speedtree\\leaf.v.hlsl
#ifdef	PT
#endif
#ifdef	FOG
#endif
 *
speedtree\\leaf.p.hlsl
#ifdef	FOG
#endif
 *
 * STFROND: ------------------------------------------------
 *
speedtree\\frond.v.hlsl
#ifdef	PT
#endif
#ifdef	FOG
#endif
 *
speedtree\\frond.p.hlsl
#ifdef	FOG
#endif
 *
 * STB: ----------------------------------------------------
 *
 * 1000
lighting\\1x\\v\\base.v.hlsl
#defis	TREE
 * 1001 == 1002
lighting\\1x\\v\\ambDiffuseDirTexture.v.hlsl
#defis	TREE
#defis	VC
 * 1003
lighting\\1x\\v\\ambDiffuseDirAndPt.v.hlsl
#defis	TREE
 * 1004
lighting\\1x\\v\\diffuseDir.v.hlsl
#defis	TREE
 * 1005
lighting\\1x\\v\\diffusePt.v.hlsl
#defis	TREE
 * 1006
lighting\\1x\\v\\base.v.hlsl
#defis	TREE
#defis	VC
 * 1007
lighting\\1x\\v\\specularDir.v.hlsl
#defis	TREE
 * 1008
lighting\\1x\\v\\specularPt.v.hlsl
#defis	TREE
 * 1009
lighting\\1x\\v\\base.v.hlsl
#ifdef	FOG
 *
 * (2000|2001)|(2002|2003)
lighting\\2x\\v\\AD.v.hlsl
#defis	LIGHTS
#if	LIGHTS <= 2
#ifdef	PROJ_SHADOW
#endif
#elif	LIGHTS <= 3
#ifdef	PROJ_SHADOW
#endif
#endif
 * (2004|2005)|(2006|2007)|(2008|2009)
lighting\\2x\\v\\ADTS.v.hlsl
#defis	TREE
#ifdef	LIGHT_PARALLEL
#ifdef	PROJ_SHADOW
#endif
#elif	LIGHT_POINT
#ifdef	PROJ_SHADOW
#endif
#elif	LIGHT_SPEC
#ifdef	PROJ_SHADOW
#endif
#endif
 * (2010|2011)
lighting\\2x\\v\\ADTS.v.hlsl
#defis	TREE
#ifdef	LIGHT_SPEC
#endif
 * (2012|2013)
lighting\\2x\\v\\DiffusePt.v.hlsl
#defis	TREE
#if	LIGHTS <= 2
#endif
#elif	LIGHTS <= 3
#endif
 * (2014|2015|2016)
lighting\\2x\\v\\Specular.v.hlsl
#defis	TREE
#ifdef	PROJ_SHADOW
#elif	POINT
#endif
 * 2017
lighting\\2x\\v\\SimpleShadow.v.hlsl
#defis	TREE
#ifdef	SHADOWMAP
#endif
 *
 * 1000
lighting\\1x\\p\\base.p.hlsl
 * 1001 == 1002
lighting\\1x\\p\\ambDiffDirTexture.p.hlsl
 * 1003
lighting\\1x\\p\\ambDiffDirAndPt.p.hlsl
 * 1004
lighting\\1x\\p\\diffuseDir.p.hlsl
 * 1005
lighting\\1x\\p\\diffusePt.p.hlsl
 * 1006
lighting\\1x\\p\\base.p.hlsl
 */

struct shaderID {
  UINT len;
  DWORD crc32;
  const char *name;
  void *ptr;
} shaderDatabase[] = {

{     116, 0xcf439d33, "PS1.3 STB1009.pso [SpeedTreeBranch, copy input v0 to output r0]"},
{     128, 0xdc4cc42c, "PS2.0 SLS1030.pso [copy input v0 to output oC0]"},
{     140, 0x5a889662, "PS3.0 SM3037.pso [clear oC0 to 0]"},
{     140, 0x5a889662, "PS3.0 SM3038.pso [clear oC0 to 0]"},
{     140, 0x5aa89662, "PS3.0 SKYSISUN.pso [clear oC0 to 0]"},
{     140, 0x5ae61c9e, "PS3.0 SM3034.pso [set oC0 to 0.901899993, 0.831399977, 0.721660018, 1]"},
{     140, 0x65d89662, "PS3.0 SM3031.pso [set oC0 to 0.5, 0.5, 0, 0]"},
{     140, 0x65d89662, "PS3.0 SM3032.pso [set oC0 to 0.5, 0.5, 0, 0]"},
{     140, 0x65e89662, "PS3.0 SKYSI.pso [set oC0 to 0, 0, 0.5, 0.5]"},
{     152, 0x592d4e9f, "PS2.X SLS2074.pso ^= SM3034.pso [set r0|oC0 to 0.901899993, 0.831399977, 0.721660018, 1]"},
{     152, 0x5943c463, "PS2.X SLS1038.pso ^= SM3038.pso [clear r0|oC0 to 0]"},
{     152, 0x59a7c463, "PS2.X SLS2065.pso ^= SM3031.pso [set r0|oC0 to 0.5, 0.5, 0, 0]"},
{     152, 0x59c1a205, "PS2.X WATERDISPLACE000.pso [set r0|oC0 to 0.899999976, 0.5, 0.5, 0.5]"},
{     152, 0x59c1a205, "PS2.X WATERDISPLACE001.pso [set r0|oC0 to 0.899999976, 0.5, 0.5, 0.5]"},
{     152, 0x6afe52ae, "PS3.0 SKYSUNOCCL.pso [set oC0 to 0.100000001, 0.100000001, 0.100000001, 1 and oDepth to 1]"},
{     152, 0xffa49f31, "PS?.? DEBUG.vso"},
{     168, 0xc4bc4409, "PS?.? DEBUG.pso"},
{     172, 0x43599e61, "VS3.0 SKYQUADSI.vso"},
{     180, 0x47f4f5b0, "PS3.0 SLS2069.pso"},
{     188, 0x3f6df713, "PS1.3 STB1006.pso [SpeedTreeBranch, no fog, full bright light]"},
{     188, 0xa4a68c35, "PS3.0 SKY.pso"},
{     188, 0xd55b4f0d, "PS?.? COPY000.pso"},
{     200, 0xd6b01d09, "PS?.? ISBLUR2001.pso"},
{     200, 0xd6b01d09, "PS?.? ISHIT2000.pso"},
{     204, 0xcd7aae16, "PS?.? SLS1003.pso"},
{     204, 0xd7a9c47f, "PS?.? SLS2058.pso"},
{     204, 0xd7a9c47f, "PS?.? SLS2060.pso"},
{     204, 0xd7a9c47f, "PS?.? SLS2062.pso"},
{     208, 0x41b29e60, "VS?.? SKYQUAD.vso"},
{     208, 0x69238e22, "VS1.1 HDR000.vso [void]"},
{     208, 0x69238e22, "VS?.? HDR001.vso"},
{     208, 0x69238e22, "VS1.1 HDR002.vso [void]"},
{     208, 0x69238e22, "VS1.1 HDR003.vso [void]"},
{     208, 0x69238e22, "VS1.1 HDR005.vso [void]"},
{     208, 0x69238e22, "VS1.1 HDR006.vso [void]"},
{     208, 0x69238e22, "VS?.? HDR007.vso"},
{     208, 0x69238e22, "VS?.? ISBLUR2000.vso"},
{     208, 0x69238e22, "VS?.? ISBLUR2001.vso"},
{     208, 0x69238e22, "VS?.? ISBLUR2002.vso"},
{     208, 0x69238e22, "VS?.? ISBLUR2004.vso"},
{     208, 0x69238e22, "VS?.? ISHIT2000.vso"},
{     208, 0x69238e22, "VS?.? ISHIT2001.vso"},
{     208, 0x69238e22, "VS?.? MAP000.vso"},
{     208, 0x69238e22, "VS?.? NIGHTEYE000.vso"},
{     208, 0x69238e22, "VS?.? WATERHMAP.vso"},
{     208, 0x6c238f22, "VS?.? WATERDISPLACE002.vso"},
{     208, 0x6c238f22, "VS?.? WATERDISPLACE003.vso"},
{     208, 0x6c238f22, "VS?.? WATERDISPLACE004.vso"},
{     208, 0x6c238f22, "VS?.? WATERDISPLACE005.vso"},
{     208, 0x6c238f22, "VS?.? WATERDISPLACE006.vso"},
{     208, 0x6c238f22, "VS?.? WATERDISPLACE007.vso"},
{     220, 0x3ec78f23, "VS?.? WATERDISPLACE001.vso"},
{     228, 0xdc6c9e42, "PS?.? SM3033.pso"},
{     232, 0x0a4af743, "VS?.? SLS2060.vso"},
{     232, 0x0a4af743, "VS?.? SLS2070.vso"},
{     232, 0x4c5aae0c, "PS2.X STLEAF2000.pso [SpeedTreeLeaf, no fog, any light, diffuse tex]"},
{     232, 0x4c7aae0c, "PS?.? SLS1004.pso"},
{     232, 0x4d2f8f27, "VS?.? COPY000.vso"},
{     232, 0xf4154f58, "PS?.? COPY001.pso"},
{     236, 0x0281ba15, "PS?.? PARTICLE.pso"},
{     236, 0x4c4c334c, "PS1.3 STB1000.pso [SpeedTreeBranch, no fog, ambient light, diffuse tex]"},
{     236, 0x6e371d59, "PS2.X HDR003.pso [sparse sampling of input]"},
{     240, 0xb8e6f756, "VS?.? SKYOCC.vso"},
{     240, 0xdfa6c443, "PS?.? SLS2073.pso"},
{     244, 0x5e9eae0d, "PS?.? SLS1031.pso"},
{     244, 0x6945f75c, "VS?.? SM3023.vso"},
{     244, 0x6945f75c, "VS?.? SM3027.vso"},
{     244, 0xb7a05b7a, "VS?.? MENUBG000.vso"},
{     248, 0x6ccf1d58, "PS?.? ISBLUR2004.pso"},
{     252, 0x5a76d565, "PS?.? WATERHMAP006.pso"},
{     252, 0x9916c449, "PS?.? SLS2059.pso"},
{     252, 0x9916c449, "PS?.? SLS2061.pso"},
{     256, 0x6ba2f658, "VS?.? ----000.vso"},
{     256, 0x6ba2f658, "VS?.? SLS1005.vso"},
{     256, 0x7378632e, "VS1.1 HDR004.vso [void]"},
{     256, 0x7378632e, "VS?.? ISBLUR2003.vso"},
{     256, 0x7378632e, "VS?.? ISHIT2002.vso"},
{     256, 0x7378632e, "VS?.? REFRACT2000.vso"},
{     260, 0x3b1f8b63, "PS?.? SKYSHORIZFADE.pso"},
{     260, 0x3b1f8b63, "PS?.? SKYTEXFADE.pso"},
{     264, 0xcb0a6218, "PS1.3 STB1004.pso [SpeedTreeBranch, no fog, any light, normal tex]"},
{     268, 0x1b45f659, "VS?.? SLS1024.vso"},
{     268, 0x1c5c6a4c, "PS?.? SLS1000.pso"},
{     268, 0x1c5c6a4c, "PS?.? SLS1039.pso"},
{     276, 0x987ef584, "PS?.? SLS2071.pso"},
{     276, 0xa14ef5b0, "PS?.? SLS2070.pso"},
{     280, 0x6ba2f65b, "VS?.? SLS1007.vso"},
{     284, 0xe7729b6b, "VS?.? WATERDISPLACE000.vso"},
{     288, 0x3d69b32f, "PS?.? SM3026.pso"},
{     288, 0x3d69b32f, "PS?.? SM3027.pso"},
{     288, 0x937283bb, "PS?.? PRECIP000.pso"},
{     288, 0x937283bb, "PS?.? PRECIP001.pso"},
{     292, 0x3d59f585, "PS?.? SLS2066.pso"},
{     296, 0x4dadae11, "PS2.X STLEAF2001.pso [SpeedTreeLeaf, fog, any light, diffuse tex]"},
{     296, 0x5cc10349, "PS?.? WATERHMAP003.pso"},
{     304, 0xbeed08ed, "VS?.? SKYFAR.vso"},
{     304, 0xce01f4d1, "PS?.? SLS1037.pso"},
{     308, 0x5ecd0348, "PS?.? WATERHMAP004.pso"},
{     312, 0xa45cf61c, "VS?.? SLS1006.vso"},
{     320, 0x15abd57d, "VS?.? GDECAL.vso"},
{     328, 0xae0808f7, "VS?.? SKYCLOUDSI.vso"},
{     328, 0xb6e1a71d, "PS2.X STFROND2000.pso [SpeedTreeFrond, no fog, any light, diffuse tex, sunlight dimmer]"},
{     328, 0xd121f718, "VS?.? SM3030.vso"},
{     332, 0x5a448069, "PS2.X HDR005.pso [brighten]"},
{     340, 0x7baaf64e, "VS?.? SLS1027.vso"},
{     348, 0x76edc94b, "PS?.? SLS1042.pso"},
{     364, 0x113e605e, "PS?.? GDECAL.pso"},
{     368, 0x0891c6a9, "PS1.3 STB1007.pso [SpeedTreeBranch, no fog, any spec, normal tex]"},
{     368, 0x4a4996fe, "PS?.? SLS1034.pso"},
{     372, 0x814ef587, "PS?.? SLS2072.pso"},
{     372, 0xe4b8f71e, "VS?.? SLS2065.vso"},
{     376, 0x39b39066, "VS?.? COPY001.vso"},
{     376, 0xf185e2d1, "PS?.? SKYSICLOUDS.pso"},
{     380, 0x2ec86914, "PS?.? ISBLUR2003.pso"},
{     380, 0x85f5a702, "PS2.X STFROND2001.pso [SpeedTreeFrond, fog, any light, diffuse tex, sunlight dimmer]"},
{     380, 0x88af5fb2, "VS?.? SLS2052.vso"},
{     384, 0x456de283, "PS?.? SKYTEX.pso"},
{     384, 0x61edaa69, "VS?.? SKY.vso"},
{     384, 0x711e2f7b, "PS?.? ISBLUR2000.pso"},
{     384, 0xd9590223, "PS1.3 STB1001.pso [SpeedTreeBranch, no fog, ambient & any light, diffuse & normal tex]"},
{     384, 0xd9590223, "PS1.3 STB1002.pso [SpeedTreeBranch, no fog, ambient & any light, diffuse & normal tex]"},
{     388, 0x176284d9, "PS?.? SLS1036.pso"},
{     388, 0x2469f586, "PS?.? SLS2067.pso"},
{     388, 0xf8d16b20, "PS?.? BOLT.pso"},
{     392, 0xdce6d65c, "VS?.? SLS1020.vso"},
{     396, 0x2af25fa0, "VS?.? SLS2056.vso"},
{     400, 0x03083b16, "PS?.? SLS1002.pso"},
{     400, 0x45cc5761, "PS?.? HDR007.pso"},
{     400, 0x6cad96e5, "PS?.? SLS1035.pso"},
{     404, 0x60183112, "VS?.? SM3018.vso"},
{     404, 0x7fa3defb, "VS?.? SLS2068.vso"},
{     404, 0xe8475fa9, "VS?.? SLS2053.vso"},
{     420, 0x4f0f0b63, "PS1.3 STB1005.pso [SpeedTreeBranch, no fog, point light, normal tex]"},
{     420, 0x5cfad64c, "VS?.? SLS1021.vso"},
{     424, 0x3929f599, "PS?.? SLS2048.pso"},
{     436, 0x2cb56ec5, "PS?.? WATERDISPLACE007.pso"},
{     436, 0xbe86358a, "PS?.? SLS1043.pso"},
{     436, 0xec26d659, "VS?.? SLS1004.vso"},
{     440, 0x1dabdee5, "VS?.? SM3025.vso"},
{     440, 0x41efc61e, "PS?.? PAR2022.pso"},
{     440, 0x4fc09256, "VS?.? PAR2028.vso"},
{     444, 0x04ff364b, "VS1.1 STB1000.vso [SpeedTreeBranch, no fog, full bright light]"},
{     448, 0x4f787565, "PS?.? SLS1006.pso"},
{     452, 0x3344ad3d, "PS?.? NIGHTEYE000.pso"},
{     452, 0x85b96f43, "PS?.? DISTLOD2001.pso"},
{     452, 0x85b96f43, "PS?.? GRASS2002.pso"},
{     452, 0xaa3feb6e, "PS?.? SLS1005.pso"},
{     456, 0x6ab4b27d, "PS?.? SM3030.pso"},
{     460, 0xec26d658, "VS?.? SLS1010.vso"},
{     464, 0xabe01d8e, "VS?.? SLS2057.vso"},
{     464, 0xfe009ffe, "PS?.? SLS1008.pso"},
{     468, 0x696be06d, "PS?.? SLS2064.pso"},
{     472, 0xacc1d65d, "VS?.? SLS1014.vso"},
{     472, 0xe57ddc12, "PS?.? SM3028.pso"},
{     472, 0xe57ddc12, "PS?.? SM3029.pso"},
{     472, 0xfc2dd649, "VS?.? SLS1025.vso"},
{     476, 0xc0118607, "PS?.? SLS2063.pso"},
{     480, 0x234ca32f, "PS?.? WATERDISPLACE006.pso"},
{     480, 0xc69ad74c, "VS?.? SLS2062.vso"},
{     480, 0xf26aa57e, "PS?.? REFRACT2000.pso"},
{     484, 0x9a83d7af, "PS?.? HAIR1002.pso"},
{     484, 0xbcc2d64c, "VS?.? SLS1012.vso"},
{     488, 0xdb9c3618, "VS1.1 STB1006.vso [SpeedTreeBranch, no fog, full bright light]"},
{     492, 0x5e07df80, "PS?.? MENUBG000.pso"},
{     492, 0x73e2aa75, "VS?.? SKYT.vso"},
{     500, 0xc9ea1d91, "VS?.? SM3020.vso"},
{     504, 0x6fd2dd02, "PS?.? WATERDISPLACE004.pso"},
{     508, 0x6c3d9f96, "PS?.? SLS1044.pso"},
{     508, 0x9870e885, "VS?.? SLS1017.vso"},
{     512, 0x507f0b39, "PS1.3 STB1008.pso [SpeedTreeBranch, no fog, point dir. spec, normal tex]"},
{     512, 0x9ccacf26, "PS1.3 STB1003.pso [SpeedTreeBranch, no fog, ambient & point light, diffuse & normal tex]"},
{     520, 0x1f6ff598, "PS?.? SLS2049.pso"},
{     520, 0xfc2ed648, "VS?.? SLS1026.vso"},
{     528, 0xc699d74d, "VS?.? SLS2063.vso"},
{     532, 0xee5c5379, "PS2.X HDR004.pso [combine blur-image with original surface]"},
{     532, 0xf898e89e, "VS?.? SLS1018.vso"},
{     536, 0xa6445b2a, "PS?.? SLS1009.pso"},
{     536, 0xba68d7a8, "PS?.? HAIR1000.pso"},
{     536, 0xba68d7a8, "PS?.? HAIR1001.pso"},
{     548, 0xa6d1a9c6, "PS2.X HDR006.pso [calculate target range]"},
{     548, 0xecfb64db, "PS2.X HDR000.pso [scale input averaged by 0.25]"},
{     552, 0x054c5b2e, "PS?.? SLS1011.pso"},
{     556, 0x429513f8, "PS?.? WATERHMAP001.pso"},
{     556, 0x42c013f8, "PS?.? WATERHMAP002.pso"},
{     564, 0x27445b30, "PS?.? SLS1013.pso"},
{     564, 0x30c51668, "VS?.? SLS2059.vso"},
{     568, 0xf2e89f90, "PS?.? SLS1045.pso"},
{     572, 0x387244ea, "VS?.? SKYCLOUDS.vso"},
{     576, 0x3a995b31, "PS?.? SLS1040.pso"},
{     576, 0x5d73166b, "VS?.? SKYHORIZFADE.vso"},
{     580, 0x844c5b34, "PS?.? SLS1015.pso"},
{     584, 0xab9b7ee3, "PS?.? WATERDISPLACE003.pso"},
{     584, 0xbe18a0ad, "PS?.? WATERDISPLACE002.pso"},
{     588, 0xb1aab2a3, "VS?.? SLS2031.vso"},
{     588, 0xb5505b34, "PS?.? SLS1010.pso"},
{     592, 0xf9b7b3ff, "VS?.? SLS1009.vso"},
{     596, 0xc47cba1d, "VS?.? SLS2044.vso"},
{     600, 0x52d01667, "VS?.? SM3022.vso"},
{     600, 0x9acdbb5a, "VS?.? SLS1003.vso"},
{     604, 0x16585b30, "PS?.? SLS1012.pso"},
{     608, 0xc3b75620, "PS?.? SLS1021.pso"},
{     616, 0x34505b2f, "PS?.? SLS1014.pso"},
{     624, 0x5474421b, "PS?.? SLS2075.pso"},
{     624, 0x60bf5624, "PS?.? SLS1023.pso"},
{     624, 0x836116cf, "VS1.1 STB1004.vso [SpeedTreeBranch, no fog, parallel dir. light]"},
{     628, 0x73dd9b99, "VS?.? SLS1022.vso"},
{     628, 0xd43f4a02, "PS?.? SM3035.pso"},
{     628, 0xd43f4a02, "PS?.? SM3036.pso"},
{     632, 0x5a42fd26, "PS?.? SLS2037.pso"},
{     632, 0x97585b2b, "PS?.? SLS1016.pso"},
{     632, 0xf4db7eeb, "PS?.? SLS1032.pso"},
{     636, 0x42b7563a, "PS?.? SLS1025.pso"},
{     636, 0xa44c5b37, "PS?.? SLS1041.pso"},
{     636, 0xb355ba63, "VS?.? SLS2066.vso"},
{     636, 0xd24995cd, "PS?.? SM3022.pso"},
{     640, 0x3d68ba55, "VS?.? SLS2027.vso"},
{     644, 0xc47fba1c, "VS?.? SLS2047.vso"},
{     648, 0x1e0d5232, "PS?.? SLS1001.pso"},
{     648, 0xf0e39abc, "VS?.? SLS2019.vso"},
{     652, 0xe1bf563e, "PS?.? SLS1027.pso"},
{     652, 0xf82f1474, "PS?.? GRASS2004.pso"},
{     656, 0x17ca8502, "VS?.? SLS2073.vso"},
{     656, 0x17ca8502, "VS?.? SLS2075.vso"},
{     656, 0xd45dbb4e, "VS?.? SLS1028.vso"},
{     660, 0x75db7ef1, "PS?.? SLS1033.pso"},
{     660, 0xd0a3563e, "PS?.? SLS1022.pso"},
{     664, 0xce00e879, "PS?.? SLS2068.pso"},
{     664, 0xd087e5b0, "VS?.? SM3013.vso"},
{     664, 0xd087e5b0, "VS?.? SM3014.vso"},
{     668, 0xfca716cf, "VS1.1 STB1001.vso [SpeedTreeBranch, no fog, parallel dir. light]"},
{     668, 0xfca716cf, "VS1.1 STB1002.vso [SpeedTreeBranch, no fog, parallel dir. light]"},
{     672, 0x0e885262, "PS?.? GRASS2003.pso"},
{     672, 0x63d29b8c, "VS?.? SLS1023.vso"},
{     676, 0x73ab563a, "PS?.? SLS1024.pso"},
{     676, 0xa2469b99, "VS?.? SLS1016.vso"},
{     676, 0xc1b1b28d, "VS?.? PAR2030.vso"},
{     680, 0x5608a981, "VS?.? SKYCLOUDSFADE.vso"},
{     684, 0x43d75178, "PS?.? SLS2000.pso"},
{     684, 0x5834fd35, "PS?.? SLS2051.pso"},
{     684, 0xb356ba62, "VS?.? SLS2067.vso"},
{     688, 0x51a35625, "PS?.? SLS1026.pso"},
{     688, 0xb72117c7, "VS?.? SM3000.vso"},
{     696, 0xf74d2811, "VS1.1 STB1009.vso [SpeedTreeBranch, fog, full bright light]"},
{     700, 0x1bebdfdc, "VS?.? SLS2035.vso"},
{     700, 0x3cee5232, "PS?.? SLS1007.pso"},
{     700, 0x95ca17d6, "VS?.? SM3002.vso"},
{     700, 0xdf8ac8e7, "PS?.? SLS2006.pso"},
{     704, 0x8339dfdf, "VS?.? SLS2045.vso"},
{     704, 0xd45ebb4f, "VS?.? SLS1029.vso"},
{     704, 0xda3baa59, "VS?.? SLS2064.vso"},
{     704, 0xf2ab5621, "PS?.? SLS1028.pso"},
{     708, 0xaa5dd705, "VS?.? SLS2037.vso"},
{     712, 0x006a1ba1, "VS?.? SM3009.vso"},
{     712, 0xcfa54d0f, "PS?.? SLS2027.pso"},
{     712, 0xdf26c84f, "VS?.? SLS1011.vso"},
{     716, 0xb774c97b, "VS?.? SLS2000.vso"},
{     716, 0xb774c97b, "VS?.? SLS2006.vso"},
{     720, 0x639bfd62, "PS?.? SLS2054.pso"},
{     724, 0x9fc1c84b, "VS?.? SLS1015.vso"},
{     724, 0xb80e1be5, "VS?.? SM3008.vso"},
{     728, 0xda648922, "PS?.? SLS2035.pso"},
{     736, 0x8fc2c85b, "VS?.? SLS1013.vso"},
{     736, 0xe25c899e, "PS?.? SLS2045.pso"},
{     740, 0xaca8d72a, "VS?.? SLS1019.vso"},
{     748, 0x1198b295, "VS?.? SLS1S000.vso"},
{     748, 0x1198b295, "VS?.? SLS1S005.vso"},
{     752, 0x833adfde, "VS?.? SLS2048.vso"},
{     756, 0x513a74ca, "PS?.? SLS2043.pso"},
{     756, 0x513a74ca, "PS?.? SLS2044.pso"},
{     756, 0x799fb3ea, "VS?.? HAIR1000.vso"},
{     756, 0xd4015ab3, "PS?.? SM3020.pso"},
{     756, 0xd4015ab3, "PS?.? SM3021.pso"},
{     760, 0x617fb294, "VS?.? SLS1S020.vso"},
{     764, 0x7965010c, "VS?.? SM3012.vso"},
{     764, 0xb39de29c, "PS?.? WATERHMAP000.pso"},
{     764, 0xf6a6e163, "VS1.1 STFROND000.vso [SpeedTreeLeaf, no fog, parallel dir. light]"},
{     764, 0xf9512c2d, "PS?.? SLS1017.pso"},
{     768, 0x0c6582e8, "VS?.? WATER001.vso"},
{     768, 0x3b2182e4, "VS?.? WATER000.vso"},
{     768, 0x7208b3b7, "VS?.? SLS2061.vso"},
{     772, 0x1198b296, "VS?.? SLS1S007.vso"},
{     776, 0xa6b3b3ad, "VS?.? SM3024.vso"},
{     776, 0xf98bba7a, "VS?.? SLS2029.vso"},
{     780, 0x4116b285, "VS?.? SLS1S006.vso"},
{     780, 0x5a592c29, "PS?.? SLS1019.pso"},
{     780, 0x7b80b2c7, "VS?.? HAIR2000.vso"},
{     780, 0x8517595c, "PS?.? PAR2023.pso"},
{     780, 0x86887367, "VS1.1 STB1007.vso [SpeedTreeBranch, no fog, parallel dir. spec]"},
{     780, 0xad3dbe19, "VS?.? SLS2042.vso"},
{     784, 0x34009a93, "VS?.? SLS2023.vso"},
{     788, 0x6bf0dff3, "VS?.? PAR2034.vso"},
{     788, 0xe58a7bcd, "VS1.1 STB1005.vso [SpeedTreeBranch, no fog, parallel dir. light]"},
{     792, 0x4f4edeff, "VS?.? SLS1008.vso"},
{     792, 0x9abd4f13, "PS?.? SLS2041.pso"},
{     792, 0xd207439d, "PS?.? SLS2001.pso"},
{     792, 0xe0af592b, "PS?.? PAR2000.pso"},
{     796, 0x970aff1b, "VS?.? PAR2016.vso"},
{     796, 0x970aff1b, "VS?.? SKIN2008.vso"},
{     796, 0xba1d010d, "VS?.? SLS2039.vso"},
{     796, 0xcd2a966a, "PS?.? SLS1029.pso"},
{     796, 0xe48c4ee9, "PS?.? SLS2002.pso"},
{     808, 0x58ecdfd3, "VS?.? PAR2024.vso"},
{     808, 0x58ecdfd3, "VS?.? SKIN2016.vso"},
{     808, 0x80a70fcc, "VS?.? ISHIT2002.pso"},
{     812, 0x6f8991fa, "VS?.? GDECALS.vso"},
{     816, 0xea452c31, "PS?.? SLS1018.pso"},
{     820, 0x5bc97200, "VS2.0 STB2014.vso [SpeedTreeBranch, no fog, parallel dir. spec]"},
{     820, 0x6cda4d1b, "PS?.? PAR2012.pso"},
{     828, 0xad3ebe18, "VS?.? SLS2043.vso"},
{     828, 0xc5c6c48d, "PS?.? SLS2038.pso"},
{     832, 0x494d2c35, "PS?.? SLS1020.pso"},
{     836, 0x6a1a52bc, "PS?.? SLS2028.pso"},
{     836, 0x8458b3bb, "VS?.? SM3031.vso"},
{     840, 0x78bb4ea7, "PS?.? SLS2007.pso"},
{     840, 0x92fe869b, "PS?.? SLS2039.pso"},
{     852, 0xe7fa9de6, "PS?.? SLS2015.pso"},
{     860, 0x18975a28, "VS2.0 STB2000.vso [SpeedTreeBranch, no fog, 2x point dir. light]"},
{     864, 0xdb13154d, "VS?.? PAR2000.vso"},
{     864, 0xdb13154d, "VS?.? SKIN2000.vso"},
{     864, 0xdd7b5b19, "VS1.1 STB1003.vso [SpeedTreeBranch, no fog, point dir. light]"},
{     872, 0x2f7d405b, "PS?.? GRASS2005.pso"},
{     872, 0xc7737af8, "VS2.0 STB2012.vso [SpeedTreeBranch, no fog, 2x point light]"},
{     876, 0x9ab63540, "VS?.? SLS2004.vso"},
{     880, 0x9fe98910, "PS?.? PAR2020.pso"},
{     880, 0xb63d2b89, "PS?.? SLS2003.pso"},
{     888, 0x6488faa2, "PS?.? ISBLUR2002.pso"},
{     888, 0x6488faa2, "PS?.? ISHIT2001.pso"},
{     888, 0x8bb3645f, "VS2.0 STB2017.vso [SpeedTreeBranch, fog, point light, shadowmap]"},
{     888, 0xc9e84d26, "PS?.? SLS2031.pso"},
{     892, 0x7cf71561, "VS?.? SLS2011.vso"},
{     892, 0x90a88688, "PS?.? SLS2056.pso"},
{     892, 0xd5fd4f86, "VS?.? SLS2033.vso"},
{     904, 0x47f446ba, "PS?.? PAR2001.pso"},
{     904, 0x930285e1, "VS?.? SLS2007.vso"},
{     904, 0xdc29890e, "PS?.? SLS2036.pso"},
{     908, 0xb9ad46e2, "PS?.? SLS2009.pso"},
{     916, 0x74261e55, "VS?.? SM3019.vso"},
{     920, 0x075b37d7, "PS?.? SLS2029.pso"},
{     920, 0x8eec70d2, "VS?.? SLS2054.vso"},
{     920, 0xeca69fa7, "PS?.? SLS2050.pso"},
{     920, 0xf2ff4d36, "PS?.? SKIN2004.pso"},
{     924, 0x417223de, "PS?.? SLS2004.pso"},
{     932, 0xe1881f7d, "VS2.0 STB2016.vso [SpeedTreeBranch, no fog, point spec]"},
{     936, 0x67c1891d, "PS?.? SKIN2008.pso"},
{     944, 0x596b08b3, "VS2.0 STB2004.vso [SpeedTreeBranch, fog, parallel dir. light]"},
{     944, 0x613e4ff6, "PS?.? PAR2025.pso"},
{     944, 0x9c0fdffc, "VS?.? PAR2026.vso"},
{     944, 0x9c0fdffc, "VS?.? SKIN2018.vso"},
{     944, 0xc96552a8, "PS?.? PAR2013.pso"},
{     944, 0xee0470cb, "VS?.? SLS2055.vso"},
{     948, 0xa4c6f2f0, "VS?.? SLS2021.vso"},
{     952, 0x31dd868f, "PS?.? SLS2057.pso"},
{     952, 0x51fcff1b, "VS?.? PAR2020.vso"},
{     952, 0x51fcff1b, "VS?.? SKIN2012.vso"},
{     964, 0x40a18277, "PS?.? SLS2016.pso"},
{     976, 0x7d0facde, "VS?.? SLS1S001.vso"},
{     976, 0x8dfeea5d, "VS?.? SM3004.vso"},
{     980, 0x0cec154f, "VS?.? PAR2008.vso"},
{     980, 0x20711e66, "VS1.1 STB1008.vso [SpeedTreeBranch, no fog, point spec.]"},
{     984, 0x5f8d95e6, "PS?.? PAR2008.pso"},
{     984, 0x7db422ad, "VS?.? SLS2050.vso"},
{     988, 0x05497696, "PS?.? SLS2042.pso"},
{     988, 0x4dcb22d5, "PS?.? PAR2024.pso"},
{     988, 0xaf15ea4c, "VS?.? SM3006.vso"},
{     996, 0x49b93565, "VS?.? SLS2003.vso"},
{     996, 0x6a3d4d35, "PS?.? PAR2016.pso"},
{     996, 0xcc745a07, "VS2.0 STB2002.vso [SpeedTreeBranch, no fog, 3x point dir. light]"},
{     996, 0xfe5fd90e, "VS1.1 STFROND001.vso [SpeedTreeLeaf, fog, parallel dir. light]"},
{    1000, 0x1de7acc6, "VS?.? SLS1S002.vso"},
{    1000, 0xa7f34f86, "VS?.? PAR2032.vso"},
{    1004, 0x13a53446, "PS?.? SLS2005.pso"},
{    1008, 0x137bd762, "PS?.? WATERHMAP005.pso"},
{    1008, 0x13907ad7, "VS2.0 STB2013.vso [SpeedTreeBranch, no fog, 3x point light]"},
{    1012, 0x0e602389, "PS?.? PAR2002.pso"},
{    1012, 0x6c575295, "PS?.? SLS2032.pso"},
{    1016, 0xc6663647, "VS1.1 STFROND002.vso [SpeedTreeLeaf, no fog, point dir. light]"},
{    1028, 0x03afeb04, "PS?.? WATER013.pso"},
{    1028, 0x9aa437c3, "PS?.? PAR2014.pso"},
{    1032, 0x1c355925, "PS?.? SLS2010.pso"},
{    1032, 0x7db722ac, "VS?.? SLS2051.vso"},
{    1032, 0xfd6259d2, "VS?.? PAR2004.vso"},
{    1032, 0xfd6259d2, "VS?.? SKIN2004.vso"},
{    1036, 0xb3fabfa0, "PS?.? SLS2040.pso"},
{    1040, 0x3f704ee1, "PS?.? PAR2004.pso"},
{    1044, 0x99575087, "PS?.? SKIN2000.pso"},
{    1044, 0xa2c42858, "PS?.? SLS2030.pso"},
{    1048, 0xb54c8d04, "VS?.? SLS2001.vso"},
{    1052, 0x5724e122, "PS?.? WATERDISPLACE005.pso"},
{    1056, 0x990e893d, "PS?.? PAR2021.pso"},
{    1056, 0xb93e213a, "VS?.? HAIR1002.vso"},
{    1060, 0xa8d43458, "PS?.? SLS2008.pso"},
{    1064, 0x02b4596a, "PS?.? SLS2011.pso"},
{    1068, 0x96845a33, "PS?.? SLS2053.pso"},
{    1072, 0x9bd7203a, "HAIR2001.vso"},
{    1080, 0xbb822038, "HAIR2002.vso"},
{    1084, 0x397f2134, "HAIR1001.vso"},
{    1084, 0x6025f2df, "SLS2025.vso"},
{    1088, 0x8a92e742, "SLS2018.pso"},
{    1092, 0xc288ad4e, "SLS2076.pso"},
{    1096, 0x01e937fd, "SLS2033.pso"},
{    1096, 0x5456033b, "PAR2018.vso"},
{    1096, 0x5456033b, "SKIN2010.vso"},
{    1096, 0xf8d68a76, "PAR2009.pso"},
{    1100, 0x94d55cec, "MAP000.pso"},
{    1108, 0x78bc1e44, "PRECIP001.vso"},
{    1116, 0x71043c1e, "SLS2012.pso"},
{    1120, 0x82fad483, "VS2.0 STB2008.vso [SpeedTreeBranch, fog, parallel dir. spec]"},
{    1120, 0xcf825287, "PAR2017.pso"},
{    1124, 0x64d6d4f9, "VS2.0 STB2015.vso [SpeedTreeBranch, no fog, parallel dir. spec, proj. shadow]"},
{    1128, 0x855437e8, "SKIN2005.pso"},
{    1132, 0x6d9d4408, "VS2.0 STB2006.vso [SpeedTreeBranch, fog, 2x point dir. light]"},
{    1136, 0xabf83c41, "PAR2003.pso"},
{    1144, 0x25cee8cd, "PAR2002.vso"},
{    1144, 0x25cee8cd, "SKIN2002.vso"},
{    1148, 0x503b4d13, "SKIN2006.pso"},
{    1152, 0x3f3b284c, "PAR2015.pso"},
{    1152, 0xdfb8b56d, "PRECIP000.vso"},
{    1156, 0xb3e2877b, "HAIR2006.vso"},
{    1164, 0x4505893d, "SKIN2009.pso"},
{    1164, 0x7fbe6c92, "PS2.X HDR002.pso [blur 3-tap]"},
{    1164, 0x9ae85126, "PAR2005.pso"},
{    1168, 0x7bcca1c0, "SLS2074.vso"},
{    1168, 0x9d5f015e, "SLS2077.pso"},
{    1172, 0x822ae8e1, "SLS2013.vso"},
{    1172, 0x88afbc96, "SM3015.vso"},
{    1172, 0x88afbc96, "SM3016.vso"},
{    1180, 0x4ee532c5, "VS2.0 STB2001.vso [SpeedTreeBranch, no fog, 2x point dir. light, proj. shadow]"},
{    1180, 0xaf9a59d0, "SLS2015.vso"},
{    1196, 0x176aef10, "PAR2010.pso"},
{    1200, 0x11bc8658, "HAIR1006.vso"},
{    1204, 0x6f2979b6, "SLS2009.vso"},
{    1204, 0x9d4337ee, "PAR2018.pso"},
{    1212, 0x2f0af88a, "SLS2019.pso"},
{    1212, 0x92a0033b, "PAR2022.vso"},
{    1212, 0x92a0033b, "SKIN2014.vso"},
{    1220, 0xa4762872, "SLS2034.pso"},
{    1224, 0x8f87b0ad, "DISTLOD2002.vso"},
{    1224, 0xeca6afea, "VS2.0 STB2005.vso [SpeedTreeBranch, fog, parallel dir. light, proj. shadow]"},
{    1236, 0x7c8a8216, "WATER012.pso"},
{    1240, 0x84785b20, "WATER011.pso"},
{    1240, 0x84785b20, "WATER014.pso"},
{    1240, 0xd4bc22f5, "SLS2013.pso"},
{    1244, 0x8355faa7, "VS1.1 STFROND003.vso [SpeedTreeLeaf, fog, point dir. light]"},
{    1248, 0xc80c341e, "PAR2006.pso"},
{    1260, 0xf231e8ce, "PAR2010.vso"},
{    1264, 0xf0982a38, "SKIN2001.pso"},
{    1272, 0xb12ab124, "SLS2017.pso"},
{    1272, 0xec27f1cf, "SLS2069.vso"},
{    1280, 0xdd6259fe, "PAR2012.vso"},
{    1292, 0x1c9f200b, "HAIR2004.vso"},
{    1296, 0x6f9822eb, "SLS2014.pso"},
{    1304, 0x3999f1d5, "SM3026.vso"},
{    1304, 0x521be7f6, "PRECIP003.vso"},
{    1304, 0xb9942129, "HAIR1004.vso"},
{    1308, 0x0ed8fe2c, "SLS2058.vso"},
{    1312, 0x3bc7fdbe, "WATER005.pso"},
{    1316, 0x9a0632eb, "VS2.0 STB2003.vso [SpeedTreeBranch, no fog, 3x point dir. light, proj. shadow]"},
{    1316, 0x9fd28a69, "SLS2021.pso"},
{    1320, 0x7f8e4707, "SKIN2002.pso"},
{    1320, 0xb2f2f0de, "PAR2011.pso"},
{    1324, 0x3cca200b, "HAIR2003.vso"},
{    1324, 0xef704c8a, "PRECIP002.vso"},
{    1328, 0x38dc2860, "PAR2019.pso"},
{    1332, 0x0145a517, "PAR2006.vso"},
{    1332, 0x0145a517, "SKIN2006.vso"},
{    1332, 0x39d52125, "HAIR1003.vso"},
{    1340, 0xdb64fe30, "SM3021.vso"},
{    1356, 0x26c537c8, "SKIN2007.pso"},
{    1360, 0x0565d939, "SLS2071.vso"},
{    1372, 0x6db42af5, "PAR2007.pso"},
{    1396, 0x7625d923, "SM3028.vso"},
{    1400, 0x373b7360, "VS2.0 STB2009.vso [SpeedTreeBranch, fog, parallel dir. spec, proj. shadow]"},
{    1408, 0x14ff874a, "HAIR2005.vso"},
{    1408, 0x41079862, "VS2.0 STB2010.vso [SpeedTreeBranch, fog, 2x point dir. spec]"},
{    1424, 0x03aa823e, "PAR2026.pso"},
{    1432, 0xa96b5305, "WATER004.pso"},
{    1432, 0xda82e3df, "VS2.0 STB2007.vso [SpeedTreeBranch, fog, 2x point dir. light, proj. shadow]"},
{    1440, 0x3a4a95ae, "SLS2022.pso"},
{    1448, 0x11168649, "HAIR1005.vso"},
{    1452, 0x94ccf5e9, "SLS2047.pso"},
{    1456, 0x507e720e, "SLS2078.pso"},
{    1464, 0x9c4b6c39, "DISTLOD2003.vso"},
{    1480, 0x53bda514, "SLS2017.vso"},
{    1484, 0xf812cb88, "SLS2020.pso"},
{    1496, 0xb5c867e5, "HAIR2000.pso"},
{    1496, 0xb5c867e5, "HAIR2001.pso"},
{    1496, 0xb5c867e5, "HAIR2002.pso"},
{    1500, 0xbdf884ef, "WATER003.pso"},
{    1508, 0xabe53524, "BOLT.vso"},
{    1532, 0x0f89de1d, "SLS2079.pso"},
{    1532, 0x7cd46c97, "HDR001.pso"},
{    1536, 0x3e6d53e5, "WATER006.pso"},
{    1548, 0xa6329df8, "PAR2027.pso"},
{    1548, 0xd2914a74, "WATER015.pso"},
{    1552, 0x12083d40, "SKIN2003.pso"},
{    1580, 0x2145a53b, "PAR2014.vso"},
{    1588, 0xb41b28c0, "PARTICLE.vso"},
{    1596, 0xae01e485, "SM3025.pso"},
{    1636, 0xf3cdf8c1, "PAR2028.pso"},
{    1664, 0x4da081b5, "WATER010.pso"},
{    1680, 0xf83649b0, "SM3024.pso"},
{    1700, 0x5bea6632, "GRASS2040.vso"},
{    1708, 0xf6e83e4c, "VS2.0 STB2011.vso [SpeedTreeBranch, fog, 2x point dir. spec, proj. shadow]"},
{    1744, 0xaac09c43, "WATER001.pso"},
{    1752, 0x478f24f8, "GRASS2042.vso"},
{    1756, 0xec82a175, "GRASS2020.vso"},
{    1760, 0x5675e62c, "PAR2029.pso"},
{    1780, 0xccdaa16b, "GRASS2021.vso"},
{    1784, 0x0995fbb1, "WATER009.pso"},
{    1804, 0x05c6487a, "GRASS2036.vso"},
{    1812, 0x496a13f3, "SLS2046.pso"},
{    1840, 0x508504d8, "GRASS2043.vso"},
{    1844, 0xa0b04853, "GRASS2037.vso"},
{    1860, 0xf80c4413, "WATER002.pso"},
{    1864, 0x11f5ebd2, "GRASS2024.vso"},
{    1888, 0x3152ebc7, "GRASS2025.vso"},
{    1912, 0x4935ec7b, "SLS2080.pso"},
{    1912, 0x784103ec, "GRASS2038.vso"},
{    1952, 0x0917fc79, "GRASS2028.vso"},
{    1952, 0x5d3703dc, "GRASS2039.vso"},
{    1960, 0xa29dd6c8, "PAR2029.vso"},
{    1976, 0x29b0fc61, "GRASS2029.vso"},
{    1984, 0xb6fa9260, "SLS1S004.vso"},
{    1988, 0x16e24068, "SLS2081.pso"},
{    2008, 0xb6fa9261, "SLS1S010.vso"},
{    2020, 0xf61d9267, "SLS1S014.vso"},
{    2032, 0xe61e9276, "SLS1S011.vso"},
{    2044, 0x952b4670, "GRASS2041.vso"},
{    2052, 0xae6ba133, "GRASS2022.vso"},
{    2060, 0xf490b7b5, "GRASS2032.vso"},
{    2072, 0xd666b7bc, "GRASS2033.vso"},
{    2108, 0x4c8ff6d3, "SLS2032.vso"},
{    2124, 0xaa23deb5, "SLS2020.vso"},
{    2128, 0xcad45c34, "WATER000.pso"},
{    2136, 0x753afe7a, "SLS2028.vso"},
{    2140, 0xb313f7c9, "SLS1S009.vso"},
{    2148, 0xd0eeff63, "SLS1S003.vso"},
{    2152, 0x2f2aa12d, "GRASS2023.vso"},
{    2160, 0x531ceb88, "GRASS2026.vso"},
{    2160, 0xf30b277c, "SLS2041.vso"},
{    2196, 0x2c94f6ff, "PAR2031.vso"},
{    2196, 0x533e9bff, "SLS2036.vso"},
{    2208, 0x15c71874, "SLS2038.vso"},
{    2212, 0x46097060, "WATER008.pso"},
{    2212, 0xd01c8c24, "SLS2002.vso"},
{    2220, 0x8eb1d490, "SM3010.vso"},
{    2224, 0xf81ddfad, "SLS1S016.vso"},
{    2228, 0xd4f5e76f, "VS1.1 STLEAF000.vso [SpeedTreeLeaf, no fog, parallel dir. light]"},
{    2232, 0xac5ad485, "SM3011.vso"},
{    2240, 0x72de6bd0, "SM3001.vso"},
{    2248, 0x4bfefc22, "GRASS2030.vso"},
{    2252, 0x50356bc5, "SM3003.vso"},
{    2260, 0x6e3fde9b, "SLS2024.vso"},
{    2260, 0xba958d01, "SLS1S012.vso"},
{    2260, 0xd25deb96, "GRASS2027.vso"},
{    2272, 0xb126fe54, "SLS2030.vso"},
{    2272, 0xcda2ba6a, "PAR2017.vso"},
{    2272, 0xcda2ba6a, "SKIN2009.vso"},
{    2272, 0xfa728d06, "SLS1S015.vso"},
{    2284, 0x12d39bdb, "PAR2025.vso"},
{    2284, 0x12d39bdb, "PAR2027.vso"},
{    2284, 0x12d39bdb, "SKIN2017.vso"},
{    2284, 0xea718d16, "SLS1S013.vso"},
{    2292, 0x12bf1948, "SLS1S017.vso"},
{    2304, 0x31309bfa, "PAR2035.vso"},
{    2328, 0x05f69ac9, "SLS1S008.vso"},
{    2348, 0xcabffc38, "GRASS2031.vso"},
{    2356, 0xb679b7ed, "GRASS2034.vso"},
{    2360, 0xbc6b51f7, "PAR2001.vso"},
{    2360, 0xbc6b51f7, "SKIN2001.vso"},
{    2376, 0xb532275b, "SLS2040.vso"},
{    2388, 0x1b8f51db, "SLS2012.vso"},
{    2388, 0x9d686715, "SLS2034.vso"},
{    2388, 0xd6022774, "SM3017.vso"},
{    2400, 0xf4e8c0a3, "SLS2008.vso"},
{    2408, 0x09beba44, "PAR2021.vso"},
{    2408, 0x09beba44, "SKIN2013.vso"},
{    2432, 0xeb374499, "GRASS2006.pso"},
{    2432, 0xeb374499, "GRASS2007.pso"},
{    2432, 0xeb374499, "GRASS2008.pso"},
{    2440, 0xd4da9bda, "SKIN2019.vso"},
{    2440, 0xd62e28c5, "SLS2022.vso"},
{    2456, 0x3738b7f4, "GRASS2035.vso"},
{    2460, 0x4a21d359, "VS1.1 STLEAF001.vso [SpeedTreeLeaf, fog, parallel dir. light]"},
{    2476, 0x7b9451f7, "PAR2009.vso"},
{    2480, 0x5e7cc2f7, "WATER007.pso"},
{    2492, 0x01eb1d50, "SLS2005.vso"},
{    2512, 0x5f9ebd64, "VS1.1 STLEAF002.vso [SpeedTreeLeaf, no fog, point dir. light]"},
{    2516, 0xfd736739, "PAR2033.vso"},
{    2528, 0x9a881ca2, "PAR2005.vso"},
{    2528, 0x9a881ca2, "SKIN2005.vso"},
{    2528, 0xfca4b100, "SM3005.vso"},
{    2540, 0xde4fb115, "SM3007.vso"},
{    2576, 0x123228ed, "SLS2026.vso"},
{    2592, 0x1c492a24, "PAR2019.vso"},
{    2592, 0x1c492a24, "SKIN2011.vso"},
{    2620, 0x6f99c096, "PAR2003.vso"},
{    2620, 0x6f99c096, "SKIN2003.vso"},
{    2668, 0xca68c095, "SLS2014.vso"},
{    2676, 0xd8701ca6, "SLS2016.vso"},
{    2700, 0x272e509a, "SLS2010.vso"},
{    2708, 0xda402a23, "PAR2023.vso"},
{    2708, 0xda402a23, "SKIN2015.vso"},
{    2744, 0xc148893d, "VS1.1 STLEAF003.vso [SpeedTreeLeaf, fog, point dir. light]"},
{    2756, 0xdf83482e, "SM3LL000.pso"},
{    2776, 0xba881c8c, "PAR2013.vso"},
{    2796, 0xaa73c0bf, "PAR2011.vso"},
{    2828, 0x49ba8dd2, "PAR2007.vso"},
{    2828, 0x49ba8dd2, "SKIN2007.vso"},
{    2860, 0x48004d25, "SM3LL008.pso"},
{    2892, 0xdd98a407, "SLS2072.vso"},
{    2912, 0x64ee6e6e, "SM3LL002.pso"},
{    2924, 0x19afa41f, "SM3029.vso"},
{    2956, 0xaf78f868, "SM3LL004.pso"},
{    2964, 0x82333833, "SM3023.pso"},
{    2976, 0x0b428dd5, "SLS2018.vso"},
{    2976, 0x2f0f1213, "SM3LL010.pso"},
{    3076, 0x69ba8dff, "PAR2015.vso"},
{    3080, 0xb8ac17cd, "PS?.? SM3LL018.pso"},
{    3132, 0x370754e1, "PS?.? SM3LL012.pso"},
{    3164, 0x1eeec3fb, "PS?.? SM3LL014.pso"},
{    3384, 0x7421f026, "PS?.? SM3LL001.pso"},
{    3480, 0xa84cf85b, "PS?.? SM3LL005.pso"},
{    3488, 0xa68add8a, "PS?.? SM3LL009.pso"},
{    3588, 0x85d5cb9d, "PS?.? SM3LL011.pso"},
{    3640, 0x19a3c3c5, "PS?.? SM3LL015.pso"},
{    3692, 0x579ae6dc, "PS?.? SM3LL019.pso"},
{    3796, 0x2c8ef0a7, "PS?.? SM3LL006.pso"},
{    3976, 0x03f7cbf8, "PS?.? SM3LL016.pso"},
{    4288, 0x284af0b2, "PS?.? SM3LL007.pso"},
{    4484, 0x8494cbe0, "PS?.? SM3LL017.pso"},
{    4688, 0x2e8f120c, "PS?.? SM3010.pso"},
{    5016, 0x3d12dc79, "PS?.? SM3LL003.pso"},
{    5212, 0x91b1e6a4, "PS?.? SM3LL013.pso"},
{    5788, 0x561ae6d4, "PS?.? SM3019.pso"},
{    6528, 0x75c4f03e, "PS?.? SM3001.pso"},
{    6632, 0xa76fdda8, "PS?.? SM3009.pso"},
{    6696, 0xa9a9f870, "PS?.? SM3005.pso"},
{    6732, 0x8430cb9c, "PS?.? SM3011.pso"},
{    6784, 0x1846c3d1, "PS?.? SM3015.pso"},
{    6960, 0x29eaf0a6, "PS?.? SM3007.pso"},
{    7156, 0x8534cbf2, "PS?.? SM3017.pso"},
{    7228, 0x2d6bf0a8, "PS?.? SM3006.pso"},
{    7408, 0x0232cbfa, "PS?.? SM3016.pso"},
{    7464, 0xed2d4875, "PS?.? SM3000.pso"},
{    7568, 0x7aae4d4b, "PS?.? SM3008.pso"},
{    7620, 0x56e06e0b, "PS?.? SM3002.pso"},
{    7664, 0x9ddcf80a, "PS?.? SM3004.pso"},
{    7788, 0x8a0b17b4, "PS?.? SM3018.pso"},
{    7840, 0x05095494, "PS?.? SM3012.pso"},
{    7872, 0x2cecc39b, "PS?.? SM3014.pso"},
{    8216, 0xbc92dc52, "PS?.? SM3003.pso"},
{    8412, 0x9011e691, "PS?.? SM3013.pso"},

};

const char *findShader(void *iface, UINT len, const DWORD* buf) {
  const char *name = "unknown";

  for (int g = 0; g < (sizeof(shaderDatabase) / sizeof(shaderID)); g++) {
    if (shaderDatabase[g].len < len)
      continue;
    if (shaderDatabase[g].len > len)
      return name;

    DWORD l, crc32 = 0;
    for (l = 0; l < ((len + 3) / 4); l++)
    	crc32 ^= buf[l];

    if (crc32 == shaderDatabase[g].crc32) {
      if (!shaderDatabase[g].ptr)
        shaderDatabase[g].ptr = iface;
      name = shaderDatabase[g].name;
    }
  }

  return name;
}

const char *findShader(void *iface) {
  for (int g = 0; g < (sizeof(shaderDatabase) / sizeof(shaderID)); g++) {
    if (shaderDatabase[g].ptr == iface)
      return shaderDatabase[g].name;
  }

  return (iface ? "unknown" : "NULL");
}

const struct formatID {
  D3DFORMAT fmt;
  const char *name;
} formatDatabase[] = {

{ D3DFMT_R8G8B8		    , "R8G8B8" },
{ D3DFMT_A8R8G8B8           , "A8R8G8B8" },
{ D3DFMT_X8R8G8B8           , "X8R8G8B8" },
{ D3DFMT_R5G6B5             , "R5G6B5" },
{ D3DFMT_X1R5G5B5           , "X1R5G5B5" },
{ D3DFMT_A1R5G5B5           , "A1R5G5B5" },
{ D3DFMT_A4R4G4B4           , "A4R4G4B4" },
{ D3DFMT_R3G3B2             , "R3G3B2" },
{ D3DFMT_A8                 , "A8" },
{ D3DFMT_A8R3G3B2           , "A8R3G3B2" },
{ D3DFMT_X4R4G4B4           , "X4R4G4B4" },
{ D3DFMT_A2B10G10R10        , "A2B10G10R10" },
{ D3DFMT_A8B8G8R8           , "A8B8G8R8" },
{ D3DFMT_X8B8G8R8           , "X8B8G8R8" },
{ D3DFMT_G16R16             , "G16R16" },
{ D3DFMT_A2R10G10B10        , "A2R10G10B10" },
{ D3DFMT_A16B16G16R16       , "A16B16G16R16" },
{ D3DFMT_A8P8               , "A8P8" },
{ D3DFMT_P8                 , "P8" },
{ D3DFMT_L8                 , "L8" },
{ D3DFMT_A8L8               , "A8L8" },
{ D3DFMT_A4L4               , "A4L4" },
{ D3DFMT_V8U8               , "V8U8" },
{ D3DFMT_L6V5U5             , "L6V5U5" },
{ D3DFMT_X8L8V8U8           , "X8L8V8U8" },
{ D3DFMT_Q8W8V8U8           , "Q8W8V8U8" },
{ D3DFMT_V16U16             , "V16U16" },
{ D3DFMT_A2W10V10U10        , "A2W10V10U10" },
{ D3DFMT_D16_LOCKABLE       , "D16_LOCKABLE" },
{ D3DFMT_D32                , "D32" },
{ D3DFMT_D15S1              , "D15S1" },
{ D3DFMT_D24S8              , "D24S8" },
{ D3DFMT_D24X8              , "D24X8" },
{ D3DFMT_D24X4S4            , "D24X4S4" },
{ D3DFMT_D16                , "D16" },
{ D3DFMT_D32F_LOCKABLE      , "D32F_LOCKABLE" },
{ D3DFMT_D24FS8             , "D24FS8" },
{ D3DFMT_D32_LOCKABLE       , "D32_LOCKABLE" },
{ D3DFMT_S8_LOCKABLE        , "S8_LOCKABLE" },
{ D3DFMT_L16                , "L16" },
{ D3DFMT_VERTEXDATA         , "VERTEXDATA" },
{ D3DFMT_INDEX16            , "INDEX16" },
{ D3DFMT_INDEX32            , "INDEX32" },
{ D3DFMT_Q16W16V16U16       , "Q16W16V16U16" },
{ D3DFMT_R16F               , "R16F" },
{ D3DFMT_G16R16F            , "G16R16F" },
{ D3DFMT_A16B16G16R16F      , "A16B16G16R16F" },
{ D3DFMT_R32F               , "R32F" },
{ D3DFMT_G32R32F            , "G32R32F" },
{ D3DFMT_A32B32G32R32F      , "A32B32G32R32F" },
{ D3DFMT_CxV8U8             , "CxV8U8" },
{ D3DFMT_A1                 , "A1" },
{ D3DFMT_A2B10G10R10_XR_BIAS, "A2B10G10R10_XR_BIAS" },
{ D3DFMT_BINARYBUFFER       , "BINARYBUFFER" },

{ D3DFMT_MULTI2_ARGB8       , "MULTI2_ARGB8" },
{ D3DFMT_UYVY               , "UYVY" },
{ D3DFMT_R8G8_B8G8          , "R8G8_B8G8" },
{ D3DFMT_YUY2               , "YUY2" },
{ D3DFMT_G8R8_G8B8          , "G8R8_G8B8" },
{ D3DFMT_DXT1               , "DXT1" },
{ D3DFMT_DXT2               , "DXT2" },
{ D3DFMT_DXT3               , "DXT3" },
{ D3DFMT_DXT4               , "DXT4" },
{ D3DFMT_DXT5               , "DXT5" },

{ (D3DFORMAT)MAKEFOURCC('I','N','T','Z'), "INTZ" },
{ (D3DFORMAT)MAKEFOURCC('D','F','2','4'), "DF24" },
{ (D3DFORMAT)MAKEFOURCC('D','F','1','6'), "DF16" },
{ (D3DFORMAT)MAKEFOURCC('R','A','W','Z'), "RAWZ" },

};

const char *findFormat(D3DFORMAT fmt) {
  for (int g = 0; g < (sizeof(formatDatabase) / sizeof(formatID)); g++) {
    if (formatDatabase[g].fmt == fmt)
      return formatDatabase[g].name;
  }

  return "unknown";
}

const struct usageID {
  DWORD use;
  const char *name;
} usageDatabase[] = {

{ 0x00000001L, "Rendertarget"},
{ 0x00000002L, "Depthstencil"},
{ 0x00000008L, "Writeonly"},
{ 0x00000010L, "Softwareprocessing"},
{ 0x00000020L, "Donotclip"},
{ 0x00000040L, "Points"},
{ 0x00000080L, "Rtpatches"},
{ 0x00000100L, "Npatches"},
{ 0x00000200L, "Dynamic"},
{ 0x00000400L, "Autogenmipmap"},
{ 0x00000800L, "Restricted Content"},
{ 0x00001000L, "Restrict_Shared Resource Driver"},
{ 0x00002000L, "Restrict_Shared Resource"},
{ 0x00004000L, "Dmap"},
{ 0x00008000L, "Query Legacybumpmap"},
{ 0x00200000L, "Query Wrapandmip"},
{ 0x00800000L, "Nonsecure"},
{ 0x10000000L, "Textapi"},

};

static char usestr[2048];
const char *findUsage(DWORD use) {
  usestr[0] = '\0';

  for (int u = 0; u < (sizeof(usageDatabase) / sizeof(usageID)); u++) {
    if (usageDatabase[u].use & use) {
      if (usestr[0] != '\0')
        strcat_s(usestr, " ");
      strcat_s(usestr, usageDatabase[u].name);
    }
  }

  if (usestr[0] != '\0')
    return usestr;
  return "Plain";
}

const struct texturestateID {
  D3DTEXTURESTAGESTATETYPE tstate;
  const char *name;
} texturestateDatabase[] = {

{ D3DTSS_COLOROP        	, "COLOROP"        	  }, /* D3DTEXTUREOP - per-stage blending controls for color channels */
{ D3DTSS_COLORARG1      	, "COLORARG1"      	  }, /* D3DTA_* (texture arg) */
{ D3DTSS_COLORARG2      	, "COLORARG2"      	  }, /* D3DTA_* (texture arg) */
{ D3DTSS_ALPHAOP        	, "ALPHAOP"        	  }, /* D3DTEXTUREOP - per-stage blending controls for alpha channel */
{ D3DTSS_ALPHAARG1      	, "ALPHAARG1"      	  }, /* D3DTA_* (texture arg) */
{ D3DTSS_ALPHAARG2      	, "ALPHAARG2"      	  }, /* D3DTA_* (texture arg) */
{ D3DTSS_BUMPENVMAT00   	, "BUMPENVMAT00"   	  }, /* float (bump mapping matrix) */
{ D3DTSS_BUMPENVMAT01   	, "BUMPENVMAT01"   	  }, /* float (bump mapping matrix) */
{ D3DTSS_BUMPENVMAT10   	, "BUMPENVMAT10"   	  }, /* float (bump mapping matrix) */
{ D3DTSS_BUMPENVMAT11   	, "BUMPENVMAT11"   	  }, /* float (bump mapping matrix) */
{ D3DTSS_TEXCOORDINDEX  	, "TEXCOORDINDEX"  	  }, /* identifies which set of texture coordinates index this texture */
{ D3DTSS_BUMPENVLSCALE  	, "BUMPENVLSCALE"  	  }, /* float scale for bump map luminance */
{ D3DTSS_BUMPENVLOFFSET 	, "BUMPENVLOFFSET" 	  }, /* float offset for bump map luminance */
{ D3DTSS_TEXTURETRANSFORMFLAGS	, "TEXTURETRANSFORMFLAGS" }, /* D3DTEXTURETRANSFORMFLAGS controls texture transform */
{ D3DTSS_COLORARG0      	, "COLORARG0"      	  }, /* D3DTA_* third arg for triadic ops */
{ D3DTSS_ALPHAARG0      	, "ALPHAARG0"      	  }, /* D3DTA_* third arg for triadic ops */
{ D3DTSS_RESULTARG      	, "RESULTARG"      	  }, /* D3DTA_* arg for result (CURRENT or TEMP) */
{ D3DTSS_CONSTANT       	, "CONSTANT"       	  }, /* Per-stage constant D3DTA_CONSTANT */

};

const char *findTextureState(D3DTEXTURESTAGESTATETYPE tstate) {
  for (int g = 0; g < (sizeof(texturestateDatabase) / sizeof(texturestateID)); g++) {
    if (texturestateDatabase[g].tstate == tstate)
      return texturestateDatabase[g].name;
  }

  return "unknown";
}

const struct samplerstateID {
  D3DSAMPLERSTATETYPE sstate;
  const char *name;
} samplerstateDatabase[] = {

{ D3DSAMP_ADDRESSU       , "ADDRESSU"      }, /* D3DTEXTUREADDRESS for U coordinate */
{ D3DSAMP_ADDRESSV       , "ADDRESSV"      }, /* D3DTEXTUREADDRESS for V coordinate */
{ D3DSAMP_ADDRESSW       , "ADDRESSW"      }, /* D3DTEXTUREADDRESS for W coordinate */
{ D3DSAMP_BORDERCOLOR    , "BORDERCOLOR"   }, /* D3DCOLOR */
{ D3DSAMP_MAGFILTER      , "MAGFILTER"     }, /* D3DTEXTUREFILTER filter to use for magnification */
{ D3DSAMP_MINFILTER      , "MINFILTER"     }, /* D3DTEXTUREFILTER filter to use for minification */
{ D3DSAMP_MIPFILTER      , "MIPFILTER"     }, /* D3DTEXTUREFILTER filter to use between mipmaps during minification */
{ D3DSAMP_MIPMAPLODBIAS  , "MIPMAPLODBIAS" }, /* float Mipmap LOD bias */
{ D3DSAMP_MAXMIPLEVEL    , "MAXMIPLEVEL"   }, /* DWORD 0..(n-1) LOD index of largest map to use (0 == largest) */
{ D3DSAMP_MAXANISOTROPY  , "MAXANISOTROPY" }, /* DWORD maximum anisotropy */
{ D3DSAMP_SRGBTEXTURE    , "SRGBTEXTURE"   }, /* Default = 0 (which means Gamma 1.0, no correction required.) else correct for Gamma = 2.2 */
{ D3DSAMP_ELEMENTINDEX   , "ELEMENTINDEX"  }, /* When multi-element texture is assigned to sampler, this indicates which element index to use.  Default = 0.  */
{ D3DSAMP_DMAPOFFSET     , "DMAPOFFSET"    }, /* Offset in vertices in the pre-sampled displacement map. Only valid for D3DDMAPSAMPLER sampler  */

};

const char *findSamplerState(D3DSAMPLERSTATETYPE sstate) {
  for (int g = 0; g < (sizeof(samplerstateDatabase) / sizeof(samplerstateID)); g++) {
    if (samplerstateDatabase[g].sstate == sstate)
      return samplerstateDatabase[g].name;
  }

  return "unknown";
}

const char *findSamplerStateValue(D3DSAMPLERSTATETYPE sstate, DWORD svalue) {
  static char buf[256];

	      switch ((D3DSAMPLERSTATETYPE)sstate) {
	      	case D3DSAMP_ADDRESSU:
	      	case D3DSAMP_ADDRESSV:
		case D3DSAMP_ADDRESSW:
		  switch ((D3DTEXTUREADDRESS)svalue) {
		    case D3DTADDRESS_WRAP:       strcpy(buf, "WRAP"); break;
		    case D3DTADDRESS_MIRROR:     strcpy(buf, "MIRROR"); break;
		    case D3DTADDRESS_CLAMP:      strcpy(buf, "CLAMP"); break;
		    case D3DTADDRESS_BORDER:     strcpy(buf, "BORDER"); break;
		    case D3DTADDRESS_MIRRORONCE: strcpy(buf, "MIRRORONCE"); break;
		    default: sprintf(buf, "%d", svalue); break;
		  }
	      	  break;
	      	// hex
	      	case D3DSAMP_BORDERCOLOR:
	          sprintf(buf, "0x%08x", svalue);
	      	  break;
	      	case D3DSAMP_MAGFILTER:
	      	case D3DSAMP_MINFILTER:
	      	case D3DSAMP_MIPFILTER:
		  switch ((D3DTEXTUREFILTERTYPE)svalue) {
		    case D3DTEXF_NONE:          strcpy(buf, "NONE"); break;
		    case D3DTEXF_POINT:         strcpy(buf, "POINT"); break;
		    case D3DTEXF_LINEAR:        strcpy(buf, "LINEAR"); break;
		    case D3DTEXF_ANISOTROPIC:   strcpy(buf, "ANISOTROPIC"); break;
		    case D3DTEXF_PYRAMIDALQUAD: strcpy(buf, "PYRAMIDALQUAD"); break;
		    case D3DTEXF_GAUSSIANQUAD:  strcpy(buf, "GAUSSIANQUAD"); break;
		    default: sprintf(buf, "%d", svalue); break;
		  }
	      	  break;
	      	// float
	      	case D3DSAMP_MIPMAPLODBIAS:	// deviation <>1.0
	      	case D3DSAMP_SRGBTEXTURE:	// gamma <>1.0
	          sprintf(buf, "%f", *((float *)&svalue));
	      	  break;
	      	// int
	      	case D3DSAMP_MAXMIPLEVEL:
	      	case D3DSAMP_MAXANISOTROPY:
	      	case D3DSAMP_ELEMENTINDEX:
	          sprintf(buf, "%d", svalue);
	      	  break;
	      	// long
	      	case D3DSAMP_DMAPOFFSET:
	          sprintf(buf, "%d", svalue);
	      	  break;
	      	// unknown
	      	default:
	          sprintf(buf, "0x%08x", svalue);
	      	  break;
	      }

  return buf;
}

const struct renderstateID {
  D3DRENDERSTATETYPE rstate;
  const char *name;
} renderstateDatabase[] = {

{ D3DRS_ZENABLE                   , "ZENABLE"                    },   /* D3DZBUFFERTYPE (or TRUE/FALSE for legacy) */
{ D3DRS_FILLMODE                  , "FILLMODE"                   },   /* D3DFILLMODE */
{ D3DRS_SHADEMODE                 , "SHADEMODE"                  },   /* D3DSHADEMODE */
{ D3DRS_ZWRITEENABLE              , "ZWRITEENABLE"               },   /* TRUE to enable z writes */
{ D3DRS_ALPHATESTENABLE           , "ALPHATESTENABLE"            },   /* TRUE to enable alpha tests */
{ D3DRS_LASTPIXEL                 , "LASTPIXEL"                  },   /* TRUE for last-pixel on lines */
{ D3DRS_SRCBLEND                  , "SRCBLEND"                   },   /* D3DBLEND */
{ D3DRS_DESTBLEND                 , "DESTBLEND"                  },   /* D3DBLEND */
{ D3DRS_CULLMODE                  , "CULLMODE"                   },   /* D3DCULL */
{ D3DRS_ZFUNC                     , "ZFUNC"                      },   /* D3DCMPFUNC */
{ D3DRS_ALPHAREF                  , "ALPHAREF"                   },   /* D3DFIXED */
{ D3DRS_ALPHAFUNC                 , "ALPHAFUNC"                  },   /* D3DCMPFUNC */
{ D3DRS_DITHERENABLE              , "DITHERENABLE"               },   /* TRUE to enable dithering */
{ D3DRS_ALPHABLENDENABLE          , "ALPHABLENDENABLE"           },   /* TRUE to enable alpha blending */
{ D3DRS_FOGENABLE                 , "FOGENABLE"                  },   /* TRUE to enable fog blending */
{ D3DRS_SPECULARENABLE            , "SPECULARENABLE"             },   /* TRUE to enable specular */
{ D3DRS_FOGCOLOR                  , "FOGCOLOR"                   },   /* D3DCOLOR */
{ D3DRS_FOGTABLEMODE              , "FOGTABLEMODE"               },   /* D3DFOGMODE */
{ D3DRS_FOGSTART                  , "FOGSTART"                   },   /* Fog start (for both vertex and pixel fog) */
{ D3DRS_FOGEND                    , "FOGEND"                     },   /* Fog end      */
{ D3DRS_FOGDENSITY                , "FOGDENSITY"                 },   /* Fog density  */
{ D3DRS_RANGEFOGENABLE            , "RANGEFOGENABLE"             },   /* Enables range-based fog */
{ D3DRS_STENCILENABLE             , "STENCILENABLE"              },   /* BOOL enable/disable stenciling */
{ D3DRS_STENCILFAIL               , "STENCILFAIL"                },   /* D3DSTENCILOP to do if stencil test fails */
{ D3DRS_STENCILZFAIL              , "STENCILZFAIL"               },   /* D3DSTENCILOP to do if stencil test passes and Z test fails */
{ D3DRS_STENCILPASS               , "STENCILPASS"                },   /* D3DSTENCILOP to do if both stencil and Z tests pass */
{ D3DRS_STENCILFUNC               , "STENCILFUNC"                },   /* D3DCMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
{ D3DRS_STENCILREF                , "STENCILREF"                 },   /* Reference value used in stencil test */
{ D3DRS_STENCILMASK               , "STENCILMASK"                },   /* Mask value used in stencil test */
{ D3DRS_STENCILWRITEMASK          , "STENCILWRITEMASK"           },   /* Write mask applied to values written to stencil buffer */
{ D3DRS_TEXTUREFACTOR             , "TEXTUREFACTOR"              },   /* D3DCOLOR used for multi-texture blend */
{ D3DRS_WRAP0                     , "WRAP0"                      },  /* wrap for 1st texture coord. set */
{ D3DRS_WRAP1                     , "WRAP1"                      },  /* wrap for 2nd texture coord. set */
{ D3DRS_WRAP2                     , "WRAP2"                      },  /* wrap for 3rd texture coord. set */
{ D3DRS_WRAP3                     , "WRAP3"                      },  /* wrap for 4th texture coord. set */
{ D3DRS_WRAP4                     , "WRAP4"                      },  /* wrap for 5th texture coord. set */
{ D3DRS_WRAP5                     , "WRAP5"                      },  /* wrap for 6th texture coord. set */
{ D3DRS_WRAP6                     , "WRAP6"                      },  /* wrap for 7th texture coord. set */
{ D3DRS_WRAP7                     , "WRAP7"                      },  /* wrap for 8th texture coord. set */
{ D3DRS_CLIPPING                  , "CLIPPING"                   },
{ D3DRS_LIGHTING                  , "LIGHTING"                   },
{ D3DRS_AMBIENT                   , "AMBIENT"                    },
{ D3DRS_FOGVERTEXMODE             , "FOGVERTEXMODE"              },
{ D3DRS_COLORVERTEX               , "COLORVERTEX"                },
{ D3DRS_LOCALVIEWER               , "LOCALVIEWER"                },
{ D3DRS_NORMALIZENORMALS          , "NORMALIZENORMALS"           },
{ D3DRS_DIFFUSEMATERIALSOURCE     , "DIFFUSEMATERIALSOURCE"      },
{ D3DRS_SPECULARMATERIALSOURCE    , "SPECULARMATERIALSOURCE"     },
{ D3DRS_AMBIENTMATERIALSOURCE     , "AMBIENTMATERIALSOURCE"      },
{ D3DRS_EMISSIVEMATERIALSOURCE    , "EMISSIVEMATERIALSOURCE"     },
{ D3DRS_VERTEXBLEND               , "VERTEXBLEND"                },
{ D3DRS_CLIPPLANEENABLE           , "CLIPPLANEENABLE"            },
{ D3DRS_POINTSIZE                 , "POINTSIZE"                  },   /* float point size */
{ D3DRS_POINTSIZE_MIN             , "POINTSIZE_MIN"              },   /* float point size min threshold */
{ D3DRS_POINTSPRITEENABLE         , "POINTSPRITEENABLE"          },   /* BOOL point texture coord control */
{ D3DRS_POINTSCALEENABLE          , "POINTSCALEENABLE"           },   /* BOOL point size scale enable */
{ D3DRS_POINTSCALE_A              , "POINTSCALE_A"               },   /* float point attenuation A value */
{ D3DRS_POINTSCALE_B              , "POINTSCALE_B"               },   /* float point attenuation B value */
{ D3DRS_POINTSCALE_C              , "POINTSCALE_C"               },   /* float point attenuation C value */
{ D3DRS_MULTISAMPLEANTIALIAS      , "MULTISAMPLEANTIALIAS"       },  // BOOL - set to do FSAA with multisample buffer
{ D3DRS_MULTISAMPLEMASK           , "MULTISAMPLEMASK"            },  // DWORD - per-sample enable/disable
{ D3DRS_PATCHEDGESTYLE            , "PATCHEDGESTYLE"             },  // Sets whether patch edges will use float style tessellation
{ D3DRS_DEBUGMONITORTOKEN         , "DEBUGMONITORTOKEN"          },  // DEBUG ONLY - token to debug monitor
{ D3DRS_POINTSIZE_MAX             , "POINTSIZE_MAX"              },   /* float point size max threshold */
{ D3DRS_INDEXEDVERTEXBLENDENABLE  , "INDEXEDVERTEXBLENDENABLE"   },
{ D3DRS_COLORWRITEENABLE          , "COLORWRITEENABLE"           },  // per-channel write enable
{ D3DRS_TWEENFACTOR               , "TWEENFACTOR"                },   // float tween factor
{ D3DRS_BLENDOP                   , "BLENDOP"                    },   // D3DBLENDOP setting
{ D3DRS_POSITIONDEGREE            , "POSITIONDEGREE"             },   // NPatch position interpolation degree. D3DDEGREE_LINEAR or D3DDEGREE_CUBIC (default)
{ D3DRS_NORMALDEGREE              , "NORMALDEGREE"               },   // NPatch normal interpolation degree. D3DDEGREE_LINEAR (default) or D3DDEGREE_QUADRATIC
{ D3DRS_SCISSORTESTENABLE         , "SCISSORTESTENABLE"          },
{ D3DRS_SLOPESCALEDEPTHBIAS       , "SLOPESCALEDEPTHBIAS"        },
{ D3DRS_ANTIALIASEDLINEENABLE     , "ANTIALIASEDLINEENABLE"      },
{ D3DRS_MINTESSELLATIONLEVEL      , "MINTESSELLATIONLEVEL"       },
{ D3DRS_MAXTESSELLATIONLEVEL      , "MAXTESSELLATIONLEVEL"       },
{ D3DRS_ADAPTIVETESS_X            , "ADAPTIVETESS_X"             },
{ D3DRS_ADAPTIVETESS_Y            , "ADAPTIVETESS_Y"             },
{ D3DRS_ADAPTIVETESS_Z            , "ADAPTIVETESS_Z"             },
{ D3DRS_ADAPTIVETESS_W            , "ADAPTIVETESS_W"             },
{ D3DRS_ENABLEADAPTIVETESSELLATION, "ENABLEADAPTIVETESSELLATION" },
{ D3DRS_TWOSIDEDSTENCILMODE       , "TWOSIDEDSTENCILMODE"        },   /* BOOL enable/disable 2 sided stenciling */
{ D3DRS_CCW_STENCILFAIL           , "CCW_STENCILFAIL"            },   /* D3DSTENCILOP to do if ccw stencil test fails */
{ D3DRS_CCW_STENCILZFAIL          , "CCW_STENCILZFAIL"           },   /* D3DSTENCILOP to do if ccw stencil test passes and Z test fails */
{ D3DRS_CCW_STENCILPASS           , "CCW_STENCILPASS"            },   /* D3DSTENCILOP to do if both ccw stencil and Z tests pass */
{ D3DRS_CCW_STENCILFUNC           , "CCW_STENCILFUNC"            },   /* D3DCMPFUNC fn.  ccw Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
{ D3DRS_COLORWRITEENABLE1         , "COLORWRITEENABLE1"          },   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
{ D3DRS_COLORWRITEENABLE2         , "COLORWRITEENABLE2"          },   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
{ D3DRS_COLORWRITEENABLE3         , "COLORWRITEENABLE3"          },   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
{ D3DRS_BLENDFACTOR               , "BLENDFACTOR"                },   /* D3DCOLOR used for a constant blend factor during alpha blending for devices that support D3DPBLENDCAPS_BLENDFACTOR */
{ D3DRS_SRGBWRITEENABLE           , "SRGBWRITEENABLE"            },   /* Enable rendertarget writes to be DE-linearized to SRGB (for formats that expose D3DUSAGE_QUERY_SRGBWRITE) */
{ D3DRS_DEPTHBIAS                 , "DEPTHBIAS"                  },
{ D3DRS_WRAP8                     , "WRAP8"                      },   /* Additional wrap states for vs_3_0+ attributes with D3DDECLUSAGE_TEXCOORD */
{ D3DRS_WRAP9                     , "WRAP9"                      },
{ D3DRS_WRAP10                    , "WRAP10"                     },
{ D3DRS_WRAP11                    , "WRAP11"                     },
{ D3DRS_WRAP12                    , "WRAP12"                     },
{ D3DRS_WRAP13                    , "WRAP13"                     },
{ D3DRS_WRAP14                    , "WRAP14"                     },
{ D3DRS_WRAP15                    , "WRAP15"                     },
{ D3DRS_SEPARATEALPHABLENDENABLE  , "SEPARATEALPHABLENDENABLE"   },  /* TRUE to enable a separate blending function for the alpha channel */
{ D3DRS_SRCBLENDALPHA             , "SRCBLENDALPHA"              },  /* SRC blend factor for the alpha channel when D3DRS_SEPARATEDESTALPHAENABLE is TRUE */
{ D3DRS_DESTBLENDALPHA            , "DESTBLENDALPHA"             },  /* DST blend factor for the alpha channel when D3DRS_SEPARATEDESTALPHAENABLE is TRUE */
{ D3DRS_BLENDOPALPHA              , "BLENDOPALPHA"               },  /* Blending operation for the alpha channel when D3DRS_SEPARATEDESTALPHAENABLE is TRUE */

};

const char *findRenderState(D3DRENDERSTATETYPE rstate) {
  for (int g = 0; g < (sizeof(renderstateDatabase) / sizeof(renderstateID)); g++) {
    if (renderstateDatabase[g].rstate == rstate)
      return renderstateDatabase[g].name;
  }

  return "unknown";
}

const char *findRenderStateValue(D3DRENDERSTATETYPE rstate, DWORD rvalue) {
  static char buf[256];

	  switch ((D3DRENDERSTATETYPE)rstate) {
	    case D3DRS_ZENABLE:
	      switch ((D3DZBUFFERTYPE)rvalue) {
		case D3DZB_FALSE: strcpy(buf, "FALSE"); break;
		case D3DZB_TRUE:  strcpy(buf, "TRUE"); break;
		case D3DZB_USEW:  strcpy(buf, "USEW"); break;
		default: sprintf(buf, "%d", rvalue); break;
	      }
	      break;
	    case D3DRS_FILLMODE:
	      switch ((D3DFILLMODE)rvalue) {
		case D3DFILL_POINT:     strcpy(buf, "POINT"); break;
		case D3DFILL_WIREFRAME: strcpy(buf, "WIREFRAME"); break;
		case D3DFILL_SOLID:     strcpy(buf, "SOLID"); break;
		default: sprintf(buf, "%d", rvalue); break;
	      }
	      break;
	    case D3DRS_SHADEMODE:
	      switch ((D3DSHADEMODE)rvalue) {
		case D3DSHADE_FLAT:    strcpy(buf, "FLAT"); break;
		case D3DSHADE_GOURAUD: strcpy(buf, "GOURAUD"); break;
		case D3DSHADE_PHONG:   strcpy(buf, "PHONG"); break;
		default: sprintf(buf, "%d", rvalue); break;
	      }
	      break;
	    case D3DRS_SRCBLEND:
	    case D3DRS_DESTBLEND:
	      switch ((D3DBLEND)rvalue) {
	        case D3DBLEND_ZERO           : strcpy(buf, "ZERO"); break;
	        case D3DBLEND_ONE            : strcpy(buf, "ONE"); break;
	        case D3DBLEND_SRCCOLOR       : strcpy(buf, "SRCCOLOR"); break;
	        case D3DBLEND_INVSRCCOLOR    : strcpy(buf, "INVSRCCOLOR"); break;
	        case D3DBLEND_SRCALPHA       : strcpy(buf, "SRCALPHA"); break;
	        case D3DBLEND_INVSRCALPHA    : strcpy(buf, "INVSRCALPHA"); break;
	        case D3DBLEND_DESTALPHA      : strcpy(buf, "DESTALPHA"); break;
	        case D3DBLEND_INVDESTALPHA   : strcpy(buf, "INVDESTALPHA"); break;
	        case D3DBLEND_DESTCOLOR      : strcpy(buf, "DESTCOLOR"); break;
	        case D3DBLEND_INVDESTCOLOR   : strcpy(buf, "INVDESTCOLOR"); break;
	        case D3DBLEND_SRCALPHASAT    : strcpy(buf, "SRCALPHASAT"); break;
	        case D3DBLEND_BOTHSRCALPHA   : strcpy(buf, "BOTHSRCALPHA"); break;
	        case D3DBLEND_BOTHINVSRCALPHA: strcpy(buf, "BOTHINVSRCALPHA"); break;
	        case D3DBLEND_BLENDFACTOR    : strcpy(buf, "BLENDFACTOR"); break;
	        case D3DBLEND_INVBLENDFACTOR : strcpy(buf, "INVBLENDFACTOR"); break;
	        case D3DBLEND_SRCCOLOR2      : strcpy(buf, "SRCCOLOR2"); break;
	        case D3DBLEND_INVSRCCOLOR2   : strcpy(buf, "INVSRCCOLOR2"); break;
		default: sprintf(buf, "%d", rvalue); break;
	      }
	      break;
	    case D3DRS_CULLMODE:
	      switch ((D3DCULL)rvalue) {
		case D3DCULL_NONE: strcpy(buf, "NONE"); break;
		case D3DCULL_CW:   strcpy(buf, "CW"); break;
		case D3DCULL_CCW:  strcpy(buf, "CCW"); break;
		default: sprintf(buf, "%d", rvalue); break;
	      }
	      break;
	    case D3DRS_FOGTABLEMODE:
	    case D3DRS_FOGVERTEXMODE:
	      switch ((D3DFOGMODE)rvalue) {
		case D3DFOG_NONE:   strcpy(buf, "NONE"); break;
		case D3DFOG_EXP:    strcpy(buf, "EXP"); break;
		case D3DFOG_EXP2:   strcpy(buf, "EXP2"); break;
		case D3DFOG_LINEAR: strcpy(buf, "LINEAR"); break;
		default: sprintf(buf, "%d", rvalue); break;
	      }
	      break;
	    case D3DRS_ZFUNC:
	    case D3DRS_ALPHAFUNC:
	    case D3DRS_STENCILFUNC:
	    case D3DRS_CCW_STENCILFUNC:
	      switch ((D3DCMPFUNC)rvalue) {
		case D3DCMP_NEVER       : strcpy(buf, "NEVER"); break;
		case D3DCMP_LESS        : strcpy(buf, "LESS"); break;
		case D3DCMP_EQUAL       : strcpy(buf, "EQUAL"); break;
		case D3DCMP_LESSEQUAL   : strcpy(buf, "LESSEQUAL"); break;
		case D3DCMP_GREATER     : strcpy(buf, "GREATER"); break;
		case D3DCMP_NOTEQUAL    : strcpy(buf, "NOTEQUAL"); break;
		case D3DCMP_GREATEREQUAL: strcpy(buf, "GREATEREQUAL"); break;
		case D3DCMP_ALWAYS      : strcpy(buf, "ALWAYS"); break;
		default: sprintf(buf, "%d", rvalue); break;
	      }
	      break;
	    case D3DRS_BLENDOP:
	    case D3DRS_SRCBLENDALPHA:
	    case D3DRS_DESTBLENDALPHA:
	    case D3DRS_BLENDOPALPHA:
	      switch ((D3DBLENDOP)rvalue) {
	        case D3DBLENDOP_ADD        : strcpy(buf, "ADD"); break;
	        case D3DBLENDOP_SUBTRACT   : strcpy(buf, "SUBTRACT"); break;
	        case D3DBLENDOP_REVSUBTRACT: strcpy(buf, "REVSUBTRACT"); break;
	        case D3DBLENDOP_MIN        : strcpy(buf, "MIN"); break;
	        case D3DBLENDOP_MAX        : strcpy(buf, "MAX"); break;
		default: sprintf(buf, "%d", rvalue); break;
	      }
	      break;
	    case D3DRS_STENCILFAIL:
	    case D3DRS_STENCILZFAIL:
	    case D3DRS_STENCILPASS:
	    case D3DRS_CCW_STENCILFAIL:
	    case D3DRS_CCW_STENCILZFAIL:
	    case D3DRS_CCW_STENCILPASS:
	      switch ((D3DSTENCILOP)rvalue) {
	        case D3DSTENCILOP_KEEP   : strcpy(buf, "KEEP"); break;
	        case D3DSTENCILOP_ZERO   : strcpy(buf, "ZERO"); break;
	        case D3DSTENCILOP_REPLACE: strcpy(buf, "REPLACE"); break;
	        case D3DSTENCILOP_INCRSAT: strcpy(buf, "INCRSAT"); break;
	        case D3DSTENCILOP_DECRSAT: strcpy(buf, "DECRSAT"); break;
	        case D3DSTENCILOP_INVERT : strcpy(buf, "INVERT"); break;
	        case D3DSTENCILOP_INCR   : strcpy(buf, "INCR"); break;
	        case D3DSTENCILOP_DECR   : strcpy(buf, "DECR"); break;
		default: sprintf(buf, "%d", rvalue); break;
	      }
	      break;
	    // bool
	    case D3DRS_ZWRITEENABLE:
	    case D3DRS_ALPHATESTENABLE:
	    case D3DRS_LASTPIXEL:
	    case D3DRS_DITHERENABLE:
	    case D3DRS_ALPHABLENDENABLE:
	    case D3DRS_FOGENABLE:
	    case D3DRS_SPECULARENABLE:
	    case D3DRS_RANGEFOGENABLE:
	    case D3DRS_STENCILENABLE:
	    case D3DRS_CLIPPING:
	    case D3DRS_LIGHTING:
	    case D3DRS_COLORVERTEX:
	    case D3DRS_LOCALVIEWER:
	    case D3DRS_NORMALIZENORMALS:
	    case D3DRS_POINTSPRITEENABLE:
	    case D3DRS_POINTSCALEENABLE:
	    case D3DRS_MULTISAMPLEANTIALIAS:
	    case D3DRS_INDEXEDVERTEXBLENDENABLE:
	    case D3DRS_ANTIALIASEDLINEENABLE:
	    case D3DRS_TWOSIDEDSTENCILMODE:
	    case D3DRS_SRGBWRITEENABLE:
	    case D3DRS_SEPARATEALPHABLENDENABLE:
	      strcpy(buf, rvalue ? "TRUE" : "FALSE");
	      break;
	    // float
	    case D3DRS_FOGSTART:
	    case D3DRS_FOGEND:
	    case D3DRS_FOGDENSITY:
	    case D3DRS_POINTSIZE:
	    case D3DRS_POINTSIZE_MIN:
	    case D3DRS_POINTSCALE_A:
	    case D3DRS_POINTSCALE_B:
	    case D3DRS_POINTSCALE_C:
	    case D3DRS_POINTSIZE_MAX:
	    case D3DRS_TWEENFACTOR:
	    case D3DRS_DEPTHBIAS:
	      sprintf(buf, "%f", *((float *)&rvalue));
	      break;
	    // long
	    case D3DRS_ALPHAREF:
	    case D3DRS_STENCILREF:
	    case D3DRS_VERTEXBLEND:
	      sprintf(buf, "%d", rvalue);
	      break;
	    // mask
	    case D3DRS_COLORWRITEENABLE:
	      sprintf(buf, "0x%02x", rvalue);
	      break;
	    // hex
	    case D3DRS_STENCILMASK:
	    case D3DRS_STENCILWRITEMASK:
	    case D3DRS_CLIPPLANEENABLE:
	    case D3DRS_MULTISAMPLEMASK:
	      sprintf(buf, "0x%08x", rvalue);
	      break;
	    // unknown
	    case D3DRS_FOGCOLOR:
	    case D3DRS_BLENDFACTOR:
	    case D3DRS_TEXTUREFACTOR:
	    case D3DRS_AMBIENT:
              sprintf(buf, "%d", rvalue);
              break;
            default:
//            sprintf(buf, "%d [%c%c%c%c]", rvalue, (rvalue >> 24) & 0xFF, (rvalue >> 16) & 0xFF, (rvalue >> 8) & 0xFF, (rvalue >> 0) & 0xFF);
              sprintf(buf, "%d [%08x]", rvalue, rvalue);
              break;
          }

  return buf;
}

const char *findFVF(DWORD FVF) {
  static char buf[256] = "";

  if ((FVF & D3DFVF_POSITION_MASK) == D3DFVF_XYZ)
    strcat(buf, " XYZ");
  if ((FVF & D3DFVF_POSITION_MASK) == D3DFVF_XYZW)
    strcat(buf, " XYZW");
  if ((FVF & D3DFVF_POSITION_MASK) == D3DFVF_XYZRHW)
    strcat(buf, " XYZRHW");
  if ((FVF & D3DFVF_POSITION_MASK) == D3DFVF_XYZB1)
    strcat(buf, " XYZB1");
  if ((FVF & D3DFVF_POSITION_MASK) == D3DFVF_XYZB2)
    strcat(buf, " XYZB2");
  if ((FVF & D3DFVF_POSITION_MASK) == D3DFVF_XYZB3)
    strcat(buf, " XYZB3");
  if ((FVF & D3DFVF_POSITION_MASK) == D3DFVF_XYZB4)
    strcat(buf, " XYZB4");
  if ((FVF & D3DFVF_POSITION_MASK) == D3DFVF_XYZB5)
    strcat(buf, " XYZB5");

  if (FVF & D3DFVF_NORMAL)
    strcat(buf, " NORMAL");
  if (FVF & D3DFVF_PSIZE)
    strcat(buf, " PSIZE");
  if (FVF & D3DFVF_DIFFUSE)
    strcat(buf, " DIFFUSE");
  if (FVF & D3DFVF_SPECULAR)
    strcat(buf, " SPECULAR");

  if ((FVF & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX0)
    strcat(buf, " TEX0");
  if ((FVF & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX1)
    strcat(buf, " TEX1");
  if ((FVF & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX2)
    strcat(buf, " TEX2");
  if ((FVF & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX3)
    strcat(buf, " TEX3");
  if ((FVF & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX4)
    strcat(buf, " TEX4");
  if ((FVF & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX5)
    strcat(buf, " TEX5");
  if ((FVF & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX6)
    strcat(buf, " TEX6");
  if ((FVF & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX7)
    strcat(buf, " TEX7");
  if ((FVF & D3DFVF_TEXCOUNT_MASK) == D3DFVF_TEX8)
    strcat(buf, " TEX8");

  return buf;
}
