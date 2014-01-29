Spotting Criminals by Face
==========================

This code is an attachment to a report on using ANN's to distinguish criminals by their facial features.

It consists out of two programs:
 - *pws-img-prep* is responsible for preparing frontal facial pictures for
   using them in the netwerk, essentially it converts the image files to
   .lms files. **NOTE: The program must be fed with pictures that have a
   width-height ratio of 0.8!**
 - *pws-neural-net* can actually train the network on a big dataset of
   faces or run a face through an already trained network.

Usage pws-img-prep
------------------
When compiled in release mode *pws-img-prep* can be used like this:
```
pws-img-prep <input> <output>
    <input> (optional)  : input image file, defaults to in.jpg (width-height ratio must be 0.8)
    <output> (optional) : output .lms file, defaults to out.lms
```
or
```
pws-img-prep ?
```
if you need help.

Usage pws-neural-net
--------------------
Using *pws-neural-net* (also in release mode) for training (learning):
```
pws-neural-net L <file>
    <file> (optional) : file to read ANN from (is created if it doesn't exist) and write to, defaults to net.dat
```
For testing a picture for criminality:
```
pws-neural-net T <file>
    <file> (mandatory) : .lms file of face (created by pws-img-prep)
```
or calling it without any arguments for help.
