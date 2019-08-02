// arduinoSender.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include "rs232.h"
#include "LELSender.h"

LELSender sender(1, 19970);

bool writeToFile=true;
bool sendFilteredValue=true;

#define real float
//struct vec2;
//typedef const vec2& crv2;
#define crv2 vec2&
#define rvc2 vec2&
//typedef vec2& rvc2;
struct vec2
{
	union {
		struct {
			real x, y;
		};
		real p[2];
	};
	inline void set(const real v[])		{ x=v[0]; y=v[1]; }
	inline void set(real a,real b) { x=a, y=b; }
	inline void get(real v[]) const		{ v[0]=x,v[1]=y; }
	inline real& operator[](int i)		{ return p[i]; }
	inline void zero(void)				{ x=y=0; }
	
	inline vec2(void)					{ zero(); }
	inline vec2(real a,real b)			{ set(a,b); }
	inline vec2(const real a[])			{ set(a); }
	inline vec2(crv2 a)					{ x=a.x; y=a.y; }
	
	inline vec2 operator- (void)const	{ return vec2(-x,-y); }
	
	inline rvc2 operator+=(crv2 a)		{ x+=a.x, y+=a.y; return *this; }
	inline rvc2 operator-=(crv2 a)		{ x-=a.x, y-=a.y; return *this; }
	inline rvc2 operator*=(real a)		{ x*=a; y*=a; return *this; }
	inline rvc2 operator/=(real a)		{ x/=a; y/=a; return *this; }
	inline vec2	operator+ (crv2 a)const { return vec2(x+a.x,y+a.y); }
	inline vec2	operator- (crv2 a)const { return vec2(x-a.x,y-a.y); }
	inline real	operator% (crv2 a)const { return x*a.x+y*a.y; }
	inline vec2	operator* (real a)const { return vec2(x*a,y*a); }
	inline vec2	operator/ (real a)const { return vec2(x/a,y/a); }
	/*friend inline vec2	operator*(real a, crv2 v) { return v*a; }
	
	inline real sqlen(void)const		{ return x*x+y*y; }
	inline real len  (void)const		{ return sqrtf(sqlen()); }
	inline vec2 unit (void)const
	{	real l=sqlen();
		if(l<EPS) return vec2(0,0);
		if(l<1+EPS&&l>1-EPS) return *this;
		return *this/sqrtf(l);
	}
	inline rvc2 normalize(void)			{ return *this=unit(); }
	friend inline real len  (crv2 a)	{ return a.len(); }
	friend inline real sqlen(crv2 a)	{ return a.sqlen(); }
	friend inline vec2 unit (crv2 a)	{ return a.unit(); }
	friend inline vec2 normalize(crv2 a){ return a.unit(); }
	
	friend inline std::ostream& operator <<(std::ostream& s,crv2 a)
	{ s<<a.x<<", "<<a.y<<" "; return s; }
	friend inline std::istream& operator >>(std::istream& s,rvc2 a)
	{ s>>a.x>>a.y; return s; }
	inline std::string toString(void)const
	{ std::stringstream s; s<<*this; return s.str(); }
	friend inline std::string toString(crv2 a) { return a.toString(); }
	friend inline vec2 toVec2(std::string str)
	{ vec2 v; std::stringstream s(str); s>>v; return v; }
	friend inline vec2 delDOF(crv2 v, crv2 uax){ return v-(v%uax)*uax; }*/
};


struct mat2;
typedef const mat2& crm2;
typedef mat2& rmt2;
struct mat2
{
	union
	{
		struct{
			real a,b,c,d;
		};
		struct{
			real m00,m01,m10,m11;
		};
		real p[4];
	};
	mat2(void):a(1),b(0),c(0),d(1){}
	mat2(float _a,float _b,float _c,float _d):a(_a),b(_b),c(_c),d(_d){}
	mat2(float x[]):a(x[0]),b(x[1]),c(x[2]),d(x[3]){}
	operator float* (void) {return p;}
	
	rmt2 operator+=(crm2 k){a+=k.a; b+=k.b; c+=k.c; d+=k.d; return *this;}
	rmt2 operator-=(crm2 k){a-=k.a; b-=k.b; c-=k.c; d-=k.d; return *this;}
	rmt2 operator*=(real f){ a*=f,b*=f,c*=f,d*=f; return *this;}
	rmt2 operator/=(real f){ a/=f,b/=f,c/=f,d/=f; return *this;}
	rmt2 operator*=(crm2 k)
	{
		float k00 = a*k.a+b*k.c,
		k01 = a*k.b+b*k.d,
		k10 = c*k.a+d*k.c,
		k11 = c*k.b+d*k.d;
		a=k00, b=k01, c=k10, d=k11;
		return *this;
	}
	
	mat2 operator- (void) { return mat2(-a,-b,-c,-d); }
	mat2 operator+ (crm2 k)const{return mat2(a+k.a,b+k.b,c+k.c,d+k.d);}
	mat2 operator- (crm2 k)const{return mat2(a-k.a,b-k.b,c-k.c,d-k.d);}
	mat2 operator* (crm2 k)const
	{ return mat2(a*k.a+b*k.c,a*k.b+b*k.d,c*k.a+d*k.c,c*k.b+d*k.d);}
	mat2 operator* (real f)const{ return mat2(a*f,b*f,c*f,d*f);}
	mat2 operator/ (real f)const{ return mat2(a/f,b/f,c/f,d/f);}
	mat2 t() const {return mat2(a,c,b,d);}
	friend mat2 t(crm2 m){ return m.t();}
	vec2 operator* (crv2 v)const{ return vec2(a*v.x+b*v.y,c*v.x+d*v.y);}
};

