/*------------------------------------------------------------------------------
--       Copyright (c) 2015-2017, VeriSilicon Inc. All rights reserved        --
--         Copyright (c) 2011-2014, Google Inc. All rights reserved.          --
--         Copyright (c) 2007-2010, Hantro OY. All rights reserved.           --
--                                                                            --
-- This software is confidential and proprietary and may be used only as      --
--   expressly authorized by VeriSilicon in a written licensing agreement.    --
--                                                                            --
--         This entire notice must be reproduced on all copies                --
--                       and may not be removed.                              --
--                                                                            --
--------------------------------------------------------------------------------
-- Redistribution and use in source and binary forms, with or without         --
-- modification, are permitted provided that the following conditions are met:--
--   * Redistributions of source code must retain the above copyright notice, --
--       this list of conditions and the following disclaimer.                --
--   * Redistributions in binary form must reproduce the above copyright      --
--       notice, this list of conditions and the following disclaimer in the  --
--       documentation and/or other materials provided with the distribution. --
--   * Neither the names of Google nor the names of its contributors may be   --
--       used to endorse or promote products derived from this software       --
--       without specific prior written permission.                           --
--------------------------------------------------------------------------------
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"--
-- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE  --
-- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE --
-- ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE  --
-- LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR        --
-- CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF       --
-- SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS   --
-- INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN    --
-- CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)    --
-- ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE --
-- POSSIBILITY OF SUCH DAMAGE.                                                --
--------------------------------------------------------------------------------
------------------------------------------------------------------------------*/

#include "jpegdechdrs.h"
#include "jpegdecutils.h"
#include "jpegdecmarkers.h"
#include "jpegdecinternal.h"
#include "jpegdecscan.h"
#include "jpegdecapi.h"
#include "dwl.h"

/*------------------------------------------------------------------------------
    2. External compiler flags
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
    3. Module defines
------------------------------------------------------------------------------*/

/* Table for Luminance & Chrominace DC coefficient to DHT as
   depicted in Standard */

static const u8 DcLumaLi[16] = {
  0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const u8 DcChromaLi[16] = {
  0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* Table for Luminance & Chrominace DC coefficient value associated to Li
   as depicted in Standard */

static const u8 DcLumaVij[12] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B
};

static const u8 DcChromaVij[12] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B
};

/* Table for Luminance & Chrominace AC coefficient to DHT  as
   depicted in Standard */

static const u8 AcLumaLi[16] = {
  0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03,
  0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7D
};

static const u8 AcChromaLi[16] = {
  0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04,
  0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77
};

/* Table for Luminance & Chrominace AC coefficient value associated to Li
   as depicted in Standard */

static const u8 AcLumaVij[162] = {
  0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13,
  0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08, 0x23, 0x42,
  0xB1, 0xC1, 0x15, 0x52, 0xD1, 0xF0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A,
  0x16, 0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x34, 0x35,
  0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A,
  0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67,
  0x68, 0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x83, 0x84,
  0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
  0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3,
  0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
  0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1,
  0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4,
  0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA
};

static const u8 AcChromaVij[162] = {
  0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51,
  0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xA1, 0xB1,
  0xC1, 0x09, 0x23, 0x33, 0x52, 0xF0, 0x15, 0x62, 0x72, 0xD1, 0x0A, 0x16, 0x24,
  0x34, 0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26, 0x27, 0x28, 0x29, 0x2A,
  0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
  0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66,
  0x67, 0x68, 0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x82,
  0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96,
  0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA,
  0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5,
  0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9,
  0xDA, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF2, 0xF3, 0xF4,
  0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA
};

