//------------------------------------------------------------------------------
//  Copyright 2007-2015 by Jyh-Ming Lien and George Mason University
//  See the file "LICENSE" for more information
//------------------------------------------------------------------------------

#include "draw.h"
#include "draw_basics.h"

#ifdef _WIN32
extern "C"{
#include "triangulate.h"
}
#else 
#include "triangulate.h"
#endif

//-----------------------------------------------------------------------------
//variables used in rendering

#define DEBUG 0

extern Camera camera;
extern Shader shader;
model * mk_model = NULL;

bool showMKS=false;
bool showMKSFacets=true;
bool showMKSCCs=false;
bool showWire=false; //on/off wireframe
bool showP=false;
bool showQ=false;
bool saveImg=false;

int current_ptsize=1;
uint current_imgID=0; //id for dumping images

//CC rendering
int CC_render=1; //CC rendering mode
                 //if CC_render == 0, all facets in the convolution are shown
                 //if CC_render == 1, only mksum facets are shown
                 //if CC_render == 2, only non-mksum facets are shown

vector<Point3d> CC_colors;
bool CC_gray=false;

triangle * selected_triangle = NULL;

//-----------------------------------------------------------------------------
// Functions and varibles from main.h
//-----------------------------------------------------------------------------
extern mascgl_workspace workspace; //workspace
extern int step_left_for_new_rot; //this controls if a new rotation direction is needed

void DisplayMSumFacets();



//-----------------------------------------------------------------------------
// Functions and varibles from main.h
//-----------------------------------------------------------------------------

//for random rotation
bool rotateScene = false;
extern REAL rot_theta;
extern Vector3d rot_vec;
extern Quaternion<REAL> current_rot;
extern int step_left_for_new_rot; //this controls if a new rotation direction is needed
extern int step_per_PI2;
extern int total_budget;

//-----------------------------------------------------------------------------
//
// keyboard/mouse/etc... callback functions
//
//-----------------------------------------------------------------------------

static void error_callback(int error, const char* description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE); break;
		case GLFW_KEY_1: showMKS = !showMKS; break;
		case GLFW_KEY_2: showP = !showP;     break;
		case GLFW_KEY_3: showQ = !showQ;     break;
		//case GLFW_KEY_4: showUpdate = !showUpdate; break;
		case GLFW_KEY_0: saveImg = !saveImg; break;  //turn on/off image dumping
		case GLFW_KEY_W: showWire = !showWire; break;
		//case GLFW_KEY_S: RotateP(); break;           //step
		case GLFW_KEY_SPACE:
			rotateScene = !rotateScene; //rotate scene
			step_left_for_new_rot = 0;
			break;
		}
	}
}


//hand cursor moving event
static void cursor_callback(GLFWwindow * window, double x, double y)
{
	//
	camera.cursor_callback(window, x, y);
}


