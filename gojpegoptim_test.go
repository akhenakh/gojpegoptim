package gojpegoptim

import (
	"bytes"
	"image/jpeg"
	"io/ioutil"
	"testing"
)

func TestJpegOptimFromBuffer(t *testing.T) {
	fi, err := ioutil.ReadFile("test.jpg")
	if err != nil {
		t.Fatal(err)
	}
	imgBytes, err := EncodeBytesOptimized(fi, 100)
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
	_, err := EncodeOptimized(b, 100)
	if err == nil {
		t.Fatal("Should be detected as an error")
	}
}
