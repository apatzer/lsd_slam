/*
 * LogReader.cpp
 *
 *  Created on: 19 Nov 2012
 *      Author: thomas
 */


// TODO:   Currently single threaded.  Uncompression could be parallelized
// in manner similar to LogWriter...

#include "LogReader.h"

#include <zlib.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <g3log/g3log.hpp>

namespace logger {

LogReader::LogReader( void )
  : currentFrame( 0 ),
    numFrames( 0 )
{
    // CHECK(fs::exists(file.c_str()));
    //
    // fp = fopen(file.c_str(), "rb");
    //
    // currentFrame = 0;
    //
    // assert(fread(&numFrames, sizeof(int32_t), 1, fp));
    //
    // depthReadBuffer = new unsigned char[numPixels * 2];
    // imageReadBuffer = new unsigned char[numPixels * 3];
}

LogReader::~LogReader()
{
  close();

    // delete [] depthReadBuffer;
    // delete [] imageReadBuffer;
    //
    // if(deCompImage != 0)
    // {
    //     cvReleaseImage(&deCompImage);
    // }
    //
    // fclose(fp);
}

FieldHandle_t LogReader::findField( const std::string &field )
{
  for( unsigned int i = 0; i < _fields.size(); ++i ) {
    if( field == _fields[i].name ) return i;
  }

  return -1;
}

bool LogReader::open( const std::string &filename )
{
  CHECK(fs::exists(fs::path(filename)));

  fp = fopen(filename.c_str(), "rb");
  if( !fp ) {
    LOG(WARNING) << "Couldn't open log file " << filename;
    return false;
  }

  currentFrame = 0;
  uint16_t version = 0;

  fread( &version, sizeof( int16_t), 1, fp );
  CHECK( version == LogFormatVersion );
  LOG(DEBUG) << "Format version: " << version;

  fread( &_featureFlags, sizeof( int16_t), 1, fp );
  LOG(DEBUG) << "Feature flags: " << _featureFlags;

  CHECK( _featureFlags != 0 ) << "Feature flags is zero? (" << _featureFlags << ")";

  fread(&numFrames, sizeof(int32_t), 1, fp);
  LOG(DEBUG) << "Num frames: " << numFrames;

	int32_t numFields = 0;
	fread(&numFields, sizeof(int32_t), 1, fp);
  LOG(DEBUG) << "Num fields: " << numFields;

  CHECK( numFields > 0 && numFields < 5 ) << "Don't believe the number of fields: " << numFields;

  _fields.clear();
	for( unsigned int i = 0; i < numFields; ++i ) {
		int32_t h, w, type, len;
    char buf[80];

		fread(&h, sizeof(int32_t), 1, fp);
		fread(&w, sizeof(int32_t), 1, fp);
		fread(&type, sizeof(int32_t), 1, fp);
		fread(&len, sizeof(int32_t), 1, fp);
    len = std::min(len,80);
		fread(buf, sizeof(std::string::value_type), len, fp);

    _fields.emplace_back( std::string( buf, len), cv::Size( w, h ), static_cast<FieldType_t>(type) );

    const Field &f( _fields.back() );
    _data.emplace_back( f.nBytes() );

    _compressed.emplace_back( f.compressedBytes() );
	}

  // Read header
  return true;
}

bool LogReader::close( void )
{
  if( fp == NULL ) return false;

  fclose( fp );
  fp = NULL;
  return true;
}


bool LogReader::grab()
{
  currentFrame++;

  for( unsigned int i = 0; i < _fields.size(); ++i ) {
    unsigned int len;
    CHECK( fread( &len, sizeof(uint32_t), 1, fp ) == sizeof(uint32_t) );
    CHECK( fread( _compressed[i].data.get(), sizeof( unsigned char), len, fp ) == len*sizeof(unsigned char));
    _compressed[i].size = len;
  }

  // Now decompress the data
  for( unsigned int i = 0; i < _fields.size(); ++i ) {
    uLongf destLen = _data[i].size;

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    {
      if( uncompress( (Bytef *)_data[i].data.get(), &destLen, (Bytef *)_compressed[i].data.get(), _compressed[i].size ) != Z_OK) {
        LOG(WARNING) << "Error uncompressing zlib data.";
        continue;
      }
    }
    LOG(DEBUG) << "PNG uncompression required " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count() << " ms";


    CHECK( destLen == _fields[i].nBytes() );
  }

  return true;

//     assert(fread(&timestamp, sizeof(int64_t), 1, fp));
//
//     assert(fread(&depthSize, sizeof(int32_t), 1, fp));
//     assert(fread(&imageSize, sizeof(int32_t), 1, fp));
//
//     assert(fread(depthReadBuffer, depthSize, 1, fp));
//

//
//     unsigned short * depthBuffer = (unsigned short *)&decompressionBuffer[0];
//     unsigned short maxVal = 0;
//     unsigned short minVal = std::numeric_limits<unsigned short>::max();
//
//     #pragma omp parallel for reduction(max : maxVal) reduction(min : minVal)
//     for(int i = 0; i < numPixels; i++)
//     {
//         if(depthBuffer[i] > maxVal)
//         {
//             maxVal = depthBuffer[i];
//         }
//
//         if(depthBuffer[i] < minVal && depthBuffer[i] != 0)
//         {
//             minVal = depthBuffer[i];
//         }
//     }
//
//     this->maxVal = maxVal;
//     this->minVal = minVal;
//
//     if(deCompImage != 0)
//     {
//         cvReleaseImage(&deCompImage);
//     }
//
//     CvMat tempMat = cvMat(1, imageSize, CV_8UC1, (void *)imageReadBuffer);
//
//     if(imageSize == numPixels * 3)
//     {
//         deCompImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
//
//         memcpy(deCompImage->imageData, imageReadBuffer, numPixels * 3);
//     }
//     else if(imageSize > 0)
//     {
//         deCompImage = cvDecodeImage(&tempMat);
//     }
//     else
//     {
//         deCompImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
//         memset(deCompImage->imageData, 0, numPixels * 3);
//     }
//
//     depth = (unsigned short *)decompressionBuffer;
//     rgb = (unsigned char *)deCompImage->imageData;
//
}

cv::Mat LogReader::retrieve( FieldHandle_t handle )
{
  CHECK( handle >= 0 && handle < _fields.size() );


  return cv::Mat( _fields[handle].size, _fields[handle].cvType(), (void *)_data[handle].data.get() );
}


// bool LogReader::hasMore()
// {
//     return currentFrame + 1 < numFrames;
// }
//
// const std::string LogReader::getFile()
// {
//     return file;
// }

 }