/*------------------------------------------------------------------------------
    4. Local function prototypes
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    5. Functions
------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------

        Function name: JpegDecDecodeFrame

        Functional description:
          Decodes frame headers

        Inputs:
          JpegDecContainer *p_dec_data      Pointer to JpegDecContainer structure

        Outputs:
          OK/NOK

------------------------------------------------------------------------------*/
JpegDecRet JpegDecDecodeFrameHdr(JpegDecContainer * p_dec_data) {
  u32 i;
  u32 width, height;
  u32 tmp1, tmp2;
  u32 Hmax = 0;
  u32 Vmax = 0;
  u32 size = 0;
  JpegDecRet ret_code;

  ret_code = JPEGDEC_OK;

  /* frame header length */
  p_dec_data->frame.Lf = JpegDecGet2Bytes(&(p_dec_data->stream));

  /* check if there is enough data */
  if(((p_dec_data->stream.read_bits / 8) + p_dec_data->frame.Lf) >
      p_dec_data->stream.stream_length)
    return (JPEGDEC_STRM_ERROR);

  /* Sample precision */
  p_dec_data->frame.P = JpegDecGetByte(&(p_dec_data->stream));
  if(p_dec_data->frame.P != 8) {
    JPEGDEC_TRACE_INTERNAL(("if ( p_dec_data->frame.P != 8)\n"));
    return (JPEGDEC_UNSUPPORTED);
  }
  /* Number of Lines */
  p_dec_data->frame.Y = JpegDecGet2Bytes(&(p_dec_data->stream));
  if(p_dec_data->frame.Y < 1) {
    return (JPEGDEC_UNSUPPORTED);
  }
  p_dec_data->frame.hw_y = p_dec_data->frame.Y;

  /* round up to next multiple-of-16 */
  p_dec_data->frame.hw_y += 0xf;
  p_dec_data->frame.hw_y &= ~(0xf);

  /* Number of samples per line */
  p_dec_data->frame.X = JpegDecGet2Bytes(&(p_dec_data->stream));
  if(p_dec_data->frame.X < 1) {
    return (JPEGDEC_UNSUPPORTED);
  }
  p_dec_data->frame.hw_x = p_dec_data->frame.X;

  /* round up to next multiple-of-16 */
  p_dec_data->frame.hw_x += 0xf;
  p_dec_data->frame.hw_x &= ~(0xf);

  /* for internal() */
  p_dec_data->info.X = p_dec_data->frame.hw_x;
  p_dec_data->info.Y = p_dec_data->frame.hw_y;

  /* check for minimum and maximum dimensions */
  if(p_dec_data->frame.hw_x < p_dec_data->min_supported_width ||
      p_dec_data->frame.hw_y < p_dec_data->min_supported_height ||
      p_dec_data->frame.hw_x > p_dec_data->max_supported_width ||
      p_dec_data->frame.hw_y > p_dec_data->max_supported_height ||
      (p_dec_data->frame.hw_x * p_dec_data->frame.hw_y) >
      p_dec_data->max_supported_pixel_amount) {
    JPEGDEC_TRACE_INTERNAL(("FRAME: Unsupported size\n"));
    return (JPEGDEC_UNSUPPORTED);
  }

  /* Number of components */
  p_dec_data->frame.Nf = JpegDecGetByte(&(p_dec_data->stream));
  if((p_dec_data->frame.Nf != 3) && (p_dec_data->frame.Nf != 1)) {
    JPEGDEC_TRACE_INTERNAL(("p_dec_data->frame.Nf != 3 && p_dec_data->frame.Nf != 1\n"));
    return (JPEGDEC_UNSUPPORTED);
  }

  /* save component specific data */
  /* Nf == number of components */
  for(i = 0; i < p_dec_data->frame.Nf; i++) {
    p_dec_data->frame.component[i].C = JpegDecGetByte(&(p_dec_data->stream));
    if(i == 0) { /* for the first component */
      /* if first component id is something else than 1 (jfif) */
      p_dec_data->scan.index = p_dec_data->frame.component[i].C;
    } else {
      /* if component ids 'jumps' */
      if((p_dec_data->frame.component[i - 1].C + 1) !=
          p_dec_data->frame.component[i].C) {
        JPEGDEC_TRACE_INTERNAL(("component ids 'jumps'\n"));
        return (JPEGDEC_UNSUPPORTED);
      }
    }
    tmp1 = JpegDecGetByte(&(p_dec_data->stream));
    p_dec_data->frame.component[i].H = tmp1 >> 4;
    if(p_dec_data->frame.component[i].H > Hmax) {
      Hmax = p_dec_data->frame.component[i].H;
    }
    p_dec_data->frame.component[i].V = tmp1 & 0xF;
    if(p_dec_data->frame.component[i].V > Vmax) {
      Vmax = p_dec_data->frame.component[i].V;
    }

    p_dec_data->frame.component[i].Tq = JpegDecGetByte(&(p_dec_data->stream));
  }

  if(p_dec_data->frame.Nf == 1) {
    Hmax = Vmax = 1;
    p_dec_data->frame.component[0].H = 1;
    p_dec_data->frame.component[0].V = 1;
  } else if(Hmax == 0 || Vmax == 0) {
    JPEGDEC_TRACE_INTERNAL(("Hmax == 0 || Vmax == 0 \n"));
    return (JPEGDEC_UNSUPPORTED);
  }

  /* JPEG_YCBCR411 horizontal size has to be multiple of 32 pels */
  if(Hmax == 4 && (p_dec_data->frame.hw_x & 0x1F)) {
    /* round up to next multiple-of-32 */
    p_dec_data->frame.hw_x += 16;
    p_dec_data->info.X = p_dec_data->frame.hw_x;

    /* check for minimum and maximum dimensions */
    if(p_dec_data->frame.hw_x > p_dec_data->max_supported_width ||
        (p_dec_data->frame.hw_x * p_dec_data->frame.hw_y) >
        p_dec_data->max_supported_pixel_amount) {
      JPEGDEC_TRACE_INTERNAL(("FRAME: Unsupported size\n"));
      return (JPEGDEC_UNSUPPORTED);
    }
  }

  /* set image pointers, calculate pixelPerRow for each component */
  width = ((p_dec_data->frame.hw_x + Hmax * 8 - 1) / (Hmax * 8)) * Hmax * 8;
  height = ((p_dec_data->frame.hw_y + Vmax * 8 - 1) / (Vmax * 8)) * Vmax * 8;

  /* calculate num_mcu_in_row and num_mcu_in_frame */
  ASSERT(Hmax != 0);
  ASSERT(Vmax != 0);
  p_dec_data->frame.num_mcu_in_row = width / (8 * Hmax);
  p_dec_data->frame.num_mcu_in_frame = p_dec_data->frame.num_mcu_in_row *
                                       (height / (8 * Vmax));

  /* reset mcuNumbers */
  p_dec_data->frame.mcu_number = 0;
  p_dec_data->frame.row = p_dec_data->frame.col = 0;

  for(i = 0; i < p_dec_data->frame.Nf; i++) {

    ASSERT(i <= 2);

    tmp1 = (width * p_dec_data->frame.component[i].H + Hmax - 1) / Hmax;
    tmp2 = (height * p_dec_data->frame.component[i].V + Vmax - 1) / Vmax;
    size += tmp1 * tmp2;
    /* pixels per row */

    p_dec_data->image.pixels_per_row[i] = tmp1;
    p_dec_data->image.columns[i] = tmp2;
    p_dec_data->frame.num_blocks[i] =
      (((p_dec_data->frame.hw_x * p_dec_data->frame.component[i].H) / Hmax +
        7) >> 3) * (((p_dec_data->frame.hw_y *
                      p_dec_data->frame.component[i].V) / Vmax + 7) >> 3);

    if(i == 0) {
      p_dec_data->image.size_luma = size;
    }
  }

  p_dec_data->image.size = size;
  p_dec_data->image.size_chroma = size - p_dec_data->image.size_luma;

  /* set mode & calculate rlc tmp size */
  ret_code = JpegDecMode(p_dec_data);
  if(ret_code != JPEGDEC_OK) {
    return (ret_code);
  }

  return (JPEGDEC_OK);
}

