package gojpegoptim

// #cgo CFLAGS: -I/opt/local/include
// #cgo LDFLAGS: -L/opt/local/lib -ljpeg
// #include <stdlib.h>
// extern int optimizeJPEG(unsigned char *inputbuffer, unsigned long inputsize, unsigned char **outputbuffer, unsigned long *outputsize, int quality);
import "C"

import (
	"errors"
	"unsafe"
)

// Options are the encoding parameters.
// Quality ranges from -1 to 100 inclusive, higher is better.
type Options struct {
	Quality int
}

// Optimize a JPEG bytes array if quality is -1, Optimize & Recompress if quality is between [0 - 100]
func EncodeBytesOptimized(srcBytes []byte, o *Options) (outBytes []byte, err error) {
	if len(srcBytes) == 0 {
		err = errors.New("Image source is empty")
		return
	}
	// Clip quality to [-1, 100].
	if o != nil {
		quality := o.Quality
		if quality < -1 {
			quality = -1
		} else if quality > 100 {
			quality = 100
		}
	}

	csrcimg := (*C.uchar)(unsafe.Pointer(&srcBytes[0]))
	cinputsize := C.ulong(len(srcBytes))
	var coutimg *C.uchar
	var coutsize C.ulong
	code := C.optimizeJPEG(csrcimg, cinputsize, &coutimg, &coutsize, C.int(o.Quality))
	if code != 0 || coutsize == 0 {
		err = errors.New("Optimize failed")
		return
	}
	outBytes = C.GoBytes(unsafe.Pointer(coutimg), C.int(coutsize))
	C.free(unsafe.Pointer(coutimg))
	return
}
