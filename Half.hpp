/* Version: MPL 1.1/LGPL 3.0
 *
 * "The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the Oblivion Graphics Extender, short OBGE.
 *
 * The Initial Developer of the Original Code is
 * Ethatron <niels@paradice-insight.us>. Portions created by The Initial
 * Developer are Copyright (C) 2009-2011 The Initial Developer.
 * All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Library General Public License Version 3 license (the
 * "LGPL License"), in which case the provisions of LGPL License are
 * applicable instead of those above. If you wish to allow use of your
 * version of this file only under the terms of the LGPL License and not
 * to allow others to use your version of this file under the MPL,
 * indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the LGPL License.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under either the MPL or the LGPL License."
 */

#ifndef	DATATYPES_1D_POSITIONS_FLOAT1D_H
#define	DATATYPES_1D_POSITIONS_FLOAT1D_H

#define	passreg		__fastcall

  /* first-bit position */
  static int passreg fbpos(short int e) {
    __asm {
	/* handle '0'-case */
	xor	eax, eax
	dec	 ax
	bsr	 ax,  cx
    };
  }

  /* first-bit position */
  static int passreg fbpos(int e) {
    __asm {
	/* handle '0'-case */
	xor	eax, eax
	dec	eax
	bsr	eax, ecx
    };
  }

/* TODO: template the rounding mode for conversion for example */
template<class DataType = unsigned short, int precision = 10>
struct floatpoint1D {

public:
  static const int size		= sizeof(DataType) * 8;
  static const int sign		= 1;
  static const int fraction	= precision;
  static const int exponent	= (size - (sign + fraction));
#undef	floatpoint1D_ROUND_NEAREST_tie_EVEN
#undef	floatpoint1D_NORANGE_CHECK
#undef	floatpoint1D_NOZERO_CHECK

	/* ------------------------------------------------------------------ */
	/* 1.5.10	->	0.?????.????????? */
  static const DataType POSs	= 0;
	/* 1.5.10	->	1.?????.????????? */
  static const DataType NEGs	= 1;

	/* ------------------------------------------------------------------ */
	/* 1.5.10	->	0.11111.????????? */
  static const DataType NANe	= ((((DataType)1) << (exponent    )) - 1);
	/* 1.5.10	->	0.00000.????????? */
  static const DataType DNRMe	= 0;
	/* 1.5.10	->	0.01111.????????? */
  static const DataType ZEROe	= ((((DataType)1) << (exponent - 1)) - 1);
	/* 1.5.10	->	0.11110.????????? 	-> 0.01111.000000000 + 0.01111.000000000 */
  static const DataType MAXe	= ZEROe + ZEROe + 0;
	/* 1.5.10	->	0.00001.????????? 	-> 0.01111.000000000 - 0.01111.000000000 */
  static const DataType MINe	= ZEROe - ZEROe + 1;

	/* ------------------------------------------------------------------ */
	/* 1.5.10	->	0.?????.000000000 */
  static const DataType ZEROf	= 0;
	/* 1.5.10	->	0.?????.111111111 */
  static const DataType MAXf	= ((((DataType)1) << (fraction    )) - 1);
	/* 1.5.10	->	0.?????.000000000 */
  static const DataType MINf	= 0;
	/* ------------------------------------------------------------------ */
	/* 1.5.10	->	0.11111.000000000 */
  static const DataType INFf	= MINf;
	/* 1.5.10	->	0.11111.????????? */
  static const DataType NANf	= MAXf;

	/* ------------------------------------------------------------------ */
	/* 1.5.10	->	?.01111.000000000 */
  static const DataType ONE	= (ZEROe - 0) << fraction;
	/* 1.5.10	->	?.01110.000000000 */
  static const DataType HALF	= (ZEROe - 1) << fraction;
	/* 1.5.10	->	0.11110.111111111 */
  static const DataType MAX	= (POSs << (size - 1)) + (MAXe << fraction) + MAXf;
	/* 1.5.10	->	1.11110.111111111 */
  static const DataType MIN	= (NEGs << (size - 1)) + (MAXe << fraction) + MAXf;
	/* 1.5.10	->	0.11111.????????1 */
  static const DataType NANp	= (POSs << (size - 1)) + (NANe << fraction) + 1;
	/* 1.5.10	->	1.11111.????????1 */
  static const DataType NANn	= (NEGs << (size - 1)) + (NANe << fraction) + 1;
	/* 1.5.10	->	0.11111.000000000 */
  static const DataType INFp	= (POSs << (size - 1)) + (NANe << fraction) + INFf;
	/* 1.5.10	->	1.11111.000000000 */
  static const DataType INFn	= (NEGs << (size - 1)) + (NANe << fraction) + INFf;
	/* 1.5.10	->	0.00000.000000000 */
  static const DataType ZEROp	= (POSs << (size - 1));
	/* 1.5.10	->	1.00000.000000000 */
  static const DataType ZEROn	= (NEGs << (size - 1));

