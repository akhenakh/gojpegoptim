package gojpegoptim

// #cgo CFLAGS: -I/opt/local/include
// #cgo LDFLAGS: -L/opt/local/lib -ljpeg
// #include <stdlib.h>
// extern void optimizeJPEG(unsigned char *inputbuffer, unsigned long inputsize, unsigned char **outputbuffer, unsigned long *outputsize, int quality);
import "C"

import (
	"errors"
	"unsafe"
)

func EncodeOptimized(srcBytes []byte, quality int) (outBytes []byte, err error) {
	if len(srcBytes) == 0 {
		err = errors.New("Image source is empty")
		return
	}
	csrcimg := (*C.uchar)(unsafe.Pointer(&srcBytes[0]))
	cinputsize := C.ulong(len(srcBytes))
	var coutimg *C.uchar
	var coutsize C.ulong
	C.optimizeJPEG(csrcimg, cinputsize, &coutimg, &coutsize, C.int(quality))
	if coutsize == 0 {
		err = errors.New("Optimize failed output is 0")
		return
	}
	outBytes = C.GoBytes(unsafe.Pointer(coutimg), C.int(coutsize))
	C.free(unsafe.Pointer(coutimg))
	return
}
