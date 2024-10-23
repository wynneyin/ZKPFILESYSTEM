// author bufanzhen
// date 20240506
// barrett reduction:
// for input z(128bit),p(64bit)
// output z mod p


#include <iostream>
using namespace std;
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "cuda.h"
#include <inttypes.h>





// compute miu = floor(b**2k/p)
// __device__ void preCompute(uint32_t P[n],uint32_t miu[2]){
//     return ;
// }

__device__   __forceinline__ void MULT(uint32_t a[],uint32_t b[],uint32_t alen,uint32_t blen,uint32_t res[]){
    uint32_t reslen = alen + blen;
    
    res[reslen-1] = 0;
    asm("mul.lo.u32 %0, %2, %3; mul.hi.u32 %1, %2, %3;"
        : "=r"(res[0]), "=r"(res[1])
        : "r"(a[0]), "r"(b[0]));
    asm("mul.lo.u32 %0, %2, %3; mul.hi.u32 %1, %2, %3;"
        : "=r"(res[2]), "=r"(res[3])
        : "r"(a[1]), "r"(b[1]));
    asm("mad.lo.cc.u32 %0, %2, %3, %0; madc.hi.cc.u32 %1, %2, %3, %1;"
        : "+r"(res[1]), "+r"(res[2])
        : "r"(a[1]), "r"(b[0]));
    asm("addc.u32 %0, %0, 0;" : "+r"(res[3]));
    asm("mad.lo.cc.u32 %0, %2, %3, %0; madc.hi.cc.u32 %1, %2, %3, %1;"
        : "+r"(res[1]), "+r"(res[2])
        : "r"(a[0]), "r"(b[1]));
    asm("addc.u32 %0, %0, 0;" : "+r"(res[3]));
    asm("mad.lo.u32.cc %0, %2, %3, %0; madc.hi.cc.u32 %1, %2, %3, %1;"
        : "+r"(res[2]), "+r"(res[3])
        : "r"(a[2]), "r"(b[0]));
    asm("addc.u32 %0, %0, 0;" : "+r"(res[4]));
    asm("mad.lo.cc.u32 %0, %2, %3, %0; madc.hi.u32 %1, %2, %3, %1;"
        : "=r"(res[3]), "+r"(res[4])
        : "r"(a[2]), "r"(b[1]));
    return ;
}


__device__ __forceinline__ void computeQhat(uint32_t Z[],uint32_t miu[],uint32_t Qhat[]){
    uint32_t tmpZ[3] = {0,0,0};
    tmpZ[0] = Z[2]<<1|Z[1]>>31;
    tmpZ[1] = Z[3]<<1|Z[2]>>31;
    tmpZ[2] = Z[3]>>31;
    uint32_t tmpQhat[4] = {0,0,0,0};
    asm("mul.hi.u32 %0, %1, %2;": "=r"(tmpQhat[0]): "r"(tmpZ[0]), "r"(miu[0]));
    asm("mul.lo.u32 %0, %2, %3; mul.hi.u32 %1, %2, %3;"
        : "=r"(tmpQhat[1]), "=r"(tmpQhat[2])
        : "r"(tmpZ[1]), "r"(miu[1]));
    asm("mad.lo.cc.u32 %0, %2, %3, %0; madc.hi.cc.u32 %1, %2, %3, %1;"
        : "+r"(tmpQhat[0]), "+r"(tmpQhat[1])
        : "r"(tmpZ[1]), "r"(miu[0]));
    asm("addc.cc.u32 %0, %0, 0;" : "+r"(tmpQhat[2]));
    asm("addc.u32 %0, %0, 0;" : "+r"(tmpQhat[3]));
    asm("mad.lo.cc.u32 %0, %2, %3, %0; madc.hi.cc.u32 %1, %2, %3, %1;"
        : "+r"(tmpQhat[0]), "+r"(tmpQhat[1])
        : "r"(tmpZ[0]), "r"(miu[1]));
    asm("addc.cc.u32 %0, %0, 0;" : "+r"(tmpQhat[2]));
    asm("addc.u32 %0, %0, 0;" : "+r"(tmpQhat[3]));
    asm("mad.lo.u32.cc %0, %2, %3, %0; madc.lo.cc.u32 %1, %2, %4, %1;"
        : "+r"(tmpQhat[1]), "+r"(tmpQhat[2])
        : "r"(tmpZ[2]), "r"(miu[0]), "r"(miu[1]));
    asm("addc.u32 %0, %0, 0;" : "+r"(tmpQhat[3]));
    asm("mad.lo.cc.u32 %0, %2, %3, %0; madc.lo.cc.u32 %1, %2, %4, %1;"
        : "+r"(tmpQhat[1]), "+r"(tmpQhat[2])
        : "r"(miu[2]), "r"(tmpZ[0]), "r"(tmpZ[1]));
    asm("addc.u32 %0, %0, 0;" : "+r"(tmpQhat[3]));
    asm("mad.lo.u32 %0, %1, %2, %0;"
        : "+r"(tmpQhat[3])
        : "r"(miu[2]), "r"(tmpZ[2]));
    Qhat[0] = tmpQhat[1]>>1|tmpQhat[2]<<31;
    Qhat[1] = tmpQhat[2]>>1|tmpQhat[3]<<31;
    Qhat[2] = tmpQhat[3]>>1;
    return ;
}