/*------------------------------------------------------------------------------

        Function name: JpegDecDecodeQuantTables

        Functional description:
          Decodes quantisation tables from the stream

        Inputs:
          JpegDecContainer *p_dec_data      Pointer to JpegDecContainer structure

        Outputs:
          OK    (0)
          NOK   (-1)

------------------------------------------------------------------------------*/

JpegDecRet JpegDecDecodeQuantTables(JpegDecContainer * p_dec_data) {
  u32 t, tmp, i;
  StreamStorage *stream = &(p_dec_data->stream);

  p_dec_data->quant.Lq = JpegDecGet2Bytes(stream);

  /* check if there is enough data for tables */
  if(((stream->read_bits / 8) + p_dec_data->quant.Lq) > stream->stream_length)
    return (JPEGDEC_STRM_ERROR);

  t = 4;

  while(t < p_dec_data->quant.Lq) {
    /* Tq value selects what table the components use */

    /* read tables and write to decData->quant */
    tmp = JpegDecGetByte(stream);
    t++;
    /* supporting only 8 bits / sample */
    if((tmp >> 4) != 0) {
      return (JPEGDEC_UNSUPPORTED);
    }
    tmp &= 0xF;
    /* set the quantisation table pointer */

    if(tmp == 0) {
      JPEGDEC_TRACE_INTERNAL(("qtable0\n"));
      p_dec_data->quant.table = p_dec_data->quant.table0;
    } else if(tmp == 1) {
      JPEGDEC_TRACE_INTERNAL(("qtable1\n"));
      p_dec_data->quant.table = p_dec_data->quant.table1;
    } else if(tmp == 2) {
      JPEGDEC_TRACE_INTERNAL(("qtable2\n"));
      p_dec_data->quant.table = p_dec_data->quant.table2;
    } else if(tmp == 3) {
      JPEGDEC_TRACE_INTERNAL(("qtable3\n"));
      p_dec_data->quant.table = p_dec_data->quant.table3;
    } else {
      return (JPEGDEC_UNSUPPORTED);
    }
    for(i = 0; i < 64; i++) {
      p_dec_data->quant.table[i] = JpegDecGetByte(stream);
      t++;
    }

  }

  return (JPEGDEC_OK);
}

