/*
 *  SVG device, (C) 2002 T Jake Luciani, Based on PicTex device, for
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define BEGIN_SUSPEND_INTERRUPTS
#define END_SUSPEND_INTERRUPTS
#include "R.h"
#include "Rmath.h"

#include "Rinternals.h"
#include "Rgraphics.h"
#include "R_ext/GraphicsDevice.h"
#include "R_ext/GraphicsEngine.h"

	/* device-specific information per SVG device */

#define DOTSperIN	72.27
#define in2dots(x) 	(DOTSperIN * x)

typedef struct {
  FILE *texfp;
  char filename[128];
  int pageno;
  int landscape;
  double width;
  double height;
  double pagewidth;
  double pageheight;
  double xlast;
  double ylast;
  double clipleft, clipright, cliptop, clipbottom;
  double clippedx0, clippedy0, clippedx1, clippedy1;

  double cex;
  double srt;
  int lty;
  int lwd;
  int col;
  int fg;
  int bg;
  int fontsize;
  int fontface;
  Rboolean debug;
  Rboolean xmlHeader;
  Rboolean onefile;	/* drop headers etc*/

} SVGDesc;


	/* Global device information */

static double charwidth[4][128] = {
{
  0.5416690, 0.8333360, 0.7777810, 0.6111145, 0.6666690, 0.7083380, 0.7222240,
  0.7777810, 0.7222240, 0.7777810, 0.7222240, 0.5833360, 0.5361130, 0.5361130,
  0.8138910, 0.8138910, 0.2388900, 0.2666680, 0.5000020, 0.5000020, 0.5000020,
  0.5000020, 0.5000020, 0.6666700, 0.4444460, 0.4805580, 0.7222240, 0.7777810,
  0.5000020, 0.8611145, 0.9722260, 0.7777810, 0.2388900, 0.3194460, 0.5000020,
  0.8333360, 0.5000020, 0.8333360, 0.7583360, 0.2777790, 0.3888900, 0.3888900,
  0.5000020, 0.7777810, 0.2777790, 0.3333340, 0.2777790, 0.5000020, 0.5000020,
  0.5000020, 0.5000020, 0.5000020, 0.5000020, 0.5000020, 0.5000020, 0.5000020,
  0.5000020, 0.5000020, 0.2777790, 0.2777790, 0.3194460, 0.7777810, 0.4722240,
  0.4722240, 0.6666690, 0.6666700, 0.6666700, 0.6388910, 0.7222260, 0.5972240,
  0.5694475, 0.6666690, 0.7083380, 0.2777810, 0.4722240, 0.6944480, 0.5416690,
  0.8750050, 0.7083380, 0.7361130, 0.6388910, 0.7361130, 0.6458360, 0.5555570,
  0.6805570, 0.6875050, 0.6666700, 0.9444480, 0.6666700, 0.6666700, 0.6111130,
  0.2888900, 0.5000020, 0.2888900, 0.5000020, 0.2777790, 0.2777790, 0.4805570,
  0.5166680, 0.4444460, 0.5166680, 0.4444460, 0.3055570, 0.5000020, 0.5166680,
  0.2388900, 0.2666680, 0.4888920, 0.2388900, 0.7944470, 0.5166680, 0.5000020,
  0.5166680, 0.5166680, 0.3416690, 0.3833340, 0.3611120, 0.5166680, 0.4611130,
  0.6833360, 0.4611130, 0.4611130, 0.4347230, 0.5000020, 1.0000030, 0.5000020,
  0.5000020, 0.5000020
},
{
  0.5805590, 0.9166720, 0.8555600, 0.6722260, 0.7333370, 0.7944490, 0.7944490,
  0.8555600, 0.7944490, 0.8555600, 0.7944490, 0.6416700, 0.5861150, 0.5861150,
  0.8916720, 0.8916720, 0.2555570, 0.2861130, 0.5500030, 0.5500030, 0.5500030,
  0.5500030, 0.5500030, 0.7333370, 0.4888920, 0.5652800, 0.7944490, 0.8555600,
  0.5500030, 0.9472275, 1.0694500, 0.8555600, 0.2555570, 0.3666690, 0.5583360,
  0.9166720, 0.5500030, 1.0291190, 0.8305610, 0.3055570, 0.4277800, 0.4277800,
  0.5500030, 0.8555600, 0.3055570, 0.3666690, 0.3055570, 0.5500030, 0.5500030,
  0.5500030, 0.5500030, 0.5500030, 0.5500030, 0.5500030, 0.5500030, 0.5500030,
  0.5500030, 0.5500030, 0.3055570, 0.3055570, 0.3666690, 0.8555600, 0.5194470,
  0.5194470, 0.7333370, 0.7333370, 0.7333370, 0.7027820, 0.7944490, 0.6416700,
  0.6111145, 0.7333370, 0.7944490, 0.3305570, 0.5194470, 0.7638930, 0.5805590,
  0.9777830, 0.7944490, 0.7944490, 0.7027820, 0.7944490, 0.7027820, 0.6111145,
  0.7333370, 0.7638930, 0.7333370, 1.0388950, 0.7333370, 0.7333370, 0.6722260,
  0.3430580, 0.5583360, 0.3430580, 0.5500030, 0.3055570, 0.3055570, 0.5250030,
  0.5611140, 0.4888920, 0.5611140, 0.5111140, 0.3361130, 0.5500030, 0.5611140,
  0.2555570, 0.2861130, 0.5305590, 0.2555570, 0.8666720, 0.5611140, 0.5500030,
  0.5611140, 0.5611140, 0.3722250, 0.4216690, 0.4041690, 0.5611140, 0.5000030,
  0.7444490, 0.5000030, 0.5000030, 0.4763920, 0.5500030, 1.1000060, 0.5500030,
  0.5500030, 0.550003 },
{
  0.5416690, 0.8333360, 0.7777810, 0.6111145, 0.6666690, 0.7083380, 0.7222240,
  0.7777810, 0.7222240, 0.7777810, 0.7222240, 0.5833360, 0.5361130, 0.5361130,
  0.8138910, 0.8138910, 0.2388900, 0.2666680, 0.5000020, 0.5000020, 0.5000020,
  0.5000020, 0.5000020, 0.7375210, 0.4444460, 0.4805580, 0.7222240, 0.7777810,
  0.5000020, 0.8611145, 0.9722260, 0.7777810, 0.2388900, 0.3194460, 0.5000020,
  0.8333360, 0.5000020, 0.8333360, 0.7583360, 0.2777790, 0.3888900, 0.3888900,
  0.5000020, 0.7777810, 0.2777790, 0.3333340, 0.2777790, 0.5000020, 0.5000020,
  0.5000020, 0.5000020, 0.5000020, 0.5000020, 0.5000020, 0.5000020, 0.5000020,
  0.5000020, 0.5000020, 0.2777790, 0.2777790, 0.3194460, 0.7777810, 0.4722240,
  0.4722240, 0.6666690, 0.6666700, 0.6666700, 0.6388910, 0.7222260, 0.5972240,
  0.5694475, 0.6666690, 0.7083380, 0.2777810, 0.4722240, 0.6944480, 0.5416690,
  0.8750050, 0.7083380, 0.7361130, 0.6388910, 0.7361130, 0.6458360, 0.5555570,
  0.6805570, 0.6875050, 0.6666700, 0.9444480, 0.6666700, 0.6666700, 0.6111130,
  0.2888900, 0.5000020, 0.2888900, 0.5000020, 0.2777790, 0.2777790, 0.4805570,
  0.5166680, 0.4444460, 0.5166680, 0.4444460, 0.3055570, 0.5000020, 0.5166680,
  0.2388900, 0.2666680, 0.4888920, 0.2388900, 0.7944470, 0.5166680, 0.5000020,
  0.5166680, 0.5166680, 0.3416690, 0.3833340, 0.3611120, 0.5166680, 0.4611130,
  0.6833360, 0.4611130, 0.4611130, 0.4347230, 0.5000020, 1.0000030, 0.5000020,
  0.5000020, 0.5000020 },
{
  0.5805590, 0.9166720, 0.8555600, 0.6722260, 0.7333370, 0.7944490, 0.7944490,
  0.8555600, 0.7944490, 0.8555600, 0.7944490, 0.6416700, 0.5861150, 0.5861150,
  0.8916720, 0.8916720, 0.2555570, 0.2861130, 0.5500030, 0.5500030, 0.5500030,
  0.5500030, 0.5500030, 0.8002530, 0.4888920, 0.5652800, 0.7944490, 0.8555600,
  0.5500030, 0.9472275, 1.0694500, 0.8555600, 0.2555570, 0.3666690, 0.5583360,
  0.9166720, 0.5500030, 1.0291190, 0.8305610, 0.3055570, 0.4277800, 0.4277800,
  0.5500030, 0.8555600, 0.3055570, 0.3666690, 0.3055570, 0.5500030, 0.5500030,
  0.5500030, 0.5500030, 0.5500030, 0.5500030, 0.5500030, 0.5500030, 0.5500030,
  0.5500030, 0.5500030, 0.3055570, 0.3055570, 0.3666690, 0.8555600, 0.5194470,
  0.5194470, 0.7333370, 0.7333370, 0.7333370, 0.7027820, 0.7944490, 0.6416700,
  0.6111145, 0.7333370, 0.7944490, 0.3305570, 0.5194470, 0.7638930, 0.5805590,
  0.9777830, 0.7944490, 0.7944490, 0.7027820, 0.7944490, 0.7027820, 0.6111145,
  0.7333370, 0.7638930, 0.7333370, 1.0388950, 0.7333370, 0.7333370, 0.6722260,
  0.3430580, 0.5583360, 0.3430580, 0.5500030, 0.3055570, 0.3055570, 0.5250030,
  0.5611140, 0.4888920, 0.5611140, 0.5111140, 0.3361130, 0.5500030, 0.5611140,
  0.2555570, 0.2861130, 0.5305590, 0.2555570, 0.8666720, 0.5611140, 0.5500030,
  0.5611140, 0.5611140, 0.3722250, 0.4216690, 0.4041690, 0.5611140, 0.5000030,
  0.7444490, 0.5000030, 0.5000030, 0.4763920, 0.5500030, 1.1000060, 0.5500030,
  0.5500030, 0.550003
}
};