	/* 1.5.10	->	2^1111 */
  static const DataType BIAS	= ZEROe;
	/* 16.16	->	1.111111111*2^1111 */
//static const int64    LIMITi	= ((1LL << fraction) + MAXf) << BIAS;	// above is INF
	/* 16.16	->	1.111111111*2^0000 */
//static const int64    LIMITn	= ((1LL << fraction) + MAXf);		// above is unprecise

private:
  /* NOTE:
   *  - operations can be done totally as halfs if (e <= 5) && (f <= 10)		hardware?
   *  - operations can be done totally as floats if (e <= 8) && (f <= 23)
   *  - operations can be done totally as doubles if (e <= 11) && (f <= 52)
   *  - operations can be done totally as long doubles if (e <= 15) && (f <= 64)	hardware!?
   *  - operations can be done totally as quads if (e <= 15) && (f <= 112)		hardware?
   *  - other cases require emulation
   */
  friend struct floatpoint1D<>;
  friend struct floatpoint1D<unsigned short, 10>;
  friend struct floatpoint1D<unsigned long, 23>;
  friend struct floatpoint1D<unsigned __int64, 52>;

  /* ---------------------------------------------------------------------------
   */
  inline void importl(const DataType tht) {
    DataType t = tht;

    /* prepare value and set sign */
    bitfield = ZEROp;
    /* zero (no negative zero possible) */
    if (t == 0)
      return;
    /* negative (two's complement) */
    if (t < 0)
      bitfield = ZEROn, t = -t;

    /* scan size */
//  DataType e = (size - 1) - lzcnt(t);
    DataType e =              fbpos(t);

    /* exponent too big */
    if (e > BIAS) {
      bitfield |= INFp;
      return;
    }

    /* scan size */
    DataType s = fraction - e;

    /* fraction too big */
    if (s < 0)
      t >>= -s, s = 0;

    /* normalize fraction and done */
    bitfield |= ((e + BIAS) << fraction) | ((t << s) & MAXf);
  }
  inline void importf(const float tht) {
    /* general fp<->fp conversion:
     */
    floatpoint1D<unsigned long, 23> A;
    A.bitfield = *((unsigned long *)(&tht));

    assert(fraction <= A.fraction);
    assert(exponent <= A.exponent);

    /* signA = A & (1 << (exponentA + fractionA))
     * signB = signA >> (exponentA - exponentB + fractionA - fractionB)
     */
    unsigned long sign = A.bitfield & A.ZEROn;
    unsigned long check = A.bitfield ^ sign;
    signed long EE, ee;

    bitfield = (DataType)(sign >> (A.size - size));

    /* the converted bias would be too small
     * if ((biasA - biasB) - eA > 0)
     *   fractionB = fractionA >> (biasA - biasB) - eA;
     */
    ee = (check >> A.fraction);

    if ((EE = (A.BIAS - BIAS) - ee) >= 0L) {
      // add additional subnormalization
      EE = (EE + (A.fraction - fraction));
      // add implicit bit if previously normal
      ee = ((ee ? -1L : 0L) & 1L); EE += ee;

      check  = (check & A.MAXf) | (ee << (A.fraction));
#ifdef	floatpoint1D_ROUND_NEAREST_tie_EVEN
      check += (check >> (EE)) & 1L;
      check += (1L << (EE - 1L)) - 1L;
#endif

      // subnormal (if not zeroed out completly, >=32 would do modulo-shift)
      if (EE <= A.fraction)
        bitfield |= (check >> EE);

      return;
    }

#ifdef	floatpoint1D_ROUND_NEAREST_tie_EVEN
    /* INF/NAN:
     *  0.1111110.11111111111111111111111 max	->	0.1111111.00000000000111111111110 nan
     *  0.1111111.00000000000000000000000 inf	->	0.1111111.00000000000111111111111 nan
     */
    check += (check >> (A.fraction - fraction)) & 1L;
    check += (1L << (A.fraction - fraction - 1L)) - 1L;
#endif

    /* the converted bias would be too big
     * if (eA - (biasA - biasB) > 0)
     */
    ee = (check >> A.fraction);
    check >>= (A.fraction - fraction);

    if ((EE = ee - (A.BIAS + BIAS)) > 0L) {
      // mask 0 if previously a number
      ee = ~(ee - A.NANe ? -1L : 0L) & MAXf;

      // infinity/nan
      bitfield |= (check & ee) | INFp;
      return;
    }

    /* trim fraction-tail */
    bitfield |= (check) - ((A.BIAS - BIAS) << fraction);
  }
  inline void importd(const double tht) {
    /* general fp<->fp conversion:
     */
    floatpoint1D<unsigned_int64, 52> A;
    A.bitfield = *((unsigned_int64 *)(&tht));

    assert(fraction <= A.fraction);
    assert(exponent <= A.exponent);

    /* signA = A & (1 << (exponentA + fractionA))
     * signB = signA >> (exponentA - exponentB + fractionA - fractionB)
     */
    unsigned_int64 sign = A.bitfield & A.ZEROn;
    unsigned_int64 check = A.bitfield ^ sign;
    signed_int64 EE, ee;

    bitfield = (DataType)(sign >> (A.size - size));

    /* the converted bias would be too small
     * if ((biasA - biasB) - eA > 0)
     *   fractionB = fractionA >> (biasA - biasB) - eA;
     */
    ee = (check >> A.fraction);

    if ((EE = (A.BIAS - BIAS) - ee) >= 0LL) {
      // add additional subnormalization
      EE = (EE + (A.fraction - fraction));
      // add implicit bit if previously normal
      ee = ((ee ? -1LL : 0LL) & 1LL); EE += ee;

      check  = (check & A.MAXf) | (ee << (A.fraction));
#ifdef	floatpoint1D_ROUND_NEAREST_tie_EVEN
      check += (check >> (EE)) & 1LL;
      check += (1LL << (EE - 1LL)) - 1LL;
#endif

      // subnormal (if not zeroed out completly, >=32 would do modulo-shift)
      if (EE <= A.fraction)
	bitfield |= (check >> EE);
      return;
    }

#ifdef	floatpoint1D_ROUND_NEAREST_tie_EVEN
    /* INF/NAN:
     *  0.1111110.11111111111111111111111 max	->	0.1111111.00000000000111111111110 nan
     *  0.1111111.00000000000000000000000 inf	->	0.1111111.00000000000111111111111 nan
     */
    check += (check >> (A.fraction - fraction)) & 1LL;
    check += (1LL << (A.fraction - fraction - 1LL)) - 1LL;
#endif

    /* the converted bias would be too big
     * if (eA - (biasA - biasB) > 0)
     */
    ee = (check >> A.fraction);
    check >>= (A.fraction - fraction);

    if ((EE = ee - (A.BIAS + BIAS)) > 0LL) {
      // mask 0 if previously a number
      ee = ~(ee - A.NANe ? -1LL : 0LL) & MAXf;

      // infinity/nan
      bitfield |= (check & ee) | INFp;
      return;
    }

    /* trim fraction-tail */
    bitfield |= (check) - ((A.BIAS - BIAS) << fraction);
  }

