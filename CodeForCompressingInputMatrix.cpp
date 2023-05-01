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
