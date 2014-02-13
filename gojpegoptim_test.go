package gojpegoptim

import (
	"bufio"
	"bytes"
	"image"
	"image/color"
	"image/jpeg"
	"io/ioutil"
	"os"
	"testing"
)

func TestJpegOptimFromBuffer(t *testing.T) {

	fi, err := ioutil.ReadFile("test.jpg")
	if err != nil {
		t.Fatal(err)
	}
	imgBytes, err := EncodeBytesOptimized(fi, &Options{100})
	if err != nil {
		t.Fatal(err)
	}
	_, err = jpeg.Decode(bytes.NewReader(imgBytes))
	if err != nil {
		t.Fatal(err)
	}
	gain := (len(fi) - len(imgBytes)) * 100 / len(fi)
	t.Log("input size", len(fi), "output size", len(imgBytes), "gain", len(fi)-len(imgBytes), gain, "%")
	if gain != 35 {
		t.Fatal("Optimization failed")
	}
}

func TestJpegOptimBadBuffer(t *testing.T) {
	b := []byte{'g', 'o', 'l', 'a', 'n', 'g'}
	_, err := EncodeBytesOptimized(b, &Options{100})
	if err == nil {
		t.Fatal("Should be detected as an error")
	}
}

func TestEncodeImageWithJpegOptim(t *testing.T) {
	m := image.NewRGBA(image.Rect(0, 0, 30, 30))
	m.Set(5, 5, color.RGBA{255, 0, 0, 255})
	w := new(bytes.Buffer)
	err := Encode(w, m, &Options{100})
	if err != nil {
		t.Fatal(err)
	}
	if len(w.Bytes()) == 0 {
		t.Fatal("error encoding, size is too small")
	}
	// open output file
	fo, err := os.Create("outputcomp.jpg")
	if err != nil {
		panic(err)
	}
	defer fo.Close()

	wo := bufio.NewWriter(fo)
	wo.Write(w.Bytes())

}