/*------------------------------------------------------------------------------

        Function name: JpegDecDecodeHuffmanTables

        Functional description:
          Decodes huffman tables from the stream

        Inputs:
          DecData *JpegDecContainer      Pointer to JpegDecContainer structure

        Outputs:
          OK    (0)
          NOK   (-1)

------------------------------------------------------------------------------*/
JpegDecRet JpegDecDecodeHuffmanTables(JpegDecContainer * p_dec_data) {
  u32 i, len, Tc, Th, tmp;
  i32 j;
  StreamStorage *stream = &(p_dec_data->stream);

  p_dec_data->vlc.Lh = JpegDecGet2Bytes(stream);

  /* check if there is enough data for tables */
  if(((stream->read_bits / 8) + p_dec_data->vlc.Lh) > stream->stream_length)
    return (JPEGDEC_STRM_ERROR);

  /* four bytes already read in */
  len = 4;

  while(len < p_dec_data->vlc.Lh) {
    tmp = JpegDecGetByte(stream);
    len++;
    Tc = tmp >> 4;  /* Table class */
    if(Tc != 0 && Tc != 1) {
      return (JPEGDEC_UNSUPPORTED);
    }
    Th = tmp & 0xF; /* Huffman table identifier */
    /* only two tables in baseline allowed */
    if((p_dec_data->frame.coding_type == SOF0) && (Th > 1)) {
      return (JPEGDEC_UNSUPPORTED);
    }
    /* four tables in progressive allowed */
    if((p_dec_data->frame.coding_type == SOF2) && (Th > 3)) {
      return (JPEGDEC_UNSUPPORTED);
    }
    /* set the table pointer */
    if(Tc) {
      /* Ac table */
      switch (Th) {
      case 0:
        JPEGDEC_TRACE_INTERNAL(("ac0\n"));
        p_dec_data->vlc.table = &(p_dec_data->vlc.ac_table0);
        break;
      case 1:
        JPEGDEC_TRACE_INTERNAL(("ac1\n"));
        p_dec_data->vlc.table = &(p_dec_data->vlc.ac_table1);
        break;
      case 2:
        JPEGDEC_TRACE_INTERNAL(("ac2\n"));
        p_dec_data->vlc.table = &(p_dec_data->vlc.ac_table2);
        break;
      case 3:
        JPEGDEC_TRACE_INTERNAL(("ac3\n"));
        p_dec_data->vlc.table = &(p_dec_data->vlc.ac_table3);
        break;
      default:
        return (JPEGDEC_UNSUPPORTED);
      }
    } else {
      /* Dc table */
      switch (Th) {
      case 0:
        JPEGDEC_TRACE_INTERNAL(("dc0\n"));
        p_dec_data->vlc.table = &(p_dec_data->vlc.dc_table0);
        break;
      case 1:
        JPEGDEC_TRACE_INTERNAL(("dc1\n"));
        p_dec_data->vlc.table = &(p_dec_data->vlc.dc_table1);
        break;
      case 2:
        JPEGDEC_TRACE_INTERNAL(("dc2\n"));
        p_dec_data->vlc.table = &(p_dec_data->vlc.dc_table2);
        break;
      case 3:
        JPEGDEC_TRACE_INTERNAL(("dc3\n"));
        p_dec_data->vlc.table = &(p_dec_data->vlc.dc_table3);
        break;
      default:
        return (JPEGDEC_UNSUPPORTED);
      }
    }

    tmp = 0;
    /* read in the values of list BITS */
    for(i = 0; i < 16; i++) {
      tmp += p_dec_data->vlc.table->bits[i] = JpegDecGetByte(stream);
      len++;
    }
    /* allocate memory for HUFFVALs */
    if(p_dec_data->vlc.table->vals != NULL) {
      /* free previously reserved table */
      DWLfree(p_dec_data->vlc.table->vals);
    }

    p_dec_data->vlc.table->vals = (u32 *) DWLmalloc(sizeof(u32) * tmp);

    /* set the table length */
    p_dec_data->vlc.table->table_length = tmp;
    /* read in the HUFFVALs */
    for(i = 0; i < tmp; i++) {
      p_dec_data->vlc.table->vals[i] = JpegDecGetByte(stream);
      len++;
    }
    /* first and last lengths */
    for(i = 0; i < 16; i++) {
      if(p_dec_data->vlc.table->bits[i] != 0) {
        p_dec_data->vlc.table->start = i;
        break;
      }
    }
    for(j = 15; j >= 0; j--) {
      if(p_dec_data->vlc.table->bits[j] != 0) {
        p_dec_data->vlc.table->last = ((u32) j + 1);
        break;
      }
    }
  }

  return (JPEGDEC_OK);
}

