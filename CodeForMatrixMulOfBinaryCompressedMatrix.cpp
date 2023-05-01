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