//handles mouse click
static void mouse_callback(GLFWwindow * window, int button, int action, int mode)
{
	//if (glutGetModifiers() != GLUT_ACTIVE_CTRL) return; //control key must be pressed
	if (mode != GLFW_MOD_CONTROL) //when ctrl is not pressed
	{
		camera.mouse_callback(window, button, action, mode);
		return; 
	}

	//
	if (mk_model == NULL) return;
	if (button != GLFW_MOUSE_BUTTON_LEFT) return;
	if (action != GLFW_PRESS) return;

	selected_triangle = NULL;

	//state.selected_wire = NULL;
	//state.diameter_of_selected_wire = 0;
	#define BUFSIZE 512
	GLuint selectBuf[BUFSIZE];
	GLint hits;

	glSelectBuffer(BUFSIZE, selectBuf);

	GLint viewport[4];
	//float ratio;

	glSelectBuffer(BUFSIZE, selectBuf);

	glGetIntegerv(GL_VIEWPORT, viewport);

	glRenderMode(GL_SELECT);
	glInitNames();

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	int width, height;
	glfwGetWindowSize(window, &width, &height);


	//glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPickMatrix(xpos, viewport[3] - ypos, 1, 1, viewport);

	//ratio = (viewport[2] + 0.0) / viewport[3];
	//gluPerspective(60`, , 0.1, model_radus * 100);
	//gluPerspective(camera.getFOV(), (width*1.0) / height, 0.1, 10000.0f);
	glm::mat4 ProjectionMatrix = camera.getProjectionMatrix();
	glMultMatrixf(&ProjectionMatrix[0][0]);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	
	glm::mat4 ViewMatrix = camera.getViewMatrix();
	//glm::mat4 VP = ProjectionMatrix*ViewMatrix;
	glMultMatrixf(&ViewMatrix[0][0]);
	{
		//DisplayMSumFacets();
		//draw each fact of the mksum
		if (mk_model != NULL)
		{
			auto& MK=getM(); //the mkinkowski sum
			for (uint i = 0; i < mk_model->t_size; i++)
			{
				glPushName(i);
				auto tri=mk_model->tris[i];
				glBegin(GL_TRIANGLES);
				for (short k = 0; k < 3; k++)
				{
					auto pos = mk_model->vertices[tri.v[k]].p.get();
					glVertex3d(pos[0], pos[1], pos[2]);
				}
				glEnd();
				glPopName();
			}
		}
	}


	////draw model itself?
	//glPushName(g_cd.get_cm()->getData()->tips.size()+1);
	//draw(g_cd.getTodoList(), g_cd.getDoneList());
	//glPopName();


	glMatrixMode(GL_PROJECTION);
	glMatrixMode(GL_MODELVIEW);
	glFlush();

	hits = glRenderMode(GL_RENDER);

	//cout << "hit = " << hits << endl;

	GLuint * ptr = (GLuint *)selectBuf;
	GLuint depth = UINT_MAX;
	int selected_pid = 0;
	int selected_qid = 0;

	for (int i = 0; i < hits; i++)  /*  for each hit  */
	{

		GLuint names_size = *ptr; ptr++;
		//cout << "names_size=" << names_size << endl;
		//if (names_size != 3) continue; //tip id, type of wire, wire id

		
		GLuint z1 = *ptr; ptr++;
		GLuint z2 = *ptr; ptr++;
		//GLuint type = *ptr; ptr++;
		GLuint pid = *ptr; ptr++;
		//GLuint qid = *ptr; ptr++;

		//cout << "type=" << (char)type << " pid=" << pid << " qid=" << qid << endl;

		if (z1 < depth)
		{
			depth = z1;
			selected_pid = pid;
			//selected_qid = qid;
		}//end z1<depth (finding the closest hit)

	}//end for each hit

	if (depth == UINT_MAX) return;

	auto & map2mkf=getM().map2mkf;
	auto selected = map2mkf[selected_pid];
	cout << "selected=(" << selected.first->pid << " ," << selected.first->qid << ") type=" << selected.first->type;	
	if (selected.second != NULL)
	{ 
		cout << " gid=" << selected.second->gid << endl; 
		for (auto & e : selected.first->f_g->edges)
		{
			if (e.valid() == false) continue;
			if( (&selected.first->f_g->faces[e.st_f] != selected.second)&& (&selected.first->f_g->faces[e.ts_f] != selected.second) ) 
				continue;

			cout << "["<<e.s<<","<<e.t<<"] manifold = "<<e.is_manifold()
				 <<", clipping = "<<e.is_clipping()
				 <<" incident size="<< e.incident_facets2.size() << " ";
			for (auto & f : e.incident_facets2) cout << f.type << " ";//cout << "("<<f.get()->pid << "," << f.get()->qid<<")" ;
			cout << endl;
		}

		selected.first->f_g->dump();
	}
	else { cout << " gid=" << selected.first->gid << endl; }

	if (selected_pid < mk_model->t_size)
	{
		selected_triangle = &mk_model->tris[selected_pid];
		//for (short d = 0; d < 3; d++) cout << " v[" << d << "].n=" << mk_model->vertices[selected_triangle->v[d]].n << endl;
		//cout << " selected triangle id=" << selected_pid<< " ring id=" << selected.first->ring_id << endl;
	}

}


//-----------------------------------------------------------------------------
//
// Rendering functions
//
//-----------------------------------------------------------------------------

