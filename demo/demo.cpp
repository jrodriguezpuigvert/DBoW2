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

#include <map>
#include <MapFrame.h>
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
void testVocCreation(const vector<vector<cv::Mat > > &features);
Mat loadDescriptors(vector<vector<cv::Mat > > &features,  vector<KeyPoint> &keypoints, Mat image);
void showMatches( Mat img1,  std::vector<KeyPoint>& keypoints1,
                  Mat img2,  std::vector<KeyPoint>& keypoints2);
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void createDatabase(OrbDatabase &db, const vector<vector<cv::Mat > > &features) ;
int queryDatabase(OrbDatabase &db, const vector<vector<cv::Mat > > &features) ;

void loadFeatures(ifstream &infile, vector<vector<Mat > > &features, string datasetPath, int nimages,std::map<int, MapFrame> &mapImageEntry);

Mat orbDescriptors(Mat image,vector<cv::KeyPoint> &keypoints,vector<vector<cv::Mat > > &features);
// number of training images
const int NIMAGES = 1508;
const int TESTNIMAGES = 1189;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void wait()
{
  cout << endl << "Press enter to continue" << endl;
  getchar();
}
static const std::string base64_chars =
                "abcdef"
                "0123456789";
// ----------------------------------------------------------------------------
int main(void)

