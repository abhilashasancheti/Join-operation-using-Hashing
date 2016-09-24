/******
Author: Abhilasha Sancheti
******/

#include <fstream>
#include <iostream>
#include <string>
#include <bits/stdc++.h>

using namespace std;
 
int record1_size,record2_size,page_size,buffersize;

int calc_hash_value(int key,int r,int m)
{
    int t= r+m;int value=0,k;
   for(int i=0;i<t;i++)
  {
     k=(1&key)*(pow(2,i));
    // cout<<r<<endl;
	key=key/2;
     value+=k;
  } 
   //int value = (int)(key/(pow(2,r)) )% m;
   return (value)%m;
}

int  matchpairs(int r1size,int r2size)
{
   if(r1size+ r2size < buffersize)
   return 1;
   else return 0;
}

void writepage(vector<int> &b1,string op)
{
    
   ofstream outfile;
  outfile.open(op.c_str() ,ios::app);
  for(int f=0;f<b1.size();f++)
  outfile << b1[f] << endl;
  outfile.close();
  b1.clear();

}

vector<int> readpage( ifstream &r, int nr)
{
    vector<int> page;
   int data;
    for(int i=0;i<nr;i++)
     {
         r>>data;
           if(r.eof())
            break;
           else
            {
              page.push_back(data);
            } 
     }
  return page;
   
}

void printresult(vector<int> *b1,string name1 ,string name2,int k,int m, int nr1,int nr2)
{
   cout<<"Performing inmemory join"<<endl;
   cout<<"Matching pairs are"<<endl;
   int i,nresult;
   nresult= page_size/(record1_size+record2_size); 
   ifstream infile1,infile2;
   infile1.open(name1.c_str());
   infile2.open(name2.c_str());
   vector<int> out;
   for(i=0;i<k;i++)
     b1[i]=readpage(infile1,nr1);
   for(i=0;i<m;i++)
     b1[k+i]=readpage(infile2,nr2);
   infile1.close();
   infile2.close();
   for(i=0;i<k;i++)
   {
           for(int j=k;j<m+k;j++)
           {
               for(int x =0;x< b1[i].size();x++)
               {
                  for(int l=0;l<b1[j].size();l++)
                  {
                     if(b1[i][x] == b1[j][l])
                        {
                              cout<<"("<<b1[i][x]<<","<<b1[j][l]<<")"<<endl;
			      out.push_back(b1[i][x]);
                              if( out.size() == nresult)
                               {
                                      writepage(out,"result.txt");
                                      out.clear();

                               } 
                         }
                  }
               }
           }
   }
   if(out.size()!=0)
     writepage(out,"result.txt");
     out.clear();
  
}

void printstats(int size1,int size2)
{
   cout<<"size of relation 1: "<<size1<<" pages"<<endl;
   cout<<"size of relation 2: "<<size2<<" pages"<<endl;
   cout<<"total number of available pages: "<<buffersize<<endl;
   cout<<"Number of buckets in hash table: "<<buffersize-1<<endl;
}

void printfilestats1(string name,int r,int *k)
{
     for(int i=0;i<buffersize-1;i++)
     {
         if(k[i]!=0)
         {
            cout<<name<<".round"<<r<<".bucket"<<i+1<<": "<<k[i]<<" pages"<<endl;
         }
     }
     cout<<"----------------------------//-----------------//---------------------//-------------"<<endl;
}
void fillbucket(string name1 , vector<int> *b1 , int r ,int nr1,int size1,int *k)
{
     //vector< int> b1[buffersize-1];
     vector<int> rel1;
      cout<<"Reading "<<name1<<endl;
      int l=1;
       ifstream infile;
      infile.open(name1.c_str());  
      for(int i=0;i<buffersize-1;i++){k[i]=0;b1[i].clear();}
      for(int i=0;i<size1;i++)
      {
          rel1 = readpage(infile,nr1);
       for(int j=0;j<rel1.size();j++)
         {
                int bindex = calc_hash_value(rel1[j],r,buffersize-1);
                if(k[bindex]==0)
		 k[bindex]++;
              if(b1[bindex].size() == nr1)
                {
                  char buffer[1000];
                  sprintf(buffer,  ".round%d.bucket%d",r,bindex+1);
                  writepage(b1[bindex],name1+string(buffer));
                  cout<<"Page for bucket "<<bindex+1<<" full. Flushed to secondary storage"<<endl;
                  b1[bindex].clear();
                  k[bindex]++;
                }
                cout<<"Tuple "<<l++<<": "<<rel1[j]<<" Mapped to bucket "<<bindex+1<<endl;
                b1[bindex].push_back(rel1[j]);
         }
      }
     infile.close();
     for( int i=0;i<buffersize-1;i++)
      {
           if(k[i]!=0)
           {
                  char buffer[1000];
                  sprintf(buffer,  ".round%d.bucket%d",r,i+1); 
                  string op = name1+ string(buffer);
                  writepage(b1[i],op);
                  b1[i].clear(); 
            }
      }
    
      cout<<"Done with "<<name1<<endl;
      cout<<"++++++++++++++++++++++++++//+++++++++++++++++++++//+++++++++++++++++"<<endl;
      cout<<"Created following files"<<endl;
      printfilestats1(name1,r,k);
}