inline void DisplayBox(const double box[6])
{
    GLdouble vertice[]=
    { box[0], box[2], box[4],
    box[1], box[2], box[4],
    box[1], box[2], box[5],
    box[0], box[2], box[5],
    box[0], box[3], box[4],
    box[1], box[3], box[4],
    box[1], box[3], box[5],
    box[0], box[3], box[5]};
    
    //line index
    GLubyte lineid[] = 
    { 0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7};
    
    //setup points
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_DOUBLE, 0, vertice);
    
    //Draw lines
    glLineWidth(1);
    glDrawElements( GL_LINES, 24, GL_UNSIGNED_BYTE, lineid );
    glDisableClientState(GL_VERTEX_ARRAY);
}
//
//inline void DisplayModel(model& M)
//{
//    //glColor3d(1,1,1);
//
//    //Draw facets
//    glEnable( GL_POLYGON_OFFSET_FILL );
//    glPolygonOffset( 0.5f, 0.5f );
//    glBegin(GL_TRIANGLES);
//    for(uint i=0;i<M.t_size;i++){
//        glNormal3fv(M.tris[i].n.get());
//        for(int k=0;k<3;k++)
//            glVertex3dv(M.vertices[M.tris[i].v[k]].p.get());
//    }
//    glEnd();
//    glDisable( GL_POLYGON_OFFSET_FILL );
//
//    //glDisable(GL_LIGHTING);
//    //glBegin(GL_LINES);
//    //for(int i=0;i<M.e_size;i++){
//    //  if(M.edges[i].type=='c') glColor3d(0,0,1);
//    //  else if(M.edges[i].type=='r') glColor3d(1,0,0);
//    //  glVertex3dv(M.vertices[M.edges[i].vid[0]].p.get());
//    //  glVertex3dv(M.vertices[M.edges[i].vid[1]].p.get());
//    //}
//    //glEnd();
//    //glEnable(GL_LIGHTING);
//
//
//

inline void DrawTriangle(model& M, triangle& tri)
{
	glBegin(GL_LINE_LOOP);
	for (int k = 0; k < 3; k++)
	{
		auto p = M.X(M.vertices[tri.v[k]].p).get();
		glVertex3d(p[0], p[1], p[2]);
	}
	glEnd();
}

inline void DisplayWiredModel(model& M)
{
    //Draw Edges
    if(showWire)
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);
        glDisable(GL_LIGHTING);
        for(uint i=0;i<M.t_size;i++){
			if (selected_triangle == &M.tris[i]){
				glColor3d(1, 0, 0);
				glLineWidth(3);
			}
			else{
				glColor3d(0, 0, 0);
				glLineWidth(1);
			}
			
			DrawTriangle(M, M.tris[i]);
        }//end for i
        glEnable(GL_LIGHTING);
		glPopAttrib();
    }
}

inline void draw_m_f_graph_wire(m_f_graph * g)
{
	if (g == NULL) return;
	if (g->faces.empty()) return;

    //draw edges
    int esize=g->edges.size();
    glBegin(GL_LINES);
    for(int i=0;i<esize;i++)
	{

        if(!g->edges[i].valid()) continue; //if invalid

        m_f_graph_face& st_f=g->faces[g->edges[i].st_f];
        m_f_graph_face& ts_f=g->faces[g->edges[i].ts_f];

        if(CC_render==1){
            if(st_f.area<SMALLNUMBER) continue; //no valid gid for st_f
            if(ts_f.area<SMALLNUMBER) continue; //no valid gid for ts_f
            if(getM().CCs[st_f.gid].state==mksum_CC::IN_CD &&
               getM().CCs[ts_f.gid].state==mksum_CC::IN_CD)
                continue;
        }

        if(CC_render==2){
            if(st_f.area<SMALLNUMBER) continue; //no valid gid for st_f
            if(ts_f.area<SMALLNUMBER) continue; //no valid gid for ts_f
            if(getM().CCs[st_f.gid].state==mksum_CC::CD_FREE &&
               getM().CCs[ts_f.gid].state==mksum_CC::CD_FREE)
                continue;
        }

        if(!g->edges[i].is_manifold())
            glColor3f(0,0,0);
		else if (g->edges[i].is_clipping())
			glColor3f(0, 1, 0);
        else
            glColor3f(0,0,0);

        Point3d p1,p2;
        g->f2d.convert(g->nodes[g->edges[i].s].pos, p1);
        g->f2d.convert(g->nodes[g->edges[i].t].pos, p2);
		glVertex3d(p1[0], p1[1], p1[2]);
		glVertex3d(p2[0], p2[1], p2[2]);
    }
    glEnd();
}


inline void draw_m_f_graph(m_f_graph * g)
{
	if (g == NULL) return;
	if (g->faces.empty()) return;

	Fill(g);

	if (!showWire) return; //nothing more to do

	draw_m_f_graph_wire(g);
}


inline void DisplayMSum_Facet_Wire(mksum_facet& f)
{
	int side = f.side();
	Point3d pos[4];
	for (int i = 0; i<side; i++){
		getM().getMPos(f.v[i], pos[i].get());
	}

	glPushAttrib(GL_CURRENT_BIT);
	glBegin(GL_LINES);
	for (int i = 0; i<side; i++){
		int k = i + 1; if (k == side) k = 0;
		if (f.nei[i].size()<2) glColor3d(0, 0, 0);
		else glColor3d(1, 0, 0);
		glVertex3d(pos[i][0], pos[i][1], pos[i][2]);
		glVertex3d(pos[k][0], pos[k][1], pos[k][2]);
	}
	glEnd();
	glPopAttrib();
}

