#include <bits/stdc++.h>
#include "sha256.h"
#include "hmac.h"
#include "server.cpp"
using namespace std;

typedef long long ll; 

#define keyLen 15
#define fkLen 15
#define hkLen 30
#define prime 56479

struct session_key
{
	ll iterationNum;
	ll key;
	session_key(){}
	session_key(ll iterNum,ll key)
	{
		this->iterationNum = iterNum;
		this->key = key;
	}
};

ll g;
ll x,X,Y;
ll fk=20063,hk;
ll stks;
session_key seks;
ll ks=539872;
Server server;

void send(string s){
	char data[s.size()];
	strcpy(data,s.c_str());
	server.sendData(data,s.size());
	server.getAck();
}

ll receive(){
	char* in = server.receiveData();
	server.sendAck();
	string str(in);
	return stoll(str);
}

ll fixedLenRand(ll len)
{
	ll r = rand();
	return r%((1<<len));
}

ll randll(ll maxVal)
{
	ll r = rand();
	return (1+r%maxVal);
}

ll power(ll base, ll exp,ll p1) 
{
  base%=p1;
    ll res=1;
    while(exp>0) {
       if(exp%2==1) res=(res*base)%p1;
       base=(base*base)%p1;
       exp/=2;
    }
    return res%p1;
}

ll compute_hash(string msg,string key)
{
	string s = hmac<SHA256>(msg,key);
	char c[16];
	for(ll i=0;i<15;i++) c[i] = s[i];
	c[15]='\0';
	return strtol(c,NULL,16);
}

void ike()
{	
	hk = receive();
	g = receive();
	Y = receive();

	cout<<"hk="<<hk<<endl;
	cout<<"g="<<g<<endl;
	cout<<"Y="<<Y<<endl;

	stks = Y;
	seks = session_key(0,fk);

	cout<<"---------------------------\n\n";
}

pair<ll,ll> ske(ll iterNum)
{
	x = randll(prime-1);
	X = power(g,x,prime);

	cout<<"X="<<X<<endl;
	cout<<"key="<<seks.key<<endl;

	ll updateTag = compute_hash(to_string(X),to_string(seks.key));
	cout<<"updateTag:"<<updateTag<<"\n";

	string in = to_string(iterNum);
	in += to_string(updateTag);
	in += to_string(X);
	in += to_string(power(stks,x,prime));
	string s = hmac<SHA256>(in,to_string(hk));

	char s1[keyLen+1];
	char s2[fkLen+1];
	for(ll i=0;i<keyLen;i++) s1[i]=s[i];
	for(ll i=0;i<fkLen;i++) s2[i]=s[i+keyLen];
	s1[keyLen] = '\0';
	s2[fkLen] = '\0';

	ll new_ks =  strtol(s1,NULL,16);
	ll new_fk = strtol(s2,NULL,16);
	cout<<"new ks : "<<new_ks<<"\n";
	cout<<"new fk : "<<new_fk<<"\n";

	seks = session_key(iterNum+1,new_fk);
	ks = new_ks;

	return make_pair(X,updateTag);
}

int main()
{
	srand(time(NULL));
	ike();

	ll n,iter=0;
	cin>>n;
	while(n != 0){
		pair<ll,ll> msg;
		if(n==1)msg = ske(iter++);
		else msg = make_pair(randll(1e7),randll(1e7));

		cout<<"sending upd = "<<msg.first<<","<<msg.second<<endl;

		send(to_string(msg.first));
		send(to_string(msg.second));

		cout<<"---------------------------\n\n";

		cin>>n;
	}
}
