#include <string>
#include <iostream>
#include <camera.h>
#include <light.h>
#include <texture.h>
#include <ws.h>

//
// create obj file from an image
//
class LightBender
{
public:

	//constructor, setup input/output
	LightBender(const mascgl_workspace& ws);

	//make sure to free image memory
	~LightBender();

	//build grid, i.e., panel_dir
	//return true when build is successful
	bool build();

	//dump the result to an obj file
	void save2obj(const std::string& filename);

protected:

	//compute the normal direction of each panel
	Vector3d compute_panel_dir(int r, int c);

private:

	std::string img_name;  //input image file
	unsigned char* img; //input image
	int img_width, img_height, img_channels;

	std::vector<light*> lights;
	Camera * camera; //camera position

	//structure paramters

	//the structure has n_row X n_col panels
	//center of the structure is located at (0,0)
	//the structure is bounded by (-structure_width/2, -structure_height/2) And
	//(structure_width/2, structure_height/2)
	//
	float structure_width, structure_height; //dimension of the structure
	float gap;         //size of the gap between cells
	int n_row, n_col;  //number of rows and cols

	//intermediate results
	std::vector< std::vector<Vector3d> > panel_dir; //(n_row X n_col) panel orientations

};
