#include "lightbender.h"
#include "soil/SOIL.h"

extern Camera camera; //defined in draw_basics.cpp

LightBender::LightBender(const mascgl_workspace& ws)
{
  this->img_name=ws.input_image_filename;  //input image file
  this->img=NULL;

  this->lights=std::vector<light*>(ws.lights.begin(), ws.lights.end());

	this->camera=&::camera; //camera position
	this->structure_width=ws.structure_width;
  this->structure_height=ws.structure_height;
	this->gap=ws.gap;
	this->n_row=ws.n_row;
  this->n_col=ws.n_col;
}

//make sure to free image memory
LightBender::~LightBender()
{
  SOIL_free_image_data(this->img);
}

//build grid, i.e., panel_dir
bool LightBender::build()
{
  //read the image from file
  this->img=SOIL_load_image(this->img_name.c_str(),&this->img_width,&this->img_height,&this->img_channels,SOIL_LOAD_RGBA);
  if(this->img==NULL){
    cerr<<"! Error: Failed to read image "<<this->img_name<<endl;
    return false; //failed to read image
  }

  cout<<"- Read "<<this->img_name<<" ("<<img_width<<","<<img_height<<") with "<<img_channels<<" channel(s)"<<endl;

  //build panel_dir using and this->camera
  panel_dir = std::vector< std::vector<mathtool::Vector3d> > (n_row, std::vector<mathtool::Vector3d>(n_col, mathtool::Vector3d()) );
  for(int r=0;r<n_row;r++)
    for(int c=0;c<n_col;c++)
      panel_dir[r][c]=compute_panel_dir(r, c);

  return true;
}

Vector3d LightBender::compute_panel_dir(int r, int c)
{
  Vector3d n;
  //
  // each panel has width  =(structure_width-(n_col+1)*gap)/n_col
	//                height =(structure_height-(n_row+1)*gap)/n_row

  //the center of panel (r,c) is determined by structure_width, structure_height, gap

  //the target color of panel (r,c) is determined by this->img, img_width & img_height

  //the panel (r,c) direction is determined by
  // - the target color of panel (r,c)
  // - this->camera, this->lights


  //return the orientation of panel (r,c)
  return n;
}

//dump the result to an obj file
void LightBender::save2obj(const std::string& filename)
{
  // build the geometry from  panel_dir

  // save the geometry to the obj file
  cout<<"- Light bender saved to "<<filename<<endl;
}