  /* ---------------------------------------------------------------------------
   */
  inline DataType exportl() const {
    if (exponentbits < BIAS)
      return 0;

    return signbits ? -fractionbits << (exponentbits - BIAS)
    		    :  fractionbits << (exponentbits - BIAS);
  }
  inline float exportf() const {
    /* general fp<->fp conversion:
     */
    floatpoint1D<unsigned long, 23> B;

    assert(fraction <= B.fraction);
    assert(exponent <= B.exponent);

    /* signA = A & (1 << (exponentA + fractionA))
     * signB = signA >> (exponentA - exponentB + fractionA - fractionB)
     */
    unsigned long sign = bitfield & ZEROn;
    unsigned long check = bitfield ^ sign;
    signed long ee;

    B.bitfield = (sign << (B.size - size));

    /* INF/NAN */
    if ((ee = check - INFp) >= 0) {
      B.bitfield += ((B.NANe) << B.fraction) +
    		    (ee << (B.fraction - fraction));

      return *((float *)(&B));
    }

    /* scan size */
//  signed long e = (size - 1) - lzcnt((signed)check);
    signed long e =              fbpos((signed)check);

    /* zero */
    if ((e) < 0)
      return *((float *)(&B));

    /* 0.00000.1????????? */
    if ((e -= fraction) < 0) {
      /* 0.00001.?????????? */
      check <<= -e;
      check += (e << fraction);

//    B.bitfield += ((e + B.BIAS - BIAS) << B.fraction) +
//  		    (check << (-e + B.fraction - fraction));
//
//    return *((float *)(&B));
    }

    B.bitfield += ((B.BIAS - BIAS) << B.fraction) +
    		  (check << (B.fraction - fraction));

    return *((float *)(&B));
  }
  inline double exportd() const {
    /* general fp<->fp conversion:
     */
    floatpoint1D<unsigned_int64, 52> B;

    assert(fraction <= B.fraction);
    assert(exponent <= B.exponent);

    /* signA = A & (1 << (exponentA + fractionA))
     * signB = signA >> (exponentA - exponentB + fractionA - fractionB)
     */
    unsigned_int64 sign = bitfield & ZEROn;
    unsigned_int64 check = bitfield ^ sign;
    signed_int64 ee;

    B.bitfield = (sign << (B.size - size));

    /* INF/NAN */
    if ((ee = check - INFp) >= 0) {
      B.bitfield += ((B.NANe) << B.fraction) +
    		    (ee << (B.fraction - fraction));

      return *((float *)(&B));
    }

    /* scan size */
//  signed long e = (size - 1) - lzcnt((signed long)(check >> 32)) + 32;
    signed long e =              fbpos((signed long)(check >> 32)) + 32;

    /* zero? */
    if ((e) < 32) {
      /* scan size */
//    e = (size - 1) - lzcnt((signed long)(check >> 0));
      e =              fbpos((signed long)(check >> 0));

      /* zero */
      if ((e) < 0)
        return *((double *)(&B));
    }

    /* 0.00000.1????????? */
    if ((e -= fraction) < 0) {
      /* 0.00001.?????????? */
      check <<= -e;
      check += (e << fraction);

//    B.bitfield += ((e + B.BIAS - BIAS) << B.fraction) +
//  		    (check << (-e + B.fraction - fraction));
//
//    return *((float *)(&B));
    }

    B.bitfield += ((B.BIAS - BIAS) << B.fraction) +
    		  (check << (B.fraction - fraction));

    return *((double *)(&B));
  }

public:
  /*
  floatpoint1D(                     ) {               }
  floatpoint1D(const float       tht) { importf(tht); }
  floatpoint1D(const double      tht) { importd(tht); }
  floatpoint1D(const DataType    tht) { importl(tht); }
  */

