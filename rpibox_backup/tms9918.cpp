/*
 * tms9918.c
 *
 * tms9918 VDP emulation.
 */

/* $Id: tms9918.c,v 1.24 2000/11/23 16:22:23 nyef Exp $ */

#include <stdlib.h>
#include <string.h>
#include "tms9918.h"
#include "ui.h"
#include "video.h"

#define TMS_RAMSIZE 0x4000

#define TF_ADDRWRITE 1

/* VDP structure definition */

struct _tms9918 {
    unsigned char flags;
    unsigned char readahead;
    unsigned char addrsave;
    unsigned char status;
    unsigned char *memory;
    unsigned char regs[8];
    unsigned short address;
    unsigned short scanline;
    u8 palette[16];
};

/* palette definition */

int tms9918_palbase_red[16] = {
    0x00, 0x00, 0x22, 0x66, 0x22, 0x44, 0xbb, 0x44,
    0xff, 0xff, 0xdd, 0xdd, 0x22, 0xdd, 0xbb, 0xff,
};

int tms9918_palbase_green[16] = {
    0x00, 0x00, 0xdd, 0xff, 0x22, 0x66, 0x22, 0xdd,
    0x22, 0x66, 0xdd, 0xdd, 0x99, 0x44, 0xbb, 0xff,
};

int tms9918_palbase_blue[16] = {
    0x00, 0x00, 0x22, 0x66, 0xff, 0xff, 0x11, 0xff,
    0x22, 0x66, 0x22, 0x99, 0x22, 0xbb, 0xbb, 0xff,
};

unsigned char tms9918_readport0(tms9918 vdp)
{
    unsigned char retval;
    
    retval = vdp->readahead;
    vdp->readahead = vdp->memory[vdp->address++];
    vdp->address &= 0x3fff;
    vdp->flags &= ~TF_ADDRWRITE;
    return retval;
}

unsigned char tms9918_readport1(tms9918 vdp)
{
    unsigned char retval;

    retval = vdp->status;
    vdp->status &= 0x1f;
    vdp->flags &= ~TF_ADDRWRITE;
    return retval;
}

void tms9918_writeport0(tms9918 vdp, unsigned char data)
{
    vdp->readahead = data;
    vdp->memory[vdp->address++] = data;
    vdp->address &= 0x3fff;
    vdp->flags &= ~TF_ADDRWRITE;
}

void tms9918_writeport1(tms9918 vdp, unsigned char data)
{
    if (vdp->flags & TF_ADDRWRITE) {
	if (data & 0x80) {
/* 	    deb_printf("tms9918: register write %d = %02x.\n", data & 7, vdp->addrsave); */
	    vdp->regs[data & 7] = vdp->addrsave;
	} else {
/* 	    deb_printf("tms9918: address write.\n"); */
	    vdp->address = (vdp->addrsave | (data << 8)) & 0x3fff;
	    if (!(data & 0x40)) {
		vdp->readahead = vdp->memory[vdp->address++];
		vdp->address &= 0x3fff;
	    }
	}
	vdp->flags &= ~TF_ADDRWRITE;
    } else {
	vdp->addrsave = data;
	vdp->flags |= TF_ADDRWRITE;
    }
}

struct sprite_data {
    u8 y_pos;
    u8 x_pos;
    u8 pattern;
    u8 color;
};

struct sprite_cache {
    int x_pos;
    u8 *pattern;
    u8 color;
};

int tms9918_cache_sprites(tms9918 vdp, struct sprite_cache *cache, int sprite_size)
{
    struct sprite_data *sprites;
    unsigned char *pattern_table;
    int num_sprites;
    int i;
    
    num_sprites = 0;

    sprites = (struct sprite_data *)&vdp->memory[(vdp->regs[5] & 0x7f) << 7];
    pattern_table = &vdp->memory[(vdp->regs[6] & 0x07) << 11];
    
    for (i = 0; i < 32; i++) {
	if (sprites[i].y_pos == 208) {
	    break;
	}
	if (sprites[i].y_pos >= vdp->scanline) {
	    continue;
	}
	if ((sprites[i].y_pos + sprite_size) < vdp->scanline) {
	    continue;
	}
	if (num_sprites == 4) {
	    vdp->status |= 0x40; /* fifth sprite flag */
	    break;
	}
	cache[num_sprites].color = sprites[i].color & 0x0f;
	cache[num_sprites].x_pos = sprites[i].x_pos;
	if (sprites[i].color & 0x80) {
	    cache[num_sprites].x_pos -= 32;
	}
	cache[num_sprites].pattern = &pattern_table[sprites[i].pattern << 3];
	cache[num_sprites].pattern += vdp->scanline - (sprites[i].y_pos + 1);
	num_sprites++;
    }

    /* fifth sprite id */
    vdp->status &= 0xe0;
    vdp->status |= i;
    
    return num_sprites;
}

