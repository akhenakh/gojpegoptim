package gojpegoptim

// #cgo LDFLAGS: -ljpeg
// #cgo darwin LDFLAGS: -L/opt/local/lib
// #cgo darwin CFLAGS: -I/opt/local/include
// #cgo freebsd LDFLAGS: -L/usr/local/lib
// #cgo freebsd CFLAGS: -I/usr/local/include
// #include <stdlib.h>
// extern int optimizeJPEG(unsigned char *inputbuffer, unsigned long inputsize, unsigned char **outputbuffer, unsigned long *outputsize, int quality);
// extern int encodeJPEG(unsigned char *inputbuffer, int width, int height, unsigned char **outputbuffer, unsigned long *outputsize, int quality);
import "C"

import (
	"errors"
	"image"
	"image/draw"
	"io"
	"unsafe"
)

// Options are the encoding parameters.
// Quality ranges from -1 to 100 inclusive, higher is better.
type Options struct {
	Quality int
}

// Encode writes the Image m to w in JPEG format with the given options.
func Encode(w io.Writer, m image.Image, o *Options) error {
	// Get an image.RGBA if it is one
	rgba, ok := m.(*image.RGBA)
	if !ok {
		b := m.Bounds()
		tm := image.NewRGBA(b)
		draw.Draw(tm, b, m, b.Min, draw.Src)
		m = tm
	}
	var coutimg *C.uchar
	var coutsize C.ulong
	input := (*C.uchar)(unsafe.Pointer(&rgba.Pix[0]))
	code := C.encodeJPEG(input, C.int(m.Bounds().Size().X), C.int(m.Bounds().Size().Y), &coutimg, &coutsize, C.int(o.Quality))
	if code != 0 || coutsize == 0 {
		return errors.New("Encoding error")
	}
	outBytes := C.GoBytes(unsafe.Pointer(coutimg), C.int(coutsize))
	w.Write(outBytes)
	C.free(unsafe.Pointer(coutimg))
	return nil
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