  inline operator DataType() const { return exportl(); }
  inline operator    float() const { return exportf(); }
  inline operator   double() const { return exportd(); }

private:
  union {
    DataType bitfield;
    struct {
#if	(__BYTE_ORDER == __BIG_ENDIAN)
      DataType 	    signbits :                            1              ,
		exponentbits : ((sizeof(DataType) * 8) - (1 + precision)),
		fractionbits :                                precision  ;
#elif	(__BYTE_ORDER == __LITTLE_ENDIAN)
      DataType 	fractionbits :                                precision  ,
		exponentbits : ((sizeof(DataType) * 8) - (1 + precision)),
		    signbits :                            1              ;
#endif
    };
  };

public:
  inline DataType raw() { return bitfield; }

  inline bool nrm() { return !!exponentbits; }
  inline bool nan() { return   (bitfield & ~ZEROn) == NANp; }
  inline bool inf() { return   (bitfield & ~ZEROn) == INFp; }
  inline bool pos() { return  !(bitfield & ZEROn); }
  inline bool neg() { return !!(bitfield & ZEROn); }

  /* ================================================================================================================================================
   * Assignment operators
   */
//inline        floatpoint1D& operator =   (const floatpoint1D& tht) {               return *this; }
  inline        floatpoint1D& operator =   (const float         tht) { importf(tht); return *this; }
  inline        floatpoint1D& operator =   (const double        tht) { importd(tht); return *this; }
  inline        floatpoint1D& operator =   (const DataType      tht) { importl(tht); return *this; }