/* Device driver actions */

static void   SVG_Activate(NewDevDesc *);
static void   SVG_Circle(double x, double y, double r,
			 int col, int fill, double gamma, int lty, double lwd, 
			 NewDevDesc *dd);
static void   SVG_Clip(double, double, double, double, NewDevDesc*);
static void   SVG_Close(NewDevDesc*);
static void   SVG_Deactivate(NewDevDesc *);
static void   SVG_Hold(NewDevDesc*);
static void   SVG_Line(double x1, double y1, double x2, double y2,
		       int col, double gamma, int lty, double lwd,
		       NewDevDesc *dd);
static Rboolean SVG_Locator(double*, double*, NewDevDesc*);
static void   SVG_Mode(int, NewDevDesc*);
static void   SVG_NewPage(int fill, double gamma, NewDevDesc *dd);
static Rboolean SVG_Open(NewDevDesc*, SVGDesc*);
static void   SVG_Polygon(int n, double *x, double *y, 
			  int col, int fill, double gamma, int lty, double lwd,
			  NewDevDesc *dd);
static void   SVG_Polyline(int n, double *x, double *y, 
			   int col, double gamma, int lty, double lwd,
			   NewDevDesc *dd);
static void   SVG_Rect(double x0, double y0, double x1, double y1,
		       int col, int fill, double gamma, int lty, double lwd,
		       NewDevDesc *dd);

