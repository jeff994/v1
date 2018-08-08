#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "config.h"
#include "xyz_robot.h"
#include "modbus.h"
 #include <boost/lambda/lambda.hpp>
#include <iostream>
#include <iterator>
#include <algorithm>

int main (int argc, char *argv[])
{
  if (argc < 2)
  {
      printf("Hello world");
      return 1;
  }
   using namespace boost::lambda;
    typedef std::istream_iterator<int> in;

    std::for_each(
        in(std::cin), in(), std::cout << (_1 * 3) << " " );
  // Step 1: Connect Robot 

  




  // If robot cannot connect then report error 

  // Step 2: Get robot ready for tasks 


  // If robot re-started, need to handle  

  return 0;
}