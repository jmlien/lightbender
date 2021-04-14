//------------------------------------------------------------------------------
//  Copyright 2007-2015 by Jyh-Ming Lien and George Mason University
//  See the file "LICENSE" for more information
//------------------------------------------------------------------------------

#ifndef _BF_MINKOWSKI_SUM_H_
#define _BF_MINKOWSKI_SUM_H_


//
//
//
// This is the main header file is shared by m+3d.cpp (single thread) and pm+3d.cpp
// (multi-thread). Functions defined in this file are mainly for text parsing and
// OpenGL rendering 
//
//
//
//#include "gettime.h"
#include "model.h"
#include "minkowski.h"
#include "ws.h"
mascgl_workspace workspace; //workspace

#ifdef _WIN32
extern "C"{
#include "triangulate.h"
}
#else 
#include "triangulate.h"
#endif

//-----------------------------------------------------------------------------
// INPUTS
string Pfile, Qfile;
string output_filename;
string input_filename;
bool showGL=true;
double perturb=0; //no perturb by default
bool neg_P=false, neg_Q=false; //negate P or/and Q
bool rev_P=false, rev_Q=false; //reverse P or/and Q


//-----------------------------------------------------------------------------
// Intermediate data
mksum g_M;
mksum& getM(){ return g_M; } //get singleton
model* P = NULL;
model* Q = NULL;
int total_cd_call=0;
int threadsize=1;

//-----------------------------------------------------------------------------
//for random rotation
double rot_theta;
Vector3d rot_vec;
Quaternion<REAL> current_rot;
int step_left_for_new_rot=0;
int play_frame_size=0;  //this allow automatic play
int step_per_PI2 = 500;
int total_budget = 100;

//-----------------------------------------------------------------------------
//read M+ from file
bool readfromfile();
void save2file(FILE * fp);
void printUsage(char * name);

//-----------------------------------------------------------------------------
bool parseArg(int argc, char ** argv)
{
    double scale=1;
    Vector3d rot_angles;
	string env_filename;

    for(int i=1;i<argc;i++){
        if(argv[i][0]=='-')
        {
            if(strcmp(argv[i],"-d")==0) getM().covering_d=atof(argv[++i]);
            //else if(strcmp(argv[i],"-rot")==0){
            //    for(int k=0;k<3;k++){
            //        if( !isdigit(argv[i+1][0]) && argv[i+1][0]!='-' && argv[i+1][0]!='+' ) break;
            //        rot_angles[k]=atof(argv[++i])*PI/180;
            //    }
            //}
            else if(strcmp(argv[i],"-scale")==0) scale=atof(argv[++i]);
            else if(strcmp(argv[i],"-output")==0) output_filename=argv[++i];
            else if(strcmp(argv[i],"-input")==0) input_filename=argv[++i];
            else if(strcmp(argv[i],"-g")==0) showGL=false;
            else if(strcmp(argv[i],"-thread")==0) threadsize=atoi(argv[++i]);
            else if(strcmp(argv[i],"-perturb")==0) perturb=atof(argv[++i]);
            else if(strcmp(argv[i],"-play")==0) play_frame_size=atoi(argv[++i]); //random rotation
			else if (strcmp(argv[i], "-budget") == 0) total_budget = atoi(argv[++i]);
			else if(strcmp(argv[i], "-step") == 0) step_per_PI2 = atoi(argv[++i]);
            else return false; //unknown
        }
        else if(argv[i][0]=='^'){
            if(strcmp(argv[i],"^P")==0) rev_P=true;
            else if(strcmp(argv[i],"^Q")==0) rev_Q=true;
        }
        else if(argv[i][0]=='~')
        {
            if(strcmp(argv[i],"~P")==0) neg_P=true;
            else if(strcmp(argv[i],"~Q")==0) neg_Q=true;
        }
        else{
			env_filename = argv[i];
        }
    }

	if (env_filename.empty())
	{
		cerr << "! Error: No .r (rendering) file is given." << endl;
		printUsage(argv[0]);
		return false;
	}

	if (env_filename.find(".r") == string::npos)
	{
		cerr << "! Error: the given file (" << env_filename << ") is not a .r (rendering) file." << endl;
		printUsage(argv[0]);
		return false;
	}

	//
	workspace.initialize(env_filename);

	//
	list<model*> input_models;
	for (auto m : workspace.models)
	{
		model* P = dynamic_cast<model*>(m);
		if (P == NULL) continue;
		if (workspace.is_wall(P)) continue;
		input_models.push_back(P);
	}

	if (input_models.size() < 2)
	{
		cerr << "! Error: File " << env_filename << " only have " << input_models.size() << " model. Two needed." << endl;
		return false;
	}

	P = dynamic_cast<model*>(input_models.front());
	Q = dynamic_cast<model*>(input_models.back());
	assert(P&&Q);

    //read Mksum directly from a file
    //note that this will over written some values above...
    if(!input_filename.empty())
        if( !readfromfile() )
            return false;

	if(showGL) cerr<<"- Press \"?\" to show GUI control keys"<<endl;
    //compute -P
    if(neg_P) P->negate();
	if (neg_Q)
	{
		Q->negate();
		swap(P, Q);
	}

    //Reverse P/Q
    //if(rev_P) P->reverse();
    //if(rev_Q) Q->reverse();

    //build cd (any transformation before this will affect CD...)
    P->buid_collision_detection();
    Q->buid_collision_detection();

    //rotate P
	current_rot = P->getCurrentOrientation();
	
    return true;
}

