/*
 *      Command line frontend program
 *
 *      Copyright (c) 1999 Mark Taylor
 *                    2000 Takehiro TOMINAGA
 *                    2010-2017 Robert Hegemann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* $Id: lame_main.c,v 1.18 2017/08/31 14:14:46 robert Exp $ */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <stdio.h>

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
char   *strchr(), *strrchr();
# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#ifdef __sun__
/* woraround for SunOS 4.x, it has SEEK_* defined here */
#include <unistd.h>
#endif

#if defined(_WIN32)
# include <windows.h>
#endif


/*
 main.c is example code for how to use libmp3lame.a.  To use this library,
 you only need the library and lame.h.  All other .h files are private
 to the library.
*/
#include "lame.h"

#include "console.h"
#include "main.h"
#include "parse.h"
#include "get_audio.h"
// #include "timestatus.h"

/* PLL 14/04/2000 */
#if macintosh
#include <console.h>
#endif

#ifdef WITH_DMALLOC
#include <dmalloc.h>
#endif


/************************************************************************
*
* main
*
* PURPOSE:  MPEG-1,2 Layer III encoder with GPSYCHO
* psychoacoustic model.
*
************************************************************************/


static FILE *
init_files(lame_global_flags * gf, char const *inPath, char const *outPath)
{
    FILE   *outf;
    /* Mostly it is not useful to use the same input and output name.
       This test is very easy and buggy and don't recognize different names
       assigning the same file
     */
    if (0 != strcmp("-", outPath) && 0 == strcmp(inPath, outPath)) {
        error_printf("Input file and Output file are the same. Abort.\n");
        return NULL;
    }

    /* open the wav/aiff/raw pcm or mp3 input file.  This call will
     * open the file, try to parse the headers and
     * set gf.samplerate, gf.num_channels, gf.num_samples.
     * if you want to do your own file input, skip this call and set
     * samplerate, num_channels and num_samples yourself.
     */
    if (init_infile(gf, inPath) < 0) {
        error_printf("Can't init infile '%s'\n", inPath);
        return NULL;
    }
    if ((outf = init_outfile(outPath, lame_get_decode_only(gf))) == NULL) {
        error_printf("Can't init outfile '%s'\n", outPath);
        return NULL;
    }

    return outf;
}


static void
printInputFormat(lame_t gfp)
{
    int const v_main = 2 - lame_get_version(gfp);
    char const *v_ex = lame_get_out_samplerate(gfp) < 16000 ? ".5" : "";
    switch (global_reader.input_format) {
    case sf_mp123:     /* FIXME: !!! */
        break;
    case sf_mp3:
        console_printf("MPEG-%u%s Layer %s", v_main, v_ex, "III");
        break;
    case sf_mp2:
        console_printf("MPEG-%u%s Layer %s", v_main, v_ex, "II");
        break;
    case sf_mp1:
        console_printf("MPEG-%u%s Layer %s", v_main, v_ex, "I");
        break;
    case sf_raw:
        console_printf("raw PCM data");
        break;
    case sf_wave:
        console_printf("Microsoft WAVE");
        break;
    case sf_aiff:
        console_printf("SGI/Apple AIFF");
        break;
    default:
        console_printf("unknown");
        break;
    }
}

