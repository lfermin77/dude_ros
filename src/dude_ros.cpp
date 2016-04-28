//ROS
#include "ros/ros.h"
#include "std_msgs/String.h"
#include "nav_msgs/GetMap.h"
#include "nav_msgs/Odometry.h"
#include "visualization_msgs/MarkerArray.h"

//openCV
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <opencv2/imgproc/imgproc.hpp>



//DuDe
#include "wrapper.hpp"
#include "Graph.hpp"








class ROS_handler
{
	ros::NodeHandle n;
	
	image_transport::ImageTransport it_;
	image_transport::Subscriber image_sub_;
	image_transport::Publisher image_pub_;	
	cv_bridge::CvImagePtr cv_ptr;
		
	std::string mapname_;
	ros::Subscriber map_sub_;
	bool saved_map_;
	
	ros::Timer timer;
	
	std::vector<std::vector<cv::Point> > Convex_Marker_;
	ros::Publisher markers_pub_ ;
	nav_msgs::MapMetaData Map_Info_;
	
	ros::Subscriber odom_sub_;
	nav_msgs::Odometry Odom_Info_;
	
	float robot_position_[2];
	cv::Point robot_position_image_;
	std::vector <cv::Point> path_;
	cv::Point position_cm_;
	float distance;
	
	int threshold_;	
	std::vector<std::vector<cv::Point> > contour_vector;
	
	float Decomp_threshold_;
	
	public:
		ROS_handler(const std::string& mapname, float threshold) : mapname_(mapname), saved_map_(false), it_(n), Decomp_threshold_(threshold)
		{


			ROS_INFO("Waiting for the map");
			map_sub_ = n.subscribe("map", 1, &ROS_handler::mapCallback, this);
			ros::Subscriber chatter_sub_ = n.subscribe("chatter", 1000, &ROS_handler::chatterCallback, this);
			odom_sub_ = n.subscribe("pose_corrected", 1, &ROS_handler::odomCallback, this);
			timer = n.createTimer(ros::Duration(0.5), &ROS_handler::metronomeCallback, this);
			image_pub_ = it_.advertise("/image_frontier", 1);
			
			cv_ptr.reset (new cv_bridge::CvImage);
			cv_ptr->encoding = "mono8";

			markers_pub_ = n.advertise<visualization_msgs::Marker>( "skeleton_marker_", 10 );

			Map_Info_.resolution=0.05; //default;
			Map_Info_.width=4000; //default;
			Map_Info_.height=4000; //default;
			
			position_cm_ = cv::Point(0,0); 
			distance=0;
		}

