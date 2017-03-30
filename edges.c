#include "edges.h"
#include <math.h>
#include <stdlib.h>

int vc_gray_edge_sobel(IVC *src, IVC *dst, int threshold) {
  unsigned char *datasrc = (unsigned char *)src->data;
  unsigned char *datadst = (unsigned char *)dst->data;
  long posX, posA, posB, posC, posD, posE, posF, posG, posH;
  float mx, my;

  // Verificacao de erros
  if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
    return 0;
  if ((src->width != dst->width) || (src->height != dst->height) ||
      (src->channels != dst->channels))
    return 0;
  if (src->channels != 1)
    return 0;

  for (int x = 1; x < src->width - 1; x++) {
    for (int y = 1; y < src->height - 1; y++) {
      posX = y * src->bytesperline + x * src->channels;
      posA = (y - 1) * src->bytesperline + (x - 1) * src->channels;
      posB = (y - 1) * src->bytesperline + x * src->channels;
      posC = (y - 1) * src->bytesperline + (x + 1) * src->channels;
      posD = y * src->bytesperline + (x - 1) * src->channels;
      posE = y * src->bytesperline + (x + 1) * src->channels;
      posF = (y + 1) * src->bytesperline + (x - 1) * src->channels;
      posG = (y + 1) * src->bytesperline + x * src->channels;
      posH = (y + 1) * src->bytesperline + (x + 1) * src->channels;

      my = ((datasrc[posF] + (datasrc[posG] * 2) + datasrc[posH]) -
            (datasrc[posA] + (datasrc[posB] * 2) + datasrc[posC])) /
           3;
      mx = ((datasrc[posC] + (datasrc[posE] * 2) + datasrc[posH]) -
            (datasrc[posA] + (datasrc[posD] * 2) + datasrc[posF])) /
           3;

      datadst[posX] = (unsigned char)sqrt(my * my + mx * mx);

      if (datadst[posX] > threshold) {
        datadst[posX] = 255;
      } else {
        datadst[posX] = 0;
      }
    }
  }

  return 1;
}
