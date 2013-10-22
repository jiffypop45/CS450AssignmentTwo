CS450/550 Assignment 1
Members: Austin Sharp, Padraic McGraw

USAGE: SimpleProgram DATA_FILENAME NUM_BUCKETS
		DATA_FILENAME must be name of file in the Data/ directory
		Default NUM_BUCKETS value is 20

This program expect the files provided to be in the following format:
# m  n  	//these are the dimensions of the grid
floatx0y0	 floatx1y0	 ...float xmy0
...
floatx0yn	 floatx1yn	 ...float xmyn

m and n are used to determine dimensions of the mesh used to display the data in the file.
As we don't restrict the size of data, be careful not to abuse this.