int tms9918_check_sprite_collision(tms9918 vdp, struct sprite_cache *cache, int sprite_size, int num_sprites)
{
    switch (num_sprites) {
    case 4:
	/* NOTE: & 0x1ff here is to ensure that the result is positive */
	if (((cache[3].x_pos - cache[2].x_pos) & 0x1ff) < sprite_size) {
	    return 1;
	}
	if (((cache[3].x_pos - cache[1].x_pos) & 0x1ff) < sprite_size) {
	    return 1;
	}
	if (((cache[3].x_pos - cache[0].x_pos) & 0x1ff) < sprite_size) {
	    return 1;
	}
    case 3:
	if (((cache[2].x_pos - cache[1].x_pos) & 0x1ff) < sprite_size) {
	    return 1;
	}
	if (((cache[2].x_pos - cache[0].x_pos) & 0x1ff) < sprite_size) {
	    return 1;
	}
    case 2:
	if (((cache[1].x_pos - cache[0].x_pos) & 0x1ff) < sprite_size) {
	    return 1;
	}
    case 1:
    default:
	/* not enough sprites to collide */
	return 0;
    }
}

void tms9918_render_sprites(tms9918 vdp, unsigned char *cur_vbp)
{
    struct sprite_cache cache[4];
    int num_sprites;
    int i;
    int sprite_size;

    sprite_size = (vdp->regs[1] & 0x02)? 16: 8;
    
    num_sprites = tms9918_cache_sprites(vdp, cache, sprite_size);

    if (tms9918_check_sprite_collision(vdp, cache, sprite_size, num_sprites)) {
	vdp->status |= 0x20; /* sprite collision flag */
    }
    
    for (i = num_sprites - 1; i >= 0; i--) {
	u8 color;
	u8 *tmp_vbp;
	u8 *pattern;
	u16 data;

	color = vdp->palette[cache[i].color];
	tmp_vbp = cur_vbp + cache[i].x_pos;
	pattern = cache[i].pattern;

	if (!cache[i].color) {
	    /* no point in drawing an invisible sprite */
	    continue;
	}

	if ((cache[i].x_pos + sprite_size) < 0) {
	    continue;
	}

	data = (pattern[0] << 8) | pattern[16];

	if (cache[i].x_pos < 0) {
	    /* clip to left edge of screen */
	    data &= 0xffff >> -cache[i].x_pos;
	} else if ((cache[i].x_pos + sprite_size) > 255) {
	    /* clip to right edge of screen */
	    data &= ((sprite_size == 8)? 0xff00: 0xffff)
		<< ((cache[i].x_pos + sprite_size + 1) & 0xff);
	}
	
	if (data & 0x8000) {
	    *tmp_vbp = color;
	}
	tmp_vbp++;
	if (data & 0x4000) {
	    *tmp_vbp = color;
	}
	tmp_vbp++;
	if (data & 0x2000) {
	    *tmp_vbp = color;
	}
	tmp_vbp++;
	if (data & 0x1000) {
	    *tmp_vbp = color;
	}
	tmp_vbp++;
	if (data & 0x0800) {
	    *tmp_vbp = color;
	}
	tmp_vbp++;
	if (data & 0x0400) {
	    *tmp_vbp = color;
	}
	tmp_vbp++;
	if (data & 0x0200) {
	    *tmp_vbp = color;
	}
	tmp_vbp++;
	if (data & 0x0100) {
	    *tmp_vbp = color;
	}
	if (sprite_size == 16) {
	    tmp_vbp++;
	    if (data & 0x80) {
		*tmp_vbp = color;
	    }
	    tmp_vbp++;
	    if (data & 0x40) {
		*tmp_vbp = color;
	    }
	    tmp_vbp++;
	    if (data & 0x20) {
		*tmp_vbp = color;
	    }
	    tmp_vbp++;
	    if (data & 0x10) {
		*tmp_vbp = color;
	    }
	    tmp_vbp++;
	    if (data & 0x08) {
		*tmp_vbp = color;
	    }
	    tmp_vbp++;
	    if (data & 0x04) {
		*tmp_vbp = color;
	    }
	    tmp_vbp++;
	    if (data & 0x02) {
		*tmp_vbp = color;
	    }
	    tmp_vbp++;
	    if (data & 0x01) {
		*tmp_vbp = color;
	    }
	}
    }
}

