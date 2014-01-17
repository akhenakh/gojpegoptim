#include <jerror.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#import "jpeglib.h"

struct my_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;   
};

typedef struct my_error_mgr * my_error_ptr;
struct my_error_mgr jcerr, jderr;

int verbose_mode = 1;
int global_error_counter = 0;

METHODDEF(void) 
my_error_exit (j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr)cinfo->err;
  (*cinfo->err->output_message) (cinfo);
  longjmp(myerr->setjmp_buffer,1);
}

METHODDEF(void)
my_output_message (j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];

  (*cinfo->err->format_message) (cinfo, buffer); 
  if (verbose_mode) printf(" (%s) ",buffer);
  global_error_counter++;
}

void optimizeJPEG(unsigned char *inputbuffer, unsigned long inputsize, unsigned char **outputbuffer, unsigned long *outputsize) {
  jvirt_barray_ptr *coef_arrays = NULL;
  struct jpeg_decompress_struct dinfo;
  struct jpeg_compress_struct cinfo;

  /* initialize decompression object */
  dinfo.err = jpeg_std_error(&jderr.pub);
  jpeg_create_decompress(&dinfo);
  jderr.pub.error_exit=my_error_exit;
  jderr.pub.output_message=my_output_message;

  /* initialize compression object */
  cinfo.err = jpeg_std_error(&jcerr.pub);
  jpeg_create_compress(&cinfo);
  jcerr.pub.error_exit=my_error_exit;
  jcerr.pub.output_message=my_output_message;

  /* Step 2: specify data source (eg, a file) */
  jpeg_mem_src(&dinfo, inputbuffer, inputsize);

  int rc = jpeg_read_header(&dinfo, TRUE);

  if (rc != 1) {
    printf("File does not seem to be a normal JPEG");
    exit(EXIT_FAILURE);
  }

  //jpeg_start_decompress(&dinfo);
  coef_arrays = jpeg_read_coefficients(&dinfo);

  jpeg_mem_dest(&cinfo, outputbuffer, outputsize);

  printf("Proc: Image is %d by %d with %d components\n", 
      dinfo.output_width, dinfo.output_height,  dinfo.output_components);

  jpeg_copy_critical_parameters(&dinfo, &cinfo);
  cinfo.optimize_coding = TRUE;

  jpeg_write_coefficients(&cinfo, coef_arrays);

  /* write markers */
  //write_markers(&dinfo,&cinfo);

  jpeg_finish_compress(&cinfo);
  jpeg_finish_decompress(&dinfo);
}

int main() {

  FILE *fp;
  fp = fopen("test.jpg","r"); // read mode
  if (fp == NULL)  {
    printf("fuck\n");
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
  optimizeJPEG(jpg_inputbuffer, fsize, &jpg_outputbuffer, &ouputsize);
  fclose(fp);
  printf("output size: %lu\n", ouputsize);

  FILE *fo;
  fo = fopen("output.jpg", "w");

  fwrite(jpg_outputbuffer, 1, ouputsize, fo);

  fclose(fo);
  free(jpg_inputbuffer);
  free(jpg_outputbuffer);
}