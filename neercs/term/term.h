//
// Neercs
//

#pragma once

#include "term/pty.h"

struct Iso2022Conversion
{
    Iso2022Conversion() { Reset(); }
    void Reset();

    /* cs = coding system/coding method: */
    /* (with standard return) */
    /* '@' = ISO-2022, */
    /* 'G' = UTF-8 without implementation level, */
    /* '8' = UTF-8 (Linux console and imitators), */
    /* and many others that are rarely used; */
    /* (without standard return) */
    /* '/G' = UTF-8 Level 1, */
    /* '/H' = UTF-8 Level 2, */
    /* '/I' = UTF-8 Level 3, */
    /* and many others that are rarely used */
    uint32_t cs;
    /* ctrl8bit = allow 8-bit controls */
    uint8_t ctrl8bit;
    /* cn[0] = C0 control charset (0x00 ... 0x1f):
     * '@' = ISO 646,
     * '~' = empty,
     * and many others that are rarely used */
    /* cn[1] = C1 control charset (0x80 ... 0x9f):
     * 'C' = ISO 6429-1983,
     * '~' = empty,
     * and many others that are rarely used */
    uint32_t cn[2];
    /* glr[0] = GL graphic charset (94-char. 0x21 ... 0x7e,
     *                              94x94-char. 0x21/0x21 ... 0x7e/0x7e),
     * and
     * glr[1] = GR graphic charset (94-char. 0xa1 ... 0xfe,
     *                              96-char. 0xa0 ... 0xff,
     *                              94x94-char. 0xa1/0xa1 ... 0xfe/0xfe,
     *                              96x96-char. 0xa0/0xa0 ... 0xff/0xff):
     * 0 = G0, 1 = G1, 2 = G2, 3 = G3 */
    uint8_t glr[2];
    /* gn[i] = G0/G1/G2/G3 graphic charset state:
     * (94-char. sets)
     * '0' = DEC ACS (VT100 and imitators),
     * 'B' = US-ASCII,
     * and many others that are rarely used for e.g. various national ASCII variations;
     * (96-char. sets)
     * '.A' = ISO 8859-1 "Latin 1" GR,
     * '.~' = empty 96-char. set,
     * and many others that are rarely used for e.g. ISO 8859-n GR;
     * (double-byte 94x94-charsets)
     * '$@' = Japanese Character Set ("old JIS") (JIS C 6226:1978),
     * '$A' = Chinese Character Set (GB 2312),
     * '$B' = Japanese Character Set (JIS X0208/JIS C 6226:1983),
     * '$C' = Korean Graphic Character Set (KSC 5601:1987),
     * '$D' = Supplementary Japanese Graphic Character Set (JIS X0212),
     * '$E' = CCITT Chinese Set (GB 2312 + GB 8565),
     * '$G' = CNS 11643 plane 1,
     * '$H' = CNS 11643 plane 2,
     * '$I' = CNS 11643 plane 3,
     * '$J' = CNS 11643 plane 4,
     * '$K' = CNS 11643 plane 5,
     * '$L' = CNS 11643 plane 6,
     * '$M' = CNS 11643 plane 7,
     * '$O' = JIS X 0213 plane 1,
     * '$P' = JIS X 0213 plane 2,
     * '$Q' = JIS X 0213-2004 Plane 1,
     * and many others that are rarely used for e.g. traditional
     * ideographic Vietnamese and BlissSymbolics;
     * (double-byte 96x96-charsets)
     * none standardized or in use on terminals AFAIK (Mule does use
     * some internally)
     */
    uint32_t gn[4];
    /* ss = single-shift state: 0 = GL, 2 = G2, 3 = G3 */
    uint8_t ss;
};

class Term : public Entity
{
public:
    Term(ivec2 size);
    ~Term();

    char const *GetName() { return "<term>"; }
    caca_canvas_t *GetCaca() { return m_caca; }

protected:
    virtual void TickGame(float seconds);
    virtual void TickDraw(float seconds, Scene &scene);

private:
    /* Terminal emulation main entry point */
    size_t ReadAnsi(void const *data, size_t size);
    size_t SendAnsi(char const *str);

    /* Utility functions for terminal emulation */
    void ReadGrcm(unsigned int argc, unsigned int const *argv);
    inline int ReadChar(unsigned char c, int *x, int *y);
    inline int ReadDuplet(unsigned char const *buffer, unsigned int *skip,
                          int top, int bottom, int width, int height);

private:
    Pty *m_pty;
    caca_canvas_t *m_caca;
    ivec2 m_size;

    /* Terminal attributes */
    char *m_title;

    int m_bell, m_init, m_report_mouse;
    int m_changed;                  /* content was updated */
    Iso2022Conversion m_conv_state; /* charset mess */

    uint8_t m_fg, m_bg;   /* ANSI-context fg/bg */
    uint8_t m_dfg, m_dbg; /* Default fg/bg */
    uint8_t m_bold, m_blink, m_italics, m_negative, m_concealed, m_underline;
    uint8_t m_faint, m_strike, m_proportional; /* unsupported */
    uint32_t m_clearattr;

    /* Mostly for fancy shit */
    void DrawFancyShit();
    float m_time;
    bool m_debug;
};