  /* ================================================================================================================================================
   * N-type operators (natural)
   */
  inline        floatpoint1D& operator ++  (                                                ) { float trr; trr = exportf(); trr += 1; importf(trr); return *this; }
  inline        floatpoint1D& operator --  (                                                ) { float trr; trr = exportf(); trr -= 1; importf(trr); return *this; }

  /* ================================================================================================================================================
   * R-type operators (real)
   */
  inline        floatpoint1D& operator +=  (const floatpoint1D& tht                         ) { float trr; trr = exportf(); trr += tht.exportf(); importf(trr); return *this; }
  inline        floatpoint1D& operator -=  (const floatpoint1D& tht                         ) { float trr; trr = exportf(); trr -= tht.exportf(); importf(trr); return *this; }
  inline        floatpoint1D& operator *=  (const floatpoint1D& tht                         ) { float trr; trr = exportf(); trr *= tht.exportf(); importf(trr); return *this; }
  inline        floatpoint1D& operator /=  (const floatpoint1D& tht                         ) { float trr; trr = exportf(); trr /= tht.exportf(); importf(trr); return *this; }

  inline        floatpoint1D& operator +=  (const float&      tht                           ) { float trr; trr = exportf(); trr += tht; importf(trr); return *this; }
  inline        floatpoint1D& operator -=  (const float&      tht                           ) { float trr; trr = exportf(); trr -= tht; importf(trr); return *this; }
  inline        floatpoint1D& operator *=  (const float&      tht                           ) { float trr; trr = exportf(); trr *= tht; importf(trr); return *this; }
  inline        floatpoint1D& operator /=  (const float&      tht                           ) { float trr; trr = exportf(); trr /= tht; importf(trr); return *this; }

  inline        floatpoint1D& operator +=  (const double&     tht                           ) { double trr; trr = exportd(); trr += tht; importd(trr); return *this; }
  inline        floatpoint1D& operator -=  (const double&     tht                           ) { double trr; trr = exportd(); trr -= tht; importd(trr); return *this; }
  inline        floatpoint1D& operator *=  (const double&     tht                           ) { double trr; trr = exportd(); trr *= tht; importd(trr); return *this; }
  inline        floatpoint1D& operator /=  (const double&     tht                           ) { double trr; trr = exportd(); trr /= tht; importd(trr); return *this; }

  inline        floatpoint1D& operator *=  (const DataType    tht                           ) { float trr; trr = exportf(); trr *= tht; importf(trr); return *this; }
  inline        floatpoint1D& operator /=  (const DataType    tht                           ) { float trr; trr = exportf(); trr /= tht; importf(trr); return *this; }

  inline        floatpoint1D& operator <<= (const DataType    tht                           ) { if (MAXe - exponentbits <= tht) exponentbits += tht; else bitfield |= INFp; return *this; }
  inline        floatpoint1D& operator >>= (const DataType    tht                           ) { if (exponentbits - MINe >= tht) exponentbits -= tht; else exponentbits = 0, fractionbits >>= tht - (exponentbits - MINe); return *this; }

  /* ================================================================================================================================================
   * R-type operators (real)
   */
  inline friend floatpoint1D  operator -   (const floatpoint1D& ths                         ) { floatpoint1D tmp = ths; tmp.bitfield ^= ZEROn; return tmp; }

