package gojpegoptim

import (
	"os"
	"testing"
)

func TestJpegOptimFromBuffer(t *testing.T) {
	fi, err := os.Open("test.jpg")
	if err != nil {
		t.Fatal(err)
	}
	defer fi.Close()
	imgBytes, err := gojpegoptim.EncodeOptimized(fi)

}