inline void DisplayMSum_Facet(mksum_facet& f)
{
    if(f.f_g==NULL || !showMKSCCs)
    {
        int side=f.side();
        Point3d pos[4];
        for(int i=0;i<side;i++){
            getM().getMPos(f.v[i],pos[i].get());
            //pos[i]=f.disc.o+(pos[i]-f.disc.o)*0.9;
        }

        //glColor3d(1,1,1); //TODO: tmp
        glEnable( GL_POLYGON_OFFSET_FILL );
        glPolygonOffset( 0.5f, 0.5f );
        glBegin(GL_POLYGON);
		auto n = f.n.get();
        glNormal3d(n[0],n[1],n[2]);
		for (int i = 0; i < side; i++) glVertex3d(pos[i][0], pos[i][1], pos[i][2]); 
        glEnd();
        glDisable( GL_POLYGON_OFFSET_FILL ); 


        if(showWire){
			DisplayMSum_Facet_Wire(f);
        }
    }
    else
        draw_m_f_graph(f.f_g);
}


inline void DisplayBall(const sphere_primitive& s, int slices, int stacks)
{
    glPushMatrix();
    const auto * pos=s.o.get();
    glTranslatef(pos[0],pos[1],pos[2]);
	static GLUquadric* sphere=NULL;
	if (sphere == NULL)
	{
		sphere = gluNewQuadric();
		gluQuadricNormals(sphere, GL_SMOOTH);
	}
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	gluSphere(sphere, s.r, slices, stacks);
    glPopMatrix();
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

inline void DisplayDisc(sphere_primitive& s, const Vector3d& n)
{
    double delta=PI2/15;//s.r/10;
    Vector3d v(drand48(),drand48(),drand48());
    v=(n%v).normalize()*s.r;
    Vector3d u=(n%v).normalize()*s.r;

    glBegin(GL_LINE_LOOP);
    //glBegin(GL_POLYGON);
        for(double r=0;r<PI2;r+=delta){
            double sin_r=sin(r);
            double cos_r=cos(r);
            Vector3d tmp_v=u*cos_r+v*sin_r;
            Point3d tmp_p=s.o+tmp_v;
			glVertex3d(tmp_p[0], tmp_p[1], tmp_p[2]);
        }
    glEnd();
}

void DisplayMSumFacets_Wire()
{
	//draw mksum facets
	MKFTS& fts = getM().fts;
	for (MKFTIT i = fts.begin(); i != fts.end(); i++)
	{
		glColor3i(0, 0, 0); //not sure why I need to do this, but this makes better rendering...

		if (i->f_g == NULL){
			if (CC_render == 1)
			if (getM().CCs[i->gid].state != mksum_CC::CD_FREE)
				continue;
			if (CC_render == 2)
			if (getM().CCs[i->gid].state == mksum_CC::CD_FREE)
				continue;
		}

		if (i->f_g == NULL)
		{
			DisplayMSum_Facet_Wire(*i);
		}
		else
		{
			draw_m_f_graph_wire(i->f_g);
		}
	}
}


void DisplayMSumFacets()
{
    //generate CC_colors 
    if(showMKSCCs){
        //---------------------
        uint ccsize=getM().CCs.size();
        if(CC_colors.size()!=ccsize){
            uint old_size=CC_colors.size();
            CC_colors.resize(ccsize,Point3d(0,0,0));
            for(uint i=old_size;i<ccsize;i++) 
                CC_colors[i].set(drand48()+0.2,drand48()+0.2,drand48()+0.2);
        }
        //---------------------
    }

    //draw mksum facets
    MKFTS& fts=getM().fts;
    for(MKFTIT i=fts.begin();i!=fts.end();i++)
	{
		glPushName(i->type);
		glPushName(i->pid);
		glPushName(i->qid);
        glColor3i(0,0,0); //not sure why I need to do this, but this makes better rendering...

        if(!showMKSFacets){
            if(i->f_g==NULL){
                if(CC_render==1)
                    if(getM().CCs[i->gid].state!=mksum_CC::CD_FREE)
                        continue;
                if(CC_render==2)
                    if(getM().CCs[i->gid].state==mksum_CC::CD_FREE)
                        continue;
            }
        }

        if(i->type=='e')
		{
            if(showMKSFacets)
                glColor3f(1,1,0.2f); //Yellowish
            else{
                
                if(CC_gray && i->f_g==NULL){
                    if( getM().CCs[i->gid].state==mksum_CC::CD_FREE)
                        glColor3f(1,1,1);
                    else
                        glColor3f(0,1,0);
                }
				else if (i->f_g == NULL && showMKSCCs)
				{
					const auto& c = CC_colors[i->gid];
					glColor3d(c[0],c[1],c[2]);
				}
            }
            DisplayMSum_Facet(*i);
        }
        else{
            if(showMKSFacets){
                if(i->v[0].pid==i->v[1].pid) 
                    glColor3f(0.2f,0.2f,1); //from Q's facet, blue-ish
                else 
                    glColor3f(1,0.2f,0.2f); //from P's facet, red-ish
            }
            else{
                if(CC_gray && i->f_g==NULL){
                    if( getM().CCs[i->gid].state==mksum_CC::CD_FREE)
                        glColor3f(1,1,1);
                    else
                        glColor3f(0,1,0);
                }
				else if (i->f_g == NULL && showMKSCCs)
				{
					const auto& c = CC_colors[i->gid];
					glColor3d(c[0],c[1],c[2]);
				}
            }

            DisplayMSum_Facet(*i);
        }
        
		glPopName();
		glPopName();
		glPopName();

    }//end for
}

inline void DisplayMSumPoints( void )
{
    //static int GID=-1;
    //if(GID==-1)
    {
        //GID=glGenLists(1);
        //glNewList(GID,GL_COMPILE);    
        list<mksum_pt>& pts=getM().free_pts;
        glBegin(GL_POINTS);
        for(list<mksum_pt>::iterator i=pts.begin();i!=pts.end();i++)
		{
			const auto& n = i->f->n;
			const auto& p = i->pos;
            glNormal3d(n[0], n[1], n[2]);
			glVertex3d(p[0], p[1], p[2]);
        }//end i
        glEnd();
        //glEndList();
    }
    //glCallList(GID);
}



//copied from meshlab
void DisplayBackground(void)
{
	float topcolor[]={1,1,1};
	float bottomcolor[]={0,0,0};
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1,1,-1,1,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLE_STRIP);
		glColor3fv(topcolor);  	glVertex2f(-1, 1);
		glColor3fv(bottomcolor);	glVertex2f(-1,-1);
		glColor3fv(topcolor);	glVertex2f( 1, 1);
		glColor3fv(bottomcolor);	glVertex2f( 1,-1);
	glEnd();
	
	glPopAttrib();
	glPopMatrix(); // restore modelview
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}


