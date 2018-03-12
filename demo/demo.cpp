/**
* File: Demo.cpp
* Date: November 2011
* Author: Dorian Galvez-Lopez
* Description: demo application of DBoW2
* License: see the LICENSE.txt file
*/

#include <iostream>
#include <vector>

#include <string>
// DBoW2
#include "DBoW2.h" // defines OrbVocabulary and OrbDatabase

#include <DUtils/DUtils.h>
#include <DVision/DVision.h>

// OpenCV
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/opencv.hpp>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <map>
#include <MapFrame.h>
#include <Base64.h>
#include <thread>

using namespace DBoW2;
using namespace DUtils;
using namespace cv;
using namespace std;
using namespace LS;
#include "ws/server_ws.hpp"
#include "ws/crypto.hpp"

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string base64_decode(std::string const& encoded_string);
static inline bool is_base64(unsigned char c);
void changeStructure(const cv::Mat &plain, vector<cv::Mat> &out);
void testVocCreation(const vector<vector<cv::Mat > > &features, string vocname);
Mat loadDescriptors(vector<cv::Mat >  &features,  vector<KeyPoint> &keypoints, Mat image);
void showMatches( Mat img1,  std::vector<KeyPoint>& keypoints1,
                  Mat img2,  std::vector<KeyPoint>& keypoints2);
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string decode64(const std::string &val);
void createDatabase(OrbDatabase &db, const vector<cv::Mat > &features) ;
int queryDatabase(OrbDatabase &db, const vector<cv::Mat >  &features) ;

void loadFeatures(ifstream &infile, vector<vector<Mat >>  &features, string datasetPath, int nimages,std::map<int, MapFrame> &mapImageEntry);

Mat orbDescriptors(Mat image,vector<cv::KeyPoint> &keypoints,vector<vector<cv::Mat > > &features);
// number of training images
const int NIMAGES = 1508;
const int TESTNIMAGES = 1189;
std::string string_to_hex(const std::string& input);

void loadFeaturesFromBase64(ifstream &dataSetBase64,vector< vector<cv::Mat >> &featuresDataBase64);

