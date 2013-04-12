#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sndfile.h>
#include "config.h"
#include "getcdaudio.h"

int
main(int argc, char *argv[])
{
  int BUF_FRAMES = 44100 * 600;

  GCA_T gca;
  SF_INFO sinfo;
  sf_count_t st_frame, ed_frame, cnt_frames;
  sf_count_t frames;
  int datasize;
  int optchar;
  int mode_info = 0;
  opterr = 0;

  memset(&gca, 0, sizeof(GCA_T));

  while((optchar = getopt(argc, argv, "b:e:o:ih")) != -1) {
    switch(optchar) {
      case 'b':
      case 'e':
        if(check_frame_format(optarg) != 0) {
          fprintf(stderr, "\"%s\" is invalid format for -%c", optarg, optchar);
          exit(EXIT_FAILURE);
        } else {
          if (optchar == 'b') {
            gca.st = strdup(optarg);
          } else {
            gca.ed = strdup(optarg);
          }
        }
        break;
      case 'o':
        gca.fname = strdup(optarg);
        break;
      case 'i':
        mode_info = 1;
        break;
      case 'h':
      case '?':
      default:
        disp_help();
        exit(EXIT_SUCCESS);
        break;
    }
  }

  if (argv[optind] == NULL) {
    fprintf(stderr, "File is not specified.\n");
    exit(EXIT_FAILURE);
  }

  // open input sound file
  gca.sf = sf_open(argv[optind], SFM_READ, &sinfo);

  // decode frame
  st_frame = str2frame(gca.st, sinfo.samplerate);
  ed_frame = str2frame(gca.ed, sinfo.samplerate);
  cnt_frames = ed_frame - st_frame;

  if (cnt_frames < 0) {
    fprintf(stderr, "Begin frame is later than end.\n");
    close_handles(&gca);
    exit(EXIT_FAILURE);
  }
  
  if (mode_info == 1) {
    disp_info(&sinfo);
    printf("Specified count of frames: %d\n", (int)cnt_frames);
    close_handles(&gca);
    exit(EXIT_SUCCESS);
  }


  // get buffer
  datasize = sizeof(short) * sinfo.channels * BUF_FRAMES;
  gca.audiodata = (short *)malloc(datasize);

  // seek frames
  sf_seek(gca.sf, st_frame, SEEK_SET);

  // open output sndfile
  if (gca.fname != NULL) {
    gca.fp = fopen(gca.fname, "w");
    if (gca.fp == NULL) {
      fprintf(stderr, "%s\n", strerror(errno));
      close_handles(&gca);
      exit(EXIT_FAILURE);
    }
  } else {
    gca.fp = stdout;
  }

  // read frame
  while(cnt_frames > 0) {
    frames = sf_readf_short(gca.sf, gca.audiodata, BUF_FRAMES);
    if (frames > cnt_frames) {
      fwrite(gca.audiodata, sizeof(short), cnt_frames * sinfo.channels, gca.fp);
    } else {
      fwrite(gca.audiodata, sizeof(short), frames * sinfo.channels, gca.fp);
    }
    cnt_frames -= frames;
  }

  close_handles(&gca);

  return 0;
}

int
check_frame_format (char *idxtime)
{
  char *buf = strdup(idxtime);
  char *p;
  int ret;

  p = strtok(buf, "0123456789:");
  ret = (p == NULL) ? 0 : 1;

  free(buf);
  return ret;
}

sf_count_t
str2frame (char *idxtime, int samplerate)
{
  sf_count_t frm = 0;
  int part;
  char *buf;
  char *idxp[10];
  int cnt, i;

  buf = strdup(idxtime);
  idxp[0] = strtok(buf, ":");

  for (cnt=1; (idxp[cnt] = strtok(NULL, ":")) != NULL; cnt++);

  cnt--;
  for(i=0; cnt-i >= 0; i++) {
    part = atoi(idxp[cnt-i]);
    switch(i) {
      case 0:
        frm += part * (samplerate/75);
        break;
      case 1:
        frm += part * samplerate;
        break;
      case 2:
        frm += part * 60 * samplerate;
        break;
      default:
        break;
    }
  }
  free(buf);
  return frm;
}

void
close_handles(GCA_T *pgca)
{
  if (pgca->st != NULL) free(pgca->st);
  if (pgca->ed != NULL) free(pgca->ed);
  if (pgca->fname != NULL) free(pgca->fname);
  if (pgca->st != NULL) free(pgca->st);
  if (pgca->ed != NULL) free(pgca->ed);
  if (pgca->audiodata != NULL) free(pgca->audiodata);

  if (pgca->fp != NULL) fclose(pgca->fp);
  if (pgca->sf != NULL) sf_close(pgca->sf);
}

void
disp_info (SF_INFO *sinfo)
{
  // print sound info
  printf("Frames: %d\n", (int)sinfo->frames);
  printf("Sample Rate: %d\n", sinfo->samplerate);
  printf("Channels: %d\n", sinfo->channels);
  printf("Format: %08X\n", sinfo->format);
  printf("Sections: %d\n", sinfo->sections);
  printf("Seekable: %d\n", sinfo->seekable);
}

void
disp_help (void)
{
  printf("%s %s\n", PACKAGE, VERSION);
  printf("Copyright (C) 2013 %s\n", AUTHOR);
  printf("\n");
  printf("Usage:\n");
  printf("%s [OPTION] INFILE\n", PACKAGE);
  printf("\n");
  printf("  -b hh:ss:ff   Begin frame.\n");
  printf("  -e hh:ss:ff   End frame.\n");
  printf("  -o OUTFILE    Output file. if not specified, output to stdout.\n");
  printf("  -i            Show information for input file.\n");
  printf("  -h            This help.\n");
  printf("\n");
  printf("INFILE should be RIFF WAV file. Output is raw PCM (LR/16bit/44.1kHz/LE)\n");
}