struct Filter1D
{
	bool inited;
	vec2 x;
	mat2 P, A, Q;
	float R;
	
	Filter1D(void): inited(false){}
	void predict(void)
	{
		x = A*x;
		P = A*P*A.t()+Q;
	}
	void update(float z)
	{
		// y = z-Hx
		float y = z-x.x;
		// S = HPH^T + R
		float S = P.a+R;
		// K = P H^TS^-1
		vec2 K = P*vec2(1.0f/S,0);
		//x = x+ Ky
		x = x+ K*y;
		// P = (I-KH)P
		P = mat2(1-K.x,0,K.y,0)*P;
	}
	float filter(float z)
	{
		if( !inited ) { x.x=z; x.y=0; inited = true; }
		predict();
		update(z);
		return x.x;
	}
	void reset(void) {inited=false;}
};

std::vector<float> input, output;
Filter1D filter;

//float sigma_z = 0.0001f;
//float sigma_alpha=0.004f;
float sigma_z = 0.00001f;
float sigma_alpha=.004f;

int sample=0;
int numSamples=300;
float threshold = 0;
int peakCount = 0;

//store prev value
float pValue=0;

float filterValue(float input)
{
	float dt = 0.03333f;
	filter.A = mat2(1,dt,0,1); // [ 1 \Delta t]
	                           // [ 0     1   ]
	filter.Q = mat2(powf(dt,4)/4,powf(dt,3)/2,powf(dt,3)/2,dt*dt)*sigma_alpha;
	filter.R = sigma_z;
	filter.P = mat2(0.000001f,0,0,0.000001f);
	
	float output=0;


	if (sample<numSamples)
	{
		if( input > pValue )
		{
			threshold+=input;
			peakCount++;
		}
		if (sample%(numSamples/4)==0)
			printf("Sample %f\a\n", sample / float (numSamples));
	}
	else if (sample == numSamples)
	{
		threshold/=peakCount;
		threshold*=1.1;
		filter.reset();

		if (sample%(numSamples/4)==0)
			printf("Sample %f\a\n", sample / float (numSamples));
	}
	else
	{
		output = filter.filter(input-threshold);
		if( output<0 ) output=0;
	}

	sample++;

	

	return output;

}

float avePeakValue=0;
float aveValue=0;
int aveNumSamples=0;
int aveNumPeakSamples=0;
float prevV=0;
void findAveValue(float v)
{
	if (v >= 1/float(1024))
	{
		aveNumSamples++;
		aveValue+=v;
		if (v > prevV)
		{
			aveNumPeakSamples++;
			avePeakValue+=v;
		}
	}
	else
	{
		if (aveNumSamples > 0)
		{
			if (aveNumSamples > 100)
				printf("\n N: %d ave: %f avepeak:%f\n", aveNumSamples, aveValue/float(aveNumSamples), avePeakValue/float(aveNumPeakSamples));
		}
		avePeakValue=0;
		aveValue=0;
		aveNumSamples=0;
		aveNumPeakSamples=0;
	}

	prevV=v;
}


int _tmain(int argc, _TCHAR* argv[])
{
	
	
	const int port=4;//3;
	const int size=128;
	unsigned char buf[size];

	int err = OpenComport(port, 57600);//115200);
	printf("%d\n", err);


	
	SYSTEMTIME systemTime;
	GetSystemTime(&systemTime);

	int num;
	int sensor;
	float value;
	FILE *fp=0;
	char fname[128];
	if (writeToFile)
	{
		//make or clear the file
		printf("WRITING TO FILE\n");
	
		_snprintf(fname, 128, "data/%d-%d-%d_%dh%dms%d.csv", systemTime.wDay, systemTime.wMonth, systemTime.wYear,  systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
		fp = fopen(fname, "w"); 
		if (!fp)
		{
			printf("err open file %s\n", fname);
			return -1;
		}
		fclose(fp);
	}



	while (1)
	{
		num = PollComport(port, buf, size);
		if (writeToFile)
			fp = fopen(fname, "a"); 
		//printf("%d\n", num);
		if (num > 0)
		{
			//printf("%s\n", buf);
			if (sscanf((const char*)buf, "s=%d;", &sensor) > 0)
			{
				value = sensor / float(1024);
				//filter it
				float fv = filterValue(value);
				//
				printf("sensor=%d, %f -> %f\r", sensor, value, fv);

				
				if (sendFilteredValue)
					sender.send(&fv, sizeof(float));
				else
					sender.send(&value, sizeof(float));

				//time-stampit
				
				GetSystemTime(&systemTime);
				fprintf(fp, "%d/%d/%d,", systemTime.wDay, systemTime.wMonth, systemTime.wYear);
				fprintf(fp, "%d:%d:%d:%d,", systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);
				//values
				fprintf(fp, "%f,%f\n", value, fv);
			}
		}

		if (writeToFile)
			fclose(fp);
		Sleep(1);

	}
	

 return 0;


}