void tms9918_render_line_mode_0(tms9918 vdp)
{
    unsigned char *cur_vbp;
    unsigned char *nametable;
    unsigned char *patterntable;
    unsigned char *colortable;
    unsigned char color0;
    unsigned char color1;
    unsigned char pattern;
    int i;
    
    cur_vbp = video_get_vbp(vdp->scanline);
    nametable = vdp->memory + ((vdp->regs[2] & 0x0f) << 10);

    patterntable = vdp->memory + ((vdp->regs[4] & 0x07) << 11);
    patterntable += vdp->scanline & 7;
    nametable += (vdp->scanline & ~7) << 2;
    colortable = vdp->memory + (vdp->regs[3] << 6);
    
    for (i = 0; i < 32; i++) {
	pattern = patterntable[nametable[i] << 3];
	color0 = vdp->palette[colortable[nametable[i] >> 3] & 15];
	color1 = vdp->palette[colortable[nametable[i] >> 3] >> 4];
	if (pattern & 0x80) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x40) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x20) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x10) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x08) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x04) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x02) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x01) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
    }
}

void tms9918_render_line_mode_1(tms9918 vdp)
{
    unsigned char *cur_vbp;
    unsigned char *nametable;
    unsigned char *patterntable;
    unsigned char pattern;
    unsigned char color0;
    unsigned char color1;
    int i;
    
    cur_vbp = video_get_vbp(vdp->scanline);
    nametable = vdp->memory + ((vdp->regs[2] & 0x0f) << 10);

    patterntable = vdp->memory + ((vdp->regs[4] & 0x07) << 11);
    patterntable += vdp->scanline & 7;
    nametable += (vdp->scanline & ~7) * 5;
    
    color0 = vdp->palette[vdp->regs[7] & 15];
    color1 = vdp->palette[vdp->regs[7] >> 4];

    for (i = 0; i < 40; i++) {
	pattern = patterntable[nametable[i] << 3];
	if (pattern & 0x80) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x40) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x20) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x10) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x08) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x04) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
    }

    /*
     * FIXME: This looks a little hacky. If it must remain perhaps
     * it should be 8 pels on either side of the main display?
     */
    for (i=0; i<16; i++)
	*cur_vbp++ = color0;
}

void tms9918_render_line_mode_2(tms9918 vdp)
{
    unsigned char *cur_vbp;
    unsigned char *nametable;
    unsigned char *patterntable;
    unsigned char *colortable;
    unsigned char pattern;
    unsigned char color0;
    unsigned char color1;
    int i;
    
    cur_vbp = video_get_vbp(vdp->scanline);
    nametable = vdp->memory + ((vdp->regs[2] & 0x0f) << 10);

    patterntable = vdp->memory + ((vdp->regs[4] & 0x04)? 0x2000: 0);
    colortable = vdp->memory + ((vdp->regs[3] & 0x80)? 0x2000: 0);
    patterntable += vdp->scanline & 7;
    colortable += vdp->scanline & 7;
    if (vdp->scanline >= 0x80) {
	if (vdp->regs[4] & 0x02) {
	    patterntable += (0x200 << 3);
	    colortable += (0x200 << 3);
	}
    } else if (vdp->scanline >= 0x40) {
	if (vdp->regs[4] & 0x01) {
	    patterntable += (0x100 << 3);
	    colortable += (0x100 << 3);
	}
    }
    
    nametable += (vdp->scanline & ~7) << 2;
    
    for (i = 0; i < 32; i++) {
	pattern = patterntable[nametable[i] << 3];
	color0 = vdp->palette[colortable[nametable[i] << 3] & 0x0f];
	color1 = vdp->palette[colortable[nametable[i] << 3] >> 4];
	if (pattern & 0x80) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x40) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x20) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x10) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x08) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x04) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x02) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
	if (pattern & 0x01) {
	    *cur_vbp++ = color1;
	} else {
	    *cur_vbp++ = color0;
	}
    }
}