		~ROS_handler()
		{

		}


/////////////////////////////	
// ROS CALLBACKS			
////////////////////////////////		
		void chatterCallback(const std_msgs::String::ConstPtr& msg)
		{
		  ROS_INFO("I heard: [%s]", msg->data.c_str());  
		}
		

		
//////////////////////////////////		
		void mapCallback(const nav_msgs::OccupancyGridConstPtr& map)
		{
			
			cv::Mat grad;
			DuDe_OpenCV_wrapper wrapp;
			wrapp.set_resolution(Map_Info_.resolution);
			wrapp.set_distance_threshold(Decomp_threshold_);
			Graph_Search Graph_searcher;
			
			Map_Info_ = map-> info;						
			std::cout <<"Map_Info_.resolution  " << Map_Info_.resolution << std::endl;

			clock_t begin = clock();
			ROS_INFO("Received a %d X %d map @ %.3f m/pix",
				map->info.width,
				map->info.height,
				map->info.resolution);
			  
			cv_ptr->header = map->header;

	// Occupancy Grid to Image


			//std::cout << "Occ_to_Image..... "; 		double start_occ2im = getTime();
			cv::Mat img(map->info.width, map->info.height, CV_8U);
			//cv::Mat_<char> img(map->info.width, map->info.height);
			img.data = (unsigned char *)(&(map->data[0]) );
//			cv::flip(img,img,0);			
//			cv::Mat reduced_image = auto_resize(img);
			


	//////////////////////////////////////////////////////////
//			cv::flip(Occ_Image,Occ_Image,0);
			cv::Rect resize_rect = wrapp.Decomposer(img);

//			wrapp.measure_performance();
			
			
	/////////////////////////////////////////////////////////		
	//  Graph Search
	/*
			insert_DuDe_Graph(wrapp, Graph_searcher);
			cv::Mat Colored_Frontier = extract_frontier(Occ_Image, wrapp, Graph_searcher);
			
			int start =0;
			//start = find_current_convex(wrapp);
			Graph_searcher.starting_node_= start;                              /////
			Graph_searcher.Graph_Node_List_[start].distance_from_start_ = 0;  //////			

			Graph_searcher.dijkstra_min_dist();
			if (Graph_searcher.frontier_connected_.size()>0){
//				Graph_searcher.print_graph_attributes();				 Graph_searcher.print_frontier_attributes();		//		Graph_searcher.print_frontier_connected();	
				Graph_searcher.frontiers_minima();
				Convex_Marker_.clear();
				Convex_Marker_.push_back( Graph_searcher.Positions_Path);
		//*
				std::vector<cv::Point> proxy;
				proxy.push_back(robot_position_image_);
				Convex_Marker_.push_back(proxy);
		///
			}
			else{
				std::cout<<"All your Nodes are belong to us "<< std::endl;
			}
			//*/
			
	////////////
	//Draw Image
	//*
			cv::Mat Drawing = cv::Mat::zeros(img.size().height, img.size().width, CV_8UC1);	
			for(int i = 0; i <wrapp.Decomposed_contours.size();i++){
				drawContours(Drawing, wrapp.Decomposed_contours, i, i+1, -1, 8);
			}	
			cv::flip(Drawing,Drawing,0);
			for(int i = 0; i <wrapp.Decomposed_contours.size();i++){
				stringstream mix;      mix<<i;				std::string text = mix.str();
				putText(Drawing, text, cv::Point(wrapp.contours_centroid[i].x, img.size().height - wrapp.contours_centroid[i].y ), cv::FONT_HERSHEY_SCRIPT_SIMPLEX, 0.5, wrapp.contours_centroid.size()+1, 1, 8);
			}	
			cv::Mat croppedRef(img, resize_rect);			
			cv::Mat croppedImage;
			croppedRef.copyTo(croppedImage);
			grad = croppedImage;
			//*/
			
	////////////////////////


//			grad = (img>90 & img<=100) ;

	//////////////////////
	/////PUBLISH
			cv_ptr->encoding = "32FC1";
			grad.convertTo(grad, CV_32F);
			grad.copyTo(cv_ptr->image);////most important
			
//			publish_Contour();
	//////////
	/////Time
			clock_t end = clock();
			double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
			std::cout <<"Current Distance  " << distance << std::endl;
			std::cerr<<"Time elapsed  "<< elapsed_secs*1000 << " ms"<<std::endl;
		}

/////////////////////////
		void odomCallback(const nav_msgs::Odometry& msg){		
			robot_position_[0] =  msg.pose.pose.position.x;
			robot_position_[1] =  msg.pose.pose.position.y;
			
			cv::Point temp_Point(100*robot_position_[0], 100*robot_position_[1]);
			if(path_.size()>0) distance += cv::norm(temp_Point - position_cm_);

			path_.push_back(temp_Point);
			position_cm_ = temp_Point;
			/*
			 distance=0;
			for(int i=1; i < path_.size(); i++){
				distance += cv::norm(path_[i] - path_[i-1]);
			}

			// */
//			cout<< "Current distance "<< distance*0.01 << endl;
		}
		
/////////////////		
		void metronomeCallback(const ros::TimerEvent&)
		{
//		  ROS_INFO("tic tac");
		  publish_Image();
//		  publish_Contour();
		}


////////////////////////
// PUBLISHING METHODS		
////////////////////////////		
		void publish_Image(){
			image_pub_.publish(cv_ptr->toImageMsg());
//			cv::imshow("OPENCV_WINDOW", cv_ptr->image);
//			cv::waitKey(3);
		}

////////////////////////////
		void publish_Contour(){

			visualization_msgs::Marker marker;
			
			marker.header.frame_id = "map";
			marker.header.stamp = ros::Time();
			marker.ns = "my_namespace";
			marker.id = 0;
			marker.type = visualization_msgs::Marker::POINTS;
			marker.action = visualization_msgs::Marker::ADD;
			marker.pose.orientation.x = 0.0;
			marker.pose.orientation.y = 0.0;
			marker.pose.orientation.z = 0.0;
			marker.pose.orientation.w = 1.0;
			marker.scale.x = 0.2;
			marker.scale.y = 0.2;
			marker.scale.z = 0.2;
			marker.color.a = 1.0; // Don't forget to set the alpha!
			marker.color.r = 0.0;
			marker.color.g = 0.0;
			marker.color.b = 1.0;
			
			
			for(int i=0;i< Convex_Marker_.size();i++){
				for(int j=0;j< Convex_Marker_[i].size();j++){

					geometry_msgs::Point point;

					point.z = 0;//.1*j;				
					
					point.x = Convex_Marker_[i].at(j).x;
					point.y = Convex_Marker_[i].at(j).y;

					point.x -= Map_Info_.width/2;
					point.y -= Map_Info_.height/2;

					point.x *= Map_Info_.resolution;
					point.y *= -Map_Info_.resolution;

					marker.points.push_back(point);
//					std::cout <<"Points X:  "<< Voronoi_Marker_[i].at(j).x + myROI_.x<<"   Y:  "<< Voronoi_Marker_[i].at(j).y + myROI_.y << std::endl;
//					std::cout <<"Points X:  "<< point.x <<"   Y:  "<< point.y << std::endl;
				}
			}
			geometry_msgs::Point point;	
			markers_pub_.publish(marker);
		}


////////////////////////
///////FRONTIER RELATED
////////////////////////////////
		void insert_DuDe_Graph(DuDe_OpenCV_wrapper  &wrapp, Graph_Search &Graph_searcher){
			Graph_searcher.initialize_Graph(wrapp.contours_centroid.size());
			for(int i=0;i<wrapp.contours_centroid.size();i++){
				Graph_searcher.Graph_Node_List_[i].Node_Position_ = wrapp.contours_centroid[i];
			}
			for(int i=0;i<wrapp.diagonal_centroid.size();i++){			
				int first  = *(wrapp.diagonal_connections[i].begin());
				int second = *(wrapp.diagonal_connections[i].begin()++);
				float distance = cv::norm(wrapp.contours_centroid[first] - wrapp.diagonal_centroid[i] ) + cv::norm(wrapp.contours_centroid[second] - wrapp.diagonal_centroid[i]);
				
				Graph_searcher.insert_edges(wrapp.diagonal_connections[i], distance, i);
			}
		}

//////////////////////////////////
		cv::Mat extract_frontier(cv::Mat Occ_Image, DuDe_OpenCV_wrapper  &wrapp, Graph_Search &Graph_searcher){
			//Occupancy Image to Free Space	
			std::cout << "Extracting Frontier..... ";
			cv::Mat thresholded_image = Occ_Image>210;
		
			cv::Mat black_image = Occ_Image<10;	
			cv::dilate(black_image, black_image, cv::Mat(), cv::Point(-1,-1), 4, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue() );
				
			cv::Mat Median_Image;
			cv::medianBlur(Occ_Image>210, Median_Image, 3);
			cv::Mat Image_in = Median_Image & ~black_image;
			 
			cv::dilate(Median_Image, Median_Image, cv::Mat(), cv::Point(-1,-1), 1, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue() );
			cv::Mat Frontier_Image = Median_Image & (~black_image & ~Image_in);	
			cv::dilate(Frontier_Image, Frontier_Image, cv::Mat(), cv::Point(-1,-1), 3, cv::BORDER_CONSTANT, cv::morphologyDefaultBorderValue() );
			
			std::cout << "done "<< std::endl;
			// Result: Frontier Image and Image_in
				
		////////////////////////////////
		//// Find Contours in Frontiers
			cv::Mat Frontiers;
			cv::Mat Drawing = cv::Mat::zeros(Occ_Image.size().height, Occ_Image.size().width, CV_8UC1);	
			for(int i = 0; i <wrapp.contours_centroid.size();i++){
				drawContours(Drawing, wrapp.Decomposed_contours, i, i+1, -1, 8);
			}						
			
			Drawing.copyTo(Frontiers, Frontier_Image );
			std::vector <std::pair<int, cv::Point > > frontier_Composite; 
		//*
			//frontier_Composite  = 
//			Frontier_enumeration(Frontiers);	
			Frontier_enumeration(Frontiers, frontier_Composite);	
			std::cout << "Inserting Frontier..... ";
			Graph_searcher.initialize_Frontier(frontier_Composite.size() );
			for(int i=0;i<frontier_Composite.size();i++){
				Graph_searcher.Frontier_Node_List_[i].Node_Position_ = frontier_Composite[i].second;
			}
			
//*
			for(int i=0;i<frontier_Composite.size();i++){
				std::set<int> frontier_set;
				frontier_set.insert(frontier_Composite[i].first);
				frontier_set.insert(-(i+1));
				
				Graph_searcher.frontier_connected_.push_back(i);
				float distance = cv::norm(frontier_Composite[i].second - wrapp.contours_centroid[frontier_Composite[i].first]) ;
				Graph_searcher.insert_edges(frontier_set, distance , i);////////
			}

			// */
			std::cout << "done "<< std::endl;
			return Frontiers;
		}

///////////////////////////////////////
		void Frontier_enumeration(cv::Mat colored_frontier_image,  std::vector <std::pair<int, cv::Point > > &  frontier_Composite){
			//int a;
			cv::Mat Frontiers;
			colored_frontier_image.copyTo(Frontiers);
			
			std::vector<std::vector<cv::Point> > Frontier_contour;
			cv::findContours(colored_frontier_image, Frontier_contour, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );
		
			
			
//			std::vector <std::pair<int, cv::Point > > frontier_Composite;
			
			for(int i=0; i <Frontier_contour.size();i++){
				std::set<int> contours_set;
				//Find number of different contours
				for(int j=0; j<Frontier_contour[i].size();j++){
					cv::Point pose = Frontier_contour[i][j];
					int value = Frontiers.at<char>(pose.y, pose.x);
					contours_set.insert(value);			
				}
		//		std::cout << "contours_set size "<<contours_set.size()<<std::endl;
				//Create map between value and position in vector of vectors
				std::vector<std::vector<cv::Point> > avg_vectors;
				avg_vectors.resize(contours_set.size());
				std::map<int,int> mapping_set;
				int counter=0;
				for(std::set<int>::iterator set_it = contours_set.begin(); set_it != contours_set.end();set_it++){
					mapping_set.insert(std::pair<int,int>(*set_it, counter) );
					counter++;
				}
		
				//*
				//Assign to different vectors accordingly
				for(int j=0; j<Frontier_contour[i].size();j++){
					cv::Point pose = Frontier_contour[i][j];
					int value = Frontiers.at<char>(pose.y, pose.x);		
					avg_vectors[ mapping_set[value]  ].push_back(pose);
				}
				//*		
				//Extract Average
				std::set<int>::iterator set_it = contours_set.begin();
				for(int k=0;k<avg_vectors.size();k++){
					cv::Point avg(0,0);
					for(int m=0;m<avg_vectors[k].size();m++){
						avg= avg + avg_vectors[k][m];
					}
					avg=cv::Point(avg.x/avg_vectors[k].size(), avg.y/avg_vectors[k].size() );
					
					std::pair<int, cv::Point > current_pair;
					current_pair.first  = *set_it-1;
					current_pair.second = avg;
					frontier_Composite.push_back(current_pair);
					set_it++;
				}
			}
			
		/*	
			for(int i=0;i<frontier_Composite.size();i++){
				std::cout << "frontier in contour "<<frontier_Composite[i].first << " located at " << frontier_Composite[i].second << std::endl;
			}
			//*/
			
//			return frontier_Composite;
		
		}

//////////////////////////
		int find_current_convex(DuDe_OpenCV_wrapper  &wrapp){
			
			robot_position_image_.x=  robot_position_[0] / Map_Info_.resolution;
			robot_position_image_.y= -robot_position_[1] / Map_Info_.resolution;
/*			
			robot_position_image_.x=  -robot_position_[1] / Map_Info_.resolution;
			robot_position_image_.y=   robot_position_[0] / Map_Info_.resolution;
	//*/		
			robot_position_image_.x += Map_Info_.height/2;
			robot_position_image_.y += Map_Info_.width/2;
			
			int a=0;
			for(int i=0;i<wrapp.Decomposed_contours.size();i++){
				if(cv::pointPolygonTest(wrapp.Decomposed_contours[i], robot_position_image_, true) >= 0){
					a = i;
					std::cout <<"Inside convex number "<< i <<" located in "<<robot_position_image_ << std::endl;
				}
			}
			return a;
		}



/////////////////////////
//UTILITY FUNCTIONS
	//////////////////////////
		cv::Mat auto_resize(cv::Mat img){
			cv::Mat Disposable_Image = img<101;
			std::vector<std::vector<cv::Point> > Explored_contour;
			std::vector<cv::Vec4i> hierarchy; //[Next, Previous, First_Child, Parent]
			cv::findContours(Disposable_Image, Explored_contour, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE );	
			
			cv::Mat croppedRef(img, boundingRect(Explored_contour[0]));			
			cv::Mat croppedImage;
			croppedRef.copyTo(croppedImage);
			return croppedImage;
		}
		
};










int main(int argc, char **argv)
{

  ros::init(argc, argv, "Dual_Decomposer");
  
  std::string mapname = "map";
  
  float decomp_th = 3;
  if (argc ==2){ decomp_th = atof(argv[1]); }


  


	ROS_handler mg(mapname, decomp_th);
//	ros::NodeHandle n;
	

//	ros::Subscriber sub = n.subscribe("chatter", 1000, chatterCallback);
// to create a subscriber, you can do this (as above):
//  ros::Subscriber subPC = n.subscribe<sensor_msgs::PointCloud2> ("camera/depth/points", 1, callback);
  ros::spin();

  return 0;
}
