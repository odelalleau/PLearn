// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
//

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// 
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org


 

/* *******************************************************      
   * $Id$
   * AUTHORS: Pascal Vincent & Yoshua Bengio
   * This file is part of the PLearn library.
   ******************************************************* */


/*! \file PLearn/plearn/display/RGBImage.h */

#ifndef RGBIMAGE_INC
#define RGBIMAGE_INC

#include <plearn/base/Array.h>
//#include "general.h"
#include <plearn/base/Object.h>
#include <plearn/base/Storage.h>
#include <plearn/math/TMat.h>
//#include "VMat.h"

namespace PLearn {
using namespace std;


class HSV;

class RGB
{
public:
  unsigned char r;
  unsigned char g;
  unsigned char b;
  
  RGB() {}
  RGB(unsigned char the_r, unsigned char the_g, unsigned char the_b)
    : r(the_r), g(the_g), b(the_b) {}

  //!  RGB(HSV hsvpix);

public:
  static const RGB BLACK;
  static const RGB WHITE;
  static const RGB RED;
  static const RGB GREEN;
  static const RGB BLUE;
  static const RGB DISCOCOLORS[];
};


class HSV
{
public:
  unsigned char h;
  unsigned char s;
  unsigned char v;

  HSV() {}
  HSV(unsigned char the_h, unsigned char the_s, unsigned char the_v)
    : h(the_h), s(the_s), v(the_v) {}

  //!  HSV(RGB rgbpix);
};


//!  uses top left coordinate system Pixel (i,j) is at row i, column j
class RGBImage
{
protected:
  int height_;
  int width_;

  PP< Storage<RGB> > storage;
  char tmpfilename[100]; //!<  temporary file in which the image may be saved when displayed

public:
  RGBImage(int the_height=0, int the_width=0);
  RGBImage(Mat r, Mat g, Mat b); //!<  r g and b must have values in range [0-255]
  ~RGBImage();

  int width() const { return width_; }
  int height() const { return height_; }
  void resize(int newheight, int newwidth);

  inline RGB getPixel(int i, int j) const; //!<  return RGB value of Pixel of row i and column j
  inline void setPixel(int i, int j, RGB value); //!<  sets RGB value of pixel in row i and column j
  void fill(RGB color);
  void clear() { fill(RGB(255,255,255)); }

  void loadPPM(const char* filename);
  void savePPM(const char* filename) const;
  void loadJPEG(const char* filename, int scale=1);
  void display() const;
  void displayAndWait() const;

  //!  remove borders around the image
  void removeBorders(int top_border,int bottom_border,
                     int left_border,int right_border);
  
  //!  remove borders of thickness "border" from image (in-place), on all sides
  void removeBorders(int border) { removeBorders(border,border,border,border); }

  //!  shrinks the box that starts at position (i,j) (in this image) and of size (h,w) 
  //!  to its intersection with this image's box (0,0,this->height,this->width_)
  void shrinkToIntersection(int& i, int& j, int& h, int& w) const;

  //!  blits srcim at position (desti,destj) in this image
  void blit(int desti, int destj, RGBImage srcim);
  RGB computeAverage() const;

  //!  Returns a vector of size r_bins*g_bins*b_bins which contains the number of image pixels 
  //!  whose colors fell in the corresponding RGB range
  Vec computeHistogram(int r_bins=16, int g_bins=16, int b_bins=16, bool do_normalize=false);
};

inline RGB
RGBImage::getPixel(int i, int j) const
{
#ifdef BOUNDCHECK
  if(!storage)
    PLERROR("In RGBImage::getPixeel(int i, int j) EMPTY IMAGE!!!");
  if(i<0 || j<0 || i>=height_ || j>=width_)
    PLERROR("In RGBImage::getPixel(int i, int j) OUT OF IMAGE BOUNDS");
#endif
  return storage->data[i*width_+j];
}

inline void
RGBImage::setPixel(int i, int j, RGB value)
{
#ifdef BOUNDCHECK
  if(!storage)
    PLERROR("In RGBImage::getPixeel(int i, int j) EMPTY IMAGE!!!");
  if(i<0 || j<0 || i>=height_ || j>=width_)
    PLERROR("In RGBImage::getPixel(int i, int j) OUT OF IMAGE BOUNDS");
#endif
  storage->data[i*width_+j] = value;
}

class RGBImageDB
{
protected:
  Array<char*> filenames;
  Array<RGBImage*> images;
  int subsample_factor;
  int remove_border;
  int max_n_images_in_memory;
  int n_images_in_memory;
  
  int append(char* filename);
  void load(char* dbfile);