static void
print_trailing_info(lame_global_flags * gf)
{
    if (lame_get_findReplayGain(gf)) {
        int     RadioGain = lame_get_RadioGain(gf);
        console_printf("ReplayGain: %s%.1fdB\n", RadioGain > 0 ? "+" : "",
                       ((float) RadioGain) / 10.0);
        if (RadioGain > 0x1FE || RadioGain < -0x1FE)
            error_printf
                ("WARNING: ReplayGain exceeds the -51dB to +51dB range. Such a result is too\n"
                 "         high to be stored in the header.\n");
    }

    /* if (the user requested printing info about clipping) and (decoding
       on the fly has actually been performed) */
    if (global_ui_config.print_clipping_info && lame_get_decode_on_the_fly(gf)) {
        float   noclipGainChange = (float) lame_get_noclipGainChange(gf) / 10.0f;
        float   noclipScale = lame_get_noclipScale(gf);

        if (noclipGainChange > 0.0) { /* clipping occurs */
            console_printf
                ("WARNING: clipping occurs at the current gain. Set your decoder to decrease\n"
                 "         the  gain  by  at least %.1fdB or encode again ", noclipGainChange);

            /* advice the user on the scale factor */
            if (noclipScale > 0) {
                console_printf("using  --scale %.2f\n", noclipScale * lame_get_scale(gf));
                console_printf("         or less (the value under --scale is approximate).\n");
            }
            else {
                /* the user specified his own scale factor. We could suggest
                 * the scale factor of (32767.0/gfp->PeakSample)*(gfp->scale)
                 * but it's usually very inaccurate. So we'd rather advice him to
                 * disable scaling first and see our suggestion on the scale factor then. */
                console_printf("using --scale <arg>\n"
                               "         (For   a   suggestion  on  the  optimal  value  of  <arg>  encode\n"
                               "         with  --scale 1  first)\n");
            }

        }
        else {          /* no clipping */
            if (noclipGainChange > -0.1)
                console_printf
                    ("\nThe waveform does not clip and is less than 0.1dB away from full scale.\n");
            else
                console_printf
                    ("\nThe waveform does not clip and is at least %.1fdB away from full scale.\n",
                     -noclipGainChange);
        }
    }

}


static int
write_xing_frame(lame_global_flags * gf, FILE * outf, size_t offset)
{
    unsigned char mp3buffer[LAME_MAXMP3BUFFER];
    size_t  imp3, owrite;

    imp3 = lame_get_lametag_frame(gf, mp3buffer, sizeof(mp3buffer));
    if (imp3 <= 0) {
        return 0;       /* nothing to do */
    }
    if (global_ui_config.silent <= 0) {
        console_printf("Writing LAME Tag...");
    }
    if (imp3 > sizeof(mp3buffer)) {
        error_printf
            ("Error writing LAME-tag frame: buffer too small: buffer size=%d  frame size=%d\n",
             sizeof(mp3buffer), imp3);
        return -1;
    }
    assert( offset <= LONG_MAX );
    if (fseek(outf, (long) offset, SEEK_SET) != 0) {
        error_printf("fatal error: can't update LAME-tag frame!\n");
        return -1;
    }
    owrite = fwrite(mp3buffer, 1, imp3, outf);
    // dump_buffer(mp3buffer, owrite);
    if (owrite != imp3) {
        error_printf("Error writing LAME-tag \n");
        return -1;
    }
    if (global_ui_config.silent <= 0) {
        console_printf("done\n");
    }
    assert( imp3 <= INT_MAX );
    return (int) imp3;
}


static int
write_id3v1_tag(lame_t gf, FILE * outf)
{
    unsigned char mp3buffer[128];
    size_t  imp3, owrite;

    imp3 = lame_get_id3v1_tag(gf, mp3buffer, sizeof(mp3buffer));
    LOG("imp3:%d", imp3);
    if (imp3 <= 0) {
        return 0;
    }
    if (imp3 > sizeof(mp3buffer)) {
        error_printf("Error writing ID3v1 tag: buffer too small: buffer size=%d  ID3v1 size=%d\n",
                     sizeof(mp3buffer), imp3);
        return 0;       /* not critical */
    }
    owrite = fwrite(mp3buffer, 1, imp3, outf);
    dump_buffer(mp3buffer, 20);
    if (owrite != imp3) {
        error_printf("Error writing ID3v1 tag \n");
        return 1;
    }
    return 0;
}


