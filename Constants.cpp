#include "Constants.h"

struct sConstants Constants;

/* -----------------------------------------------------------------------------
 */

void sConstants::Update() {
  Update(v1_2_416::GetRenderer());
  UpdateSun();

//ShaderManager *sm = ShaderManager::GetSingleton();
//sm->UpdateFrameConstants();
}

void sConstants::Update(v1_2_416::NiDX9Renderer *Renderer) {
  float (_cdecl * GetTimer)(bool, bool) = (float( *)(bool, bool))0x0043F490; // (TimePassed,GameTime)
  int gtime = GetTimer(0, 1); LARGE_INTEGER tick;

  QueryPerformanceCounter(&tick);

  iTikTiming.z = (__int64)((tick.QuadPart * 1000 * 1000) / iTikTiming.w);
  iTikTiming.y = (__int64)((tick.QuadPart * 1000 * 1   ) / iTikTiming.w);
  iTikTiming.x = (__int64)((tick.QuadPart * 1    * 1   ) / iTikTiming.w);

  fTikTiming.z = (float)(tick.QuadPart) * 1000 * 1000 / fTikTiming.w;
  fTikTiming.y = (float)(tick.QuadPart) * 1000 * 1    / fTikTiming.w;
  fTikTiming.x = (float)(tick.QuadPart) * 1    * 1    / fTikTiming.w;

  iGameTime.x = gtime;
  iGameTime.w = (gtime     ) % 60;
  iGameTime.z = (gtime / 60) % 60;
  iGameTime.y = (gtime / 60) / 60;

  fGameTime.x = gtime;
  fGameTime.w = ((int)gtime     ) % 60;
  fGameTime.z = ((int)gtime / 60) % 60;
  fGameTime.y = ((int)gtime / 60) / 60;

  // Noon is at (20:00 + 06:00) / 2 == 13:00 // [-PI,0,+PI]
  fGameTime.x = M_PI * (gtime - (13 * 60 * 60)) / (14 * 60 * 60);

  /* deprecated */
#ifndef	NO_DEPRECATED
  time.x = gtime;
  time.w = (int)(gtime     ) % 60;
  time.z = (int)(gtime / 60) % 60;
  time.y = (int)(gtime / 60) / 60;
#endif

#define Units2Centimeters	0.1428767293691635
#define Units2Meters		0.001428767293691635
  PlayerCharacter *PlayerContainer = (*g_thePlayer);
  PlayerPosition.x = PlayerContainer->posX * Units2Meters;
  PlayerPosition.y = PlayerContainer->posY * Units2Meters;
  PlayerPosition.z = PlayerContainer->posZ * Units2Meters * 4;
}

void sConstants::UpdateSun() {
  OBGEfork::Sky *pSky = OBGEfork::Sky::GetSingleton();
  OBGEfork::Sun *pSun = pSky->sun;
  TESClimate *climate = pSky->firstClimate;
  TESWeather *weather = pSky->firstWeather;

  SunTiming.x = climate->sunriseBegin * 10 * 60;
  SunTiming.y = climate->sunriseEnd   * 10 * 60;
  SunTiming.z = climate->sunsetBegin  * 10 * 60;
  SunTiming.w = climate->sunsetEnd    * 10 * 60;

  v1_2_416::NiNode *SunContainer = pSun->SunBillboard.Get()->ParentNode;
  float deltaz = SunDir.z;
  bool SunHasBenCulled = SunContainer->m_flags.individual.AppCulled;

  SunDir.x = SunContainer->m_localTranslate.x;
  SunDir.y = SunContainer->m_localTranslate.y;
  SunDir.z = SunContainer->m_localTranslate.z;
  SunDir.Normalize3();

#if 1
  /* calculation of the real position of the sun */
  /* length of day in "seconds" */
  float daylength = SunTiming.w - SunTiming.x;
  float highnoon = SunTiming.x + (daylength / 2);
  float midnight = highnoon - (12 * 60 * 60);
  /* length of day in "degree / 2" */
  float dayangleh = daylength   * (M_PI     / (24 * 60 * 60));
  float midangle  = midnight    * (M_PI * 2 / (24 * 60 * 60));
  float curangle  = iGameTime.x * (M_PI * 2 / (24 * 60 * 60));
  /* dis-position of tangent-line from the center
   * of the circle in [0,1] == [center,radius], d
   * lies on the rotated z-axis midnight->highnoon
   */
  float deltapos = 1.0 * cos(dayangleh);

  /* calculate unit-positions on the circle [0,1] */
  SunPos.x =  sin(curangle - midangle);
  SunPos.y = 0;
  SunPos.z = -cos(curangle - midangle);

  /* apply dis-poition and renormalize */
  SunPos.y = SunDir.y;
  SunPos.z = SunPos.z - deltapos;
  SunPos.Normalize3();

  /* pitch sun-circle
  SunPos.y = SunPos.z * sin(M_PI / 2 - dayangleh);
  SunPos.z = SunPos.z * cos(M_PI / 2 - dayangleh);
  SunPos.Normalize3(); */

  SunDir = SunPos;
#endif

#if 0
  // Sunrise is at 06:00, Sunset at 20:00
  if ((iGameTime.x > SunTiming.w + (10 * 60)) ||
      (iGameTime.x < SunTiming.x - (10 * 60)))
    SunDir.z = -SunDir.z;
  else if ((iGameTime.x > SunTiming.z - (10 * 60))) {
    /* needs to go down aways */
    if ((fabs(deltaz) - SunDir.z) <= 0.0)
      SunDir.z = -SunDir.z;
  }
  else if ((iGameTime.x < SunTiming.y + (10 * 60))) {
    /* needs to go up aways */
    if ((fabs(deltaz) - SunDir.z) >= 0.0)
      SunDir.z = -SunDir.z;
  }
//if ((GameTime.y < 6) || (GameTime.y >= 21))
//  SunDir.z = -fabs(SunDir.z);
#endif

  SunDir.w = SunHasBenCulled;
}