static void   SVG_Resize(double *left, double *right,
			 double *bottom, double *top,
			 NewDevDesc *dd);
static double SVG_StrWidth(char *str, int font,
			   double cex, double ps, NewDevDesc *dd);
static void   SVG_Text(double x, double y, char *str, 
		       double rot, double hadj, 
		       int col, double gamma, int font, double cex, double ps,
		       NewDevDesc *dd);
static void   SVG_MetricInfo(int c, int font, double cex, double ps,
			     double* ascent, double* descent,
			     double* width, NewDevDesc *dd);

/* Support routines */

static char MyColBuf[8];
static char HexDigits[] = "0123456789ABCDEF";

char *col2RGBname(unsigned int col)
{
  MyColBuf[0] = '#';
  MyColBuf[1] = HexDigits[(col >>  4) & 15];
  MyColBuf[2] = HexDigits[(col	    ) & 15];
  MyColBuf[3] = HexDigits[(col >> 12) & 15];
  MyColBuf[4] = HexDigits[(col >>  8) & 15];
  MyColBuf[5] = HexDigits[(col >> 20) & 15];
  MyColBuf[6] = HexDigits[(col >> 16) & 15];
  MyColBuf[7] = '\0';
  return &MyColBuf[0];
}

/*Get Device point from user point*/
/*void GetSvgDevicePoint(double x, double y)
 {
   GEDevDesc *dd = GEcurrentDevice();

   x = toDeviceX(x,
   GConvert(x, y, GE_NDC, GE_DEVICE, dd);
   }
*/ 
/*Get User Point from device point*/
/*void GetSvgUserPoint(double x, double y)
{
  DevDesc *dd = GEcurrentDevice();
  x = fromDeviceX(x
  GConvert(x, y, GE_DEVICE, GE_NDC, dd);
}
*/
/*Get Device points for an array of user points*/
/*void GetSvgDevicePoints(double *x, double *y, int *n)
{
  int i;
  
  GEDevDesc *dd = GEcurrentDevice();

  for(i = 0; i < *n; i++){
    GConvert(x+i,y+i, GE_NDC, DEVICE, dd);
  }
}
*/  
/*Get Device Boundries i.e. Width & Height*/
  /*void GetSvgDeviceBoundry(double *w, double *h)
{
  DevDesc *dd = CurrentDevice();
  SVGDesc *ptd = (SVGDesc *) dd->deviceSpecific;

  *w = in2dots(ptd->width);
  *h = in2dots(ptd->height);
}
*/
static void SetLinetype(int newlty, int newlwd, NewDevDesc *dd, int
			fgcol, int col)
{
  SVGDesc *ptd = (SVGDesc *) dd->deviceSpecific;
  
  int code;
  ptd->lty = newlty;
  ptd->lwd = newlwd;

  /* code is set as follows */
  /* code == 0, nothing to draw */
  /* code == 1, outline only */
  /* code == 2, fill only */
  /* code == 3, outline and fill */
  
  code = 2 * (R_ALPHA(fgcol) == 0) + (R_ALPHA(col) == 0);

  /*Set line size + color*/
  fprintf(ptd->texfp,"style=\"stroke-width:%d;",newlwd);
  

  /*  fprintf(ptd->texfp,"stroke-width:%d;stroke:%s",
  	  newlwd,col2RGBname(col));
  */
  if(code == 0){
    
    fprintf(ptd->texfp,"stroke:%s","none");
    fprintf(ptd->texfp,";fill:%s","none");
  }else if( code == 1){
    fprintf(ptd->texfp,"stroke:%s",col2RGBname(col));
    fprintf(ptd->texfp,";fill:%s","none");
  }else if( code == 2){
    fprintf(ptd->texfp,"stroke:%s",col2RGBname(fgcol));
    fprintf(ptd->texfp,";fill:%s",col2RGBname(fgcol));
  }else if(code == 3){
    fprintf(ptd->texfp,"stroke:%s",col2RGBname(col));
    fprintf(ptd->texfp,";fill:%s",col2RGBname(fgcol));
  }
  
 
  /*Set fill color
  fprintf(ptd->texfp,";fill:");
  if(fgcol != NA_INTEGER){
    fprintf(ptd->texfp,"%s",col2RGBname(fgcol));
  } else {
    fprintf(ptd->texfp,"none");
  }
  */
  /*Set line pattern type*/
  if (ptd->lty) {
    /*Need to support other types of lines*/
    switch(ptd->lty){
    default:
      fprintf(ptd->texfp,";stroke-dasharray");
      break;
    }
    
  }
  
  fprintf(ptd->texfp,"\"");
}

    
static void SetFont(int face, int size, SVGDesc *ptd)
{
    int lface=face, lsize= size;
    if(lface < 1 || lface > 4) lface = 1;
    if(lsize < 1 || lsize > 24) lsize = 10;
    
    fprintf(ptd->texfp, " style=\"font-size:%d\" ",
      lsize);
    ptd->fontsize = lsize;
    ptd->fontface = lface;
    
}

