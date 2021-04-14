//------------------------------------------------------------------------------
//  Copyright 2007-2015 by Jyh-Ming Lien and George Mason University
//  See the file "LICENSE" for more information
//------------------------------------------------------------------------------


#ifndef _MKSUM_DRAW_H_
#define _MKSUM_DRAW_H_

#include <time.h>
#include "minkowski.h"
#include "minkowski-facet.h"

//-----------------------------------------------------------------------------
// Draw the scene
//-----------------------------------------------------------------------------

void draw(mksum& MK);

//
//void Fill(m_f_graph * g);
//
////copied from meshlab
//void DisplayBackground(void);
//void Display_MKSUM();
//
////-----------------------------------------------------------------------------
//void save_PDF(string pdf_name);
//
////-----------------------------------------------------------------------------
//void Display(void);
//
////-----------------------------------------------------------------------------
//// regular openGL callback functions
//bool InitGL();
//void Reshape(int w, int h);
//void TimerCallback(int value);
//void printGUIKeys();
//
////keyboard event function
//void Keyboard(unsigned char key, int x, int y);

///////////////////////////////////////////////////////////////////////////
void drawFill(m_f_graph * g, list<uint>& vids);
void Fill(m_f_graph * g);

#endif //_MKSUM_DRAW_H_