/* -----------------------------------------------------------------------------
 */

#if 0
typedef	v1_2_416::NiVector3 float3;
typedef	v1_2_416::NiVector4 float4;

#define lerp(ths, tht, morph)	(ths + ((tht - ths) * morph))

#define	PLAYER_HEIGHT	PlayerPosition.z

/* Light properties */
static const float3 cust_SunIntensity(1.0, 0.960784, 0.949019);

/* A&P dimensions */
static const float fCoverDimension   =           4000.0;
static const float fAtmosphereRadius =        6432797.0;
static const float fCloudRadius      =                     6380797.0;
static const float fPlanetRadius     =                                 6372797.0;
static const float fThicknessInv     = 1.0 / (6432797.0 -              6372797.0);
static const float fDistanceInv      =        6380797.0 / (6380797.0 - 6372797.0);

//efine	SUN_SIZE	(1024 - (256 * pow(1.0 - -SunDir.z, 0.125)))
#define	SUN_SIZE	(1024)
#define	SUN_BRIGHTNESS	(400)
#define	HORIZONT_BIAS	11.0

inline float get_height_coefficient(float3 Eye, float fPlanetRadius, float fAtmosphereRadius)
{
  float r = Eye.GetLength();
  return
    sqrt(                r * r                 - fPlanetRadius * fPlanetRadius) /
    sqrt(fAtmosphereRadius * fAtmosphereRadius - fPlanetRadius * fPlanetRadius);
}

#define ONEmEPINV 1.0280912f
inline float get_suna_coefficient(float x)
{
  // 1 / (1.0f - exp(-3.6f))
  return (1.0f - exp(-2.8f * x - 0.8f)) * ONEmEPINV;
}

void sConstants::UpdateSunCoEffs() {

    /* scattering shader ------------------------------------- */

	float cust_fIndRay    = 1.0;
	float cust_fIndMie    = 1.0;
	float cust_fMieCoefSB = 0.0025;

#define	VARYING_EFFICIENTS
#ifdef	VARYING_EFFICIENTS
	/*  morning:   1.5, 4.0, 0.0085
	 *  noon:      1.0, 1.0, 0.0025
	 *  afternoon: 2.0, 0.5, 0.0015
	 */
	float3 T; T.Set(-SunDir.x, 0, -SunDir.z); T.Normalize();
	float noon = T.z;//-T.z;//saturate(dot(float3(0, 0, 1), T));

	if (T.x < 0) {
		cust_fIndRay    = lerp(2.0000, 1.0000, noon);
		cust_fIndMie    = lerp(0.5000, 1.0000, noon);
		cust_fMieCoefSB = lerp(0.0015, 0.0025, noon);
	}
	else {
		cust_fIndRay    = lerp(1.5000, 1.0000, noon);
		cust_fIndMie    = lerp(4.0000, 1.0000, noon);
		cust_fMieCoefSB = lerp(0.0085, 0.0025, noon);
	}
#endif

	SunCoEffs.x = cust_fIndRay;
	SunCoEffs.y = cust_fIndMie;
	SunCoEffs.z = cust_fMieCoefSB;

}

void sConstants::UpdateSunColor() {

    /* sun color ----------------------------------------------------------- */

	/* we normally should put the real player-position here (from where?)
	 *
	 * additionally, the reflections' pass camera is below surface, we could
	 * intersect the view-vector with the planet's surface to get an individual
	 * view-vector per pixel, though I don't know if the few hundret meters we
	 * can go around really affect the sky's appearance
	 *
	 * for the moment we neglect any variable position of the camera
	 */
	float3 Eye; Eye.Set(0.0, 0.0, fPlanetRadius + max(PLAYER_HEIGHT, 0));	// Eye is 0km up in global
//	float3 L = -SunDir.xyz;

	// new parameterization
	// if we are directly on the planet-surface this is 0 and 1 for above atmosphere
	// range [0.0, 1.0] or more
	float height = get_height_coefficient(Eye, fPlanetRadius, fAtmosphereRadius);

	/* eye-normal */
	float3 nEye; nEye = Eye; nEye.Normalize();

	/* zenith:   dot([0,0,1], [0,0,-1]) = -1 = 0.0
	 * horizont: dot([0,0,1], [0,-1,0]) =  0 = 0.5
	 */
	float4 tl = tex2Dlod(cust_samplerOpticalDepth, float4(height, 0.5f * (dot(nEye, -SunDir)) + 0.5f, 0, 0));

	float4 cust_SunColor = tex2Dlod(cust_samplerSun, float4(0.5, (dot(-SunDir, -SunDir) - 1.0) * SUN_SIZE + 0.5, 0, 0));

	float3 sunIntensityAtt = float3(1,1,1) * exp(
		-cust_fIndRay * tl.xyz
		-cust_fIndMie * tl.www
	);

	float4 sunColor = float4(SUN_BRIGHTNESS * cust_SunColor.rgb * sunIntensityAtt, cust_SunColor.g);

        return sunColor;
}
#endif
