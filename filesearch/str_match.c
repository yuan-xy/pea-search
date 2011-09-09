#include "str_match.h"
#include "math.h"
#include "util.h"

static int DIFF = 'a'-'A';

#define LOWER_CASE(x) ((x)>='a' && (x)<='z')
#define UPPER_CASE(x) ((x)>='A' && (x)<='Z')

#define STR_CMP(s1,s2,len,case_sensitive)			\
		if(case_sensitive){							\
			if(memcmp(s1,s2,len)==0) return 1;		\
		}else{										\
			if(strnicmp(s1,s2,len)==0) return 1;	\
		}	

void preQsBc(pUTF8 x, int m, char qsBc[],BOOL case_sensitive) {
   int i;
   for (i = 0; i < ASIZE; ++i){
	      qsBc[i] = m + 1;
   }
   for (i = 0; i < m; ++i){
	  UTF8 ccc = x[i];
      qsBc[ccc] = m - i;
      if(!case_sensitive){
    	  if(UPPER_CASE(ccc)){
    		  qsBc[ccc+DIFF] = m - i;
    	  }else if(LOWER_CASE(ccc)){
    		  qsBc[ccc-DIFF] = m - i;
    	  }
      }
   }

}


INLINE BOOL QS(pUTF8 x, int m, pUTF8 y, int n,char qsBc[], BOOL case_sensitive) {
   int j=0;
   while (j <= n - m) {   /* Searching */
		STR_CMP(x,y + j,m,case_sensitive);
		j += qsBc[y[j + m]];               /* shift */
   }
   return 0;
}

INLINE BOOL brute_force(pUTF8 x, int m, pUTF8 y, int n, BOOL case_sensitive) {
	int i=0;
	pUTF8 pos=y;
	for(i=0;i<=n-m;i++){
		STR_CMP(pos,x,m,case_sensitive);
		pos++;
	}
	return 0;
}

#define isalpha_utf8(x) (isalpha((x)) || (x) >= 0xC0)

INLINE BOOL word_match(pUTF8 x, int m, pUTF8 y, int n, BOOL case_sensitive) {
	int i=0;
	pUTF8 pos=y;
	for(i=0;i<=n-m;i++){
		if(case_sensitive){
			if(strncmp(pos,x,m)==0){
				if(i==0 || !isalpha(y[i-1])){
					if(i==n-m || !isalpha(y[i+m])) return 1;
				}
			}
		}else{
			if(strnicmp(pos,x,m)==0){
				if(i==0 || !isalpha(y[i-1])){
					if(i==n-m || !isalpha(y[i+m])) return 1;
				}
			}
		}
		pos++;
	}
	return 0;
}

INLINE BOOL begin_match(pUTF8 x, int m, pUTF8 y, int n, BOOL case_sensitive) {
	STR_CMP(y,x,m,case_sensitive);
	return 0;
}

INLINE BOOL end_match(pUTF8 x, int m, pUTF8 y, int n, BOOL case_sensitive) {
	STR_CMP(y+(n-m),x,m,case_sensitive);
	return 0;
}


#define CHAR_EQU(x,y,c) ( c? (x)==(y) : ((x)==(y) || (UPPER_CASE(x) && ((y)-(x))==DIFF) || (LOWER_CASE(x) && ((x)-(y))==DIFF) ) )

void preKmp(pUTF8 x, int m, char kmpNext[], BOOL case_sensitive) {
   int i, j;

   i = 0;
   j = kmpNext[0] = -1;
   while (i < m) {
      while (j > -1 && !CHAR_EQU(x[i],x[j],case_sensitive) )
         j = kmpNext[j];
      i++;
      j++;
      if (CHAR_EQU(x[i],x[j],case_sensitive))
         kmpNext[i] = kmpNext[j];
      else
         kmpNext[i] = j;
   }
}


BOOL KMP(pUTF8 x, int m, pUTF8 y, int n, char kmpNext[],BOOL case_sensitive) {
   int i, j;
   i = j = 0;
   while (j < n) {
      while (i > -1 && !CHAR_EQU(x[i],y[j],case_sensitive))
         i = kmpNext[i];
      i++;
      j++;
      if (i >= m) {
         return 1;
         i = kmpNext[i];
      }
   }
   return 0;
}


