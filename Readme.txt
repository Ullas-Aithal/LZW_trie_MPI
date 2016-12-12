 How to execute
1. keep input1.txt used as input to the parallel execution
2. serialInput.txt used as input to the serial execution
3. Keep all input and .cpp files in same folder.
4. Running serial code
   >g++ LZW_Serial.cpp
   >./a.out
5. Running paralle code
   >mpiCC LZW_Parallel.cpp
   >mpiexec -np <no of processors> ./a.ou