void parseDataSet(ifstream &dataSetBase64,vector< vector<cv::Mat >> &featuresDataBase64,std::map<int, MapFrame> &mapImageEntry);
void previoustest(ifstream &dataSet,string str);
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void wait()
{
  cout << endl << "Press enter to continue" << endl;
  getchar();
}
// ----------------------------------------------------------------------------
static  std::map<int, MapFrame> mapImageDataSet;
static int currentImageId=-1,lastImageId=-1;
static OrbDatabase db2("ls_db_voc_2_3.yml.gz");
int main(void)
{
  std::map<int, MapFrame> mapImageTestSet;
  std::ifstream dataSetPoints("../demo/dataset/pointcloudtest.txt");
  std::ifstream testSetPoints("../demo/dataset/16-01-2018/pointcloudtest.txt");
  vector<vector<cv::Mat > > featuresDataSet,featuresTestSet;
  string dataSetImages("../demo/dataset/16-01-2018/");
  string testSetImages("../demo/dataset/16-01-2018/");
  string jpg(".jpg");
  Mat imgTest,imgData,mat1;

  // load the vocabulary from disk
// CREATE

//    featuresDataSet.reserve(NIMAGES);
//    cout << "Loading Features" << endl;


    parseDataSet(dataSetPoints, featuresDataSet, mapImageDataSet);
    cout << "Creating Vocabulary" <<featuresDataSet.size()<< endl;
//    testVocCreation(featuresDataSet,"ls_voc_2_3.yml.gz");
//
//    OrbVocabulary voc("ls_voc_2_3.yml.gz");    cout << "Creating Database" << endl;
//
//    OrbDatabase db(voc,false,0);
//    cout << "Creating Database" << endl;
//    cout << "Inserting Items in Database" << endl;
//    for(int i = 0;i< featuresDataSet.size();i++){
//        db.add(featuresDataSet[i]);
//    }
//
//    db.save("ls_db_voc_2_3.yml.gz");

//    for(int i = 0;i< featuresDataSet.size();i++){
//        queryDatabase(db, featuresDataSet[i]);
//    }
//    cout << "Saving Database" << endl;

  cout << "Test Completed" << endl;

  cout << endl;
    cout << "***************************************" << endl;

    WsServer server;
    server.config.address = "192.168.96.240";
    server.config.port = 8000;
    auto &echo = server.endpoint["/"];

    echo.on_message = [](shared_ptr<WsServer::Connection> connection, shared_ptr<WsServer::Message> message) {

//        cout << "ON_MESSAGE "<<" Size: " << message->size()<< endl;
        auto message_str = message->string();
        string ts,parsedString;
//        cout << "Server: Message received: \"" << message_str << "\" from " << connection.get() << endl;

//        cout << "Server: Sending message \"" << message_str << "\" to " << connection.get() << endl;
//        cout <<"ON_MESSAGE "<< " Decoding std::string decode64"<< endl;

        std::string decodestr =  boost::beast::detail::base64_decode((message_str));

//        cout <<"ON_MESSAGE Length: "<< decodestr.length()<< endl;
//
//        cout <<"ON_MESSAGE "<< " Transforming mat into FEATURES * 32 "<< endl;

        vector<cv::Mat >  foundFeatures;
        foundFeatures.reserve(1);
        int index = 0;
        foundFeatures.resize(decodestr.length()/32+1);
//        cout<<foundFeatures.at(0)<<endl;
        for (size_t i = 0; i < decodestr.length() ;i = i+32)
        {
            std::string straux = decodestr.substr(i,32);
            Mat aux(1, 32, CV_8UC1);
            std::vector<int> vectordata(straux.begin(),straux.end());
//            cout<<"Straux Length"<<straux.length()<<" "<<endl;
//            cout<<"Straux "<<straux<<" "<<endl;
//            cout<<"vectordata Length"<<vectordata.size()<<endl;

            for (size_t j = 0; j < vectordata.size()-1; j++)
            {
                aux.at<uchar>(0, j) = vectordata.at(j);
            }

            foundFeatures.at(index) = aux.row(0);
            index++;
            straux.clear();
            vectordata.clear();
        }

        currentImageId = queryDatabase(db2, foundFeatures);

        auto send_stream = make_shared<WsServer::SendStream>();
        if(currentImageId !=-1){
            MapFrame mapFrame =  mapImageDataSet[currentImageId];
            cout<<mapFrame.imageName<<endl;
            *send_stream << mapFrame.imageName;
        }else{
            *send_stream << -1;

        }

        // connection->send is an asynchronous function
        connection->send(send_stream, [](const SimpleWeb::error_code &ec) {
            if(ec) {
                cout << "Server: Error sending message. " <<
                     // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
                     "Error: " << ec << ", error message: " << ec.message() << endl;
            }
        });
        currentImageId = -1;


    };

    echo.on_open = [](shared_ptr<WsServer::Connection> connection) {
        cout << "Server: Opened connection " << connection.get() << endl;
    };

    // See RFC 6455 7.4.1. for status codes
    echo.on_close = [](shared_ptr<WsServer::Connection> connection, int status, const string & /*reason*/) {
        cout << "Server: Closed connection " << connection.get() << " with status code " << status << endl;
    };

    // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
    echo.on_error = [](shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code &ec) {
        cout << "Server: Error in connection " << connection.get() << ". "
             << "Error: " << ec << ", error message: " << ec.message() << endl;
    };

    thread server_thread([&server]() {
        // Start WS-server
        server.start();
    });
    // Wait for server to start so that the client can connect
    this_thread::sleep_for(chrono::seconds(1));

    server_thread.join();


    return 0;
}
void previoustest(ifstream &dataSet,string str){
    string line,ts;

    while (getline(dataSet, line)) {

            string jpg(".jpg");
            cout<<str<<line<<endl;
            Mat img1 = imread(str + line + jpg, IMREAD_GRAYSCALE);
            namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
            imshow( "Display window", img1 );                   // Show our image inside it.

            waitKey(0);

    }
}