static void SVG_Activate(NewDevDesc *dd)
{
}

static void SVG_Deactivate(NewDevDesc *dd)
{
}

static void SVG_MetricInfo(int c, int font, double cex, double ps,
			   double* ascent, double* descent,
			   double* width, NewDevDesc *dd)
{
  /* metric information not available => return 0,0,0 */
  *ascent  = 0.0;
  *descent = 0.0;
  *width   = 0.0;
}

/* Initialize the device */

static Rboolean SVG_Open(NewDevDesc *dd, SVGDesc *ptd)
{
  ptd->fontsize = 0;
  ptd->fontface = 0;
  ptd->debug = FALSE;
  
  ptd->fg = dd->startcol;
  ptd->bg = dd->startfill;
  ptd->col = ptd->fg;
  
  if (!((int)(ptd->texfp) = R_fopen(R_ExpandFileName(ptd->filename), "w")))
    return FALSE;
  
  if(ptd->xmlHeader)
    fprintf(ptd->texfp,"<?xml version=\"1.0\" standalone=\"yes\"?>\n");
  
  fprintf(ptd->texfp,"<svg width=\"%.2f\" height=\"%.2f\" ",
	  in2dots(ptd->width), in2dots(ptd->height));
  fprintf(ptd->texfp,"viewBox=\"0,0,%.2f,%.2f\">\n",
	  in2dots(ptd->width), in2dots(ptd->height));
  
  fprintf(ptd->texfp,"<desc>R SVG Plot!</desc>\n");
  
  fprintf(ptd->texfp,
	  "<rect width=\"100%%\" height=\"100%%\" style=\"fill:%s\"/>\n",
	  col2RGBname(ptd->bg));
  
  /* ensure that line drawing is set up at the first */
  /* graphics call */
  ptd->lty = -1;
  ptd->lwd = -1;
  
  ptd->pageno++;
  return TRUE;
}