inline Vector3d getRandVector()
{
	Vector3d v((REAL)drand48(), (REAL)drand48(), (REAL)drand48());
	for (int i = 0; i<3; i++)
	if (drand48()>0.5) v[i] = -v[i];
	v = v.normalize();
	return v;
}


void RotateP(mksum& MK)
{
    //if(step_left_for_new_rot<=0){ //time's up, another one!
    //    step_left_for_new_rot=(int)(200*drand48());
    //    rot_vec.set(drand48(),drand48(),drand48());
    //    for(int i=0;i<3;i++)
    //        if(drand48()>0.5) rot_vec[i]=-rot_vec[i];
    //    rot_vec=rot_vec.normalize();
    //    rot_theta=(drand48()+0.5f)*PI2/200;
    //}

    //double cos_2=cos(rot_theta/2);
    //double sin_2=sin(rot_theta/2);
    //Quaternion tmp(cos_2,sin_2*rot_vec);
    //current_rot=(tmp*current_rot).normalize();

    //model& P=getP();
    //Matrix3x3 tmpM=current_rot.getMatrix();
    //tmpM.get(P.current_rot);
    //P.rotate(tmpM);

	//time's up, another one!
	if (step_left_for_new_rot <= 0)
	{ 
		if (total_budget <= 0) exit(1);
		step_left_for_new_rot = step_per_PI2;
		rot_vec = getRandVector();
		rot_theta = (PI2) / (step_per_PI2 + 1);
		if (total_budget>0) total_budget--;
	}

	REAL cos_2 = cos(rot_theta / 2);
	REAL sin_2 = sin(rot_theta / 2);
	Quaternion<REAL> tmp(cos_2, sin_2*rot_vec);
	current_rot = (tmp*current_rot).normalize();
	MK.P->setCurrentTransform(MK.P->getCurrentTranslation(), current_rot, MK.P->getCurrentScale());

	//update....
	//double start=getTime();
	MK.update();
	//double end=getTime();

	//static int totoal_sim_step=0;
	//cout<<"- Update Time="<<end-start<<" ms"<<endl;
	selected_triangle = NULL;
	step_left_for_new_rot--;
}


