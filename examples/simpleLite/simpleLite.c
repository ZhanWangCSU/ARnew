/* this is a test
 *  simpleLite.c
 *
 *  Some code to demonstrate use of ARToolKit.
 *
 *  Press '?' while running for help on available key commands.
 *
 *  Disclaimer: IMPORTANT:  This Daqri software is supplied to you by Daqri
 *  LLC ("Daqri") in consideration of your agreement to the following
 *  terms, and your use, installation, modification or redistribution of
 *  this Daqri software constitutes acceptance of these terms.  If you do
 *  not agree with these terms, please do not use, install, modify or
 *  redistribute this Daqri software.
 *
 *  In consideration of your agreement to abide by the following terms, and
 *  subject to these terms, Daqri grants you a personal, non-exclusive
 *  license, under Daqri's copyrights in this original Daqri software (the
 *  "Daqri Software"), to use, reproduce, modify and redistribute the Daqri
 *  Software, with or without modifications, in source and/or binary forms;
 *  provided that if you redistribute the Daqri Software in its entirety and
 *  without modifications, you must retain this notice and the following
 *  text and disclaimers in all such redistributions of the Daqri Software.
 *  Neither the name, trademarks, service marks or logos of Daqri LLC may
 *  be used to endorse or promote products derived from the Daqri Software
 *  without specific prior written permission from Daqri.  Except as
 *  expressly stated in this notice, no other rights or licenses, express or
 *  implied, are granted by Daqri herein, including but not limited to any
 *  patent rights that may be infringed by your derivative works or by other
 *  works in which the Daqri Software may be incorporated.
 *
 *  The Daqri Software is provided by Daqri on an "AS IS" basis.  DAQRI
 *  MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 *  THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE, REGARDING THE DAQRI SOFTWARE OR ITS USE AND
 *  OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 *
 *  IN NO EVENT SHALL DAQRI BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 *  OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 *  MODIFICATION AND/OR DISTRIBUTION OF THE DAQRI SOFTWARE, HOWEVER CAUSED
 *  AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 *  STRICT LIABILITY OR OTHERWISE, EVEN IF DAQRI HAS BEEN ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  Copyright 2015 Daqri LLC. All Rights Reserved.
 *  Copyright 2002-2015 ARToolworks, Inc. All Rights Reserved.
 *
 *  Author(s): Philip Lamb.
 *
 */

// ============================================================================
//	Includes
// ============================================================================

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#  define snprintf _snprintf
#endif
#include <stdlib.h>					// malloc(), free()
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
#include <AR/config.h>
#include <AR/video.h>
#include <AR/param.h>			// arParamDisp()
#include <AR/ar.h>
#include <AR/gsub_lite.h>

// ============================================================================
//	Constants
// ============================================================================

#define VIEW_SCALEFACTOR		1.0         // Units received from ARToolKit tracking will be multiplied by this factor before being used in OpenGL drawing.
#define VIEW_DISTANCE_MIN		40.0        // Objects closer to the camera than this will not be displayed. OpenGL units.
#define VIEW_DISTANCE_MAX		10000.0     // Objects further away from the camera than this will not be displayed. OpenGL units.
#define NUMBER_OF_MARKERS       5

// ============================================================================
//	Global variables
// ============================================================================

// Preferences.
static int windowed = TRUE;                     // Use windowed (TRUE) or fullscreen mode (FALSE) on launch.
static int windowWidth = 640;					// Initial window width, also updated during program execution.
static int windowHeight = 480;                  // Initial window height, also updated during program execution.
static int windowDepth = 32;					// Fullscreen mode bit depth.
static int windowRefresh = 0;					// Fullscreen mode refresh rate. Set to 0 to use default rate.



// Image acquisition.
static ARUint8		                              *gARTImage[NUMBER_OF_MARKERS] = {NULL};
static int                               gARTImageSavePlease[NUMBER_OF_MARKERS] = {FALSE};
// Marker detection.
static ARHandle		                              *gARHandle[NUMBER_OF_MARKERS] = {NULL};
static ARPattHandle	                          *gARPattHandle[NUMBER_OF_MARKERS] = {NULL};
static long			                  gCallCountMarkerDetect[NUMBER_OF_MARKERS] = {0};
static ARParamLT                                  *gCparamLT[NUMBER_OF_MARKERS] = {NULL};
static ARGL_CONTEXT_SETTINGS_REF               gArglSettings[NUMBER_OF_MARKERS] = {NULL};
static int                                       imageNumber[NUMBER_OF_MARKERS] = {0};
static ARdouble                                          err[NUMBER_OF_MARKERS] = {0};
static int                                                 j[NUMBER_OF_MARKERS] = {0};
static int                                                 k[NUMBER_OF_MARKERS] = {0};
static ARdouble                                            p[NUMBER_OF_MARKERS][16];
static ARdouble                                            m[NUMBER_OF_MARKERS][16];
static AR_LABELING_THRESH_MODE                    threshMode[NUMBER_OF_MARKERS];


static int  i;
static int gShowHelp = 1;
static int gShowMode = 1;
static int gDrawRotate = FALSE;
static float gDrawRotateAngle = 0;			// For use in drawing.



// ============================================================================
//	Function prototypes.
// ============================================================================

static void print(const char *text, const float x, const float y, int calculateXFromRightEdge, int calculateYFromTopEdge);
static void drawBackground(const float width, const float height, const float x, const float y);
static void printHelpKeys();
static void printMode();

typedef struct
{
    AR3DHandle *gAR3DHandle;
    char *patt_name;
    int patt_id;
    int gPatt_found;
    double width;
    double center[2];
    double trans[3][4];
}
OBJECT_T;



