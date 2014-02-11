gojpegoptim
===========

An in memory jpegoptim chain toolkit for Go.

jpegoptim is a widely used tool to optimize jpeg size, but must implementation are using disk as buffer which is annoying on a resize farm.  
Here is an alternative using all in memory structs.  

    go get github.com/akhenakh/gojpegoptim

Roadmap
=======
* RGB struct to jpegbuffer gojpegoptim.Encode()

Thanks
======
Frank Denis  
Dailymotion
