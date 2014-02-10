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
	imgBytes, err := EncodeOptimized(fi, 100)
	if err != nil {
		t.Fatal(err)
	}
	_, err = jpeg.Decode(bytes.NewReader(imgBytes))
	if err != nil {
		t.Fatal(err)
	}

	t.Log("input size", len(fi), "output size", len(imgBytes), "gain", len(fi)-len(imgBytes), (len(fi)-len(imgBytes))*100/len(fi), "%")
}