void tms9918_render_line_mode_3(tms9918 vdp)
{
    unsigned char *cur_vbp;
    unsigned char *nametable;
    unsigned char *patterntable;
    unsigned char color0;
    unsigned char color1;
    unsigned char pattern;
    int i;
    
    cur_vbp = video_get_vbp(vdp->scanline);
    nametable = vdp->memory + ((vdp->regs[2] & 0x0f) << 10);
    
    patterntable = vdp->memory + ((vdp->regs[4] & 0x07) << 11);
    patterntable += (vdp->scanline & 28) >> 2;
    nametable += (vdp->scanline & ~7) << 2;
    
    for (i = 0; i < 32; i++) {
	pattern = patterntable[nametable[i] << 3];
	color0 = vdp->palette[pattern & 15];
	color1 = vdp->palette[pattern >> 4];
	/* FIXME: long (32bit) writes should be faster */
	*cur_vbp++ = color1;
	*cur_vbp++ = color1;
	*cur_vbp++ = color1;
	*cur_vbp++ = color1;
	*cur_vbp++ = color0;
	*cur_vbp++ = color0;
	*cur_vbp++ = color0;
	*cur_vbp++ = color0;
    }
}

void tms9918_render_line_mode_unknown(tms9918 vdp)
{
    // deb_printf("tms9918: unsupported display mode %c%c%c.\n",
	       // (vdp->regs[1] & 0x10)? '1': '-',
	       // (vdp->regs[0] & 0x02)? '2': '-',
	       // (vdp->regs[1] & 0x08)? '3': '-');
}

typedef void (*tms9918_linerenderer)(tms9918 vdp);
tms9918_linerenderer tms9918_linerenderers[8] = {
    tms9918_render_line_mode_0,       /* 0 */
    tms9918_render_line_mode_1,       /* 1 */
    tms9918_render_line_mode_2,       /* 2 */
    tms9918_render_line_mode_unknown, /* 1 + 2 */
    tms9918_render_line_mode_3,       /* 3 */
    tms9918_render_line_mode_unknown, /* 1 + 3 */
    tms9918_render_line_mode_unknown, /* 2 + 3 */
    tms9918_render_line_mode_unknown, /* 1 + 2 + 3 */
};

void tms9918_render_line(tms9918 vdp)
{
    unsigned char *cur_vbp;
    int mode;

    cur_vbp = video_get_vbp(vdp->scanline);

    /* set up background color */
    if (vdp->regs[7] & 0x0f) {
	vdp->palette[0] = vdp->palette[vdp->regs[7] & 0x0f];
    } else {
	vdp->palette[0] = vdp->palette[1];
    }
    
    if (!(vdp->regs[1] & 0x40)) {
	memset(cur_vbp, vdp->palette[1], 256);
	return;
    }

    mode = 0;
    if (vdp->regs[1] & 0x10) {
	mode |= 1;
    }
    if (vdp->regs[0] & 0x02) {
	mode |= 2;
    }
    if (vdp->regs[1] & 0x08) {
	mode |= 4;
    }
    tms9918_linerenderers[mode](vdp);

    if (!(mode & 1)) {
	tms9918_render_sprites(vdp, cur_vbp);
    }
}

int tms9918_periodic(tms9918 vdp)
{
    if (vdp->scanline < 192) {
	tms9918_render_line(vdp);
    } else if (vdp->scanline == 192) {
	video_display_buffer();
	vdp->status |= 0x80; /* signal vblank */
    }
    if (vdp->scanline == 261) {
	vdp->scanline = 0;
    } else {
	vdp->scanline++;
    }
    return (vdp->status & 0x80) && (vdp->regs[1] & 0x20);
}