OBJECT_T  object[NUMBER_OF_MARKERS] = {{ NULL,   "Data/kanji.patt",  FALSE, 0 , 80.0 , {0.0, 0.0}},
                                       { NULL,    "Data/hiro.patt",  FALSE, 0 , 80.0 , {0.0, 0.0}},
                                       { NULL, "Data/sample1.patt",  FALSE, 0 , 80.0 , {0.0, 0.0}},
                                       { NULL, "Data/sample2.patt",  FALSE, 0 , 80.0 , {0.0, 0.0}},
                                       { NULL, "Data/marker16.pat",  FALSE, 0 , 80.0 , {0.0, 0.0}}};

// ============================================================================
//	Functions
// ============================================================================

// Something to look at, draw a rotating colour cube.
static void DrawLine(void)
{
	const GLfloat lineWidth = 100.0;
	glLineWidth(lineWidth);
    glBegin(GL_LINES);
	glColor4ub(0, 220, 255, 120);
	GLfloat x;
	GLfloat lastx = 300.0f, lasty = 135.0f, lastz = 0.0f;
	for (x = -700.0f; x < 300.0f; x += 1.0f)
    {
        glVertex3f(lastx,lasty,lastz);
        lastx = x;
        lasty = 135.0f;
        lastz = 0.0f;
    }
    glEnd();
}
// Something to look at, draw a rotating colour Line.

static void DrawLine1(void)
{
	const GLfloat lineWidth = 100.0;
	glLineWidth(lineWidth);
    glBegin(GL_LINES);
	glColor4ub(0, 220, 255, 120);
	GLfloat x;
	GLfloat lastx=900.0f, lasty=135.0f, lastz=0.0f;
	for (x = -100.0f; x < 900.0f; x += 1.0f)
    {
        glVertex3f(lastx,lasty,lastz);
        lastx = x;
        lasty = 135.0f;
        lastz = 0.0f;
    }
    glEnd();
}

static void DrawLine2(void)
{
	const GLfloat lineWidth = 100.0;
	glLineWidth(lineWidth);
    glBegin(GL_LINES);
	glColor4ub(0, 220, 255, 120);
	GLfloat x;
	GLfloat lastx = 100.0f, lasty = 135.0f, lastz = 0.0f;
	for (x = -900.0f; x < 100.0f; x += 1.0f)
    {
        glVertex3f(lastx,lasty,lastz);
        lastx = x;
        lasty = 135.0f;
        lastz = 0.0f;
    }
    glEnd();
}

static void DrawLine3(void)
{
	const GLfloat lineWidth = 100.0;
	glLineWidth(lineWidth);
    glBegin(GL_LINES);
	glColor4ub(0, 220, 255, 120);
	GLfloat x;
	GLfloat lastx = 500.0f, lasty = 135.0f, lastz = 0.0f;
	for (x = -500.0f; x < 500.0f; x += 1.0f)
    {
        glVertex3f(lastx,lasty,lastz);
        lastx = x;
        lasty = 135.0f;
        lastz = 0.0f;
    }
    glEnd();
}

static void DrawLine4(void)
{
	const GLfloat lineWidth = 100.0;
	glLineWidth(lineWidth);
    glBegin(GL_LINES);
	glColor4ub(0, 220, 255, 120);
	GLfloat x;
	GLfloat lastx = 700.0f, lasty = 135.0f, lastz = 0.0f;
	for (x = -300.0f; x < 700.0f; x += 1.0f)
    {
        glVertex3f(lastx,lasty,lastz);
        lastx = x;
        lasty = 135.0f;
        lastz = 0.0f;
    }
    glEnd();
}


static void DrawArrow1(void)
{
    glBegin(GL_QUADS);
    // Green
    glColor4ub(     0,    255,      0,   255);
    // Top face (y = 1.0f)
    glVertex3f( 20.0f,  20.0f, -20.0f + 50.0f);
    glVertex3f(-20.0f,  20.0f, -20.0f + 50.0f);
    glVertex3f(-20.0f,  20.0f,  20.0f + 50.0f);
    glVertex3f( 20.0f,  20.0f,  20.0f + 50.0f);

    // Orange
    glColor4ub(   122,    255,      0,   255);
    // Bottom face (y = -1.0f)
    glVertex3f( 20.0f, -20.0f,  20.0f + 50.0f);
    glVertex3f(-20.0f, -20.0f,  20.0f + 50.0f);
    glVertex3f(-20.0f, -20.0f, -20.0f + 50.0f);
    glVertex3f( 20.0f, -20.0f, -20.0f + 50.0f);

    // Red
    glColor4ub(   255,      0,      0,   255);
    // Front face  (z = 1.0f)
    glVertex3f( 20.0f,  20.0f,  20.0f + 50.0f);
    glVertex3f(-20.0f,  20.0f,  20.0f + 50.0f);
    glVertex3f(-20.0f, -20.0f,  20.0f + 50.0f);
    glVertex3f( 20.0f, -20.0f,  20.0f + 50.0f);

    // Yellow
    glColor4ub(   255,    122,      0,   255);
    // Back face (z = -1.0f)
    glVertex3f( 20.0f, -20.0f, -20.0f + 50.0f);
    glVertex3f(-20.0f, -20.0f, -20.0f + 50.0f);
    glVertex3f(-20.0f,  20.0f, -20.0f + 50.0f);
    glVertex3f( 20.0f,  20.0f, -20.0f + 50.0f);

    // Blue
    glColor4ub(     0,      0,    255,   255);
    // Left face (x = -1.0f)
    glVertex3f(-20.0f,  20.0f,  20.0f + 50.0f);
    glVertex3f(-20.0f,  20.0f, -20.0f + 50.0f);
    glVertex3f(-20.0f, -20.0f, -20.0f + 50.0f);
    glVertex3f(-20.0f, -20.0f,  20.0f + 50.0f);

    // Magenta
    glColor4ub(   255,      0,    122,   255);
    // Right face (x = 1.0f)
    glVertex3f( 20.0f,  20.0f, -20.0f + 50.0f);
    glVertex3f( 20.0f,  20.0f,  20.0f + 50.0f);
    glVertex3f( 20.0f, -20.0f,  20.0f + 50.0f);
    glVertex3f( 20.0f, -20.0f, -20.0f + 50.0f);

    //glTranslatef(0.0f, 0.0f,    50.0f);
    glEnd();
}