void applyhash(vector<int> *b1,string name1,string name2,int r,int nhash,int size1,int size2,int nr1,int nr2)
{
   
   printstats(size1,size2);
   if(r<=nhash)
   {
      char buffer[1000];
      int k[buffersize-1],m[buffersize-1];
      cout<<endl<<"hashing Round"<<r<<endl;
      fillbucket(name1,b1,r,nr1,size1,k);
      fillbucket(name2,b1,r,nr2,size2,m);
     
      for(int i=0;i< buffersize-1;i++)
      {
          if(k[i] ==0 && m[i] ==0)
          {
            cout<<"Bucket "<<i+1<<": none of the tuples mapped to either "<<name1<<" or "<<name2<<". No further processing required."<<endl;
          }
         else if(k[i] ==0)
         {
            cout<<"Bucket "<<i+1<<": no matching tuples from "<<name1<<". No further processing required."<<endl;
         }
         else if(m[i] ==0)
         {
            cout<<"Bucket "<<i+1<<": no matching tuples from "<<name2<<". No further processing required."<<endl;
         }
         else
         {
             char buffer1[1000];
               sprintf(buffer1,  ".round%d.bucket%d",r,i+1); 
              string name11 = name1 + string(buffer1);
	      string name21 = name2 + string(buffer1);
              if(matchpairs(k[i],m[i]))
               { 
                  cout<<"Bucket "<<i+1<<": Total size is "<<k[i]+m[i]<<" pages. ";
                  printresult(b1,name11,name21,k[i],m[i],nr1,nr2);
                 
               }
            else
            {
               cout<<"Bucket "<<i+1<<": Total size is "<<k[i]+m[i]<<" pages. Cannot perform inmemory join."<<endl;  
		if(r+1 >nhash)
{
   printstats(k[i],m[i]);
cout<<"Cannot compute join "<<name11<<" and "<<name21<<" do not fit in memory and numnber of hashes reached maximum limit."<<endl;  
      return; 
}
else{               cout<<"Performing hash round "<<r+1<<" for round"<<r<<".bucket"<<i+1<<endl;    
               applyhash(b1,name11,name21,r+1,nhash,k[i],m[i],nr1,nr2); }
               cout<<"***************************************-----------------------------***********************"<<endl;
            }

         }
      }
   }
   else
   {
      cout<<"Cannot compute join "<<name1<<" and "<<name2<<" do not fit in memory"<<endl;  
      return; 
    }
}


int main()
 {
   string relation1,relation2;
   int nhash,n1=0,n2=0;
   int size1,size2,data;
   cin>>record1_size>>record2_size>>page_size>>buffersize>>nhash;
   vector<int> b1[buffersize-1];
   ifstream infile1,infile2; 
   infile1.open("relation1"); 
   infile2.open("relation2");
   cout << "Reading from the file1" << endl; 
   while(1)
   { 
      infile1 >> data; 
      if(infile1.eof())
      break;
      n1++;
      cout << data << endl;
     
   }
   cout << "Reading from the file2" << endl; 
   while(1)
   { 
      infile2 >> data; 
      if(infile2.eof())
      break;
      n2++;
      cout << data << endl;
      
   }
   infile1.close();
   infile2.close();
   int nr1 = page_size/record1_size;
   int nr2 = page_size/record2_size;
   int extp1 = n1%nr1;
   int extp2 = n2%nr2;
   if ( extp1>0)
      size1 = 1 + n1/nr1;
   else
      size1 = n1/nr1;
   if ( extp2>0)
      size2 = 1 + n2/nr2;
   else
      size2 = n2/nr2;

  if(matchpairs(size1,size2))
   {
      cout<<"size of relation 1: "<<size1<<" pages"<<endl;
      cout<<"size of relation 2: "<<size2<<" pages"<<endl;
      cout<<"total number of available pages: "<<buffersize<<endl;
      cout<<"Number of buckets in hash table: "<<buffersize-1<<endl;
      printresult(b1,"relation1","relation2",size1,size2,nr1,nr2);
   
   }else
     applyhash(b1,"relation1","relation2",1,nhash,size1,size2,nr1,nr2);
}
