#include <bits/stdc++.h>
#include "sha256.h"
#include "hmac.h"
#include "client.cpp"
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
ll y,Y,X;
ll fk=20063,hk;
ll stkr;
session_key sekr;
ll kr=539872;
Client client;


void send(string s){
	char data[s.size()];
	strcpy(data,s.c_str());
	client.sendData(data,s.size());
	client.getAck();
}

ll receive(){
	char* in = client.receiveData();
	client.sendAck();
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
	hk = fixedLenRand(hkLen);
	g = randll(prime-1);
	y = randll(prime-1);
	Y = power(g,y,prime);

	cout<<"hk="<<hk<<endl;
	cout<<"g="<<g<<endl;
	cout<<"Y="<<Y<<endl;

	send(to_string(hk));
	send(to_string(g));
	send(to_string(Y));


	stkr = y;
	sekr = session_key(0,fk);
	cout<<"kr="<<kr<<endl;
	cout<<"---------------------------\n\n";
}

void rke(ll iterNum,pair<ll,ll>& msg)
{
	ll X = msg.first, updateTag = msg.second;
	cout<<"X="<<X<<endl;
	cout<<"updateTag="<<updateTag<<endl;
	cout<<"key="<<sekr.key<<endl;

	ll newup = compute_hash(to_string(X),to_string(sekr.key));
	if (newup != updateTag) 
	{
		cout<<"rejected\n";
		return;
	}
	cout<<"accepted\n";

	string in = to_string(iterNum);
	in += to_string(updateTag);
	in += to_string(X);
	in += to_string(power(X,stkr,prime));
	string s = hmac<SHA256>(in,to_string(hk));

	char s1[keyLen+1];
	char s2[fkLen+1];
	for(ll i=0;i<keyLen;i++) s1[i]=s[i];
	for(ll i=0;i<fkLen;i++) s2[i]=s[i+keyLen];
	s1[keyLen] = '\0';
	s2[fkLen] = '\0';

	ll new_kr =  strtol(s1,NULL,16);
	ll new_fk = strtol(s2,NULL,16);
	cout<<"new kr : "<<new_kr<<"\n";
	cout<<"new fk : "<<new_fk<<"\n";

	sekr = session_key(iterNum+1,new_fk);
	kr = new_kr;
}

int main()
{
	srand(time(NULL));
	client.connectToServer();

	ike();
	ll iter = 0;
	while(1){
		ll X = receive();
		ll upTag = receive();
		pair<ll,ll> msg = make_pair(X,upTag);
		rke(iter++,msg);

		cout<<"---------------------------\n\n";
	}
}