void printUsage(char * name)
{
    int offset=20;
    cerr<<"Usage: "<<name<<" [options] P.obj Q.obj \n"
        <<"options (version: "<<mksum::getVersion()<<"):\n\n";

    cerr<<left<<setw(offset)<<"-thread value"<<"Specify the number of threads to be used (default is 1)\n";
    cerr<<left<<setw(offset)<<"-perturb value"<<"Perturb P by value. Useful if P=Q or facets of P/Q are parallel\n";
    cerr<<left<<setw(offset)<<"-output *.obj"<<"specify an output filename. "
                            <<"The Minkowski sum will be dumped into this file \n";
    cerr<<left<<setw(offset)<<"-g"<<"disable openGL visualization\n";
    cerr<<left<<setw(offset)<<"-play"<<"rondomly rotate P at every frame (can be very slow)\n";
    cerr<<left<<setw(offset)<<"~P"<<"Use the negative image of P instead of P\n";
    cerr<<left<<setw(offset)<<"~Q"<<"Use the negative image of Q instead of Q\n";
    
    cerr<<left<<setw(offset)<<"\n[Options for point-based computation]\n";
    cerr<<left<<setw(offset)<<"-d  value "<<"Set sampling density (d-covering) for both P and Q\n";
    cerr<<left<<setw(offset)<<"-dP value "<<"Set sampling density (d-covering) for P\n";
    cerr<<left<<setw(offset)<<"-dQ value "<<"Set sampling density (d-covering) for Q\n";
    

    offset=50;
    cerr<<"\n\n";
    cerr<<"Example 1: "<<name<<" -g ball.obj cube.obj\n";
    cerr<<"This command generates the Minkowski sum of  ball and cube"
        <<"without displaying it\n\n";
    cerr<<"Example 2: "<<name<<" -thread 4 -d 0.1 -output ball+cube.obj ball.obj cube.obj\n";
    cerr<<"This command generates a point set with 0.1-covering using 4 "
        <<"threads and save the resulting Minkowski sum in ball+cube.obj \n\n";
    cerr<<"\n-- Report bugs to: Jyh-Ming Lien jmlien@cs.gmu.edu";
    cerr<<endl; //done
}

//-----------------------------------------------------------------------------

bool readfromfile()
{
    FILE * fp=fopen(input_filename.c_str(),"rb");
    if(fp==NULL){
        cerr<<"! Error: Cannot open file ("<<input_filename<<")"<<endl;
        return false;
    }
    //----------------------------------------------------------------------------
    //save file is filename is provided
    int freeptsize;
    fread(&freeptsize,sizeof(int),1,fp);
    fread(&getM().covering_d,sizeof(double),1,fp);
    fread(&neg_P,sizeof(bool),1,fp);
    fread(&neg_Q,sizeof(bool),1,fp);
    //----------------------------------------------------------------------------
    list<mksum_pt>& pts=getM().free_pts;
    mksum_pt tmp;
    for(list<mksum_pt>::iterator i=pts.begin();i!=pts.end();i++){
        fread(tmp.pos.get(),sizeof(double)*3,1,fp);
        fread(tmp.f->n.get(),sizeof(double)*3,1,fp);
    }//end i

    fclose(fp);
    return true;
}