void parseDataSet(ifstream &dataSetBase64,vector< vector<cv::Mat >> &featuresDataBase64,std::map<int, MapFrame> &mapImageEntry){
    std::string ts;
    string line;
    string message_str;
    char command;

    vector<cv::Mat >  foundFeatures;
    int imageCounter=0;
    while (getline(dataSetBase64, line)) {
        stringstream iss(line);
        //Check Command
        switch (line[0]){
            case 'b':
                if(iss>>command>>message_str){
//                    cout << "BASE64:" << message_str<< endl;
                    std::string decodestr =  boost::beast::detail::base64_decode((message_str));

                    foundFeatures.reserve(1);
                    int index = 0;
                    foundFeatures.resize(decodestr.length()/32+1);
//        cout<<foundFeatures.at(0)<<endl;
                    for (size_t i = 0; i < decodestr.length() ;i = i+32)
                    {
                        std::string straux = decodestr.substr(i,32);
                        Mat aux(1, 32, CV_8UC1);
                        std::vector<int> vectordata(straux.begin(),straux.end());
//            cout<<"Straux Length"<<straux.length()<<" "<<endl;
//            cout<<"Straux "<<straux<<" "<<endl;
//            cout<<"vectordata Length"<<vectordata.size()<<endl;

                        for (size_t j = 0; j < vectordata.size()-1; j++)
                        {
                            aux.at<uchar>(0, j) = vectordata.at(j);
                        }

                        foundFeatures.at(index) = aux.row(0);
                        index++;
                        straux.clear();
                        vectordata.clear();

                    }
                    featuresDataBase64.push_back(foundFeatures);
                }else{
                    break;
                }

                break;
            case 'p':
                break;
            case 's':

                break;
            case 't':
                if(iss>>command>>ts){
                    MapFrame mapFrame;
                    mapFrame.imageName = ts;
                    mapImageEntry.insert(make_pair(imageCounter,mapFrame) );
                    imageCounter++;
                }
                break;
            default:
                break;
        }

    }
}

void loadFeaturesFromBase64(ifstream &dataSetBase64,vector< vector<cv::Mat >> &featuresDataBase64){
    std::string ts;
    string line;
    string message_str;
    char command;
    vector<KeyPoint> kpts1;
    Mat out0,out1,desc1,desc2,match;
    vector<cv::Mat >  foundFeatures;
    float x, y;
    int i=0 ;

    while (getline(dataSetBase64, line)) {

//        Mat img1;
        stringstream iss(line);

        if(iss >> ts){
//
//            string jpg(".jpg");
//            img1 = imread(datasetPath + ts + jpg, IMREAD_GRAYSCALE);
            cout << "Image:" << ts << " -> "<<i << endl;
            while (getline(dataSetBase64, line)){
                stringstream iss(line);
                if(iss>>command>>message_str){
//                    cout << "BASE64:" << message_str<< endl;

                    std::string decodestr =  boost::beast::detail::base64_decode((message_str));

                    foundFeatures.reserve(1);
                    int index = 0;
                    foundFeatures.resize(decodestr.length()/32+1);
//        cout<<foundFeatures.at(0)<<endl;
                    for (size_t i = 0; i < decodestr.length() ;i = i+32)
                    {
                        std::string straux = decodestr.substr(i,32);
                        Mat aux(1, 32, CV_8UC1);
                        std::vector<int> vectordata(straux.begin(),straux.end());
//            cout<<"Straux Length"<<straux.length()<<" "<<endl;
//            cout<<"Straux "<<straux<<" "<<endl;
//            cout<<"vectordata Length"<<vectordata.size()<<endl;

                        for (size_t j = 0; j < vectordata.size()-1; j++)
                        {
                            aux.at<uchar>(0, j) = vectordata.at(j);
                        }

                        foundFeatures.at(index) = aux.row(0);
                        index++;
                        straux.clear();
                        vectordata.clear();
                    }
                }else{
                    break;
                }
            }
            i++;
        }
        featuresDataBase64.push_back(foundFeatures);
    }
}



void loadFeatures(ifstream &infile, vector<vector<Mat >>  &features, string datasetPath, int nimages,std::map<int, MapFrame> &mapImageEntry) {
  std::string ts;
  std:string line;
  vector<KeyPoint> kpts1;
  Mat out0,out1,desc1,desc2,match;

  float x, y;
  int i=0 ;
  features.reserve(nimages);

  while (getline(infile, line)) {

    Mat img1;
    stringstream iss(line);
    if(iss >> ts){

      string jpg(".jpg");
      img1 = imread(datasetPath + ts + jpg, IMREAD_GRAYSCALE);
      //      cout << "Image:" << ts << " -> "<<i << endl;
      while (getline(infile, line)){
        stringstream iss(line);
        if(iss >> x >> y){
          KeyPoint kp1;
          kp1.pt.x = x;
          kp1.pt.y = y;
          kpts1.push_back(kp1);
        }else{
          break;

        }
      }

//      desc1= loadDescriptors(features,kpts1,img1);
      desc1= orbDescriptors(img1,kpts1,features);
      MapFrame mapFrame;
      mapFrame.keyPoints = kpts1;
      mapFrame.imageName = ts;
      mapImageEntry.insert(make_pair(i,mapFrame) );
      kpts1.clear();
      img1.deallocate();
        i++;
      }
  }
}
Mat loadDescriptors(vector<cv::Mat  > &features,  vector<KeyPoint> &keypoints, Mat image){
  cv::Ptr<cv::ORB> orb = cv::ORB::create();
  Mat descriptors;
  orb->compute(image,keypoints,descriptors);
//    cout<<"DESCRIPTOR!:"<<descriptors<<endl;
  changeStructure(descriptors, features);
  return descriptors;
}

