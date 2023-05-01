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