void save2file(ofstream& fout, m_f_graph * g,  m_f_graph_face * f, int base)
{
    int ringN=1;
    int ringVN[1]; //number of vertices for each ring
    int vN=ringVN[0]=f->vids.size(); //total number of vertices
    if( vN<3 ) return;
    int tN=(vN-2)+2*(ringN-1);       //(n-2)+2*(#holes)
    double * V=new double[vN*2];     //to hole vertices pos
    int *T=new int[3*tN];            //to hole resulting triangles
    
    //copy vertices
    int id=0;
    vector<uint> vids;
    vids.reserve(f->vids.size());
    for(list<uint>::iterator i=f->vids.begin();i!=f->vids.end();id++,i++){
        vids.push_back(*i);
        Point2d& pt=g->nodes[*i].pos;
        V[id*2]=pt[0]*1e10;
        V[id*2+1]=pt[1]*1e10;
   }
    
    FIST_PolygonalArray(ringN, ringVN, (double (*)[2])V, &tN, (int (*)[3])T);

    for(int i=0;i<tN;i++){
        fout<<"f ";
        for(int j=0;j<3;j++){
            fout<<vids[T[i*3+j]]+base<<" ";
        }
        fout<<"\n";
    }//for 

    delete [] T;
    delete [] V;
}

void save2file(FILE * fp)
{
    //configurations of P and Q
    //fwrite(P->current_rot,sizeof(double),9,fp);
    //fwrite(Q->current_rot,sizeof(double),9,fp);
    static uint save_count=0;
    char number[256];
    sprintf(number,"-%010d",save_count);
    string obj_file_name=Pfile+"+"+Qfile+number+".obj";
    int name_len=obj_file_name.size();
    fwrite(&name_len,sizeof(int),1,fp);
    fprintf(fp,"%s", obj_file_name.c_str());

    //save obj file here
    ofstream fout(obj_file_name.c_str());
    //save vertices
    Point3d pos;
    for(MKFTIT i=getM().fts.begin();i!=getM().fts.end();i++){
        if(i->f_g==NULL){
            if( getM().CCs[i->gid].state!=mksum_CC::CD_FREE ) 
                continue;
            int vsize=i->side();
            for(int j=0;j<vsize;j++){
                getM().getMPos(i->v[j],pos.get());
                fout<<"v "<<pos<<"\n";
            }//j
        }
        else{
            int vsize=i->f_g->nodes.size();
            for(int k=0;k<vsize;k++){
                i->f_g->f2d.convert(i->f_g->nodes[k].pos,pos);
                fout<<"v "<<pos<<"\n";
            }//end k
        }
    }//i

    //output facets
    uint vid=1;
    for(MKFTIT i=getM().fts.begin();i!=getM().fts.end();i++){
        if(i->f_g==NULL){
            if( getM().CCs[i->gid].state!=mksum_CC::CD_FREE ) 
                continue;
            int vsize=i->side();
            fout<<"f ";
            for(int j=0;j<vsize;j++){
                fout<<vid<<" ";
                vid++;
            }//j
            fout<<"\n";
        }
        else{
            int fsize=i->f_g->faces.size();
            for(int k=0;k<fsize;k++){
                m_f_graph_face * f=&i->f_g->faces[k];
                if(f->area<SMALLNUMBER)
                    continue;
                if( getM().CCs[ f->gid].state!=mksum_CC::CD_FREE ) 
                    continue;
                save2file(fout,i->f_g,f,vid);
            }//end k
            vid+=i->f_g->nodes.size();
        }
    }//i

    fout<<flush;
    fout.close();
    //done save obj file here

    save_count++;
}


//void computeCOM_R()
//{
//    //-------------------------------------------------------------------------
//    // compute center of mass and R...
//    COM.set(0,0,0);
//
//    //for(uint i=0;i<getM().free_pts.size();i++){
//    int total=0;
//    for(MKFTIT i=getM().fts.begin();i!=getM().fts.end();i++){
//        int ptsize=i->side();
//        for(int j=0;j<ptsize;j++){
//            double pos[3];
//            getM().getMPos(i->v[j],pos);
//            COM[0]+=pos[0];
//            COM[1]+=pos[1];
//            COM[2]+=pos[2];
//        }
//        total+=ptsize;
//    }
//    for(int j=0;j<3;j++) COM[j]/=total;
//
//    R=0;
//    for(MKFTIT i=getM().fts.begin();i!=getM().fts.end();i++){
//        int ptsize=(i->type=='e')?4:3;
//        for(int j=0;j<ptsize;j++){
//            double pos[3];
//            getM().getMPos(i->v[j],pos);
//            Point3d tmp(pos);
//            double d=(tmp-COM).normsqr();
//            if(d>R) R=d;
//        }
//    }
//    R=sqrt(R);
//}

#endif //_BF_MINKOWSKI_SUM_H_