/* Interactive Resize */

static void SVG_Resize(double *left, double *right,
		       double *bottom, double *top,
		       NewDevDesc *dd)
{
}

static void SVG_Clip(double x0, double x1, double y0, double y1,
		     NewDevDesc *dd)
{
    SVGDesc *ptd = (SVGDesc *) dd->deviceSpecific;

    ptd->clipleft = x0;
    ptd->clipright = x1;
    ptd->clipbottom = y0;
    ptd->cliptop = y1;
}

	/* Start a new page */

static void SVG_NewPage(int fill, double gamma, NewDevDesc *dd)
{
    SVGDesc *ptd = (SVGDesc *) dd->deviceSpecific;

    int face, size;
    
    if(ptd->onefile){
    
    }else if (ptd->pageno) {

      fprintf(ptd->texfp,"</svg>\n");
      if(ptd->xmlHeader)
	fprintf(ptd->texfp,"<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n");
      fprintf(ptd->texfp,"<svg width=\"%.2f\" height=\"%.2f\" ",
	    in2dots(ptd->width), in2dots(ptd->height));
      fprintf(ptd->texfp,"viewBox=\"0,0,%.2f,%.2f\">\n",
	    in2dots(ptd->width), in2dots(ptd->height));
      fprintf(ptd->texfp,"<desc>R SVG Plot!</desc>\n");
      ptd->pageno++;
      
    }else ptd->pageno++;

    face = ptd->fontface;
    size = ptd->fontsize;
    ptd->fontface = 0;
    ptd->fontsize = 0;
    /*SetFont(face, size, ptd);*/
}

/* Close down the driver */

static void SVG_Close(NewDevDesc *dd)
{
	SVGDesc *ptd = (SVGDesc *) dd->deviceSpecific;

	fprintf(ptd->texfp, "</svg>\n");
	
	fclose(ptd->texfp);

	free(ptd);
}


static void SVG_Line(double x1, double y1, double x2, double y2,
		     int col, double gamma, int lty, double lwd,
		     NewDevDesc *dd)
{
  SVGDesc *ptd = (SVGDesc *) dd->deviceSpecific;
     
  fprintf(ptd->texfp, "<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" ",
	  x1, y1, x2);
  
  fprintf(ptd->texfp, "y2=\"%.2f\" ",
	    y2);
  
  SetLinetype(lty, lwd, dd, NA_INTEGER,col);	
  fprintf(ptd->texfp, "/>\n");
}

static void SVG_Polyline(int n, double *x, double *y, 
			 int col, double gamma, int lty, double lwd,
			 NewDevDesc *dd)
{
  int i;
  SVGDesc *ptd = (SVGDesc *) dd->deviceSpecific;
  fprintf(ptd->texfp,"<polyline points=\""); 
  
  for (i=0; i<n; i++) {
    fprintf(ptd->texfp,"%.2f , %.2f ",x[i],y[i]);
  }
  fprintf(ptd->texfp,"\" ");
    
  SetLinetype(lty, lwd, dd,NA_INTEGER,col);
  
  fprintf(ptd->texfp, "/>\n");
}