static void DrawArrow2(void)
{
    glBegin(GL_QUADS);

    // Green
    glColor4ub(     0,    255,      0,   255);
    // Top face (y = 1.0f)
    glVertex3f( 40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f( 40.0f,  40.0f,  40.0f + 40.0f);

    // Orange
    glColor4ub(   122,    255,      0,   255);
    // Bottom face (y = -1.0f)
    glVertex3f( 40.0f, -40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f, -40.0f + 40.0f);
    glVertex3f( 40.0f, -40.0f, -40.0f + 40.0f);

    // Red
    glColor4ub(   255,      0,      0,   255);
    // Front face  (z = 1.0f)
    glVertex3f( 40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f,  40.0f + 40.0f);
    glVertex3f( 40.0f, -40.0f,  40.0f + 40.0f);

    // Yellow
    glColor4ub(   255,    122,      0,   255);
    // Back face (z = -1.0f)
    glVertex3f( 40.0f, -40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f( 40.0f,  40.0f, -40.0f + 40.0f);

    // Blue
    glColor4ub(     0,      0,    255,   255);
    // Left face (x = -1.0f)
    glVertex3f(-40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f,  40.0f + 40.0f);

    // Magenta
    glColor4ub(   255,      0,    122,   255);
    // Right face (x = 1.0f)
    glVertex3f( 40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f( 40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f( 40.0f, -40.0f,  40.0f + 40.0f);
    glVertex3f( 40.0f, -40.0f, -40.0f + 40.0f);

    //glTranslatef(0.0f, 0.0f,    50.0f);
    glEnd();
}


static void DrawArrow3(void)
{
    glBegin(GL_QUADS);

    // Green
    glColor4ub(     0,    255,      0,   255);
    // Top face (y = 1.0f)
    glVertex3f( 40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f( 40.0f,  40.0f,  40.0f + 40.0f);

    // Orange
    glColor4ub(   122,    255,      0,   255);
    // Bottom face (y = -1.0f)
    glVertex3f( 40.0f, -40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f, -40.0f + 40.0f);
    glVertex3f( 40.0f, -40.0f, -40.0f + 40.0f);

    // Red
    glColor4ub(   255,      0,      0,   255);
    // Front face  (z = 1.0f)
    glVertex3f( 40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f,  40.0f + 40.0f);
    glVertex3f( 40.0f, -40.0f,  40.0f + 40.0f);

    // Yellow
    glColor4ub(   255,    122,      0,   255);
    // Back face (z = -1.0f)
    glVertex3f( 40.0f, -40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f( 40.0f,  40.0f, -40.0f + 40.0f);

    // Blue
    glColor4ub(     0,      0,    255,   255);
    // Left face (x = -1.0f)
    glVertex3f(-40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f,  40.0f + 40.0f);

    // Magenta
    glColor4ub(   255,      0,    122,   255);
    // Right face (x = 1.0f)
    glVertex3f( 40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f( 40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f( 40.0f, -40.0f,  40.0f + 40.0f);
    glVertex3f( 40.0f, -40.0f, -40.0f + 40.0f);

    //glTranslatef(0.0f, 0.0f,    50.0f);
    glEnd();
}

static void DrawArrow4(void)
{
    glBegin(GL_QUADS);

    // Green
    glColor4ub(     0,    255,      0,   255);
    // Top face (y = 1.0f)
    glVertex3f( 40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f( 40.0f,  40.0f,  40.0f + 40.0f);

    // Orange
    glColor4ub(   122,    255,      0,   255);
    // Bottom face (y = -1.0f)
    glVertex3f( 40.0f, -40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f, -40.0f + 40.0f);
    glVertex3f( 40.0f, -40.0f, -40.0f + 40.0f);

    // Red
    glColor4ub(   255,      0,      0,   255);
    // Front face  (z = 1.0f)
    glVertex3f( 40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f,  40.0f + 40.0f);
    glVertex3f( 40.0f, -40.0f,  40.0f + 40.0f);

    // Yellow
    glColor4ub(   255,    122,      0,   255);
    // Back face (z = -1.0f)
    glVertex3f( 40.0f, -40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f( 40.0f,  40.0f, -40.0f + 40.0f);

    // Blue
    glColor4ub(     0,      0,    255,   255);
    // Left face (x = -1.0f)
    glVertex3f(-40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f,  40.0f + 40.0f);

    // Magenta
    glColor4ub(   255,      0,    122,   255);
    // Right face (x = 1.0f)
    glVertex3f( 40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f( 40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f( 40.0f, -40.0f,  40.0f + 40.0f);
    glVertex3f( 40.0f, -40.0f, -40.0f + 40.0f);

    //glTranslatef(0.0f, 0.0f,    50.0f);
    glEnd();
}

static void DrawArrow5(void)
{
    glBegin(GL_QUADS);

    // Green
    glColor4ub(     0,    255,      0,   255);
    // Top face (y = 1.0f)
    glVertex3f( 40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f( 40.0f,  40.0f,  40.0f + 40.0f);

    // Orange
    glColor4ub(   122,    255,      0,   255);
    // Bottom face (y = -1.0f)
    glVertex3f( 40.0f, -40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f, -40.0f + 40.0f);
    glVertex3f( 40.0f, -40.0f, -40.0f + 40.0f);

    // Red
    glColor4ub(   255,      0,      0,   255);
    // Front face  (z = 1.0f)
    glVertex3f( 40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f,  40.0f + 40.0f);
    glVertex3f( 40.0f, -40.0f,  40.0f + 40.0f);

    // Yellow
    glColor4ub(   255,    122,      0,   255);
    // Back face (z = -1.0f)
    glVertex3f( 40.0f, -40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f( 40.0f,  40.0f, -40.0f + 40.0f);

    // Blue
    glColor4ub(     0,      0,    255,   255);
    // Left face (x = -1.0f)
    glVertex3f(-40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f(-40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f, -40.0f + 40.0f);
    glVertex3f(-40.0f, -40.0f,  40.0f + 40.0f);

    // Magenta
    glColor4ub(   255,      0,    122,   255);
    // Right face (x = 1.0f)
    glVertex3f( 40.0f,  40.0f, -40.0f + 40.0f);
    glVertex3f( 40.0f,  40.0f,  40.0f + 40.0f);
    glVertex3f( 40.0f, -40.0f,  40.0f + 40.0f);
    glVertex3f( 40.0f, -40.0f, -40.0f + 40.0f);

    //glTranslatef(0.0f, 0.0f,    50.0f);
    glEnd();
}























// ============================================================================
//	Functions
// ============================================================================
static int openVideo(char *vconf)
{
    if (arVideoOpen(vconf) < 0)
    {
    	ARLOGe("setupCamera(): Unable to open connection to camera.\n");
    	return (FALSE);
	}
    return (TRUE);
}

static int (capStart(void))
{
    if (arVideoCapStart() != 0)
	{
    	ARLOGe("setupCamera(): Unable to begin camera data capture.\n");
		return (FALSE);
	}
	return (TRUE);
}


static int setupCamera1(const char *cparam_name, ARParamLT *cparamLT_p2[], ARHandle *arhandle2[], AR3DHandle *ar3dhandle2[])
{
    ARParam			cparam;
	int				xsize, ysize;
    AR_PIXEL_FORMAT pixFormat;
    // Open the video path.

    // Find the size of the window.
    if (arVideoGetSize(&xsize, &ysize) < 0)
    {
        ARLOGe("setupCamera(): Unable to determine camera frame size.\n");
        arVideoClose();
        return (FALSE);
    }
    ARLOGi("Camera image size (x,y) = (%d,%d)\n", xsize, ysize);

	// Get the format in which the camera is returning pixels.
	pixFormat = arVideoGetPixelFormat();
	if (pixFormat == AR_PIXEL_FORMAT_INVALID)
	{
    	ARLOGe("setupCamera(): Camera is using unsupported pixel format.\n");
        arVideoClose();
		return (FALSE);
	}

    if (arParamLoad(cparam_name, 1, &cparam) < 0)
    {
        ARLOGe("setupCamera(): Error loading parameter file %s for camera.\n", cparam_name);
        arVideoClose();
        return (FALSE);
    }

    if (cparam.xsize != xsize || cparam.ysize != ysize)
    {
        ARLOGw("*** Camera Parameter resized from %d, %d. ***\n", cparam.xsize, cparam.ysize);
        arParamChangeSize(&cparam, xsize, ysize, &cparam);
    }

#ifdef DEBUG

    ARLOG("*** Camera Parameter ***\n");
    arParamDisp(&cparam);
#endif

    if ((*cparamLT_p2 = arParamLTCreate(&cparam, AR_PARAM_LT_DEFAULT_OFFSET)) == NULL)
    {
        ARLOGe("setupCamera(): Error: arParamLTCreate.\n");
        return (FALSE);
    }

    if ((*arhandle2 = arCreateHandle(*cparamLT_p2)) == NULL)
    {
        ARLOGe("setupCamera(): Error: arCreateHandle.\n");
        return (FALSE);
    }

    if (arSetPixelFormat(*arhandle2, pixFormat) < 0)
    {
        ARLOGe("setupCamera(): Error: arSetPixelFormat.\n");
        return (FALSE);
    }

	if (arSetDebugMode(*arhandle2, AR_DEBUG_DISABLE) < 0)
	{
        ARLOGe("setupCamera(): Error: arSetDebugMode.\n");
        return (FALSE);
    }

    if ((*ar3dhandle2 = ar3DCreateHandle(&cparam)) == NULL)
	{
        ARLOGe("setupCamera(): Error: ar3DCreateHandle.\n");
        return (FALSE);
    }
    return (TRUE);
}

static int setupMarker(const char *patt_name, int *patt_id, ARHandle *arhandle, ARPattHandle **pattHandle_p)
{
    if ((*pattHandle_p = arPattCreateHandle()) == NULL)
    {
        ARLOGe("setupMarker(): Error: arPattCreateHandle.\n");
        return (FALSE);
    }

	if ((*patt_id = arPattLoad(*pattHandle_p, patt_name)) < 0)
	{
		ARLOGe("setupMarker(): Error loading pattern file %s.\n", patt_name);
		arPattDeleteHandle(*pattHandle_p);
		return (FALSE);
	}
    arPattAttach(arhandle, *pattHandle_p);

	return (TRUE);
}

static void cleanup(void)
{
    int i;
    for (i = 0; i < NUMBER_OF_MARKERS; i++)
    {
        arglCleanup(gArglSettings[i]);
        gArglSettings[i] = NULL;
        arPattDetach(gARHandle[i]);
        arPattDeleteHandle(gARPattHandle[i]);
    }
    arVideoCapStop();
    for (i = 0; i < NUMBER_OF_MARKERS; i++)
    {
        ar3DDeleteHandle(&object[i].gAR3DHandle);
        arDeleteHandle(gARHandle[i]);
        arParamLTFree(&gCparamLT[i]);
    }
    arVideoClose();
}



static void Keyboard(unsigned char key, int x, int y)
{
	int mode, threshChange = 0;
    AR_LABELING_THRESH_MODE modea;
	switch (key)
	{
		case 0x1B:						// Quit.
		case 'Q':
		case 'q':
			cleanup();
			exit(0);
			break;
		case ' ':
			gDrawRotate = !gDrawRotate;
			break;
		case 'X':
		case 'x':
		    for (i = 0; i < NUMBER_OF_MARKERS; i++)
            {
                arGetImageProcMode(gARHandle[i], &mode);
            }
            switch (mode)
            {
                case AR_IMAGE_PROC_FRAME_IMAGE:  mode = AR_IMAGE_PROC_FIELD_IMAGE; break;
                case AR_IMAGE_PROC_FIELD_IMAGE:
                default: mode = AR_IMAGE_PROC_FRAME_IMAGE; break;
            }
            for (i = 0; i < NUMBER_OF_MARKERS; i++)
            {
                arSetImageProcMode(gARHandle[i], mode);
            }
			break;
		case 'C':
		case 'c':
            for (i = 0; i < NUMBER_OF_MARKERS; i++)
            {
                ARLOGe("*** Camera - %f (frame/sec)\n", (double)gCallCountMarkerDetect[i]/arUtilTimer());
                gCallCountMarkerDetect[i] = 0;
                arUtilTimerReset();
                break;
			}

		case 'a':
		case 'A':
		    for (i = 0; i < NUMBER_OF_MARKERS; i++)
            {
                arGetLabelingThreshMode(gARHandle[i], &modea);
			}
            switch (modea)
            {
                case AR_LABELING_THRESH_MODE_MANUAL:        modea = AR_LABELING_THRESH_MODE_AUTO_MEDIAN; break;
                case AR_LABELING_THRESH_MODE_AUTO_MEDIAN:   modea = AR_LABELING_THRESH_MODE_AUTO_OTSU; break;
                case AR_LABELING_THRESH_MODE_AUTO_OTSU:     modea = AR_LABELING_THRESH_MODE_AUTO_ADAPTIVE; break;
                case AR_LABELING_THRESH_MODE_AUTO_ADAPTIVE: modea = AR_LABELING_THRESH_MODE_AUTO_BRACKETING; break;
                case AR_LABELING_THRESH_MODE_AUTO_BRACKETING:
                default: modea = AR_LABELING_THRESH_MODE_MANUAL; break;
            }
            for (i = 0; i < NUMBER_OF_MARKERS; i++)
            {
                arSetLabelingThreshMode(gARHandle[i], modea);
            }
			break;
		case '-':
			threshChange = -5;
			break;
		case '+':
		case '=':
			threshChange = +5;
			break;
		case 'D':
		case 'd':
            for (i = 0; i < NUMBER_OF_MARKERS; i++)
            {
                arGetDebugMode(gARHandle[i], &mode);
                arSetDebugMode(gARHandle[i], !mode);
			}
			break;
        case 's':
        case 'S':
            for (i = 0; i < NUMBER_OF_MARKERS; i++)
            {
                if (!gARTImageSavePlease[i]) gARTImageSavePlease[i] = TRUE;
                break;
            }
		case '?':
		case '/':
            gShowHelp++;
            if (gShowHelp > 1) gShowHelp = 0;
			break;
        case 'm':
        case 'M':
            gShowMode = !gShowMode;
            break;
		default:
			break;
	}

	if (threshChange)
	{
		for (i = 0; i < NUMBER_OF_MARKERS; i++)
		{
            int threshhold[i];
            arGetLabelingThresh(gARHandle[i], &threshhold[i]);
            threshhold[i] += threshChange;
            if (threshhold[i] < 0) threshhold[i] = 0;
            if (threshhold[i] > 255) threshhold[i] = 255;
            arSetLabelingThresh(gARHandle[i], threshhold[i]);
		}
	}
}



static void mainLoop0(void)
{
	static int ms_prev;
	int ms;
	float s_elapsed;
	ARUint8 *image;

	// Find out how long since mainLoop() last ran.
	ms = glutGet(GLUT_ELAPSED_TIME);
	s_elapsed = (float)(ms - ms_prev) * 0.001f;
	if (s_elapsed < 0.01f) return; // Don't update more often than 100 Hz.
	ms_prev = ms;

	// Grab a video frame.
	if ((image = arVideoGetImage()) != NULL)
	{
        for (i = 0; i < NUMBER_OF_MARKERS; i++)
        {
            gARTImage[i] = image;	// Save the fetched image.
            if (gARTImageSavePlease[i])
            {
                char imageNumberText[i][15];
                sprintf(imageNumberText[i], "image-%04d.jpg", imageNumber[i]++);
                if (arVideoSaveImageJPEG(gARHandle[i]->xsize, gARHandle[i]->ysize, gARHandle[i]->arPixelFormat, gARTImage[i], imageNumberText[i], 75, 0) < 0)
                {
                    ARLOGe("Error saving video image.\n");
                }
                gARTImageSavePlease[i] = FALSE;
            }
            gCallCountMarkerDetect[i]++;
        }

		for (i = 0; i < NUMBER_OF_MARKERS; i++)
        {
            if (arDetectMarker(gARHandle[i], gARTImage[i]) < 0)
            {
                exit(-1);
            }
            k[i] = -1;
            printf("*********************");
            printf("\n");
            printf("marker_num222 is    ");
            printf("%d  ",gARHandle[i]->marker_num);
            printf("\n");
            printf("*********************");
            printf("\n");
            printf("\n");
            for (j[i] = 0; j[i] < gARHandle[i]->marker_num; j[i]++)
            {
                printf("*********************");
                printf("\n");
                printf("markerInfo222[");
                printf("%d  ",j[i]);
                printf("].id is   ");
                printf("%d  ", gARHandle[i]->markerInfo[j[i]].id);
                printf("\n");
                printf("*********************");
                printf("\n");
                printf("\n");

                if (gARHandle[i]->markerInfo[j[i]].id == object[i].patt_id)
                {
                    if (k[i] == -1)
                    {
                        k[i] = j[i];
                    }
                    else if (gARHandle[i]->markerInfo[j[i]].cf > gARHandle[i]->markerInfo[k[i]].cf)
                    {
                        k[i] = j[i];
                    }
                }
            }
            if (k[i] == 0)
            {
                // Get the transformation between the marker and the real camera into gPatt_trans.
                err[i] = arGetTransMatSquare(object[i].gAR3DHandle, &(gARHandle[i]->markerInfo[0]), object[i].width, object[i].trans);
                object[i].gPatt_found = TRUE;
            }
            else
            {
                object[i].gPatt_found = FALSE;
            }
        }
		glutPostRedisplay();
	}
}


//
//	This function is called on events when the visibility of the
//	GLUT window changes (including when it first becomes visible).
//
static void Visibility(int visible0)
{
	if (visible0 == GLUT_VISIBLE)
	{
		glutIdleFunc(mainLoop0);
	}
	else
	{
		glutIdleFunc(NULL);
	}
}

//
//	This function is called when the
//	GLUT window is resized.
//
static void Reshape(int w, int h)
{
    windowWidth = w;
    windowHeight = h;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);

	// Call through to anyone else who needs to know about window sizing here.
}

//
// This function is called when the window needs redrawing.
//
static void Display(void)
{
    // Select correct buffer for this context.
    glDrawBuffer(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the buffers for new frame.

    for (i = 0; i < NUMBER_OF_MARKERS; i++)
    {
        arglPixelBufferDataUpload(gArglSettings[i], gARTImage[i]);
        arglDispImage(gArglSettings[i]);
        gARTImage[i] = NULL; // Invalidate image data.
        // Projection transformation.
        arglCameraFrustumRH(&(gCparamLT[i]->param), VIEW_DISTANCE_MIN, VIEW_DISTANCE_MAX, p[i]);
    }
    glMatrixMode(GL_PROJECTION);
    for (i = 0; i < NUMBER_OF_MARKERS; i++)
    {
#ifdef ARDOUBLE_IS_FLOAT
        glLoadMatrixf(p[i]);
#else
        glLoadMatrixd(p[i]);
    }
#endif
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    // Viewing transformation.
    glLoadIdentity();
    // Lighting and geometry that moves with the camera should go here.
    // (I.e. must be specified before viewing transformations.)
    //none





    if (object[0].gPatt_found)
    {
        // Calculate the camera position relative to the marker.
        // Replace VIEW_SCALEFACTOR with 1.0 to make one drawing unit equal to 1.0 ARToolKit units (usually millimeters).
        arglCameraViewRH((const ARdouble (*)[4])object[0].trans, m[0], VIEW_SCALEFACTOR);
        int n, h=0;
        //printf("*******\n");
        for(n=0;n<3;n++)
        {
            for(h=0;h<4;h++)
            {
                printf("%.4f  ",object[0].trans[n][h]);
            }
            printf("\n");
        }
#ifdef ARDOUBLE_IS_FLOAT
        glLoadMatrixf(m[0]);
#else
        glLoadMatrixd(m[0]);
#endif
        // All lighting and geometry to be drawn relative to the marker goes here.
        DrawLine();
        DrawArrow1();
    }   // gPatt_found





    if (object[1].gPatt_found)
    {
        // Calculate the camera position relative to the marker.
        // Replace VIEW_SCALEFACTOR with 1.0 to make one drawing unit equal to 1.0 ARToolKit units (usually millimeters).
        arglCameraViewRH((const ARdouble (*)[4])object[1].trans, m[1], VIEW_SCALEFACTOR);
        int n, h = 0;
        //printf("*******\n");
        for(n=0;n<3;n++)
        {
            for(h=0;h<4;h++)
            {
                printf("%.4f  ",object[1].trans[n][h]);
            }
            printf("\n");
        }
#ifdef ARDOUBLE_IS_FLOAT
        glLoadMatrixf(m[1]);
#else
        glLoadMatrixd(m[1]);
#endif
        // All lighting and geometry to be drawn relative to the marker goes here.
        DrawLine1();
        DrawArrow2();
    }


    if (object[2].gPatt_found)
    {
        // Calculate the camera position relative to the marker.
        // Replace VIEW_SCALEFACTOR with 1.0 to make one drawing unit equal to 1.0 ARToolKit units (usually millimeters).
        arglCameraViewRH((const ARdouble (*)[4])object[2].trans, m[2], VIEW_SCALEFACTOR);
        int n, h = 0;
        //printf("*******\n");
        for(n=0;n<3;n++)
        {
            for(h=0;h<4;h++)
            {
                printf("%.4f  ",object[2].trans[n][h]);
            }
            printf("\n");
        }
#ifdef ARDOUBLE_IS_FLOAT
        glLoadMatrixf(m[2]);
#else
        glLoadMatrixd(m[2]);
#endif
        // All lighting and geometry to be drawn relative to the marker goes here.
        DrawArrow2();
        DrawLine2();
    }


    if (object[3].gPatt_found)
    {
        // Calculate the camera position relative to the marker.
        // Replace VIEW_SCALEFACTOR with 1.0 to make one drawing unit equal to 1.0 ARToolKit units (usually millimeters).
        arglCameraViewRH((const ARdouble (*)[4])object[3].trans, m[3], VIEW_SCALEFACTOR);
        int n, h = 0;
        //printf("*******\n");
        for(n=0;n<3;n++)
        {
            for(h=0;h<4;h++)
            {
                printf("%.4f  ",object[3].trans[n][h]);
            }
            printf("\n");
        }
#ifdef ARDOUBLE_IS_FLOAT
        glLoadMatrixf(m[2]);
#else
        glLoadMatrixd(m[3]);
#endif
        // All lighting and geometry to be drawn relative to the marker goes here.
        DrawArrow2();
        DrawLine3();
    }


    if (object[4].gPatt_found)
    {
        // Calculate the camera position relative to the marker.
        // Replace VIEW_SCALEFACTOR with 1.0 to make one drawing unit equal to 1.0 ARToolKit units (usually millimeters).
        arglCameraViewRH((const ARdouble (*)[4])object[4].trans, m[4], VIEW_SCALEFACTOR);
        int n, h = 0;
        //printf("*******\n");
        for(n=0;n<3;n++)
        {
            for(h=0;h<4;h++)
            {
                printf("%.4f  ",object[4].trans[n][h]);
            }
            printf("\n");
        }
#ifdef ARDOUBLE_IS_FLOAT
        glLoadMatrixf(m[2]);
#else
        glLoadMatrixd(m[4]);
#endif
        // All lighting and geometry to be drawn relative to the marker goes here.
        DrawArrow2();
        DrawLine4();
    }





















    // Any 2D overlays go here.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, (GLdouble)windowWidth, 0, (GLdouble)windowHeight, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    // Draw help text and mode.
    //
    if (gShowMode)
    {
        printMode();
    }
    if (gShowHelp)
    {
        if (gShowHelp == 1)
        {
            printHelpKeys();
        }
    }
    glutSwapBuffers();
}




int main(int argc, char** argv)
{
	char glutGamemode[32];
	char cparam_name[] = "Data/camera_para.dat";
	char vconf[] = "-device=LinuxV4L2";
	//char *patt_name[2] = { "Data/kanji.patt","Data/hiro.patt"};

    //
	// Library inits.
	//

	glutInit(&argc, argv);
    if (!openVideo(vconf))
    {
        exit(-1);
    }

    for (i = 0; i < NUMBER_OF_MARKERS; i++)
    {
        if (!setupCamera1(cparam_name, &gCparamLT[i], &gARHandle[i], &object[i].gAR3DHandle))
        {
            ARLOGe("main(): Unable to set up AR camera.\n");
            exit(-1);
        }
	}

    if (!capStart())
    {
        exit(-1);
    }

	// Set up GL context(s) for OpenGL to draw into.
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	if (!windowed)
	{
		if (windowRefresh) sprintf(glutGamemode, "%ix%i:%i@%i", windowWidth, windowHeight, windowDepth, windowRefresh);
		else sprintf(glutGamemode, "%ix%i:%i", windowWidth, windowHeight, windowDepth);
		glutGameModeString(glutGamemode);
		glutEnterGameMode();
	}

	else
	{
		glutInitWindowSize(windowWidth, windowHeight);
		glutCreateWindow(argv[0]);
	}


    for (i = 0; i < NUMBER_OF_MARKERS; i++)
    {
    	if ((gArglSettings[i] = arglSetupForCurrentContext(&(gCparamLT[i]->param), arVideoGetPixelFormat())) == NULL)
        {
            ARLOGe("main(): arglSetupForCurrentContext() returned error.\n");
            cleanup();
            exit(-1);
        }
        arglSetupDebugMode(gArglSettings[i], gARHandle[i]);
        arUtilTimerReset();
    }

	// Setup ARgsub_lite library for current OpenGL context.
    for (i = 0; i < NUMBER_OF_MARKERS; i++)
    {
        if (!setupMarker(object[i].patt_name, &object[i].patt_id, gARHandle[i], &gARPattHandle[i]))
        {
            ARLOGe("main(): Unable to set up AR marker0.\n");
            cleanup();
            exit(-1);
        }
    }

	// Register GLUT event-handling callbacks.
	// NB: mainLoop() is registered by Visibility.
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutVisibilityFunc(Visibility);
	glutKeyboardFunc(Keyboard);
	glutMainLoop();
	return (0);
}

//
// The following functions provide the onscreen help text and mode info.
//

static void print(const char *text, const float x, const float y, int calculateXFromRightEdge, int calculateYFromTopEdge)
{
    int i, len;
    GLfloat x0, y0;

    if (!text) return;

    if (calculateXFromRightEdge)
    {
        x0 = windowWidth - x - (float)glutBitmapLength(GLUT_BITMAP_HELVETICA_10, (const unsigned char *)text);
    }

    else
    {
        x0 = x;
    }

    if (calculateYFromTopEdge)
    {
        y0 = windowHeight - y - 10.0f;
    }

    else
    {
        y0 = y;
    }
    glRasterPos2f(x0, y0);

    len = (int)strlen(text);
    for (i = 0; i < len; i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, text[i]);
}

static void drawBackground(const float width, const float height, const float x, const float y)
{
    GLfloat vertices[4][2];

    vertices[0][0] = x; vertices[0][1] = y;
    vertices[1][0] = width + x; vertices[1][1] = y;
    vertices[2][0] = width + x; vertices[2][1] = height + y;
    vertices[3][0] = x; vertices[3][1] = height + y;
    glLoadIdentity();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glEnableClientState(GL_VERTEX_ARRAY);
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);	// 50% transparent black.
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Opaque white.
    //glLineWidth(1.0f);
    //glDrawArrays(GL_LINE_LOOP, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_BLEND);
}

static void printHelpKeys()
{
    int i;
    GLfloat  w, bw, bh;
    const char *helpText[] =
    {
        "Keys:\n",
        " ? or /        Show/hide this help.",
        " q or [esc]    Quit program.",
        " d             Activate / deactivate debug mode.",
        " m             Toggle display of mode info.",
        " a             Toggle between available threshold modes.",
        " - and +       Switch to manual threshold mode, and adjust threshhold up/down by 5.",
        " x             Change image processing mode.",
        " c             Calulcate frame rate.",
    };
#define helpTextLineCount (sizeof(helpText)/sizeof(char *))

    bw = 0.0f;
    for (i = 0; i < helpTextLineCount; i++)
    {
        w = (float)glutBitmapLength(GLUT_BITMAP_HELVETICA_10, (unsigned char *)helpText[i]);
        if (w > bw) bw = w;
    }
    bh = helpTextLineCount * 10.0f /* character height */+ (helpTextLineCount - 1) * 2.0f /* line spacing */;
    drawBackground(bw, bh, 2.0f, 2.0f);

    for (i = 0; i < helpTextLineCount; i++) print(helpText[i], 2.0f, (helpTextLineCount - 1 - i)*12.0f + 2.0f, 0, 0);;
}

static void printMode()
{
    int len, thresh, line, mode, xsize, ysize;
    ARdouble tempF;
    char text[256], *text_p;

    glColor3ub(255, 255, 255);
    line = 1;

    // Image size and processing mode.
    arVideoGetSize(&xsize, &ysize);
    for (i = 0; i < NUMBER_OF_MARKERS; i++)
    {
        arGetImageProcMode(gARHandle[i], &mode);
    }

	if (mode == AR_IMAGE_PROC_FRAME_IMAGE) text_p = "full frame";
	else text_p = "even field only";
    snprintf(text, sizeof(text), "Processing %dx%d video frames %s", xsize, ysize, text_p);
    print(text, 2.0f,  (line - 1)*12.0f + 2.0f, 0, 1);
    line++;

    // Threshold mode, and threshold, if applicable.
    for (i = 0; i < NUMBER_OF_MARKERS; i++)
    {
        arGetLabelingThreshMode(gARHandle[i], &threshMode[i]);
        switch (threshMode[i])
        {
            case AR_LABELING_THRESH_MODE_MANUAL: text_p = "MANUAL"; break;
            case AR_LABELING_THRESH_MODE_AUTO_MEDIAN: text_p = "AUTO_MEDIAN"; break;
            case AR_LABELING_THRESH_MODE_AUTO_OTSU: text_p = "AUTO_OTSU"; break;
            case AR_LABELING_THRESH_MODE_AUTO_ADAPTIVE: text_p = "AUTO_ADAPTIVE"; break;
            case AR_LABELING_THRESH_MODE_AUTO_BRACKETING: text_p = "AUTO_BRACKETING"; break;
            default: text_p = "UNKNOWN"; break;
        }
    }

    snprintf(text, sizeof(text), "Threshold mode: %s", text_p);




    if (threshMode[0] != AR_LABELING_THRESH_MODE_AUTO_ADAPTIVE
     || threshMode[1] != AR_LABELING_THRESH_MODE_AUTO_ADAPTIVE
     || threshMode[2] != AR_LABELING_THRESH_MODE_AUTO_ADAPTIVE)


    {
        for (i = 0; i < NUMBER_OF_MARKERS; i++)
        {
            arGetLabelingThresh(gARHandle[i], &thresh);
        }
        len = (int)strlen(text);
        snprintf(text + len, sizeof(text) - len, ", thresh=%d", thresh);
    }
    print(text, 2.0f,  (line - 1)*12.0f + 2.0f, 0, 1);
    line++;

    // Border size, image processing mode, pattern detection mode.
    for (i = 0; i < NUMBER_OF_MARKERS; i++)
    {
        arGetBorderSize(gARHandle[i], &tempF);
    }
    snprintf(text, sizeof(text), "Border: %0.1f%%", tempF*100.0);
    for (i = 0; i < NUMBER_OF_MARKERS; i++)
    {
         arGetPatternDetectionMode(gARHandle[i], &mode);
    }
    switch (mode)
    {
        case AR_TEMPLATE_MATCHING_COLOR: text_p = "Colour template (pattern)"; break;
        case AR_TEMPLATE_MATCHING_MONO: text_p = "Mono template (pattern)"; break;
        case AR_MATRIX_CODE_DETECTION: text_p = "Matrix (barcode)"; break;
        case AR_TEMPLATE_MATCHING_COLOR_AND_MATRIX: text_p = "Colour template + Matrix (2 pass, pattern + barcode)"; break;
        case AR_TEMPLATE_MATCHING_MONO_AND_MATRIX: text_p = "Mono template + Matrix (2 pass, pattern + barcode "; break;
        default: text_p = "UNKNOWN"; break;
    }

    len = (int)strlen(text);
    snprintf(text + len, sizeof(text) - len, ", Pattern detection mode: %s", text_p);
    print(text, 2.0f,  (line - 1)*12.0f + 2.0f, 0, 1);
    line++;

    // Window size.
    snprintf(text, sizeof(text), "Drawing into %dx%d window", windowWidth, windowHeight);
    print(text, 2.0f,  (line - 1)*12.0f + 2.0f, 0, 1);
    line++;

}