__device__ __forceinline__ void computeR2(uint32_t r2[], uint32_t qHat[],uint32_t p[]){
    asm("mul.lo.u32 %0, %2, %3; mul.hi.u32 %1, %2, %3;"
        : "=r"(r2[0]), "=r"(r2[1])
        : "r"(qHat[0]), "r"(p[0]));
    asm("mul.lo.u32 %0, %1, %2;"
        : "=r"(r2[2])
        : "r"(qHat[1]), "r"(p[1]));
    asm("mad.lo.u32.cc %0, %2, %3, %0; madc.hi.u32 %1, %2, %3, %1;"
        : "+r"(r2[1]), "+r"(r2[2])
        : "r"(qHat[1]), "r"(p[0]));
    asm("mad.lo.u32.cc %0, %2, %3, %0; madc.hi.u32 %1, %2, %3, %1;"
        : "+r"(r2[1]), "+r"(r2[2])
        : "r"(qHat[0]), "r"(p[1]));
    asm("mul.lo.u32 %0, %1, %2;"
        : "+r"(r2[2])
        : "r"(qHat[2]), "r"(p[0]));
    r2[2] = r2[2]&0x1;
    return ;
}


__device__ void BarrettRedc(uint32_t Z[],uint32_t p[],uint32_t miu[],uint32_t res[]){
    // compute q_hat
    uint32_t qHat[3] = {0,0,0};
    computeQhat(Z,miu,qHat);

    // compute r
    uint32_t r1[3] = {Z[0],Z[1],Z[2]&0x1};
    uint32_t r2[3] = {0,0,0};
    computeR2(r2,qHat,p);
    

    // compare R1 and R2 compute r=R1-R2
    // if r1<r2 r1=r1+x**(k+1)
    if(r1[2]<r2[2]||((r1[2]==r2[2])&&r1[1]<r2[1])||(((r1[2]==r2[2])&&(r1[1]==r2[1])&&(r1[0]<r2[0])))){
        r1[2]+=1;
    }
    asm("sub.cc.u32 %0, %0, %1;" : "+r"(r1[0]) : "r"(r2[0]));
    asm("subc.cc.u32 %0, %0, %1;" : "+r"(r1[1]) : "r"(r2[1]));
    asm("subc.cc.u32 %0, %0, %1;" : "+r"(r1[2]) : "r"(r2[2]));

    // now r1 is r
    while((r1[2]>0)||(r1[1]>p[1])||((r1[1]==p[1])&&(r1[0]>=p[0]))){
        asm("sub.cc.u32 %0, %0, %1;" : "+r"(r1[0]) : "r"(p[0]));
        asm("subc.cc.u32 %0, %0, %1;" : "+r"(r1[1]) : "r"(p[1]));
        asm("subc.cc.u32 %0, %0, 0;" : "+r"(r1[2]));
    }
    res[1] = r1[1];
    res[0] = r1[0];
    return ;

}

__global__ void func1(uint32_t Z[],uint32_t P[],uint32_t res[]){
    
    uint32_t miu[3] =   {0x708a9a6b,0xb1dbb0f0,0x1};
    BarrettRedc(Z,P,miu,res);
    return ;
}


int main(){

    uint32_t Plen = 64;
    uint32_t P[2]   =   {0x34f7fbb5,0x970dd129};
    uint32_t Z[4] =   {0xb694bf5b,0xe90cbb0e,0xf6b92885,0xde43d922};



    uint32_t *d_Z,*d_P,*d_res;
    uint32_t *res = new uint32_t[2];
    cudaMalloc((void**)&d_P , sizeof(uint32_t) * 2);
    cudaMalloc((void**)&d_Z , sizeof(uint32_t) * 4);
    cudaMalloc((void**)&d_res , sizeof(uint32_t) * 2);
    cudaMemcpy( d_P,P , sizeof(uint32_t) * 2, cudaMemcpyHostToDevice);
    cudaMemcpy( d_Z,Z , sizeof(uint32_t) * 4, cudaMemcpyHostToDevice);

    
    func1<<<1,32>>>(d_Z,d_P,d_res);


    cudaMemcpy( res, d_res, sizeof(uint32_t) * 2, cudaMemcpyDeviceToHost);
    for(int i = 0;i<2;i++){
        cout<<std::hex<<res[i]<<endl;
    }
    return 0;
}