/* String Width in Rasters */
/* For the current font in pointsize fontsize */

static double SVG_StrWidth(char *str, int font,
			   double cex, double ps, NewDevDesc *dd)
{
    SVGDesc *ptd = (SVGDesc *) dd->deviceSpecific;

    char *p;
    int size;
    double sum;
    size =  cex * ps + 0.5;
    /*SetFont(font, size, ptd);*/
    sum = 0;
    for(p=str ; *p ; p++)
      sum += charwidth[ptd->fontface][(int)*p];

    return sum * size;
}


/* Possibly Filled Rectangle */
static void SVG_Rect(double x0, double y0, double x1, double y1,
		     int col, int fill, double gamma, int lty, double lwd,
		     NewDevDesc *dd)
{
  double tmp;
  SVGDesc *ptd = (SVGDesc *) dd->deviceSpecific;
  
  /*Make sure width and height are positive*/
  if(x0 >= x1){
    tmp = x0;
    x0 = x1;
    x1 = tmp;
  }

  if(y0 >= y1){
    tmp = y0;
    y0 = y1;
    y1 = tmp;
  }
  
  fprintf(ptd->texfp,
	  "<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" ",
	  x0,y0,x1-x0,y1-y0);


  SetLinetype(lty, lwd, dd, fill, col);
  fprintf(ptd->texfp," />\n");
}

static void SVG_Circle(double x, double y, double r,
		       int col, int fill, double gamma, int lty, double lwd,
		       NewDevDesc *dd)
{
  SVGDesc *ptd = (SVGDesc *) dd->deviceSpecific;
  
  
  fprintf(ptd->texfp,
	  "<circle cx=\"%.2f\" cy=\"%.2f\" r=\"%.2f\" ",
	  x,y,r*1.5);
  
  SetLinetype(lty, lwd, dd, fill, col);
  
  fprintf(ptd->texfp," />\n");

}

static void SVG_Polygon(int n, double *x, double *y, 
			int col, int fill, double gamma, int lty, double lwd,
			NewDevDesc *dd)
{
  int i;
  
  SVGDesc *ptd = (SVGDesc *) dd->deviceSpecific;


  fprintf(ptd->texfp,"<polygon points=\""); 
  
  for (i=0; i<n; i++) {
    fprintf(ptd->texfp,"%.2f , %.2f ",x[i],y[i]);
  }

  fprintf(ptd->texfp,"\" ");

  SetLinetype(lty, lwd, dd, fill, col);

  fprintf(ptd->texfp," />\n");
}

static void textext(char *str, SVGDesc *ptd)
{

    for( ; *str ; str++)
        switch(*str) {


        default:
            fputc(*str, ptd->texfp);
            break;
        }

}


/* Rotated Text */

static void SVG_Text(double x, double y, char *str, 
		     double rot, double hadj, 
		     int col, double gamma, int font, double cex, double ps,
		     NewDevDesc *dd)
{
  int size;

  SVGDesc *ptd = (SVGDesc *) dd->deviceSpecific;
  
  size = cex * ps + 0.5;
  
  fprintf(ptd->texfp,"<text transform=\"translate(%.2f,%.2f) ",x,y);
  if(rot != 0)
    fprintf(ptd->texfp," rotate(%0.0f)\" ",-1.0*rot);
  else
    fprintf(ptd->texfp,"\" ");
  
  SetFont(font, size, ptd);
  
  fprintf(ptd->texfp,">");

  textext(str, ptd);
  
  fprintf(ptd->texfp,"</text>\n");
}

/* Pick */
static Rboolean SVG_Locator(double *x, double *y, NewDevDesc *dd)
{
    return FALSE;
}


/* Set Graphics mode - not needed for PS */
static void SVG_Mode(int mode, NewDevDesc* dd)
{
}

/* GraphicsInteraction() for the Mac */
static void SVG_Hold(NewDevDesc *dd)
{
}