void tms9918_init_palette(tms9918 vdp)
{
    int i;

    for (i = 0; i < 16; i++) {
	vdp->palette[i] = i;//vid_pre_xlat[i];
    }
}

tms9918 tms9918_create(void)
{
    tms9918 retval;

    retval = (tms9918) calloc(1, sizeof(struct _tms9918));
    if (retval) {
	retval->memory = (unsigned char *)calloc(1, TMS_RAMSIZE);
	if (retval->memory) {
	    retval->scanline = 0;
            video_setsize(256, 192);
	    video_setpal(16, tms9918_palbase_red, tms9918_palbase_green, tms9918_palbase_blue);
	    tms9918_init_palette(retval);
	} else {
	    free(retval);
	    retval = NULL;
	}
    }
    
    if (!retval) {
	//deb_printf("tms9918_create(): out of memory.\n");
    }

    return retval;
}

/*
 * $Log: tms9918.c,v $
 * Revision 1.24  2000/11/23 16:22:23  nyef
 * added preliminary mode 1 line renderer
 *
 * Revision 1.23  2000/08/26 00:57:15  nyef
 * removed all reference to sms_psg (moved out to the driver level)
 *
 * Revision 1.22  2000/01/18 01:43:17  nyef
 * fixed bugs in the handling of invisible sprites and right clipped sprites
 *
 * Revision 1.21  2000/01/15 21:35:39  nyef
 * added preliminary implementation of sprite collision (not tested)
 *
 * Revision 1.20  2000/01/15 20:59:49  nyef
 * hacked in something to handle transparent colors in the background
 *
 * Revision 1.19  2000/01/15 20:24:07  nyef
 * added clipping of sprites to the screen edges
 *
 * Revision 1.18  2000/01/15 19:29:28  nyef
 * added mode 3 emulation from AmiDog (not tested)
 *
 * Revision 1.17  2000/01/15 19:25:36  nyef
 * fixed a bug with the pattern table in tms9918_render_line_mode_0()
 *
 * Revision 1.16  2000/01/15 17:58:27  nyef
 * added (untested) implementation of the "fifth sprite" status bits
 *
 * Revision 1.15  2000/01/15 16:29:09  nyef
 * fixed to check all 32 sprites (instead of just the first 31)
 *
 * Revision 1.14  2000/01/15 15:50:37  nyef
 * split sprite rendering up into two functions
 * changed to use a structure to hold sprite data for rendering
 * added support for "early clock" to the sprite renderer
 *
 * Revision 1.13  1999/12/04 06:14:10  nyef
 * fixed obi-wan error in the sprite renderer
 *
 * Revision 1.12  1999/11/27 23:06:04  nyef
 * added preliminary support for 16x16 sprites
 *
 * Revision 1.11  1999/11/27 22:51:56  nyef
 * added preliminary sprite implementation
 *
 * Revision 1.10  1999/11/27 21:26:30  nyef
 * added colortable to sms9918_render_line_mode_2()
 *
 * Revision 1.9  1999/11/27 21:13:41  nyef
 * cleaned up the patterntable logic in tms9918_render_line_mode_2()
 *
 * Revision 1.8  1999/11/27 20:32:16  nyef
 * rebuilt the video renderer mode selection to use a table of procpointers
 *
 * Revision 1.7  1999/11/27 20:25:02  nyef
 * split out tms9918_render_line_mode_[02]() from tms9918_render_line()
 *
 * Revision 1.6  1999/11/27 20:16:44  nyef
 * disabled debug output on register write
 *
 * Revision 1.5  1999/11/27 19:18:21  nyef
 * moved the vdp data structure in from tms9918.h
 * stripped out the procpointers from the data structure
 *
 * Revision 1.4  1999/06/10 01:50:08  nyef
 * fixed pattern lookup in mode 2
 *
 * Revision 1.3  1999/06/10 00:41:59  nyef
 * fixed colors in mode 0
 *
 * Revision 1.2  1999/06/09 00:34:08  nyef
 * added preliminary implementation of modes 0 and 2
 *
 * Revision 1.1  1999/06/08 01:49:25  nyef
 * Initial revision
 *
 */
