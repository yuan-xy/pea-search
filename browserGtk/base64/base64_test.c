
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"


/*  Test cases from rfc2440 (OpenPGP Message Format)
 *    section 6.5 (Examples of Radix-64).
 */

int validate       (const unsigned char *src, const unsigned char *dst);
int encode_block   (unsigned char *dst, int *dstlen,
                    const unsigned char *src, int srclen);
int encode_context (unsigned char *dst, int *dstlen,
                    const unsigned char *src, int srclen);
int decode_block   (unsigned char *dst, int *dstlen,
                    const unsigned char *src, int srclen);
int decode_context (unsigned char *dst, int *dstlen,
                    const unsigned char *src, int srclen);


int
main (int argc, char *argv[])
{
    const unsigned char src1[] = { 0x14, 0xfb, 0x9c, 0x03, 0xd9, 0x7e, 0x00 };
    const unsigned char src2[] = { 0x14, 0xfb, 0x9c, 0x03, 0xd9, 0x00 };
    const unsigned char src3[] = { 0x14, 0xfb, 0x9c, 0x03, 0x00 };

    const unsigned char dst1[] = "FPucA9l+";
    const unsigned char dst2[] = "FPucA9k=";
    const unsigned char dst3[] = "FPucAw==";

    if ( (validate (src1, dst1) < 0)
      || (validate (src2, dst2) < 0)
      || (validate (src3, dst3) < 0) ) {
        exit (EXIT_FAILURE);
    }
    exit (EXIT_SUCCESS);
}


int
validate (const unsigned char *src, const unsigned char *dst)
{
    int n;
    unsigned char buf[9];

    if (encode_block (buf, &n, src, strlen (src)) < 0)
        return (-1);
    if (n != strlen (dst))
        return (-1);
    if (strncmp (dst, buf, n))
        return (-1);

    if (decode_block (buf, &n, dst, strlen (dst)) < 0)
        return (-1);
    if (n != strlen (src))
        return (-1);
    if (strncmp (src, buf, n))
        return (-1);

    if (encode_context (buf, &n, src, strlen (src)) < 0)
        return (-1);
    if (n != strlen (dst))
        return (-1);
    if (strncmp (dst, buf, n))
        return (-1);

    if (decode_context (buf, &n, dst, strlen (dst)) < 0)
        return (-1);
    if (n != strlen (src))
        return (-1);
    if (strncmp (src, buf, n))
        return (-1);

    return (0);
}


int
encode_block (unsigned char *dst, int *dstlen,
              const unsigned char *src, int srclen)
{
    return (base64_encode_block (dst, dstlen, src, srclen));
}


int
encode_context (unsigned char *dst, int *dstlen,
                const unsigned char *src, int srclen)
{
    base64_ctx x;
    int i;
    int n;
    int m;

    if (base64_init (&x) < 0)
        return (-1);
    for (i=0, n=0; i<srclen; i++) {
        if (base64_encode_update (&x, dst, &m, src + i, 1) < 0)
            return (-1);
        dst += m;
        n += m;
    }
    if (base64_encode_final (&x, dst, &m) < 0)
        return (-1);
    if (base64_cleanup (&x) < 0)
        return (-1);
    n += m;
    *dstlen = n;
    return (0);
}


int
decode_block (unsigned char *dst, int *dstlen,
              const unsigned char *src, int srclen)
{
    return (base64_decode_block (dst, dstlen, src, srclen));
}


int
decode_context (unsigned char *dst, int *dstlen,
                const unsigned char *src, int srclen)
{
    base64_ctx x;
    int i;
    int n;
    int m;

    if (base64_init (&x) < 0)
        return (-1);
    for (i=0, n=0; i<srclen; i++) {
        if (base64_decode_update (&x, dst, &m, src + i, 1) < 0)
            return (-1);
        dst += m;
        n += m;
    }
    if (base64_decode_final (&x, dst, &m) < 0)
        return (-1);
    if (base64_cleanup (&x) < 0)
        return (-1);
    n += m;
    *dstlen = n;
    return (0);
}