void preBmBc(pUTF8 x, int m, int bmBc[], BOOL case_sensitive) {
   int i;
   for (i = 0; i < ASIZE; ++i)
      bmBc[i] = m;
   for (i = 0; i < m - 1; ++i){
	  UTF8 ccc = x[i];
      bmBc[ccc] = m - i - 1;
      if(!case_sensitive){
    	  if(UPPER_CASE(ccc)){
    		  bmBc[ccc+DIFF] = m - i -1;
    	  }else if(LOWER_CASE(ccc)){
    		  bmBc[ccc-DIFF] = m - i -1;
    	  }
      }
   }
}


void suffixes(pUTF8 x, int m, int *suff, BOOL case_sensitive) {
   int f, g, i;

   suff[m - 1] = m;
   g = m - 1;
   for (i = m - 2; i >= 0; --i) {
      if (i > g && suff[i + m - 1 - f] < i - g)
         suff[i] = suff[i + m - 1 - f];
      else {
         if (i < g)
            g = i;
         f = i;
         while (g >= 0 && CHAR_EQU(x[g],x[g + m - 1 - f],case_sensitive) )
            --g;
         suff[i] = f - g;
      }
   }
}

void preBmGs(pUTF8 x, int m, int bmGs[], BOOL case_sensitive) {
   int i, j, suff[XSIZE];

   suffixes(x, m, suff ,case_sensitive);

   for (i = 0; i < m; ++i)
      bmGs[i] = m;
   j = 0;
   for (i = m - 1; i >= 0; --i)
      if (suff[i] == i + 1)
         for (; j < m - 1 - i; ++j)
            if (bmGs[j] == m)
               bmGs[j] = m - 1 - i;
   for (i = 0; i <= m - 2; ++i)
      bmGs[m - 1 - suff[i]] = m - 1 - i;
}

void preBM(pUTF8 x, int m, int prestr[], BOOL case_sensitive){
	preBmGs(x, m, prestr,case_sensitive);
	preBmBc(x, m, prestr+XSIZE,case_sensitive);
}

BOOL BM(pUTF8 x, int m, pUTF8 y, int n,int preNext[], BOOL case_sensitive) {
   int i, j;
   int *bmGs = preNext;
   int *bmBc = preNext+XSIZE;
   j = 0;
   while (j <= n - m) {
      for (i = m - 1; i >= 0 && CHAR_EQU(x[i],y[i + j],case_sensitive); --i);
      if (i < 0) {
         return 1;
         j += bmGs[0];
      }
      else
         j += max(bmGs[i], bmBc[y[i + j]] - m + 1 + i);
   }
   return 0;
}

BOOL TBM(pUTF8 x, int m, pUTF8 y, int n, int preNext[], BOOL case_sensitive) {
   int bcShift, i, j, shift, u, v, turboShift;
   int *bmGs = preNext;
   int *bmBc = preNext+XSIZE;
   j = u = 0;
   shift = m;
   while (j <= n - m) {
      i = m - 1;
      while (i >= 0 && CHAR_EQU(x[i],y[i + j],case_sensitive)) {
         --i;
         if (u != 0 && i == m - 1 - shift)
            i -= u;
      }
      if (i < 0) {
         return 1;
         shift = bmGs[0];
         u = m - shift;
      }
      else {
         v = m - 1 - i;
         turboShift = u - v;
         bcShift = bmBc[y[i + j]] - m + 1 + i;
         shift = max(turboShift, bcShift);
         shift = max(shift, bmGs[i]);
         if (shift == bmGs[i])
            u = min(m - shift, v);
         else {
           if (turboShift < bcShift)
              shift = max(shift, u + 1);
           u = 0;
         }
      }
      j += shift;
   }
   return 0;
}

void pre_bndm_hz_multi(pUTF8 x, int m, int *prestr){
	int i, s = 1;
	for (i = m - 1; i >= 0; i--) {
		*(prestr+x[i]) |= s;
		s <<= 1; //m>32时s会溢出,如果定义char prestr，则m>8时prestr就会溢出
	}
}

INLINE int gen_shift(int M,int r){
	int mask = ((1<<(M-1)) -1)<<1;
	int tmp = (mask << M);
	int i;
	for(i=0;i<r-1;i++){
		mask |= tmp;
		tmp = (tmp << M);
	}
	return mask;
}

INLINE int gen_mask(int M,int r){
	int mask = 1<<(M-1);
	int tmp = (mask << M);
	int i;
	for(i=0;i<r-1;i++){
		mask |= tmp;
		tmp = (tmp << M);
	}
	return mask;
}