//
//void TimerCallback(int value)
//{
//    //not in simuation state
//    if( !rotateScene ) return;
//    {
//        cout<<"------------------------------"<<endl;
//        RotateP();
//        //update....
//        double start=getTime();
//        getM().update();
//        double end=getTime();
//        cout<<"- Update Time="<<end-start<<" ms"<<endl;
//    }
//
//    //in simuation state
//    glutPostRedisplay();
//    glutTimerFunc(30, TimerCallback,value);
//    step_left_for_new_rot--;
//}
//
//void printGUIKeys()
//{
//	int offset=5;
//    cerr<<"GUI keys (version: "<<mksum::getVersion()<<"):\n\n";
//    cerr<<left<<setw(offset)<<"1"<<"show/hide Minkowski sum\n";
//    cerr<<left<<setw(offset)<<"2"<<"show/hide P\n";
//    cerr<<left<<setw(offset)<<"3"<<"show/hide Q\n";
//    cerr<<left<<setw(offset)<<"4"<<"show/hide convolution\n";
//    cerr<<left<<setw(offset)<<"5"<<"show/hide Minkowski sum facets\n";
//    cerr<<left<<setw(offset)<<"0"<<"save the screen to a pdf file\n";
//    cerr<<left<<setw(offset)<<"%"<<"change color\n";
//    cerr<<left<<setw(offset)<<"c"<<"show/hide internal/external facets\n";
//    cerr<<left<<setw(offset)<<"g"<<"change external to grey and internal to green\n";
//    cerr<<left<<setw(offset)<<"w"<<"show/hide wire frame\n";
//    cerr<<left<<setw(offset)<<"?"<<"show this\n";
//    cerr<<endl; //done
//
//}
//
////keyboard event function
//void Keyboard( unsigned char key, int x, int y )
//{
//    // find closest colorPt3D if ctrl is pressed...
//    switch( key ){
//        case 27: exit(0);
//        case '1' : showMKS=!showMKS; break;
//        case '2' : showP=!showP;
//                   showMKSCCs=false; 
//                   showMKSFacets=false;
//                   break;
//        case '3' : showQ=!showQ;   
//                   showMKSCCs=false; 
//                   showMKSFacets=false;
//                   break;
//        case '4' : showMKSFacets=!showMKSFacets; 
//                   showMKSCCs=false; 
//                   showQ=showP=false;
//                   break;
//        case '5' : showMKSCCs=!showMKSCCs; 
//                   showMKSFacets=false; 
//                   showQ=showP=false;
//                   CC_render=0;
//                   CC_gray=false;
//                   break;
//        case '%' : CC_colors.clear(); break;
//        case 'c' : CC_render++; CC_render=CC_render%3; break;//change cc rendering scheme
//        case 'g' : CC_gray=!CC_gray; break;
//        case '0' : saveImg=!saveImg; break;  //turn on/off image dumping
//        case 'w' : showWire=!showWire; break; 
//        case '?' : printGUIKeys(); break;
//        case ' ' : rotateScene=!rotateScene; //rotate scene
//                   if(rotateScene)
//                    glutTimerFunc(10, TimerCallback, clock());
//                   else
//                    glutTimerFunc(10,NULL,0);
//                   break;
//    }
//
//    glutPostRedisplay();
//}



