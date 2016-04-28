//openCV
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <opencv2/imgproc/imgproc.hpp>


// DuDe
#include "dude.h"
#include "dude_use.h"
#include "draw_decoration.h"
#include "SE2d_skeleton.h"
#include "SE2d.h"

#include "Quaternion.h"
#include "polygon.h"
#include "diagonal2.h"
#include "polyline.h"
#include "dude_use.h"
#include "gettime.h"
#include "holediag.h"
#include "dude_param.h"




class DuDe_OpenCV_wrapper{

public:
	
	string g_filename; // filename
	string g_concavity_measure; // concavity_measure name
	double g_alpha, g_beta; // for selecting cutting direction

	double g_tau; //tolerance	
	float Area_threshold;
	float distance_threshold;
	float resolution;
	
	c_dude dude;
	Draw_Decoration draw_decoration;		

	vector<c_polygon*> finalPolygonPieces; //the result polygons, they can be exported as individual files	

	std::vector<cv::Point> Parent_contour;
	
//	Nodes
	std::vector<std::vector<cv::Point> > Decomposed_contours;
	std::vector<cv::Point> contours_centroid;
	std::vector<std::set<int> > contours_connections;

// Edges
	std::vector<cv::Point> diagonal_centroid;
	std::vector<std::set<int> > diagonal_connections;
	
	
	DuDe_OpenCV_wrapper(){
	    Area_threshold=20;
	    distance_threshold = 3; //3 meters default
	    resolution = 0.05; // pixel/meters
	    g_tau= distance_threshold/resolution; // threshold in pixels NOT percentage!
	    dude= c_dude();
	}
	
	~DuDe_OpenCV_wrapper(){
	}
	///////////////////////////////////
	void set_resolution(float resolution_in);
	///////////////////////////////////
	void set_distance_threshold(float distance_threshold_in);
	///////////////////////////////////
//	void set_Tau(float decomp_th ){g_tau=decomp_th;}
	///////////////////////////////////
	void insert_contour_to_poly(std::vector<cv::Point> contour_in, 	c_ply& polygon );
	////////////////////////////////////////
	cv::Rect Decomposer(cv::Mat Occ_Image);
	//////////////////////////////////////////////////////////////
	void Dual_decompose(c_polygon& polygons);
	//////////////////////
	void prepare_skeleton();
	//////////////////////////////
	void export_all_svg_files();
	/////////////////////////////////////
	void estimate_COM_R_Box() ;
	/////////////////////////////
	void extract_contour_from_polygon();
	///////////////////////////////////////
	void extract_graph();	
	////////////////////////////	
	void print_graph();
	////////////////////////////	
	void measure_performance();
/////////////////////////////////////////		
	
		
		
		
		
	














	
};
