Rboolean SVGDeviceDriver(NewDevDesc *dd, char *filename, char *bg, char *fg,
			 double width, double height, Rboolean debug, 
			 Rboolean xmlHeader, Rboolean onefile)
{
    SVGDesc *ptd;

    if (!(ptd = (SVGDesc *) malloc(sizeof(SVGDesc))))
	return FALSE;

    strcpy(ptd->filename, filename);
    
    dd->startfill = Rf_str2col(bg);
    dd->startcol = Rf_str2col(fg);
    dd->startps = 10;
    dd->startlty = 0;
    dd->startfont = 1;
    dd->startgamma = 1;

    dd->newDevStruct = 1;
    
    dd->activate = SVG_Activate;
    dd->deactivate = SVG_Deactivate;
    dd->open = SVG_Open;
    dd->close = SVG_Close;
    dd->clip = SVG_Clip;
    dd->size = SVG_Resize;
    dd->newPage = SVG_NewPage;
    dd->line = SVG_Line;
    dd->text = SVG_Text;
    dd->strWidth = SVG_StrWidth;
    dd->rect = SVG_Rect;
    dd->circle = SVG_Circle;
    dd->polygon = SVG_Polygon;
    dd->polyline = SVG_Polyline;
    dd->locator = SVG_Locator;
    dd->mode = SVG_Mode;
    dd->hold = SVG_Hold;
    dd->metricInfo = SVG_MetricInfo;

    /* Screen Dimensions in Pixels */

    dd->left = 0;		/* left */
    dd->right = in2dots(width);/* right */
    dd->bottom = in2dots(height);		/* bottom */
    dd->top = 0;  /* top */
    ptd->width = width;
    ptd->height = height;
    ptd->xmlHeader = xmlHeader;
    ptd->onefile   = onefile;

    if( ! SVG_Open(dd, ptd) ) 
	return FALSE;

    /* Base Pointsize */
    /* Nominal Character Sizes in Pixels */

    dd->cra[0] =	 (6.0/12.0) * 10.0;
    dd->cra[1] =	(10.0/12.0) * 10.0;

    /* Character Addressing Offsets */
    /* These offsets should center a single */
    /* plotting character over the plotting point. */
    /* Pure guesswork and eyeballing ... */

    dd->xCharOffset =  0; /*0.4900;*/
    dd->yCharOffset =  0; /*0.3333;*/
    dd->yLineBias = 0; /*0.1;*/

    /* Inches per Raster Unit */
    /* We use printer points, i.e. 72.27 dots per inch : */
    dd->ipr[0] = dd->ipr[1] = 1./DOTSperIN;

    dd->canResizePlot = FALSE;
    dd->canChangeFont = TRUE;
    dd->canRotateText = TRUE;
    dd->canResizeText = TRUE;
    dd->canClip = FALSE;
    dd->canHAdj = 0;
    dd->canChangeGamma = FALSE;

    ptd->lty = 1;
    ptd->pageno = 0;
    ptd->debug = debug;
    
    dd->deviceSpecific = (void *) ptd;
    dd->displayListOn = FALSE;
    return TRUE;
}

static  GEDevDesc *RSvgDevice(char **file, char **bg, char **fg,
			      double *width, double *height, int *debug, 
			      int *xmlHeader, int *onefile)
{
    GEDevDesc *dd;
    NewDevDesc *dev;

    if(debug[0] == NA_LOGICAL) debug = FALSE;

    R_CheckDeviceAvailable();
    BEGIN_SUSPEND_INTERRUPTS {
	if (!(dev = (NewDevDesc *) Calloc(1,NewDevDesc)))
	    return;
	/* Do this for early redraw attempts */
	dev->displayList = R_NilValue;
	
	if(!SVGDeviceDriver(dev, file[0], bg[0], fg[0], width[0], height[0], debug[0],xmlHeader[0],onefile[0])) {
	    free(dev);
	    error("unable to start device SVG");
	}
	gsetVar(install(".Device"), mkString("devSVG"), R_NilValue);
	dd = GEcreateDevDesc(dev);
        dd->newDevStruct = 1;
	Rf_addDevice((DevDesc*) dd);
	GEinitDisplayList(dd);
	
    } END_SUSPEND_INTERRUPTS;
    
    return(dd);
}

void do_SVG(char **file, char **bg, char **fg, double *width, double *height, 
	    int *debug, int *xmlHeader, int *onefile)
{
  char *vmax;

  vmax = vmaxget();
  
  RSvgDevice(file, bg, fg, width, height, debug, xmlHeader, onefile);
  
  vmaxset(vmax);
}


