
#include <sycl/sycl.hpp>
#include <iomanip>

using namespace sycl;

int main() {
    
    size_t bs=8,n=67,m=5;

    //# Define vectors for m8atrices
    std::vector<float> matrix_a(bs*n);
    std::vector<float> matrix_b(n*m);
    std::vector<float> matrix_c(bs*m);
    std::vector<unsigned int> matrix_ba((n/32+1)*bs),matrix_bb((n/32+1)*m);
    
    int basize=(n+31)/32*bs,   asize=bs*n;
    int bbsize=(n+31)/32*m,   bsize=m*n;

    float v1 = 2.f;
    float v2 = 3.f;
    
    for(int i=0;i<asize;i++)matrix_a[i]=-1;
    for(int i=0;i<bsize;i++)matrix_b[i]=1;
    
   
    queue q;
    std::cout << "Offload Device : " << q.get_device().get_info<info::device::name>() << std::endl;
    
    buffer a(matrix_a);
    buffer b(matrix_b);
    buffer c(matrix_c);
    buffer ba(matrix_ba),bb(matrix_bb);
  
    q.submit([&](handler &h){
      accessor A(a, h, read_only);
      accessor bA(ba, h, write_only);
       

      h.parallel_for(nd_range<1>(basize,bs), [=](nd_item<1> item) {

            const int i = item.get_global_id(0);
            
            const int x = item.get_local_id(0);
            
            int r=i/((n+31)/32);
          int bc=i%((n+31)/32);
          unsigned int bi=0;
          int astart=bc*32;
          for(int c=0;c<32;c++)
          {
              bi=bi<<1;
              int t=r*n+astart+c;
              if(astart+c<n)
                  bi=bi|(A[t]>0?0:1);
          }
          bA[i]=bi;
            
      });
    });
        
        
    q.submit([&](handler &h){
        accessor B(b, h, read_only);
        accessor bB(bb, h, write_only);

      h.parallel_for(nd_range<1>(bbsize,bbsize), [=](nd_item<1> item) {

            const int i = item.get_global_id(0);
            if(i>=bbsize)return;
            const int x = item.get_local_id(0);
            int c=i%m;
          int br=i/m;
          int rs=br*32,re=rs+32;
          unsigned int t=0;
          for(int r=rs;r<re;r++)
          {
              t=t<<1;
              int idx=r*m+c;
              if(r<n)
                  t=t|(B[idx]>0?0:1);
              
          }
          bB[i]=t;
            
            
      });
   });
    q.submit([&](handler &h){
        accessor bB(bb, h, read_only);
        accessor bA(ba, h, read_only);
        accessor C(c, h, write_only);
      h.parallel_for(nd_range<1>(bs*m,bs), [=](nd_item<1> item) {

            const int i = item.get_global_id(0);
            
            const int x = item.get_local_id(0);
              int c=i%m;
              int r=i/m;
              int k=(n+31)/32;
              unsigned int v=0;
              for(int x=0;x<k;x++)
              {
                  unsigned int tt=bA[r*k+x]^bB[x*m+c];
                        tt = tt - ((tt >> 1) & 0x55555555);
                        tt = (tt & 0x33333333) + ((tt >>2) & 0x33333333);
                        tt = (tt + (tt >> 4)) & 0x0f0f0f0f;
                        tt = tt + (tt >>8);
                        tt = tt + (tt >>16);
                        tt = tt & 0x3f;
                  /*int bitc=0;
                  for(int j=0;j<32;j++)
                  {
                      if(tt&(1<<j))bitc++;
                  }*/ 
                    
                  v+=tt;
              }
              C[i]=(float)v;
            
            
      });
   });
    
    host_accessor ha(ba, read_only);
    host_accessor hb(bb, read_only);
    host_accessor hc(c, read_only);
    
    //# Print Output and Verification
    auto FAIL = 0;
    std::cout<<std::hex;
    for(int i=0;i<bs*m;i++)
    {
        if(i%m==0)std::cout<<std::endl;
        std::cout<<matrix_c[i]<<" ";
        
    }
    for(int i=0;i<basize;i++)
    {
        if(i%((n+31)/32)==0)std::cout<<std::endl;
        std::cout<<matrix_ba[i]<<" ";
        
    }
    for(int i=0;i<bbsize;i++)
    {
        if(i%m==0)std::cout<<std::endl;
        std::cout<<matrix_bb[i]<<" ";
        
    }
    
    if(FAIL == 1) std::cout << "FAIL\n"; else std::cout << "PASS\n";

    return 0;
}