static int
lame_encoder_loop(lame_global_flags * gf, FILE * outf, int nogap, char *inPath, char *outPath)
{
    unsigned char mp3buffer[LAME_MAXMP3BUFFER];
    int     Buffer[2][1152];
    int     iread, imp3, owrite, in_limit=0;
    size_t  id3v2_size = 0;

    // encoder_progress_begin(gf, inPath, outPath);

    // id3v2_size = lame_get_id3v2_tag(gf, 0, 0);
    // LOG("id3v2_size:%d", (int)id3v2_size);

    // if (id3v2_size > 0) {
    //     unsigned char *id3v2tag = malloc(id3v2_size);
    //     if (id3v2tag != 0) {
    //         size_t  n_bytes = lame_get_id3v2_tag(gf, id3v2tag, id3v2_size);
    //         size_t  written = fwrite(id3v2tag, 1, n_bytes, outf);
    //         free(id3v2tag);
    //         if (written != n_bytes) {
    //             encoder_progress_end(gf);
    //             error_printf("Error writing ID3v2 tag \n");
    //             return 1;
    //         }
    //     }
    // }
    // else {
    //     unsigned char* id3v2tag = getOldTag(gf);
    //     LOG("id3v2tag:%s", (const char *)id3v2tag);
    //     id3v2_size = sizeOfOldTag(gf);
    //     LOG("(OldTag)id3v2_size:%d", (int)id3v2_size);
    //     if ( id3v2_size > 0 ) {
    //         size_t owrite = fwrite(id3v2tag, 1, id3v2_size, outf);
    //         if (owrite != id3v2_size) {
    //             encoder_progress_end(gf);
    //             error_printf("Error writing ID3v2 tag \n");
    //             return 1;
    //         }
    //     }
    // }
    // if (global_writer.flush_write == 1) {
    //     fflush(outf);
    // }

    /* do not feed more than in_limit PCM samples in one encode call
       otherwise the mp3buffer is likely too small
     */
    in_limit = lame_get_maximum_number_of_samples(gf, sizeof(mp3buffer));
    LOG("in_limit:%d", in_limit);
    if (in_limit < 1)
        in_limit = 1;

    /* encode until we hit eof */
    do {
        /* read in 'iread' samples */
        iread = get_audio(gf, Buffer);
        // LOG("iread:%d", iread);

        if (iread >= 0) {
            const int* buffer_l = Buffer[0];
            const int* buffer_r = Buffer[1];
            int     rest = iread;
            do {
                int const chunk = rest < in_limit ? rest : in_limit;
                // encoder_progress(gf);

                /* encode */

                imp3 = lame_encode_buffer_int(gf, buffer_l, buffer_r, chunk,
                                              mp3buffer, sizeof(mp3buffer));
                buffer_l += chunk;
                buffer_r += chunk;
                rest -= chunk;

                /* was our output buffer big enough? */
                if (imp3 < 0) {
                    if (imp3 == -1)
                        error_printf("mp3 buffer is not big enough... \n");
                    else
                        error_printf("mp3 internal error:  error code=%i\n", imp3);
                    return 1;
                }
                owrite = (int) fwrite(mp3buffer, 1, imp3, outf);
                if (owrite != imp3) {
                    error_printf("Error writing mp3 output \n");
                    return 1;
                }
            } while (rest > 0);
        }
        if (global_writer.flush_write == 1) {
            fflush(outf);
        }
    } while (iread > 0);

    if (nogap)
        imp3 = lame_encode_flush_nogap(gf, mp3buffer, sizeof(mp3buffer)); /* may return one more mp3 frame */
    else
        imp3 = lame_encode_flush(gf, mp3buffer, sizeof(mp3buffer)); /* may return one more mp3 frame */

    if (imp3 < 0) {
        if (imp3 == -1)
            error_printf("mp3 buffer is not big enough... \n");
        else
            error_printf("mp3 internal error:  error code=%i\n", imp3);
        return 1;

    }

    // encoder_progress_end(gf);

    LOG("(remaining)imp3:%d", imp3);
    owrite = (int) fwrite(mp3buffer, 1, imp3, outf);
    if (owrite != imp3) {
        error_printf("Error writing mp3 output \n");
        return 1;
    }
    if (global_writer.flush_write == 1) {
        fflush(outf);
    }

    // LOG("write_id3v1_tag");
    // imp3 = write_id3v1_tag(gf, outf);
    // if (global_writer.flush_write == 1) {
    //     fflush(outf);
    // }
    // if (imp3) {
    //     return 1;
    // }
    LOG("write_xing_frame:%d(offset)", id3v2_size);
    write_xing_frame(gf, outf, id3v2_size);
    if (global_writer.flush_write == 1) {
        fflush(outf);
    }
    if (global_ui_config.silent <= 0) {
        print_trailing_info(gf);
    }
    return 0;
}