/*------------------------------------------------------------------------------

        Function name: JpegDecDefaultHuffmanTables

        Functional description:
          Configure decoder to use default huffman tables (Standard)

        Inputs:
          DecData *JpegDecContainer      Pointer to JpegDecContainer structure

        Outputs:

------------------------------------------------------------------------------*/
void JpegDecDefaultHuffmanTables(JpegDecContainer * p_dec_data) {
  u32 i, k, len, tmp;
  i32 j;
  u32 table = 0;
  u8 * stream_bits = 0;
  u8 * stream_vals = 0;
  tmp = 0;

  for(k = 0; k < 4; k++) {
    /* set the table pointer */
    table = k;
    switch (table) {
    case 0:
      JPEGDEC_TRACE_INTERNAL(("ac0\n"));
      p_dec_data->vlc.table = &(p_dec_data->vlc.ac_table0);
      stream_bits = (u8 *)AcLumaLi;
      stream_vals = (u8 *)AcLumaVij;
      break;
    case 1:
      JPEGDEC_TRACE_INTERNAL(("ac1\n"));
      p_dec_data->vlc.table = &(p_dec_data->vlc.ac_table1);
      stream_bits = (u8 *)AcChromaLi;
      stream_vals = (u8 *)AcChromaVij;
      break;
    case 2:
      JPEGDEC_TRACE_INTERNAL(("dc0\n"));
      p_dec_data->vlc.table = &(p_dec_data->vlc.dc_table0);
      stream_bits = (u8 *)DcLumaLi;
      stream_vals = (u8 *)DcLumaVij;
      break;
    case 3:
      JPEGDEC_TRACE_INTERNAL(("dc1\n"));
      p_dec_data->vlc.table = &(p_dec_data->vlc.dc_table1);
      stream_bits = (u8 *)DcChromaLi;
      stream_vals = (u8 *)DcChromaVij;
      break;
    }

    /* read in the values of list BITS */
    for(i = 0; i < 16; i++) {
      tmp += p_dec_data->vlc.table->bits[i] = stream_bits[i];
      len++;
    }
    /* allocate memory for HUFFVALs */
    if(p_dec_data->vlc.table->vals != NULL) {
      /* free previously reserved table */
      DWLfree(p_dec_data->vlc.table->vals);
    }

    p_dec_data->vlc.table->vals = (u32 *) DWLmalloc(sizeof(u32) * tmp);

    /* set the table length */
    p_dec_data->vlc.table->table_length = tmp;
    /* read in the HUFFVALs */
    for(i = 0; i < tmp; i++) {
      p_dec_data->vlc.table->vals[i] = stream_vals[i];
      len++;
    }
    /* first and last lengths */
    for(i = 0; i < 16; i++) {
      if(p_dec_data->vlc.table->bits[i] != 0) {
        p_dec_data->vlc.table->start = i;
        break;
      }
    }
    for(j = 15; j >= 0; j--) {
      if(p_dec_data->vlc.table->bits[j] != 0) {
        p_dec_data->vlc.table->last = ((u32) j + 1);
        break;
      }
    }
  }
}