int shifts[10]={0x6,0x36,0x1b6,0xdb6,0x6db6,0x36db6,0x1b6db6,0xdb6db6,0x6db6db6,0x36db6db6};
int masks[10]={0x4,0x24,0x124,0x924,0x4924,0x24924,0x124924,0x924924,0x4924924,0x24924924};

int bndm_hz_multi(pUTF8 x, int m, pUTF8 y, int n, int *preNext) {
	int i, j, d, last;
	int M = 3, r=m/3;
	int shift = shifts[r-1], mask = masks[r-1];
	my_assert(m<32 && m%3==0, 0);
	j = 0;
	while (j <= n - M) {
		i = M - 1;
		last = M;
		d = ~0;
		while (i >= 0 && d != 0) {
			int pre = *(preNext + y[j + i]);
			d &= pre;
			i--;
			if ((d & mask) != 0) {
				if (i >= 0){
					last = i + 1;
				}else{
					return j+M; //found position j..j+m
				}
			}
			d = (d << 1) & shift;
		}
		j += last;
	}
	return 0;
}

void preBNDM(pUTF8 x, int m, int prestr[], BOOL case_sensitive){
	int i, s = 1;
	//memset(prestr, 0, ASIZE);
	for (i = m - 1; i >= 0; i--) {
		prestr[x[i]] |= s;
		if (!case_sensitive) {
			char ccc = x[i];
			if (UPPER_CASE(ccc)) {
				prestr[ccc + DIFF] |= s;
			} else if (LOWER_CASE(ccc)) {
				prestr[ccc - DIFF] |= s;
			}
		}
		s <<= 1; //m>32时s会溢出,如果定义char prestr，则m>8时prestr就会溢出
	}
}

BOOL BNDM(pUTF8 x, int m, pUTF8 y, int n, int preNext[], BOOL case_sensitive) {
	//Backward Nondeterministic Dawg Matching algorithm
	int i, j, d, last;
	if (m > 8*sizeof(void *) ) m=8*sizeof(void *); //查询字符串长度不能大于CPU位宽
	j = 0;
	while (j <= n - m) {
		i = m - 1;
		last = m;
		d = ~0;
		while (i >= 0 && d != 0) {
			d &= preNext[y[j + i]];
			i--;
			if (d != 0) {
				if (i >= 0)
					last = i + 1;
				else
					return 1; //found position j..j+m
			}
			d <<= 1;
		}
		j += last;
	}
	return 0;
}

static unsigned int lim;
void preShiftOr(pUTF8 x, int m, int preNext[], BOOL case_sensitive) {
	unsigned int j;
	int i;
	for (i = 0; i < ASIZE; ++i)
		preNext[i] = ~0;
	for (lim = i = 0, j = 1; i < m; ++i, j <<= 1) {
		preNext[x[i]] &= ~j;
		if (!case_sensitive) {
			char ccc = x[i];
			if (UPPER_CASE(ccc)) {
				preNext[ccc + DIFF] &= ~j;
			} else if (LOWER_CASE(ccc)) {
				preNext[ccc - DIFF] &= ~j;
			}
		}
		lim |= j;
	}
	lim = ~(lim >> 1);
	//return (lim);
}

BOOL ShiftOr(pUTF8 x, int m, pUTF8 y, int n, int preNext[], BOOL case_sensitive){
  unsigned int state;
  int j;
  for (state = ~0, j = 0; j < n; ++j) {
    state = (state<<1) | preNext[y[j]];
    if (state < lim)
      return 1; //OUTPUT(j - m + 1);
  }
  return 0;
}

extern void preBOM(pUTF8 x, int m, int prestr[], BOOL case_sensitive);
extern BOOL BOM(pUTF8 x, int m, pUTF8 y, int n, int preNext[], BOOL case_sensitive);

void preProcessPattern(pUTF8 x, int m, int prestr[], BOOL case_sensitive){
	preBNDM(x, m, prestr,case_sensitive);
	//preShiftOr(x, m, prestr, case_sensitive);
}

BOOL SUBSTR(pUTF8 x, int m, pUTF8 y, int n, int preNext[], BOOL case_sensitive){
	return BNDM(x,m,y,n,preNext,case_sensitive);
	//return brute_force(x,m,y,n,case_sensitive);
}
