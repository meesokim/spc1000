/*
 * video.h
 *
 * display screen management
 */

/* $Id: video.h,v 1.10 2000/08/22 02:33:44 nyef Exp $ */

#ifndef __VIDEO_H__
#define __VIDEO_H__

/* control functions */

void video_run(void);
void video_enter_deb(void);
void video_leave_deb(void);
void video_display_buffer(void);
void video_setsize(int x, int y);
void video_setpal(int num_colors, int *red, int *green, int *blue);
unsigned char *video_get_vbp(int line);

extern unsigned char *vid_pre_xlat;

#endif /* __VIDEO_H__ */

/*
 * $Log: video.h,v $
 * Revision 1.10  2000/08/22 02:33:44  nyef
 * removed video_init() and video_shutdown() prototypes (they should be
 * internal to the UI/video layer, not part of the public interface)
 *
 * Revision 1.9  2000/05/15 01:22:40  nyef
 * changed not to use procpointers in the interface
 *
 * Revision 1.8  2000/05/07 02:12:31  nyef
 * added "extern" to some variable declarations
 *
 * Revision 1.7  1999/02/07 17:06:26  nyef
 * added video_setsize() interface
 *
 * Revision 1.6  1999/02/06 03:37:12  nyef
 * removed video_events() interface
 *
 * Revision 1.5  1999/01/05 04:28:26  nyef
 * added hacked up interface to allow setting the palette.
 *
 * Revision 1.4  1998/08/29 22:12:30  nyef
 * added video buffer interface for the PPU in the form of video_get_vbp.
 *
 * Revision 1.3  1998/08/28 03:04:37  nyef
 * added in the char pointer "vid_pre_xlat" in order to move some of
 * the video interface into the PPU without causing too much of an
 * abstraction violation.
 *
 * Revision 1.2  1998/08/01 00:55:00  nyef
 * added video_run() in anticipation of a GUI version.
 *
 * Revision 1.1  1998/07/11 22:19:18  nyef
 * Initial revision
 *
 */
