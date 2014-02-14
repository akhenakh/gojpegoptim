/* compile this file with:
 * clang -g -Wall  -L/opt/local/lib -ljpeg -I/opt/local/include main.c ../c-gojpegoptim.c  -o test
 */
#include <stdio.h>
#include <stdlib.h>

extern void optimizeJPEG(unsigned char *inputbuffer, unsigned long inputsize, unsigned char **outputbuffer, unsigned long *outputsize, int quality);
extern int encodeJPEG(unsigned char *inputbuffer, int width, int height, unsigned char **outputbuffer, unsigned long *outputsize, int quality);

int main() { 
  FILE *fp;
  fp = fopen("test.jpg","r"); 
  if (fp == NULL)  {
    printf("fuck\n");
    exit(0);
  }
  fseek (fp , 0 , SEEK_END);
 
  unsigned long fsize;
  fsize = ftell(fp);
  rewind(fp);
 
  unsigned char *jpg_inputbuffer;
  unsigned char *jpg_outputbuffer = NULL;
 
  jpg_inputbuffer = (unsigned char*) malloc(fsize + 100);
  fread(jpg_inputbuffer, 1, fsize, fp);
  printf("input size: %lu\n", fsize);
  unsigned long ouputsize = 0;
  optimizeJPEG(jpg_inputbuffer, fsize, &jpg_outputbuffer, &ouputsize, 100);
 
  fclose(fp);
  printf("output size: %lu\n", ouputsize);
 
  FILE *fo;
  fo = fopen("output.jpg", "w");
 
  fwrite(jpg_outputbuffer, 1, ouputsize, fo);
 
  fclose(fo);
  free(jpg_inputbuffer);
  free(jpg_outputbuffer);

  unsigned char* outputbuffer;
  unsigned long osize = 0;
  unsigned char* inputbuffer = malloc(sizeof(unsigned char)*8*8*4);
  unsigned char* p = inputbuffer;
  p[8] = 8; 

  int code = encodeJPEG(inputbuffer, 8, 8, &outputbuffer, &osize, 100);
  if (code !=0 || osize == 0) {
    printf("Error encoding\n");
    return 2;
  }
  printf("input size uncompressed %lu compressed %lu\n", sizeof(unsigned char)*8*8*4, osize);
  fo = fopen("testout.jpg","w");
  fwrite(outputbuffer, 1, osize, fo);
  fflush(fo);
  fclose(fo);
  free(inputbuffer);
  free(outputbuffer);
  return 0;
}
