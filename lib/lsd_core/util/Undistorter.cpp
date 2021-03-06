/**
* This file is part of LSD-SLAM.
*
* Copyright 2013 Jakob Engel <engelj at in dot tum dot de> (Technical University of Munich)
* For more information see <http://vision.in.tum.de/lsdslam>
*
* LSD-SLAM is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* LSD-SLAM is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with LSD-SLAM. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Undistorter.h"

#include <fstream>

#include "util/settings.h"

namespace lsd_slam
{

Undistorter::~Undistorter()
{
}

Undistorter* Undistorter::getUndistorterForFile(const std::string &configFilename)
{
	printf("Reading Calibration from file %s",configFilename.c_str());


	std::ifstream f(configFilename.c_str());
	if (!f.good())
	{
		f.close();

		std::string altFilename = packagePath+"calib/"+configFilename;
		printf(" ... not found!\n Trying %s", altFilename.c_str());

		f.open(altFilename.c_str());

		if (!f.good())
		{
			printf(" ... not found. Cannot operate without calibration, shutting down.\n");
			f.close();
			return 0;
		}
	}

	printf(" ... found!\n");

	std::string l1;
	std::getline(f,l1);
	f.close();



	float ic[10];
	if(std::sscanf(l1.c_str(), "%f %f %f %f %f %f %f %f",
			&ic[0], &ic[1], &ic[2], &ic[3], &ic[4],
			&ic[5], &ic[6], &ic[7]) == 8)
	{
		LOG(INFO) << "found OpenCV camera model, building rectifier.";
		Undistorter* u = new UndistorterOpenCV(configFilename.c_str());
		if(!u->isValid()) return 0;
		return u;
	}
	else if(std::sscanf(l1.c_str(), "%f %f %f %f",
				&ic[0], &ic[1], &ic[2], &ic[3]) == 4)
	{
		LOG(INFO) << "found Logger camera model, building rectifier.";
		Undistorter* u = new UndistorterLogger(configFilename.c_str());
		if(!u->isValid()) return 0;
		return u;
} else	{
		LOG(INFO) << "found ATAN camera model, building rectifier.";
		Undistorter* u = new UndistorterPTAM(configFilename.c_str());
		if(!u->isValid()) return 0;
		return u;
	}
}

}
