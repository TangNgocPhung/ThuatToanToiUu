#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

int main()
{
    srand(time(NULL));

    int n=rand()%10+1;

    cout<<n<<endl;

    for(int i=1;i<=n;i++)
        cout<<rand()%100<<" ";

    cout<<endl;

    int q=5;

    cout<<q<<endl;

    while(q--)
    {
        int l=rand()%n+1;
        int r=rand()%n+1;

        if(l>r)
            swap(l,r);

        cout<<l<<" "<<r<<endl;
    }
}