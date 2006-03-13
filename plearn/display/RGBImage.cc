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

#include <fstream>
#include "RGBImage.h"
#include <plearn/math/TMat_maths.h>

namespace PLearn {
using namespace std;

const RGB RGB::BLACK(0,0,0);
const RGB RGB::WHITE(255,255,255);
const RGB RGB::RED(255,0,0);
const RGB RGB::GREEN(0,255,0);
const RGB RGB::BLUE(0,0,255);
const RGB RGB::DISCOCOLORS[] = {RGB::RED, RGB::GREEN, RGB::BLUE};


/************/
/* RGBImage */
/************/

RGBImage::RGBImage(int the_height, int the_width)
  :height_(the_height), width_(the_width)
{
  tmpnam(tmpfilename);
  if(height_>0 && width_>0)
    {
      storage = new Storage<RGB>(height_*width_);
      clear();
    }
}

RGBImage::RGBImage(Mat r, Mat g, Mat b)
  :height_(r.length()), width_(r.width())
{
  tmpnam(tmpfilename);
  storage = new Storage<RGB>(height_*width_);
  for(int i=0; i<height_; i++)
    for(int j=0; j<width_; j++)
      setPixel(i,j,RGB((unsigned char)r(i,j), (unsigned char)g(i,j), (unsigned char)b(i,j)));
}


RGBImage::~RGBImage()
{
  unlink(tmpfilename);
}

void
RGBImage::resize(int newheight, int newwidth)
{
  if(storage)
    storage->resize(newheight*newwidth);
  else
    storage = new Storage<RGB>(newheight*newwidth);
  height_ = newheight;
  width_ = newwidth;
}

void
RGBImage::removeBorders(int top_border,int bottom_border,
                        int left_border,int right_border)
{
  if(!storage)
    PLERROR("Empty RGBImage");
  int newwidth = width_ - (left_border + right_border);
  int newheight = height_ - (top_border + bottom_border);
  if (newheight < 1)
    PLERROR("RGBImage::removeBorders(%d,%d,%d,%d): can't remove more than height_(%d)",
          top_border,bottom_border,left_border,right_border,height_);
  if (newwidth < 1)
    PLERROR("RGBImage::removeBorders(%d,%d,%d,%d): can't remove more than width_(%d)",
          top_border,bottom_border,left_border,right_border,width_);
  PP< Storage<RGB> > newstorage = new Storage<RGB>(newheight*newwidth);
  RGB* d= newstorage->data;
  RGB* data = storage->data;
  for (int r=0;r<newheight;r++)
    for (int c=0;c<newwidth;c++,d++)
      *d = data[(top_border+r)*width_+(left_border+c)];
  height_=newheight;
  width_=newwidth;
  storage = newstorage;
}

void
RGBImage::fill(RGB color)
{
  if(!storage)
    PLERROR("Empty RGBImage");
  RGB* data = storage->data;
  for(int pos=0; pos<height_*width_; pos++)
    data[pos] = color;
}

void 
RGBImage::loadPPM(const char* filename)
{
  int ncolors;
  ifstream in(filename);
  if(!in)
    PLERROR("In RGBImage::loadPPM could not open file %s for reading",filename);

  char imagetype[20];
  int newheight, newwidth;
  in >> imagetype >> newwidth >> newheight >> ncolors;
  char tmp[100];
  in.getline(tmp,99); // skip until end of line
  if(strcmp(imagetype,"P6")!=0)
    PLERROR("In RGBImage::loadPPM unsupported format (not a P6 PPM)");
  resize(newheight, newwidth);
  RGB* data = storage->data;
  PLWARNING("In RGBImage::loadPPM - Code has been changed to compile, but hasn't been tested (remove this warning if it works)");
  // The following line:
  //  in.read((unsigned char*)data,height_*width_*3);
  // has been replaced by:
  in.read((char*)data,height_*width_*3);
}

void 
RGBImage::savePPM(const char* filename) const
{
  if(!storage)
    PLERROR("Empty RGBImage");
  RGB* data = storage->data;
  ofstream out(filename);
  if(!out)
    PLERROR("In RGBImage::savePPM could not open file for writing");
  out << "P6\n" << width_ << ' ' << height_ << ' ' << 255 << endl;
  PLWARNING("In RGBImage::loadPPM - Code has been changed to compile, but hasn't been tested (remove this warning if it works)");
  // The following line:
  //  out.write((unsigned char*)data, height_*width_*3);
  // has been replaced by:
  out.write((char*)data, height_*width_*3);
  out.flush();
}

void 
RGBImage::display() const
{
  savePPM(tmpfilename);
  char command[1000];
  sprintf(command,"xv %s &",tmpfilename);
  system(command);
}

void 
RGBImage::loadJPEG(const char* filename, int scale)
{
  char command[1000];
  sprintf(command,"djpeg -pnm -scale 1/%d %s > %s", scale, filename, tmpfilename);
  system(command);
  cout << command << endl;
  loadPPM(tmpfilename);
  unlink(tmpfilename);
}

void 
RGBImage::displayAndWait() const
{
  savePPM(tmpfilename);
  char command[1000];
  sprintf(command,"xv %s",tmpfilename);
  system(command);
  unlink(tmpfilename);
}

void 
RGBImage::shrinkToIntersection(int& i, int& j, int& h, int& w) const
{
  int istart = MAX(0,i);
  int iend = MIN(i+h,height_);
  int jstart = MAX(0,j);
  int jend = MIN(j+w,width_);

  i = istart;
  j = jstart;
  h = MAX(iend-istart,0);
  w = MAX(jend-jstart,0);
}

void 
RGBImage::blit(int desti, int destj, const RGBImage srcim)
{
  int blit_i = desti;
  int blit_j = destj;
  int blit_h = srcim.height_;
  int blit_w = srcim.width_;

  shrinkToIntersection(blit_i,blit_j,blit_h,blit_w);
  if(blit_h==0 || blit_w==0)
    return;

  int src_i = blit_i-desti;
  int src_j = blit_j-destj;

  for(int i=0; i<blit_h; i++)
    for(int j=0; j<blit_w; j++)
      setPixel(blit_i+i, blit_j+j, srcim.getPixel(src_i+i,src_j+j));
}

RGB
RGBImage::computeAverage() const
{
  if(!storage)
    PLERROR("Empty RGBImage");
  RGB* data = storage->data;
  int r = 0;
  int g = 0;
  int b = 0;
  int npixels = height_*width_;
  for(int pos=0; pos<npixels; pos++)
    {
      RGB& pixl = data[pos];
      r += pixl.r;
      g += pixl.g;
      b += pixl.b;
    }
  return RGB(r/npixels, g/npixels, b/npixels);
}

Vec 
RGBImage::computeHistogram(int r_bins, int g_bins, int b_bins, bool do_normalize)
{
  Vec histo(r_bins*g_bins*b_bins);
  for(int i=0; i<height_; i++)
    for(int j=0; j<width_; j++)
      {
        RGB pix = getPixel(i,j);
        int r_bin = pix.r*r_bins/256;
        int g_bin = pix.g*g_bins/256;
        int b_bin = pix.b*b_bins/256;
        int pos =  r_bin*g_bins*b_bins + g_bin*b_bins  + b_bin;
        histo[pos]++;
      }
  if(do_normalize)
    histo /= height_*width_;
  return histo;
}

/******************/
/*** RGBImageDB ***/
/******************/

RGBImageDB::RGBImageDB(int the_subsample_factor, int the_remove_border, int the_max_n_images_in_memory)
  : subsample_factor(the_subsample_factor), remove_border(the_remove_border), 
    max_n_images_in_memory(the_max_n_images_in_memory), n_images_in_memory(0)
{
}

RGBImageDB::RGBImageDB(char* dbfilename, int the_subsample_factor,int the_remove_border, int the_max_n_images_in_memory)
  : subsample_factor(the_subsample_factor), remove_border(the_remove_border),
    max_n_images_in_memory(the_max_n_images_in_memory), n_images_in_memory(0)    
{
  load(dbfilename);
}

void 
RGBImageDB::load(char* dbfile)
{
  ifstream in(dbfile);
  if(!in)
    PLERROR("In RGBImageDB::load could not open dbfile %s for reading",dbfile);
  
  char filename[300];
  int classnum;

  int n = 0;
  in >> filename >> classnum;
  while(in)
    {
      int imageid = append(filename);
      n++;
      imageIdAndClass.resize(n,2);
      imageIdAndClass(n-1,0) = imageid;
      imageIdAndClass(n-1,1) = classnum;
      in >> filename >> classnum;
    }
}

int
RGBImageDB::append(char* filename)
{
  filenames.append(strdup(filename));
  images.append(0);
  return filenames.size()-1;
}

RGBImage
RGBImageDB::getImage(int imageid)
{
  if(!images[imageid])
    {
      RGBImage* newimage = new RGBImage;
      newimage->loadJPEG(filenames[imageid],subsample_factor);
      if (remove_border>0)
        newimage->removeBorders(remove_border);
      n_images_in_memory++;
      if (n_images_in_memory>max_n_images_in_memory)
        {
          for(int i=0; i<images.size(); i++)
            if(images[i]!=0)
              {
                delete images[i];
                images[i] = 0;
              }
        }
      images[imageid] = newimage;
    }
  return *images[imageid];
}


Mat 
RGBImageDB::computeHistogramRepresentation(int r_bins, int g_bins, int b_bins, bool do_normalize)
{
  Mat dataset(imageIdAndClass.length(), r_bins*g_bins*b_bins+1);
  Mat histograms = dataset.subMatColumns(0,r_bins*g_bins*b_bins);
  Mat classnums = dataset.lastColumn();

  for(int i=0; i<imageIdAndClass.length(); i++)
    {
      RGBImage im = getImage((int)imageIdAndClass(i,0));
      histograms(i) << im.computeHistogram(r_bins,g_bins,b_bins,do_normalize);
      classnums(i,0) = imageIdAndClass(i,0);
    }
  return dataset;
}


RGBImageDB::~RGBImageDB()
{
  for(int i=0; i<filenames.size(); i++)
    {
      free(filenames[i]);
      if(images[i])
        delete images[i];
    }
}




/*****************************/
/*** RGBImageVMatrix ***/
/*****************************/

RGBImageVMatrix::
RGBImageVMatrix(RGBImage the_image,
                     const Vec& drow, const Vec& dcol,
                     real the_scale,
                     real the_offset)
  :image(the_image),
   delta_row(drow), delta_column(dcol),
   scale(the_scale), offset_(the_offset)
{
  if (delta_row.length()!=delta_column.length())
    PLERROR("RGBImageVMatrix: delta_row(%d) and delta_column(%d) have"
          " differing sizes",delta_row.length(),delta_column.length());
  width_ = 3 * (delta_row.length() + 1);
  if (delta_row.length()>0)
    {
      max_delta_row = (int)max(delta_row);
      int min_delta_row = (int)min(delta_row);
      max_delta_column = (int)max(delta_column);
      int min_delta_column = (int)min(delta_column);
      first_row = MAX(0,-min_delta_row);
      first_column = MAX(0,-min_delta_column);
      bottom_border_row = MIN(image.height(),image.height()-max_delta_row);
      right_border_col = MIN(image.width(),image.width()-max_delta_column);
    }
  else
    {
      first_row = 0;
      first_column = 0;
      bottom_border_row = image.height();
      right_border_col = image.width();
    }
  n_cols = right_border_col - first_column;
  reset();
}

void RGBImageVMatrix::setImage(RGBImage new_image)
{
  image = new_image;
  if (delta_row.length()>0)
    {
      bottom_border_row = 
        MIN(image.height(),image.height()-max_delta_row);
      right_border_col = 
        MIN(image.width(),image.width()-max_delta_column);
    }
  else
    {
      bottom_border_row = image.height();
      right_border_col = image.width();
    }
  n_cols = right_border_col - first_column;
  reset();
}

int RGBImageVMatrix::length()
{ return (bottom_border_row-first_row)*n_cols; }

int RGBImageVMatrix::width() 
{ return width_; }

void RGBImageVMatrix::reset()
{ 
  current_i=first_row;
  current_j=first_column;
}

void RGBImageVMatrix::seek(int position)
{
  if (position>=length())
    PLERROR("RGBImageVMatrix::seek(%d > size=%d)",position,length());
  int i = position / n_cols;
  current_i = first_row + i;
  current_j = first_column + position - i*n_cols;
}

int RGBImageVMatrix::position()
{
  return (current_j-first_column) + (current_i-first_row)*n_cols;
}

void RGBImageVMatrix::sample(Vec& samplevec)
{
  samplevec.resize(width_);
  real *sample = samplevec.data();
  real *dr = delta_row.data();
  real *dc = delta_column.data();
  int np = delta_row.length();
  // copy the value of its neighbors
  for (int p=0; p<np; p++) {
    int offset_i=(int)dr[p];
    int offset_j=(int)dc[p];
    RGB parent_rgb = image.getPixel(current_i+offset_i,current_j+offset_j);
    *(sample++) = (parent_rgb.r+offset_)*scale;
    *(sample++) = (parent_rgb.g+offset_)*scale;
    *(sample++) = (parent_rgb.b+offset_)*scale;
  }   

  // copy the value of the pixel
  RGB target_rgb = image.getPixel(current_i,current_j);
  *(sample++) = (target_rgb.r+offset_)*scale;
  *(sample++) = (target_rgb.g+offset_)*scale;
  *(sample++) = (target_rgb.b+offset_)*scale;

  // update current_i and current_j, traversing in
  // the usual left-to-right / top-to-bottom order:
  current_j++;
  if (current_j==right_border_col)
    {
      current_j = first_column;
      current_i++;
      if (current_i==bottom_border_row)
        current_i = first_row;
    }
}

/*****************************/
/*** RGBImagesVMatrix ***/
/*****************************/

RGBImagesVMatrix::
RGBImagesVMatrix(RGBImageDB& imagesdb,
                      const Vec& delta_row,
                      const Vec& delta_col, 
                      bool appendclass,
                      real scale,
                      real offset_)
  :image_distr(imagesdb.getImage((int)imagesdb.imageIdAndClass(0,0)),delta_row,delta_col,scale,offset_),
   images_db(imagesdb), current_image(0), append_class(appendclass),
   length_(0), image_start(imagesdb.imageIdAndClass.length()) 
{ 
  width_ = 3*(delta_row.length()+1);
  if(append_class)
    width_++;

  for (int i=0;i<images_db.imageIdAndClass.length();i++)
    {
      image_start[i] = length_;
      image_distr.setImage(images_db.getImage((int)imagesdb.imageIdAndClass(i,0)));
      length_ += image_distr.length();
    }
  if (append_class)
    {
      int n = image_distr.width();
      pixelsAndClass.resize(n+1);
      pixels = pixelsAndClass.subVec(0,n);
    }
}

int RGBImagesVMatrix::length() 
{ return length_; }

int RGBImagesVMatrix::width() 
{ return width_; }

void RGBImagesVMatrix::reset() 
{ 
  current_image = 0;
  image_distr.setImage(images_db.getImage((int)images_db.imageIdAndClass(0,0)));
  image_distr.reset();
}

void RGBImagesVMatrix::sample(Vec& samplevec) 
{ 
  samplevec.resize(width_);
  if (append_class)
    {
      Vec tmpvec = samplevec.subVec(0,width_-1);
      image_distr.sample(tmpvec);
      samplevec[width_-1] = images_db.imageIdAndClass(current_image,1);
    }
  else
    image_distr.sample(samplevec);
      
  if (image_distr.first())
    {
      current_image++;
      if (current_image == images_db.imageIdAndClass.length())
        current_image = 0;
      image_distr.setImage(images_db.getImage((int)images_db.imageIdAndClass(current_image,0)));
    }
}  


void RGBImagesVMatrix::seek(int position)
{
  int n=images_db.imageIdAndClass.length();
  int i;
  for (i=0;i<n && position>=image_start[i];i++);
  current_image = i-1;
  image_distr.setImage(images_db.getImage((int)images_db.imageIdAndClass(current_image,0)));
  image_distr.seek(int(position-image_start[current_image]));
}
     
  
int RGBImagesVMatrix::position()
{
  return (int)image_start[current_image] + image_distr.position();
}

bool RGBImagesVMatrix::firstSampleOfObject()
{
  return image_distr.first();
}

int RGBImagesVMatrix::nSamplesOfObject()
{
  return image_distr.length();
}


int RGBImagesVMatrix::nObjects()
{
  return images_db.imageIdAndClass.length(); 
}


/*** ImageVMatrix ***/

/*

ImageVMatrix::ImageVMatrix(const Mat& the_data, 
                                     int the_ncol_image, const Mat& the_parents,
                                     int the_border_size=1, real the_scale=1.0,
                                     real the_offset=0.0)
  :data(the_data), n_values_per_pixel(the_data.width()), ncol_image(the_ncol_image), 
   parents(the_parents), border_size(the_border_size), 
   scale(the_scale), offset(the_offset)
{
     width_ = n_values_per_pixel * (the_parents.length() + 1);
     reset();
}


void ImageVMatrix::nextPixel()
{
  // skip the first row and the first column in the image
  // this will be changed
  currentpos++;
  while ((currentpos % ncol_image < border_size) ||
         (currentpos % ncol_image >= ncol_image - border_size))
    currentpos++;
  if(currentpos>=data.length()-ncol_image*border_size) // cycling
    reset();
}

void ImageVMatrix::reset()
{ 
  currentpos = border_size*ncol_image; 
  nextPixel();
}

void ImageVMatrix::sample(Vec& samplevec)
{
  samplevec.resize(width_);
  real *sample = samplevec.data();
  real *pixel_value;

  // copy the value of its parents
  for (int p=0; p<parents.length(); p++) 
    {
      int offset_x=(int)parents(p,0);
      int offset_y=(int)parents(p,1);
      pixel_value = data[currentpos+offset_x+ncol_image*offset_y]; 
      for (int i=0; i<n_values_per_pixel; i++)
        sample[i] = (pixel_value[i]+offset_) * scale;
    }   

  // copy the value of the pixel
  pixel_value=data[currentpos];
  for (int i=0; i<n_values_per_pixel; i++)
    sample[i + n_values_per_pixel * parents.length()]
      = (pixel_value[i]+offset_) * scale;

  nextPixel();
}

int ImageVMatrix::length()
{ return data.length() - 2*border_size*ncol_image -
    2*border_size*(data.length()/ncol_image-2*border_size); }

int ImageVMatrix::width()
{ return width_; }

*/

} // end of namespace PLearn
