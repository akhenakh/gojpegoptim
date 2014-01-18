#include <jerror.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#import "jpeglib.h"

#define EXIF_JPEG_MARKER   JPEG_APP0+1
#define EXIF_IDENT_STRING  "Exif\000\000"
#define EXIF_IDENT_STRING_SIZE 6

#define IPTC_JPEG_MARKER   JPEG_APP0+13

#define ICC_JPEG_MARKER   JPEG_APP0+2
#define ICC_IDENT_STRING  "ICC_PROFILE\0"
#define ICC_IDENT_STRING_SIZE 12

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

// Take a buffer containing a jpeg and return an optimized jpeg
void optimizeJPEG(unsigned char *inputbuffer, unsigned long inputsize, unsigned char **outputbuffer, unsigned long *outputsize, int quality) {
  jvirt_barray_ptr *coef_arrays = NULL;
  JSAMPARRAY buf = NULL;
  struct jpeg_decompress_struct dinfo;
  struct jpeg_compress_struct cinfo;
  int j;
  jpeg_saved_marker_ptr exif_marker = NULL;
  jpeg_saved_marker_ptr iptc_marker = NULL;
  jpeg_saved_marker_ptr icc_marker = NULL;
  jpeg_saved_marker_ptr cmarker; 
  int all_normal = 1;
  int all_progressive = 0;

  if (quality > 100)
    quality = 100;

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

  /* setup error handling for decompress */
  if (setjmp(jderr.setjmp_buffer)) {
    jpeg_abort_decompress(&dinfo);
    if (buf) {
      for (j=0;j<dinfo.output_height;j++) free(buf[j]);
      free(buf); buf=NULL;
    }
    printf(" [ERROR]\n");
    return;
   }

  /* prepare to decompress */
  global_error_counter=0;
  
  jpeg_mem_src(&dinfo, inputbuffer, inputsize);

  if (jpeg_read_header(&dinfo, TRUE) != JPEG_HEADER_OK) {
    return;
  }

  /* check for Exif/IPTC markers */
  exif_marker=NULL;
  iptc_marker=NULL;
  icc_marker=NULL;
  cmarker=dinfo.marker_list;
  while (cmarker) {
    if (cmarker->marker == EXIF_JPEG_MARKER) {
      if (!memcmp(cmarker->data,EXIF_IDENT_STRING,EXIF_IDENT_STRING_SIZE)) 
  exif_marker=cmarker;
    }
    if (cmarker->marker == IPTC_JPEG_MARKER) {
      iptc_marker=cmarker;
    }
    if (cmarker->marker == ICC_JPEG_MARKER) {
      if (!memcmp(cmarker->data,ICC_IDENT_STRING,ICC_IDENT_STRING_SIZE)) 
  icc_marker=cmarker;
    }
    cmarker=cmarker->next;
  }

  jpeg_mem_dest(&cinfo, outputbuffer, outputsize);

  printf("Proc: Image is %d by %d with %d components target quality:%d\n", 
      dinfo.output_width, dinfo.output_height,  dinfo.output_components, quality);

 
  cinfo.optimize_coding = TRUE;

  if (quality>-1 ) {
     jpeg_start_decompress(&dinfo);

     buf = malloc(sizeof(JSAMPROW)*dinfo.output_height);
     if (!buf) {
      return; 
     }
     for (j=0;j<dinfo.output_height;j++) {
       buf[j]=malloc(sizeof(JSAMPLE)*dinfo.output_width*
         dinfo.out_color_components);
       if (!buf[j]) return;
     }

     while (dinfo.output_scanline < dinfo.output_height) {
       jpeg_read_scanlines(&dinfo,&buf[dinfo.output_scanline],
         dinfo.output_height-dinfo.output_scanline);
     }

    cinfo.in_color_space=dinfo.out_color_space;
    cinfo.input_components=dinfo.output_components;
    cinfo.image_width=dinfo.image_width;
    cinfo.image_height=dinfo.image_height;
    jpeg_set_defaults(&cinfo); 
    jpeg_set_quality(&cinfo,quality,TRUE);
    if ( (dinfo.progressive_mode || all_progressive) && !all_normal )
      jpeg_simple_progression(&cinfo);
    cinfo.optimize_coding = TRUE;

    j=0;
    jpeg_start_compress(&cinfo,TRUE);
     
    
    /* write image */
    while (cinfo.next_scanline < cinfo.image_height) {
      jpeg_write_scanlines(&cinfo,&buf[cinfo.next_scanline],
         dinfo.output_height);
    }
  } else {
    coef_arrays = jpeg_read_coefficients(&dinfo);
    jpeg_copy_critical_parameters(&dinfo, &cinfo);
    jpeg_write_coefficients(&cinfo, coef_arrays);
  } 

  jpeg_finish_compress(&cinfo);
  jpeg_finish_decompress(&dinfo);
}

int main() {

  FILE *fp;
  fp = fopen("test.jpg","r"); // read mode
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
  optimizeJPEG(jpg_inputbuffer, fsize, &jpg_outputbuffer, &ouputsize, -1);

  fclose(fp);
  printf("output size: %lu\n", ouputsize);

  FILE *fo;
  fo = fopen("output.jpg", "w");

  fwrite(jpg_outputbuffer, 1, ouputsize, fo);

  fclose(fo);
  free(jpg_inputbuffer);
  free(jpg_outputbuffer);
}