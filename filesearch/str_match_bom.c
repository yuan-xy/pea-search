#include <windows.h>
#include "str_match.h"

#define FALSE      0
#define TRUE       1
#define UNDEFINED 0x7FFFFF

struct _cell {
	int element;
	struct _cell *next;
};
typedef struct _cell List, *pList;

int getTransition(char *x, int p, pList L[], char c) {
   pList cell = NULL;

   if (p > 0 && x[p - 1] == c)
      return(p - 1);
   else {
      cell = L[p];
      while (cell != NULL)
         if (x[cell->element] == c)
            return(cell->element);
         else
            cell = cell->next;
      return(UNDEFINED);
   }
}


void setTransition(int p, int q, pList L[]) {
   pList cell;
   cell = (pList)malloc(sizeof(struct _cell));
   if (cell == NULL) exit(1);
   cell->element = q;
   cell->next = L[p];
   L[p] = cell;
}


void oracle(char *x, int m, char T[], pList L[]) {
   int i, p, q;
   int S[XSIZE + 1];
   char c;

   S[m] = m + 1;
   for (i = m; i > 0; --i) {
      c = x[i - 1];
      p = S[i];
      while (p <= m &&
             (q = getTransition(x, p, L, c)) ==
             UNDEFINED) {
         setTransition(p, i - 1, L);
         p = S[p];
      }
      S[i - 1] = (p == m + 1 ? m : q);
   }
   p = 0;
   while (p <= m) {
      T[p] = TRUE;
      p = S[p];
   }
}

void preBOM(pUTF8 x, int m, int preNext[], BOOL case_sensitive) {
	pList *L = (pList *)preNext;
	char *T = (char *)(preNext+XSIZE + 1);
	memset(L, NULL, (m + 1) * sizeof(void *));
	memset(T, FALSE, (m + 1) * sizeof(char));
	oracle(x, m, T, L);
}

BOOL BOM(pUTF8 x, int m, pUTF8 y, int n, int preNext[], BOOL case_sensitive) {
	pList *L = (pList *)preNext;
	char *T = (char *)(preNext+XSIZE + 1);
   int i, j, p, period, q, shift;
   j = 0;
   while (j <= n - m) {
      i = m - 1;
      p = m;
      shift = m;
      while (i + j >= 0 &&
             (q = getTransition(x, p, L, y[i + j])) !=
             UNDEFINED) {
         p = q;
         if (T[p] == TRUE) {
            period = shift;
            shift = i;
         }
         --i;
      }
      if (i < 0) {
         return 1;// OUTPUT(j);
         shift = period;
      }
      j += shift;
   }
   return 0;
}

