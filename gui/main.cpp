//------------------------------------------------------------------------------
//  Copyright 2007-2015 by Jyh-Ming Lien and George Mason University
//  See the file "LICENSE" for more information
//------------------------------------------------------------------------------

#include "draw.h"
#include "dm+3d.h"


//
//
//
// This is a main file that uses a single thread to compute the Minkowski sum
//
//
//



//
//
//
//
//   compute Minkowski sum points....
//
//
//
//

//-----------------------------------------------------------------------------
//collect points on the minkowski sum surface using specified filters

void MKS_create()
{
	if (getM().initialize(workspace.dymsum_method_params)==false)
	{
		cerr << "! Error: Failed to initialize the minkowski sum" << endl;
		exit(1);
	}

    getM().build(P,Q);
}


//compute the point based minkwoski sum
void ComputeMKSum()
{
    double start=getTime();
    MKS_create();
    double end=getTime();

    cout<<"\n- Total Computation Time="<< end-start <<" ms"
        <<"\n- Total CD calls="<<getM().total_cd_call;
    if(!getM().free_pts.empty())
        cout<<"\n- There are "<<getM().free_pts.size()<<" boundary points with d="<<getM().covering_d;
    cout<<endl;

    //save data to file
    if(!output_filename.empty()){
        //open output file
		FILE * fp=fopen(output_filename.c_str(),"wb");
		fprintf(fp,"%s %s\n", Pfile.c_str(), Qfile.c_str());
		assert(fp);
		fwrite(&neg_P,sizeof(bool),1,fp);
		fwrite(&neg_Q,sizeof(bool),1,fp);
		save2file(fp);
        fclose(fp);
    }
}

//
//
//
//
//   The MAIN function
//
//
//
//

int main(int argc, char ** argv)
{
    //
    cout.precision(10);
	srand48(0);

    //cout<<fixed;
    //
    double start=getTime();
    if(!parseArg(argc,argv)){
        printUsage(argv[0]);
        return 1;
    }	
    double end=getTime();
    cout<<"- Initialization takes "<<end-start<<" ms"<<endl;

    //compute/read MK sum here
    if(input_filename.empty()) //no existing restul, compute one
    {
        ComputeMKSum();
    }

    /////////////////////////////////////////////////////////////////
    //setup glut/gli
    if(showGL){
		draw(getM());
    }
	else{
		step_left_for_new_rot=0;
		for(int i=0;i<play_frame_size-1;i++){
			cout<<"------------------------------"<<endl;
			
			//RotateP();

			//update....
			double start=getTime();
			getM().update();
			double end=getTime();
			cout<<"- Update Time="<<end-start<<" ms"<<endl;

			if(!output_filename.empty()){
				//open output file
				FILE * fp=fopen(output_filename.c_str(),"a+b");
				assert(fp);
				save2file(fp);
				fclose(fp);
			}

		}//end i
	}

    return 0;
}