static int
lame_encoder(lame_global_flags * gf, FILE * outf, int nogap, char *inPath, char *outPath)
{
    int     ret;

    ret = lame_encoder_loop(gf, outf, nogap, inPath, outPath);
    fclose(outf);       /* close the output file */
    close_infile();     /* close the input file */
    return ret;
}

int Mp3Encoder(lame_t *encoder, const char *pcm, const char *mp3)
{
    char    inPath[PATH_MAX + 1];
    char    outPath[PATH_MAX + 1];
    // char    nogapdir[PATH_MAX + 1];
    /* support for "nogap" encoding of up to 200 .wav files */
#define MAX_NOGAP 200
    // int     nogapout = 0;
    // int     max_nogap = MAX_NOGAP;
    // char    nogap_inPath_[MAX_NOGAP][PATH_MAX + 1];
    // char   *nogap_inPath[MAX_NOGAP];
    // char    nogap_outPath_[MAX_NOGAP][PATH_MAX + 1];
    // char   *nogap_outPath[MAX_NOGAP];

    int     ret;
    int     i;
    FILE   *outf = NULL;
    lame_t gf;
    int first = 0;

    // if (argc <= 1) {
    //     // usage(stderr, argv[0]); /* no command-line args, print usage, exit  */
    //     error_printf("insufficient arguments\n");
    //     return 1;
    // }

    // memset(inPath, 0, sizeof(inPath));
    // memset(nogap_inPath_, 0, sizeof(nogap_inPath_));
    // for (i = 0; i < MAX_NOGAP; ++i) {
    //     nogap_inPath[i] = &nogap_inPath_[i][0];
    // }
    // memset(nogap_outPath_, 0, sizeof(nogap_outPath_));
    // for (i = 0; i < MAX_NOGAP; ++i) {
    //     nogap_outPath[i] = &nogap_outPath_[i][0];
    // }

    /* parse the command line arguments, setting various flags in the
     * struct 'gf'.  If you want to parse your own arguments,
     * or call libmp3lame from a program which uses a GUI to set arguments,
     * skip this call and set the values of interest in the gf struct.
     * (see the file API and lame.h for documentation about these parameters)
     */
    // ret = parse_args(gf, argc, argv, inPath, outPath, nogap_inPath, &max_nogap);
    // strcpy(inPath, argv[1]);
    // strcpy(outPath, argv[2]);
    strcpy(inPath, pcm);
    strcpy(outPath, mp3);

    if (!*encoder) {
      first = 1;
      *encoder = lame_init();
      if (!*encoder) {
        error_printf("fatal error during initialization\n");
        return 1;
      }
      lame_set_msgf(*encoder, &frontend_msgf);
      lame_set_errorf(*encoder, &frontend_errorf);
      lame_set_debugf(*encoder, &frontend_debugf);
      lame_set_num_channels(*encoder, 1);
      lame_set_quality(*encoder, 2);
      lame_set_in_samplerate(*encoder, 24000);
      lame_set_out_samplerate(*encoder, 24000);
      // lame_set_brate(*encoder, 96);
      // lame_set_mode(*encoder, MONO);
      global_reader.input_format = sf_raw;
    }
    gf = *encoder;

#define dump_attribute(x) LOG(#x ":%d", (int)lame_get_##x(gf))
    // dump_attribute(in_samplerate);
    // dump_attribute(num_channels);
    // dump_attribute(out_samplerate);
    // dump_attribute(brate);
    // dump_attribute(quality);
    // dump_attribute(VBR);

    // if (ret < 0) {
    //     return ret == -2 ? 0 : 1;
    // }
    if (global_ui_config.update_interval < 0.)
        global_ui_config.update_interval = 2.;

    // if (outPath[0] != '\0' && max_nogap > 0) {
    //     strncpy(nogapdir, outPath, PATH_MAX + 1);
    //     nogapdir[PATH_MAX] = '\0';
    //     nogapout = 1;
    // }

    /* initialize input file.  This also sets samplerate and as much
       other data on the input file as available in the headers */
    // if (max_nogap > 0) {
    //       /* for nogap encoding of multiple input files, it is not possible to
    //        * specify the output file name, only an optional output directory. */
    //       for (i = 0; i < max_nogap; ++i) {
    //           char const* outdir = nogapout ? nogapdir : "";
    //           if (generateOutPath(nogap_inPath[i], outdir, ".mp3", nogap_outPath[i]) != 0) {
    //               error_printf("processing nogap file %d: %s\n", i+1, nogap_inPath[i]);
    //               return -1;
    //           }
    //       }
    //       outf = init_files(gf, nogap_inPath[0], nogap_outPath[0]);
    // }
    // else {
         outf = init_files(gf, inPath, outPath);
    // }
    if (outf == NULL) {
        close_infile();
        return -1;
    }
    /* turn off automatic writing of ID3 tag data into mp3 stream
     * we have to call it before 'lame_init_params', because that
     * function would spit out ID3v2 tag data.
     */
    if (first) {
      lame_set_write_id3tag_automatic(gf, 0);

      /* Now that all the options are set, lame needs to analyze them and
       * set some more internal options and check for problems
       */
      ret = lame_init_params(gf);
      if (ret < 0) {
        if (ret == -1) {
          display_bitrates(stderr);
        }
        // error_printf("fatal error during initialization\n");
        fclose(outf);
        close_infile();
        return ret;
      }

      if (global_ui_config.silent > 0) {
        global_ui_config.brhist = 0; /* turn off VBR histogram */
      }
    }

    // if (lame_get_decode_only(gf)) {
    //     /* decode an mp3 file to a .wav */
    //     ret = lame_decoder(gf, outf, inPath, outPath);
    // }
    // else if (max_nogap == 0) {
    //     /* encode a single input file */
         ret = lame_encoder(gf, outf, 0, inPath, outPath);
    // }
    // else {
    //     /* encode multiple input files using nogap option */
    //     for (i = 0; i < max_nogap; ++i) {
    //         int     use_flush_nogap = (i != (max_nogap - 1));
    //         if (i > 0) {
    //             /* note: if init_files changes anything, like
    //                samplerate, num_channels, etc, we are screwed */
    //             outf = init_files(gf, nogap_inPath[i], nogap_outPath[i]);
    //             if (outf == NULL) {
    //                 close_infile();
    //                 return -1;
    //             }
    //             /* reinitialize bitstream for next encoding.  this is normally done
    //              * by lame_init_params(), but we cannot call that routine twice */
    //             lame_init_bitstream(gf);
    //         }
    //         lame_set_nogap_total(gf, max_nogap);
    //         lame_set_nogap_currentindex(gf, i);
    //         ret = lame_encoder(gf, outf, use_flush_nogap, nogap_inPath[i], nogap_outPath[i]);
    //     }
    // }
    return ret;
}