  inline friend floatpoint1D  operator +   (const floatpoint1D& ths, const floatpoint1D& tht) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr += tht.exportf(); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D  operator -   (const floatpoint1D& ths, const floatpoint1D& tht) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr -= tht.exportf(); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D  operator *   (const floatpoint1D& ths, const floatpoint1D& tht) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr *= tht.exportf(); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D  operator /   (const floatpoint1D& ths, const floatpoint1D& tht) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr /= tht.exportf(); tmp.importf(trr); return tmp; }

  inline friend floatpoint1D  operator +   (const floatpoint1D& ths, const float&        tht) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr += tht; tmp.importf(trr); return tmp; }
  inline friend floatpoint1D  operator -   (const floatpoint1D& ths, const float&        tht) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr -= tht; tmp.importf(trr); return tmp; }
  inline friend floatpoint1D  operator *   (const floatpoint1D& ths, const float&        tht) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr *= tht; tmp.importf(trr); return tmp; }
  inline friend floatpoint1D  operator /   (const floatpoint1D& ths, const float&        tht) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr /= tht; tmp.importf(trr); return tmp; }

  inline friend floatpoint1D  operator +   (const floatpoint1D& ths, const double&       tht) { floatpoint1D tmp; double trr; trr = ths.exportd(); trr += tht; tmp.importd(trr); return tmp; }
  inline friend floatpoint1D  operator -   (const floatpoint1D& ths, const double&       tht) { floatpoint1D tmp; double trr; trr = ths.exportd(); trr -= tht; tmp.importd(trr); return tmp; }
  inline friend floatpoint1D  operator *   (const floatpoint1D& ths, const double&       tht) { floatpoint1D tmp; double trr; trr = ths.exportd(); trr *= tht; tmp.importd(trr); return tmp; }
  inline friend floatpoint1D  operator /   (const floatpoint1D& ths, const double&       tht) { floatpoint1D tmp; double trr; trr = ths.exportd(); trr /= tht; tmp.importd(trr); return tmp; }

  inline friend floatpoint1D  operator *   (const floatpoint1D& ths, const DataType      tht) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr *= tht; tmp.importf(trr); return tmp; }
  inline friend floatpoint1D  operator /   (const floatpoint1D& ths, const DataType      tht) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr /= tht; tmp.importf(trr); return tmp; }

  inline friend float         operator +   (const float&        ths, const floatpoint1D& tht) { float tmp; tmp = tht.exportf(); tmp = ths - tmp; return tmp; }
  inline friend float         operator -   (const float&        ths, const floatpoint1D& tht) { float tmp; tmp = tht.exportf(); tmp = ths + tmp; return tmp; }
  inline friend float         operator *   (const float&        ths, const floatpoint1D& tht) { float tmp; tmp = tht.exportf(); tmp = ths * tmp; return tmp; }
  inline friend float         operator /   (const float&        ths, const floatpoint1D& tht) { float tmp; tmp = tht.exportf(); tmp = ths / tmp; return tmp; }

  inline friend double        operator +   (const double&       ths, const floatpoint1D& tht) { double tmp; tmp = tht.exportd(); tmp = ths - tmp; return tmp; }
  inline friend double        operator -   (const double&       ths, const floatpoint1D& tht) { double tmp; tmp = tht.exportd(); tmp = ths + tmp; return tmp; }
  inline friend double        operator *   (const double&       ths, const floatpoint1D& tht) { double tmp; tmp = tht.exportd(); tmp = ths * tmp; return tmp; }
  inline friend double        operator /   (const double&       ths, const floatpoint1D& tht) { double tmp; tmp = tht.exportd(); tmp = ths / tmp; return tmp; }

  inline friend DataType      operator *   (const DataType      ths, const floatpoint1D& tht) { floatpoint1D tmp; tmp.importl(ths); tmp *= tht; return tmp.exportl(); }
  inline friend DataType      operator /   (const DataType      ths, const floatpoint1D& tht) { floatpoint1D tmp; tmp.importl(ths); tmp /= tht; return tmp.exportl(); }

  inline friend floatpoint1D  operator <<  (const floatpoint1D& ths, const DataType      tht) { floatpoint1D tmp; tmp = ths; tmp <<= tht; return tmp; }
  inline friend floatpoint1D  operator >>  (const floatpoint1D& ths, const DataType      tht) { floatpoint1D tmp; tmp = ths; tmp >>= tht; return tmp; }

  /* ================================================================================================================================================
   * C-type operators (compare)
   */
  inline friend bool          operator !   (const floatpoint1D  ths                         ) { return !(ths.bitfield & ~ZEROn); }
  inline friend bool          operator !=  (const floatpoint1D  ths, const floatpoint1D  tht) { return (ths.bitfield != tht.bitfield); }
  inline friend bool          operator ==  (const floatpoint1D  ths, const floatpoint1D  tht) { return (ths.bitfield == tht.bitfield); }

  inline friend bool          operator >   (const floatpoint1D  ths, const floatpoint1D  tht) { float a, b; a = ths.exportf(); b = tht.exportf(); return (a >  b); }
  inline friend bool          operator >=  (const floatpoint1D  ths, const floatpoint1D  tht) { float a, b; a = ths.exportf(); b = tht.exportf(); return (a >= b); }
  inline friend bool          operator <=  (const floatpoint1D  ths, const floatpoint1D  tht) { float a, b; a = ths.exportf(); b = tht.exportf(); return (a <= b); }
  inline friend bool          operator <   (const floatpoint1D  ths, const floatpoint1D  tht) { float a, b; a = ths.exportf(); b = tht.exportf(); return (a <  b); }

  inline friend bool          operator ||  (const floatpoint1D  ths, const floatpoint1D  tht) { float a, b; a = ths.exportf(); b = tht.exportf(); return (a || b); }
  inline friend bool          operator &&  (const floatpoint1D  ths, const floatpoint1D  tht) { float a, b; a = ths.exportf(); b = tht.exportf(); return (a && b); }

  /* ================================================================================================================================================
   * M-type operators (math)
   */
  inline friend floatpoint1D sqrt (const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = sqrt (trr); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D cbrt (const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = cbrt (trr); tmp.importf(trr); return tmp; }

  /* ================================================================================================================================================
   * M-type operators (math)
   */
  inline friend floatpoint1D sin  (const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = sinf  (trr); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D cos  (const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = cosf  (trr); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D tan  (const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = tanf  (trr); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D asin (const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = asinf (trr); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D acos (const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = acosf (trr); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D atan (const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = atanf (trr); tmp.importf(trr); return tmp; }
//inline friend floatpoint1D atan2(const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = atan2f(trr); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D sinh (const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = sinhf (trr); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D cosh (const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = coshf (trr); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D tanh (const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = tanhf (trr); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D asinh(const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = asinhf(trr); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D acosh(const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = acoshf(trr); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D atanh(const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = atanhf(trr); tmp.importf(trr); return tmp; }

  /* ================================================================================================================================================
   * M-type operators (math)
   */
  inline friend floatpoint1D log2 (const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = log2 (trr); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D log  (const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = log  (trr); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D log10(const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = log10(trr); tmp.importf(trr); return tmp; }

  inline friend floatpoint1D exp  (const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = exp  (trr); tmp.importf(trr); return tmp; }
  inline friend floatpoint1D pow  (const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = pow  (trr); tmp.importf(trr); return tmp; }

  /* ================================================================================================================================================
   * M-type operators (math)
   */
  inline friend floatpoint1D inv  (const floatpoint1D ths) { floatpoint1D tmp; float trr; trr = ths.exportf(); trr = 1.0f / trr; tmp.importf(trr); return tmp; }
  inline friend DataType     rem  (const floatpoint1D ths) { return exponentbits; }
  inline friend DataType     quo  (const floatpoint1D ths) { return fractionbits; }
  inline friend floatpoint1D abs  (const floatpoint1D ths) { floatpoint1D tmp = ths; tmp.bitfield &= ~ZEROn; return tmp; }

  /* ================================================================================================================================================
   * M-type operators (math)
   */
  inline friend floatpoint1D floor(const floatpoint1D ths) { abort(); }
  inline friend floatpoint1D ceil (const floatpoint1D ths) { abort(); }
  inline friend floatpoint1D rnear(const floatpoint1D ths) { abort(); }
  inline friend floatpoint1D rzero(const floatpoint1D ths) { abort(); }
  inline friend floatpoint1D rinfp(const floatpoint1D ths) { abort(); }
  inline friend floatpoint1D rinfn(const floatpoint1D ths) { abort(); }

#undef	importl
#undef	importf
#undef	exportl
#undef	exportf
};

typedef floatpoint1D<unsigned short,10> half;

#endif