// ----------------------------------------------------------------------------

void changeStructure(const cv::Mat &plain, vector<cv::Mat> &out)
{
  out.resize(plain.rows);

  for(int i = 0; i < plain.rows; ++i)
  {
    out[i] = plain.row(i);
//      cout<<out[i];
  }
}

// ----------------------------------------------------------------------------

void testVocCreation(const vector<vector<cv::Mat > > &features, string vocname)
{
  // branching factor and depth levels
  const int k = 9;
  const int L = 3;
  const WeightingType weight = TF_IDF;
  const ScoringType score = L1_NORM;

  OrbVocabulary voc(k, L, weight, score);


  cout << "Creating a small " << k << "^" << L << " vocabulary..." << endl;
  voc.create(features);
  cout << "... done!" << endl;

  cout << "Vocabulary information: " << endl
       << voc << endl << endl;
  // save the vocabulary to disk
  cout << endl << "Saving vocabulary..." << endl;
  voc.save(vocname);
  cout << "Done" << endl;
}

// ----------------------------------------------------------------------------

int queryDatabase(OrbDatabase &db, const vector<cv::Mat >  &features){
//  cout << "Database information: " << endl << db << endl;

  // and query the database
//  cout << "Querying the database: " << endl;

  QueryResults ret;
  double highScore = 0;
  int position = 0;

  db.query(features, ret,120);
  // ret[0] is always the same image in this case, because we added it to the
  // database. ret[1] is the second best match.

  if(ret.size()>0){
      cout <<"Image 0: "<<ret[0].Id<< " Score: "<< ret[0].Score<<endl;
      cout <<"Image 1: "<<ret[1].Id<< " Score: "<< ret[1].Score<<endl;
      cout <<"Image 2: "<<ret[2].Id<< " Score: "<< ret[2].Score<<endl;
      cout<<endl;
      if(ret[0].Score > 0.3){
          return ret[0].Id;
      }else{
          return -1;
      }
  }


}

void showMatches( Mat img1,  std::vector<KeyPoint>& keypoints1,
                  Mat img2,  std::vector<KeyPoint>& keypoints2){

  vector<cv::Mat >  features;
  features.reserve(200);
  Mat match;

  BFMatcher matcher(NORM_L2, true);   //BFMatcher matcher(NORM_L2);
  Mat desc1 = loadDescriptors(features,keypoints1,img1);
  Mat desc2 = loadDescriptors(features,keypoints2, img2);
//  Mat desc1 = orbDescriptors(img1,keypoints1,features);
//  Mat desc2 = orbDescriptors(img2,keypoints2,features);
  vector<DMatch> filteredMatches12, matches12, matches21;
  matcher.match( desc1,desc2, matches12 );
  matcher.match( desc2,desc1, matches21 );
  for( size_t i = 0; i < matches12.size(); i++ )
  {
      DMatch forward = matches12[i];
      DMatch backward = matches21[forward.trainIdx];
      if( backward.trainIdx == forward.queryIdx )
         filteredMatches12.push_back( forward );
  }

  drawMatches(img1,keypoints1,img2,keypoints2, filteredMatches12, match);
  imshow("MATCHES", WINDOW_NORMAL);
  imshow("MATCHES",match);
  waitKey();

}
Mat orbDescriptors(Mat image,vector<cv::KeyPoint> &keypoints,vector<vector<cv::Mat > > &features){
    cv::Mat mask;
    cv::Mat descriptors;
    cv::Ptr<cv::ORB> orb = cv::ORB::create();
    orb->detectAndCompute(image, mask, keypoints, descriptors);
    features.push_back(vector<cv::Mat >());
    changeStructure(descriptors, features.back());
    return descriptors;
}