///////////////////////////////////////////////////////////////////////////
void drawFill(m_f_graph * g, list<uint>& vids)
{
    if(vids.size()>3){
        int ringN=1;
        int ringVN[1]; //new int[ringN];     //number of vertices for each ring
        
        int vN=ringVN[0]=vids.size();               //total number of vertices
        
        if( vN<3 ) return;
        int tN=(vN-2)+2*(ringN-1);       //(n-2)+2*(#holes)
        double * V=new double[vN*2];     //to hole vertices pos
        int *T=new int[3*tN];            //to hole resulting triangles
        
        //copy vertices
        vector<Point2d> pts; pts.reserve(vids.size());
        {   int id=0;
            //cout<<"draw f =";
            for(list<uint>::iterator i=vids.begin();i!=vids.end();id++,i++){
                Point2d& pt=g->nodes[*i].pos;
                pts.push_back(pt);
                V[id*2]=pt[0]*1e10;
                V[id*2+1]=pt[1]*1e10;
                //cout<<"("<<pt[0]<<","<<pt[1]<<") ";
            }
            //cout<<endl;
        }
        
        FIST_PolygonalArray(ringN, ringVN, (double (*)[2])V, &tN, (int (*)[3])T);
        {
            glBegin(GL_TRIANGLES);
			auto n = g->f2d.f3d->n.get();
            glNormal3d(n[0],n[1],n[2]);
            for(int i=0;i<tN;i++){
                for(int j=0;j<3;j++){
                    int tid=T[i*3+j];
                    const Point2d& pos=pts[tid];
                    Point3d tmp;
                    g->f2d.convert(pos,tmp);
					glVertex3d(tmp[0], tmp[1], tmp[2]);
                }
            }
            glEnd();
        }

        delete [] T;
        delete [] V;
    }
    else{
        glBegin(GL_TRIANGLES);
		auto n = g->f2d.f3d->n.get();
		glNormal3d(n[0], n[1], n[2]);
        for(list<uint>::iterator i=vids.begin();i!=vids.end();i++){
            Point2d& pt=g->nodes[*i].pos;
            Point3d tmp;
            g->f2d.convert(pt,tmp);
			glVertex3d(tmp[0], tmp[1], tmp[2]);
        }
        glEnd();
    }
}

void Fill(m_f_graph * g)
{
    //return;

    //typedef list<cd_polygon>::const_iterator PIT;
    int fsize=g->faces.size();

    //setup color
    //static map<m_f_graph_face *, Point3d> colors;
    
    //if(colorid>=0) glDeleteLists(colorid,1);
    //colorid=glGenLists(1);
    //glNewList(colorid,GL_COMPILE);
    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( 0.5f, 0.5f );

    for( int i=0;i<fsize;i++ ){
        m_f_graph_face * f=&g->faces[i];

        //cout<<"area="<<f->area<<endl;

        if(f->area<SMALLNUMBER)
            continue;

        if(CC_render==1)
        if(getM().CCs[f->gid].state!=mksum_CC::CD_FREE) 
            continue;

        if(CC_render==2)
            if(getM().CCs[f->gid].state==mksum_CC::CD_FREE) 
            continue;

        if(CC_gray){
            if( getM().CCs[f->gid].state==mksum_CC::CD_FREE)
                glColor3f(1,1,1);
            else
                glColor3f(0,1,0);
        }
		else
		{
			auto c = CC_colors[f->gid].get();
			glColor3d(c[0],c[1],c[2]);
		}
        //glColor3f(1,0,0);
        drawFill(g,f->vids);
    }
    glDisable(GL_POLYGON_OFFSET_FILL);
    //glEndList();
}


//-----------------------------------------------------------------------------
//
// the main drawing function
//
//-----------------------------------------------------------------------------

//load the given model M into GL buffers
void loadModelBuffers(mksum& MK, M_buffers& buffer)
{
	if (mk_model != NULL)
	{
		delete mk_model;
		buffer.destory_buffers();
		mk_model = NULL;
	}

	mk_model = MK.createModel();

	if (mk_model!=NULL)
		buffer = loadModelBuffers(*mk_model);
}