{
//  std::map<int, MapFrame> mapImageDataSet;
//  std::map<int, MapFrame> mapImageTestSet;
//  std::ifstream dataSetPoints("../demo/dataset/16-01-2018/pointcloudtest.txt");
//  std::ifstream testSetPoints("../demo/dataset/16-01-2018/pointcloudtest.txt");
//  vector<vector<cv::Mat > > featuresDataSet,featuresTestSet;
//  string dataSetImages("../demo/dataset/16-01-2018/");
//  string testSetImages("../demo/dataset/16-01-2018/");
//  string jpg(".jpg");
//  Mat imgTest,imgData;
//  // load the vocabulary from disk
//
//  OrbVocabulary voc("small_voc.yml.gz");
//
//  OrbDatabase db(voc, false, 0);
//
//  loadFeatures(dataSetPoints, featuresDataSet,dataSetImages, NIMAGES, mapImageDataSet);
//
//  testVocCreation(featuresDataSet);
//
//  wait();
//
//  // false = do not use direct index
//  // (so ignore the last param)
//  // The direct index is useful if we want to retrieve the features that
//  // belong to some vocabulary node.
//  // db creates a copy of the vocabulary, we may get rid of "voc" now
//
//  createDatabase(db,featuresDataSet);
//
//  loadFeatures(testSetPoints, featuresTestSet,testSetImages ,NIMAGES,mapImageTestSet);
//
//  for(int i = 0; i<mapImageTestSet.size();i++) {
//    vector<vector<cv::Mat > > foundFeatures;
//    foundFeatures.reserve(2);
//    imgTest = imread(testSetImages + mapImageTestSet[i].imageName + jpg, IMREAD_GRAYSCALE);
//
//    loadDescriptors(foundFeatures,mapImageTestSet[i].keyPoints,imgTest);
////    orbDescriptors(imgTest,mapImageTestSet[i].keyPoints,foundFeatures);
//    int result = queryDatabase(db,foundFeatures);
//    if(result != -1){
//        MapFrame mapFrame =  mapImageDataSet[result];
//        imgData = imread(dataSetImages + mapImageDataSet[result].imageName + jpg, IMREAD_GRAYSCALE);
//        showMatches(imgTest,mapImageTestSet[i].keyPoints,imgData,mapImageDataSet[result].keyPoints);
//    }
//  }
//  cout << "Test Completed" << endl;

  cout << endl;
    cout << "***************************************" << endl;

    WsServer server;
    server.config.address = "192.168.178.115";
    server.config.port = 8000;
    auto &echo = server.endpoint["/"];

    echo.on_message = [](shared_ptr<WsServer::Connection> connection, shared_ptr<WsServer::Message> message) {
        cout << "Size: " << message->size()<< endl;
        auto message_str = message->string();
//        String  SimpleWeb::Crypto::Base64::decode(message_str);
        std::string str = base64_decode(message_str);
        cout << "Length :"<<str.length();

        //BASE64 to MAT in Opencv
        cv::Mat mat(154,32,CV_8UC1, str.data());

        cout << "Server: Message received: \"" << message_str << "\" from " << connection.get() << endl;

        cout << "Server: Sending message \"" << message_str << "\" to " << connection.get() << endl;

        auto send_stream = make_shared<WsServer::SendStream>();
        *send_stream << message_str;
        // connection->send is an asynchronous function
        connection->send(send_stream, [](const SimpleWeb::error_code &ec) {
            if(ec) {
                cout << "Server: Error sending message. " <<
                     // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
                     "Error: " << ec << ", error message: " << ec.message() << endl;
            }
        });
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

void loadFeatures(ifstream &infile, vector<vector<Mat > > &features, string datasetPath, int nimages,std::map<int, MapFrame> &mapImageEntry) {
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

      desc1= loadDescriptors(features,kpts1,img1);
//      desc1= orbDescriptors(img1,kpts1,features);
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
Mat loadDescriptors(vector<vector<cv::Mat > > &features,  vector<KeyPoint> &keypoints, Mat image){
  cv::Ptr<cv::ORB> orb = cv::ORB::create();
  Mat descriptors;
  orb->compute(image,keypoints,descriptors);
  features.push_back(vector<cv::Mat >());
  changeStructure(descriptors, features.back());
  return descriptors;
}

// ----------------------------------------------------------------------------

void changeStructure(const cv::Mat &plain, vector<cv::Mat> &out)
{
  out.resize(plain.rows);

  for(int i = 0; i < plain.rows; ++i)
  {
    out[i] = plain.row(i);
  }
}

// ----------------------------------------------------------------------------

void testVocCreation(const vector<vector<cv::Mat > > &features)
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

  // lets do something with this vocabulary
  cout << "Matching images against themselves (0 low, 1 high): " << endl;
  //  BowVector v1, v2;
  //  for(int i = 0; i < NIMAGES; i++)
  //  {
  //    voc.transform(features[i], v1);
  //    for(int j = 0; j < NIMAGES; j++)
  //    {
  //      voc.transform(features[j], v2);
  //
  //      double score = voc.score(v1, v2);
  //      cout << "Image " << i << " vs Image " << j << ": " << score << endl;
  //    }
  //  }

  // save the vocabulary to disk
  cout << endl << "Saving vocabulary..." << endl;
  voc.save("small_voc.yml.gz");
  cout << "Done" << endl;
}

// ----------------------------------------------------------------------------

void createDatabase(OrbDatabase &db, const vector<vector<cv::Mat > > &features) {
  cout << "Creating a small database..." << endl;

  // add images to the database
  for (int i = 0; i < NIMAGES; i++) {
    db.add(features[i]);
  }

  cout << "... done!" << endl;
}

int queryDatabase(OrbDatabase &db, const vector<vector<cv::Mat > > &features){
  cout << "Database information: " << endl << db << endl;

  // and query the database
  cout << "Querying the database: " << endl;

  QueryResults ret;
  double highScore = 0;
  int position = 0;

  db.query(features[0], ret,9000);
  // ret[0] is always the same image in this case, because we added it to the
  // database. ret[1] is the second best match.

  cout <<"Image: "<<ret[1].Id<< " Score: "<< ret[1].Score;

    cout << endl;
    if(ret[1].Score > 0.4){

        return ret[1].Id;
    } else{
        return -1;
    }
}

void showMatches( Mat img1,  std::vector<KeyPoint>& keypoints1,
                  Mat img2,  std::vector<KeyPoint>& keypoints2){

  vector<vector<cv::Mat > > features;
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
// ----------------------------------------------------------------------------
std::string base64_decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}

static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}