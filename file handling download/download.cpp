#include<iostream>
#include<fstream>
#include<stdio.h>
#include<stdlib.h>
using namespace std;
int main()
{
        ifstream fs;
        ofstream ft;
        char ch, fname1[20], fname2[20];
        cout<<"Enter source file name in Server---> ";
        cin>>fname1;
        fs.open(fname1);
	cout<<"FileSize"<<sizeof(fname1);
        if(!fs)
        {
                cout<<"---ERROR---"<<endl;
                cout<<"File Not Found";
                exit(1);
        }
        cout<<"Enter the file name to copy---> ";
        cin>>fname2;
        ft.open(fname2);
	const char* dst="D:\\web";
        if(!ft)
        {
                cout<<"Error in opening target file..!!";
                fs.close();
                exit(2);
        }
        while(fs.eof()==0)
        {
                fs>>ch;
                ft<<ch;
        }
        cout<<"File Downloaded successfully..!!"<<endl;

        fs.close();
        ft.close();
        return 0;
}
