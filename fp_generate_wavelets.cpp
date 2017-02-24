//==========================================================================
// Name        : fp_generate_wavelets.cpp
// Author      : Martin Saly
// Version     : 1.0
// Description : Fingerprints processing - wavelets generation from patterns
//==========================================================================

// compile with -std=c++11

/*
 standalone program version to be run using e.g.:
 ./fp_generate_wavelets < datalog.csv

 where datalog.csv has expected structure:

 lineid;timestamp;meas;diff;curavg;isdetect;isalarm;iswait;patternid
 1;10-03-2016 15:27:00.875012;68988;0;199.6;0;0;0;0
 2;10-03-2016 15:27:00.875075;68798;-190;199.581;0;0;0;0
 3;10-03-2016 15:27:00.875139;68588;-210;199.602;0;0;0;0

 After processing, following data are outputted to the current directory:

 wavelet_1.txt 		corresponding to pattern no 1
 wavelet_2.txt 		corresponding to pattern no 2
 ... etc.

 */

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include "wavelet.hpp"

// prototypes
void generate_daub4();

// structure for time storing

int main(int argc, char* argv[]) {

	// if at least one argument passed, evaluate number of them
	if (argc != 2) {
		std::cerr << "Argument error: must pass filename as argument"
				<< std::endl;
		std::cerr << "Program terminated" << std::endl;
		return 1;
	}

	// debug
	// std::cerr << std::endl << "(debug) Input file: "<< argv[1] << std::endl;

	// variables to read input file
	std::ifstream infile(argv[1]);
	std::string line;
	int s1, s2, s3, s4, s5, s6, s7, s8;

	// variables for patterns processing
	int patid, lastpatid = 0, processpatid = 0;

	double diff;

	// variables for waveletes generation
	std::vector<double> seqdata;
	int n, namend;
	double* d;
	double* v; 	// wavelets results
	double* w; 	// wavelets inverse results (helper)

	//debug
	// std::cout << "(debug) seqdata.size(): " << seqdata.size() << std::endl;

	while (std::getline(infile, line)) {
		std::istringstream iss(line);

		//debug
		//std::cout << "(debug) find: " << line.find("lineid") << std::endl;
		//std::cout << "(debug) npos: " << std::string::npos << std::endl;

		// skip some lines, e.g. header
		if (line.find("lineid") == std::string::npos) {

			// debug
			//std::cout << "(debug) line: "<< line << std::endl;

			// parse line
			s1 = line.find(";", 0);
			s2 = line.find(";", s1 + 1);
			s3 = line.find(";", s2 + 1);
			s4 = line.find(";", s3 + 1);
			s5 = line.find(";", s4 + 1);
			s6 = line.find(";", s5 + 1);
			s7 = line.find(";", s6 + 1);
			s8 = line.find(";", s7 + 1);

			//debug
			//std::cout << "(debug) parse ; positions: " << s1 << " " <<  s2 << " "  << s3 << " "  << s4 << " "  << s5 << " "  << s6 << " "  << s7 << " "  << s8 << " "  << std::endl;

			// convert values of diff anc current pattern
			patid = atoi(line.substr(s8 + 1).c_str());
			diff = atof(line.substr(s3 + 1, s4 - s3 - 1).c_str());

			// debug
			// std::cout << "(debug) diff, patternid: " << diff << "," << patid << std::endl;

			// verify if new pattern and trigger for processing
			if (patid != lastpatid and patid != 0) {
				// debug
				//std::cout << "(debug) line: "<< line << std::endl;
				//std::cout << "(debug) pattern change" << std::endl;
				processpatid = patid;
			}

			lastpatid = patid;

			// processing current pattern, if exists
			if (processpatid > 0) {

				// debug
				//std::cout << "(debug) processpatid: "<< processpatid << std::endl;
				//std::cout << "(debug) patid: "<< patid << std::endl;

				if (patid == processpatid) {
					// processing current pattern - load data to seqdata
					seqdata.push_back(diff);

					// debug
					// std::cout << "(debug) processing pattern: " << processpatid << std::endl;
					// std::cout << "(debug) seqdata.size():" << seqdata.size() << std::endl;

				} else {
					// pattern changed - finalize the current pattern
					// std::cout << "(debug) finalizing pattern: " << processpatid << std::endl;
					// std::cout << "(debug) seqdata.size():" << seqdata.size() << std::endl;

					// process seqdata

					// cut vector to n and copy to array d
					//seqdata.resize(n);
					d = &seqdata[0];

					// find appropriate n - nearest biggest power of 2
					n = seqdata.size();

					//debug
					// std::cout << "(debug) original n: " << n << std::endl;
					// std::cout << "(debug) log2(n): " << log2(n) << std::endl;

					namend = pow(2.0,trunc(log2(n))+1);

					// debug
					// std::cout << "(debug) amended n: " << n << std::endl;

					v = daub4_transform(namend, d);
					w = daub4_transform_inverse(namend, v);

					// output original size only

					// debug
					for (int i = 0; i < n - 2; i++) {
								std::cout << "(debug)  " << std::setw(2) << i << "  " << std::setw(10) << d[i]
										<< "  " << std::setw(10) << v[i] << "  " << std::setw(10)
										<< w[i] << "\n";
					}

					// output to file
					std::ofstream outputfile;
					outputfile.open ("wavelet_"+std::to_string(processpatid)+".txt");
					for (int i = 0; i < n - 2; i++) {
						outputfile << v[i] << std::endl;
					}

					outputfile.close();

					// cleanse seqdata for future use
					processpatid = 0;
					seqdata.clear();
				}

			}

		}
	}


	return 0;
}