void draw(mksum& MK)
{
	bool debug = false;

	GLFWwindow * window = initGLcontext(workspace, "MASC 3D Minkowski Sum", error_callback, key_callback, mouse_callback, cursor_callback);
	if (window == NULL) return;

	//initialize openGL flags
	setupGLflags();

	//VBO
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	//-----
	// prepare models for rendering
	//-----
	vector<M_buffers> buffers;

	for (list<object3D *>::iterator i = workspace.models.begin(); i != workspace.models.end(); i++)
	{
		object3D * obj = *i;
		if (dynamic_cast<model*>(obj)) //this object is a mesh
		{
			model* mesh = dynamic_cast<model*>(obj);
			M_buffers mb = loadModelBuffers(*mesh);
			buffers.push_back(mb);
		}
	}
	//-----

	//initialize the main shader
	if (workspace.vertex_shader_name.empty() == false && workspace.fragment_shader_name.empty() == false)
		shader.init(workspace.vertex_shader_name.c_str(), workspace.fragment_shader_name.c_str());

	glUseProgram(shader.id());

	//-----
	//get uniform variables
	shader.addvarible("MVP");
	shader.addvarible("V");
	shader.addvarible("M");
	shader.addvarible("MV3x3");
	shader.addvarible("DepthBiasMVP");
	shader.addvarible("shadowMap");
	shader.addvarible("color_texture");
	shader.addvarible("material.diffuse");
	shader.addvarible("material.specular");
	shader.addvarible("material.emission");
	shader.addvarible("material.shininess");
	shader.addvarible("light.pos");
	shader.addvarible("light.diffuse");
	shader.addvarible("light.specular");
	shader.addvarible("light.ambient");
	shader.addvarible("light.att_const");
	shader.addvarible("light.att_linear");
	//-----

	//Let's have light!
	setupLight(workspace, shader);



	//
	// the main rendering loop
	//
	//glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
	double last_time = glfwGetTime();

	//create models for mksum
	if (debug == false)
	{
		buffers.push_back(M_buffers());
		loadModelBuffers(MK, buffers.back());
	}

	//create depthVP for shadow
	glm::mat4 depthVP;
	createVPfromLight(workspace, workspace.lights.front(), depthVP);
	GLuint depthTexture = renderDepth(workspace, depthVP, buffers);

	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == false)
	{
		if (rotateScene)
		{
			if (glfwGetTime() - last_time > 0.033)
			{
				last_time = glfwGetTime();
				//update the MK sum
				RotateP(MK);
				loadModelBuffers(MK, buffers.back());
				//
				// render depth map from light0
				//
				glDeleteTextures(1, &depthTexture);
				createVPfromLight(workspace, workspace.lights.front(), depthVP);
				depthTexture = renderDepth(workspace, depthVP, buffers);
			}
		}//end if (rotateScene)



		//-----------------------------------------------------------------------
		//
		// draw scene to screen...
		// setup model, view, projection matrix
		//
		//-----------------------------------------------------------------------

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		DisplayBackground();

		glEnable( GL_POLYGON_OFFSET_FILL );
		glPolygonOffset( 0.5f, 0.5f );
		shader.bind();

		glm::mat4 ProjectionMatrix = camera.getProjectionMatrix();
		glm::mat4 ViewMatrix = camera.getViewMatrix();

		//bind shadow texture...
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glUniform1i(shader.value("shadowMap"), 0);

		//here we draw objects
		bool wallonly = false;
		bool texture = true;
		drawMesh(workspace, shader, ProjectionMatrix, ViewMatrix, depthVP, buffers, wallonly, texture);
		
		glDisable(GL_POLYGON_OFFSET_FILL);

		//done....
		shader.unbind();

		///-----------------------------------------------------------------------------------
		//draw wires
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glm::mat4 VP = camera.getProjectionMatrix()*camera.getViewMatrix();
		glMultMatrixf(&VP[0][0]);
		if(debug) DisplayMSumFacets();

		if (showWire)
		{
			for (vector<M_buffers>::iterator i = buffers.begin(); i != buffers.end(); i++)
			{
				if (workspace.is_wall(i->m)) continue; //not a wall and only wall should be rendered...
				DisplayWiredModel(*(i->m));
			}
			DisplayMSumFacets_Wire();
		}

		if (selected_triangle != NULL && mk_model != NULL)
		{
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glColor3d(1, 0, 0);
			glLineWidth(3);
			DrawTriangle(*mk_model, *selected_triangle);
			glPopAttrib();
		}

		/*
		for (auto& m : buffers)
		{
			if (m.m == NULL) continue;
			DisplayWiredModel(*m.m);
		}
		*/

		//draw bounding sphere.
		const query_sphere const * bd = getM().get_spheric_boundary();
		if (bd != NULL)
		{
			glColor4f(0.5f,0.5f,0.75f,0.250f);
			DisplayBall(*bd, 20, 20);
		}

		
		//glColor3f(0.95f, 0.25f, 0.25f);
		//for (auto& tmp_ball : ballsinball)
		//{
		//	DisplayBall(tmp_ball, 20, 20);
		//}

		//sphere_primitive tmp_ball;
		//tmp_ball.o=Point3d(38.69933874, 39.35848574, 8.100400344);
		//tmp_ball.r = 10;
		//glColor3f(0.5f, 0.75f, 0.75f);
		//DisplayBall(tmp_ball, 20, 20);

		////----------------------------------------------------------------------------------

		// Swap buffers
		glfwSwapBuffers(window);
		if (rotateScene)
			glfwPollEvents();
		else
			glfwWaitEvents();
	}

	exit(1); //TODO: the following two lines crashes the program...
	glfwDestroyWindow(window);
	glfwTerminate();
}

