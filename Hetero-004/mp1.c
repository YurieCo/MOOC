// MP 1 Vector Addition 
#include <wb.h>

#define wbCheck(stmt) do {                                                    \
        cudaError_t err = stmt;                                               \
        if (err != cudaSuccess) {                                             \
            wbLog(ERROR, "Failed to run stmt ", #stmt);                       \
            wbLog(ERROR, "Got CUDA error ...  ", cudaGetErrorString(err));    \
            return -1;                                                        \
        }                                                                     \
    } while(0)

__global__ void vecAdd(float *in1, float *in2, float *out, int len) {
  //@@ Insert code to implement vector addition here
  int index = threadIdx.x +blockDim.x*blockIdx.x;
  if (index < len)
  {
	  out[index] = in1[index] + in2[index];
  }
}

int main(int argc, char **argv) {
  wbArg_t args;
  int inputLength;
  float *hostInput1;
  float *hostInput2;
  float *hostOutput;
  float *deviceInput1;
  float *deviceInput2;
  float *deviceOutput;

  args = wbArg_read(argc, argv);

  wbTime_start(Generic, "Importing data and creating memory on host");
  hostInput1 = ( float * )wbImport(wbArg_getInputFile(args, 0), &inputLength);
  hostInput2 = ( float * )wbImport(wbArg_getInputFile(args, 1), &inputLength);
  hostOutput = ( float * )malloc(inputLength * sizeof(float));
  wbTime_stop(Generic, "Importing data and creating memory on host");

  wbLog(TRACE, "The input length is ", inputLength);

  wbTime_start(GPU, "Allocating GPU memory.");
  //@@ Allocate GPU memory here
  wbCheck(cudaMalloc( (void **)&deviceInput1, sizeof(float) * inputLength));
  wbCheck(cudaMalloc( (void **)&deviceInput2, sizeof(float) * inputLength));
  wbCheck(cudaMalloc( (void **)&deviceOutput, sizeof(float) * inputLength));
  
  wbTime_stop(GPU, "Allocating GPU memory.");

  wbTime_start(GPU, "Copying input memory to the GPU.");
  //@@ Copy memory to the GPU here
  wbCheck(cudaMemcpy(deviceInput1, hostInput1, sizeof(float) * inputLength, cudaMemcpyHostToDevice));
  wbCheck(cudaMemcpy(deviceInput2, hostInput2, sizeof(float) * inputLength, cudaMemcpyHostToDevice));

  wbTime_stop(GPU, "Copying input memory to the GPU.");

  //@@ Initialize the grid and block dimensions here
  dim3 dimGrid((inputLength - 1) / 256 + 1, 1, 1);
  dim3 dimBlock(256, 1, 1);

  wbTime_start(Compute, "Performing CUDA computation");
  //@@ Launch the GPU Kernel here
  vecAdd<<<dimGrid,dimBlock>>>(deviceInput1, deviceInput2, deviceOutput, inputLength);

  cudaDeviceSynchronize();
  wbTime_stop(Compute, "Performing CUDA computation");

  wbTime_start(Copy, "Copying output memory to the CPU");
  //@@ Copy the GPU memory back to the CPU here
  wbCheck(cudaMemcpy(hostOutput, deviceOutput, sizeof(float) * inputLength, cudaMemcpyDeviceToHost));
  

  wbTime_stop(Copy, "Copying output memory to the CPU");

  wbTime_start(GPU, "Freeing GPU Memory");
  //@@ Free the GPU memory here
  wbCheck(cudaFree(deviceInput1));
  wbCheck(cudaFree(deviceInput2));
  wbCheck(cudaFree(deviceOutput));

  wbTime_stop(GPU, "Freeing GPU Memory");

  wbSolution(args, hostOutput, inputLength);

  free(hostInput1);
  free(hostInput2);
  free(hostOutput);

  return 0;
}