/*------------------------------------------------------------------------------

        Function name: JpegDecMode

        Functional description:
          check YCBCR mode

        Inputs:
          JpegDecContainer *p_dec_data      Pointer to JpegDecContainer structure

        Outputs:
          OK/NOK

------------------------------------------------------------------------------*/
JpegDecRet JpegDecMode(JpegDecContainer * p_dec_data) {
  /*  check input format */
  if(p_dec_data->frame.Nf == 3) {
    /* JPEG_YCBCR420 */
    if(p_dec_data->frame.component[0].H == 2 &&
        p_dec_data->frame.component[0].V == 2 &&
        p_dec_data->frame.component[1].H == 1 &&
        p_dec_data->frame.component[1].V == 1 &&
        p_dec_data->frame.component[2].H == 1 &&
        p_dec_data->frame.component[2].V == 1) {
      p_dec_data->info.y_cb_cr_mode = JPEGDEC_YUV420;
      p_dec_data->info.X = p_dec_data->frame.hw_x;
      p_dec_data->info.Y = p_dec_data->frame.hw_y;

      /* calculate new output size if slice mode used */
      if(p_dec_data->info.slice_mb_set_value) {
        /* Y */
        p_dec_data->image.size_luma = (p_dec_data->info.X *
                                       (p_dec_data->info.slice_mb_set_value *
                                        16));

        /* CbCr */
        p_dec_data->image.size_chroma = p_dec_data->image.size_luma / 2;

      }
    }
    /* JPEG_YCBCR422 */
    else if(p_dec_data->frame.component[0].H == 2 &&
            p_dec_data->frame.component[0].V == 1 &&
            p_dec_data->frame.component[1].H == 1 &&
            p_dec_data->frame.component[1].V == 1 &&
            p_dec_data->frame.component[2].H == 1 &&
            p_dec_data->frame.component[2].V == 1) {
      p_dec_data->info.y_cb_cr_mode = JPEGDEC_YUV422;
      p_dec_data->info.X = (p_dec_data->frame.hw_x);
      p_dec_data->info.Y = (p_dec_data->frame.hw_y);

      /* check if fill needed */
      if((p_dec_data->frame.Y & 0xF) && (p_dec_data->frame.Y & 0xF) <= 8)
        p_dec_data->info.fill_bottom = 1;

      /* calculate new output size if slice mode used */
      if(p_dec_data->info.slice_mb_set_value) {
        /* Y */
        p_dec_data->image.size_luma = (p_dec_data->info.X *
                                       (p_dec_data->info.slice_mb_set_value *
                                        16));

        /* CbCr */
        p_dec_data->image.size_chroma = p_dec_data->image.size_luma;
      }
    }
    /* JPEG_YCBCR440 */
    else if(p_dec_data->frame.component[0].H == 1 &&
            p_dec_data->frame.component[0].V == 2 &&
            p_dec_data->frame.component[1].H == 1 &&
            p_dec_data->frame.component[1].V == 1 &&
            p_dec_data->frame.component[2].H == 1 &&
            p_dec_data->frame.component[2].V == 1) {
      p_dec_data->info.y_cb_cr_mode = JPEGDEC_YUV440;
      p_dec_data->info.X = (p_dec_data->frame.hw_x);
      p_dec_data->info.Y = (p_dec_data->frame.hw_y);

      /* check if fill needed */
      if((p_dec_data->frame.X & 0xF) && (p_dec_data->frame.X & 0xF) <= 8)
        p_dec_data->info.fill_right = 1;

      /* calculate new output size if slice mode used */
      if(p_dec_data->info.slice_mb_set_value) {
        /* Y */
        p_dec_data->image.size_luma = (p_dec_data->info.X *
                                       (p_dec_data->info.slice_mb_set_value *
                                        16));

        /* CbCr */
        p_dec_data->image.size_chroma = p_dec_data->image.size_luma;
      }
    }
    /* JPEG_YCBCR444 : NOT SUPPORTED */
    else if(p_dec_data->frame.component[0].H == 1 &&
            p_dec_data->frame.component[0].V == 1 &&
            p_dec_data->frame.component[1].H == 1 &&
            p_dec_data->frame.component[1].V == 1 &&
            p_dec_data->frame.component[2].H == 1 &&
            p_dec_data->frame.component[2].V == 1) {
      p_dec_data->info.y_cb_cr_mode = JPEGDEC_YUV444;
      p_dec_data->info.X = p_dec_data->frame.hw_x;
      p_dec_data->info.Y = p_dec_data->frame.hw_y;

      /* check if fill needed */
      if((p_dec_data->frame.X & 0xF) && (p_dec_data->frame.X & 0xF) <= 8)
        p_dec_data->info.fill_right = 1;

      if((p_dec_data->frame.Y & 0xF) && (p_dec_data->frame.Y & 0xF) <= 8)
        p_dec_data->info.fill_bottom = 1;

      /* calculate new output size if slice mode used */
      if(p_dec_data->info.slice_mb_set_value) {
        /* Y */
        p_dec_data->image.size_luma = (p_dec_data->info.X *
                                       (p_dec_data->info.slice_mb_set_value *
                                        16));

        /* CbCr */
        p_dec_data->image.size_chroma = p_dec_data->image.size_luma * 2;
      }
    }
    /* JPEG_YCBCR411 */
    else if(p_dec_data->frame.component[0].H == 4 &&
            p_dec_data->frame.component[0].V == 1 &&
            p_dec_data->frame.component[1].H == 1 &&
            p_dec_data->frame.component[1].V == 1 &&
            p_dec_data->frame.component[2].H == 1 &&
            p_dec_data->frame.component[2].V == 1) {
      p_dec_data->info.y_cb_cr_mode = JPEGDEC_YUV411;
      p_dec_data->info.X = (p_dec_data->frame.hw_x);
      p_dec_data->info.Y = (p_dec_data->frame.hw_y);

      /* check if fill needed */
      if((p_dec_data->frame.Y & 0xF) && (p_dec_data->frame.Y & 0xF) <= 8)
        p_dec_data->info.fill_bottom = 1;

      /* calculate new output size if slice mode used */
      if(p_dec_data->info.slice_mb_set_value) {
        /* Y */
        p_dec_data->image.size_luma = (p_dec_data->info.X *
                                       (p_dec_data->info.slice_mb_set_value *
                                        16));

        /* CbCr */
        p_dec_data->image.size_chroma = p_dec_data->image.size_luma / 2;
      }
    } else {
      return (JPEGDEC_UNSUPPORTED);
    }
  } else if(p_dec_data->frame.Nf == 1) {
    /* 4:0:0 */
    if((p_dec_data->frame.component[0].V == 1) ||
        (p_dec_data->frame.component[0].H == 1)) {
      p_dec_data->info.y_cb_cr_mode = JPEGDEC_YUV400;
      p_dec_data->info.X = (p_dec_data->frame.hw_x);
      p_dec_data->info.Y = (p_dec_data->frame.hw_y);

      /* check if fill needed */
      if((p_dec_data->frame.X & 0xF) && (p_dec_data->frame.X & 0xF) <= 8)
        p_dec_data->info.fill_right = 1;

      if((p_dec_data->frame.Y & 0xF) && (p_dec_data->frame.Y & 0xF) <= 8)
        p_dec_data->info.fill_bottom = 1;

      /* calculate new output size if slice mode used */
      if(p_dec_data->info.slice_mb_set_value) {
        /* Y */
        p_dec_data->image.size_luma =
          ((((p_dec_data->info.X +
              15) / 16) * 16) * (p_dec_data->info.slice_mb_set_value *
                                 16));

        /* CbCr */
        p_dec_data->image.size_chroma = 0;
      }
    } else {
      return (JPEGDEC_UNSUPPORTED);
    }
  } else {
    return (JPEGDEC_UNSUPPORTED);
  }

#ifdef JPEGDEC_ERROR_RESILIENCE
  if(p_dec_data->info.fill_bottom) {
    p_dec_data->info.Y -= 16;
    p_dec_data->frame.hw_y -= 16;
  }
#endif /* JPEGDEC_ERROR_RESILIENCE */

  /* save the original sampling format for progressive use */
  p_dec_data->info.y_cb_cr_mode_orig = p_dec_data->info.y_cb_cr_mode;

  return (JPEGDEC_OK);
}