  RGBImageDB(int the_subsample_factor=1,int remove_border=0, int the_max_n_images_in_memory=10); 

public:
  //!  These are two column matrices 
  //!  (first column: imageid, second column: classnum)
  Mat imageIdAndClass;

  RGBImageDB(char* dbfilename, int the_subsample_factor=1,int remove_border=0, int the_max_n_images_in_memory=10);

  RGBImage getImage(int imageid);

  //!  Returns for each image a vector of size r_bins*g_bins*b_bins+1 which contains the number of image pixels 
  //!  whose colors fell in the corresponding RGB range, and last element is the class number
  Mat computeHistogramRepresentation(int r_bins=16, int g_bins=16, int b_bins=16, bool do_normalize=false);

  ~RGBImageDB();
};


class RGBImagesVMatrix;

/*!   RGBImageVMatrix is a subclass of distribution
  that allows to sample pixels from an RGB image, along
  with neighboring pixels.
  Sampling from this distribution gives a vector containing 
  first the value of the chosen neighbors of the pixel and after the 
  the value of the pixel itself. The neighbors are specified
  by relative row and column positions. The distribution cycles
  in left-to-right top-down order through all the pixels (except
  the borders where the neighbors are outside the image).
  RGB values may be translated and then scaled
  if scale and offset_are given. 
*/
class RGBImageVMatrix: public Object
{
  friend class RGBImagesVMatrix;

protected:
  RGBImage image;
  Vec delta_row; //!<  row displacement of neighbors
  Vec delta_column; //!<  column displacement of neighbors
  real scale; //!<  preprocessing of RGB values:
  real offset_; //!<  new_pixel = (old_pixel + offset_)*scale;
  int width_; //!<  (neighbors.length()+1)*3
  int max_delta_row, max_delta_column;
  int first_row; //!<  first row that we can sample
  int first_column; //!<  first column that we can sample
  int bottom_border_row; //!<  first row that is in the bottom border
  int right_border_col; //!<  first column that is in the right border
  int n_cols; //!<  right_border_col-first_column
  int current_i, current_j; //!<  current (row,column) of target pixel

public:
/*!     The delta_row and delta_col vectors specify the
    relative position of the neighbors whose RGB values
    will be put before the "target" pixel's RGB values
    in each sample. Scale and offset_allow to linearly
    transform each R, G, and B value, with 
       new_pixel = (old_pixel + offset_)*scale
*/
  RGBImageVMatrix(RGBImage image, 
                       const Vec& delta_row, const Vec& delta_col,
                       real scale=1.0, 
                       real offset_=0.0);
  virtual int width();
  virtual int length();
  virtual void reset();
  virtual void sample(Vec& samplevec);
  virtual void seek(int position);
  virtual int position();

  //!  short hand for (position()==0)
  virtual bool first() 
    { return (current_i==first_row) && (current_j==first_column); }

  //!  replace the active image by this one
  void setImage(RGBImage new_image);
};


/*!   A collection of images that is traversed pixel
  by pixel in the fashion presribed by an RGBImageVMatrix.
  The images are traversed in order, and the correct
  class can optionally be appended as the last column.
*/
class RGBImagesVMatrix: public Object
{
protected:
  RGBImageVMatrix image_distr;
  RGBImageDB& images_db;
  int current_image;
  bool append_class;
  int length_;
  Vec pixelsAndClass;
  Vec pixels;
  Vec image_start; //!<  starting "position" (re: position() method) for each image
  int width_;

public:
  RGBImagesVMatrix(RGBImageDB& images_db,
                        const Vec& delta_row,
                        const Vec& delta_col, 
                        bool append_class,
                        real scale=1.0,
                        real offset_=0.0);

  virtual int width();
  virtual int length();
  virtual void reset();
  virtual void sample(Vec& samplevec);
  virtual void seek(int position);
  virtual int position();
  virtual bool firstSampleOfObject();
  virtual int nSamplesOfObject();
  virtual int nObjects();
};

/*!   ImageVMatrix extends MemoryVMatrix for images
  Sampling from this distribution gives a vector containing 
  first the value of the parents of the pixel and after the 
  the value of the pixel itself. 

class ImageVMatrix: public VMatrix
{
protected:
  Mat data;
  int width_;
  int n_values_per_pixel;
  int ncol_image;
  Mat parents;
  int border_size;
  real scale;
  real offset_;
  int currentpos;
  
  void nextPixel();

public:
  ImageVMatrix(const Mat& the_data, int the_ncol_image, 
                    const Mat& the_parents,int the_border_size=1, 
                    real scale=1.0, real offset_=0.0);
  virtual int length();
  virtual int width();
  virtual void sample(Vec& samplevec);
  virtual void reset();

};
 */

} // end of namespace PLearn

#endif

