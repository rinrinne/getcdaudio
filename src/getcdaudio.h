#ifndef GETAUDIO_H
#define GETAUDIO_H

#define AUTHOR "rinrinne a.k.a. rin_ne"

struct _GCA {
  SNDFILE *sf;
  char *fname;
  FILE *fp;
  char *st;
  char *ed;
  short *audiodata;
};

typedef struct _GCA GCA_T;

void disp_help(void);
void disp_info(SF_INFO *);
sf_count_t str2frame(char *, int);
int check_frame_format(char *);
void close_handles(GCA_T *);

#endif
