#pragma once
#include <cmath>
#include <Windows.h>
#include <d3d9types.h>
#include <iostream>



namespace Structs
{
	class UClass {
	public:
		BYTE _padding_0[0x40];
		UClass* SuperClass;
	};

	class UObject {
	public:
		PVOID VTableObject;
		DWORD ObjectFlags;
		DWORD InternalIndex;
		UClass* Class;
		BYTE _padding_0[0x8];
		UObject* Outer;

		inline BOOLEAN IsA(PVOID parentClass) {
			for (auto super = this->Class; super; super = super->SuperClass) {
				if (super == parentClass) {
					return TRUE;
				}
			}

			return FALSE;
		}
	};

	class FUObjectItem {
	public:
		UObject* Object;
		DWORD Flags;
		DWORD ClusterIndex;
		DWORD SerialNumber;
		DWORD SerialNumber2;
	};

	class TUObjectArray {
	public:
		FUObjectItem* Objects[9];
	};

	class GObjects {
	public:
		TUObjectArray* ObjectArray;
		BYTE _padding_0[0xC];
		DWORD ObjectCount;
	};

	inline GObjects* objects = nullptr;
	static GObjects* objectsv2 = nullptr;

	struct FLinearColor
	{
		float R;
		float G;
		float B;
		float A;

		FLinearColor()
		{
			R = G = B = A = 0;
		}

		FLinearColor(float R, float G, float B, float A)
		{
			this->R = R;
			this->G = G;
			this->B = B;
			this->A = A;
		}
	};


	struct FMatrix
	{
		double M[4][4];
	};
	static FMatrix* myMatrix = new FMatrix();

	typedef struct
	{
		double X, Y, Z;
	} FVector;

	struct FVector2D
	{
		FVector2D() : x(0.f), y(0.f)
		{

		}

		FVector2D(double _x, double _y) : x(_x), y(_y)
		{

		}
		~FVector2D()
		{

		}
		double x, y;
	};

	template<class T>
	struct TArray
	{
		friend struct FString;

	public:
		inline TArray()
		{
			Data = nullptr;
			Count = Max = 0;
		};

		inline int Num() const
		{
			return Count;
		};

		inline T& operator[](int i)
		{
			return Data[i];
		};

		inline const T& operator[](int i) const
		{
			return Data[i];
		};

		inline bool IsValidIndex(int i) const
		{
			return i < Num();
		}

	private:
		T* Data;
		int32_t Count;
		int32_t Max;
	};

	class FText {
	private:
		char _padding_[0x28];
		PWCHAR Name;
		DWORD Length;
	public:


		inline PWCHAR c_wstr() {
			return Name;
		}

		inline char* c_str()
		{

			char sBuff[255];
			wcstombs((char*)sBuff, (const wchar_t*)this->Name, sizeof(sBuff));
			return sBuff;
		}
	};

	struct FString : private TArray<wchar_t>
	{
		inline FString()
		{
		};

		FString(const wchar_t* other)
		{
			Max = Count = *other ? std::wcslen(other) + 1 : 0;

			if (Count)
			{
				Data = const_cast<wchar_t*>(other);
			}
		};

		inline bool IsValid() const
		{
			return Data != nullptr;
		}

		inline const wchar_t* c_str() const
		{
			return Data;
		}

		std::string ToString() const
		{
			auto length = std::wcslen(Data);

			std::string str(length, '\0');

			std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

			return str;
		}
	};
}

class Vector3
{
public:
	Vector3();
	Vector3(double, double, double);
	~Vector3();

	double x{ }, y{ }, z{ };

	void make_absolute() {
		x = std::abs(x);
		y = std::abs(y);
		z = std::abs(z);
	}

	Vector3& operator+=(const Vector3& v) 
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	Vector3& operator-=(const Vector3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	Vector3& operator*=(const double v) 
	{
		x *= v;
		y *= v;
		z *= v;
		return *this;
	}

	Vector3 operator+(const Vector3& v) {
		return Vector3{ x + v.x, y + v.y, z + v.z };
	}

	Vector3 operator-(const Vector3& v) {
		return Vector3{ x - v.x, y - v.y, z - v.z };
	}

	Vector3& operator+(const double& v)
	{
		x = x + v;
		y = y + v;
		z = z + v;
		return *this;
	}

	Vector3& operator-(const double& v)
	{
		x = x - v;
		y = y - v;
		z = z - v;
		return *this;
	}

	Vector3 operator*(const double v) 
	{
		return Vector3{ x * v, y * v, z * v };
	}

	Vector3 operator/(const double fl) const
	{
		return Vector3(x / fl, y / fl, z / fl);
	}

	Vector3 operator+(const Vector3& v) const
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	bool operator==(const Vector3& v) const 
	{
		return x == v.x && y == v.y && z == v.z;
	}

	bool operator!=(const Vector3& v) const
	{
		return x != v.x || y != v.y || z != v.z;
	}

	Vector3 operator*(const double fl) const
	{
		return Vector3(x * fl, y * fl, z * fl);
	}

	Vector3 operator*(const Vector3& v) const 
	{
		return Vector3(x * v.x, y * v.y, z * v.z);
	}

	Vector3 operator-(const Vector3& v) const 
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3& operator/=(const double fl)
	{
		x /= fl;
		y /= fl;
		z /= fl;
		return *this;
	}

	double operator[](const int i) const
	{
		return ((double*)this)[i];
	}

	double& operator[](const int i) {
		return reinterpret_cast<double*>(this)[i];
	}


	inline double Customsqrtf_(double x)
	{
		union { double f; uint32_t i; } z = { x };
		z.i = 0x5f3759df - (z.i >> 1);
		z.f *= (1.5f - (x * 0.5f * z.f * z.f));
		z.i = 0x7EEEEEEE - z.i;
		return z.f;
	}


	double Custompowf_(double x, int y)
	{
		double temp;
		if (y == 0)
			return 1;
		temp = Custompowf_(x, y / 2);
		if ((y % 2) == 0) {
			return temp * temp;
		}
		else {
			if (y > 0)
				return x * temp * temp;
			else
				return (temp * temp) / x;
		}
	}

	inline double Distance(Vector3 v)
	{
		return double(Customsqrtf_(Custompowf_(v.x - x, 2.0) + Custompowf_(v.y - y, 2.0) + Custompowf_(v.z - z, 2.0)));
	}

	inline double Length() 
	{
		return (double)(Customsqrtf_(x * x + y * y + z * z));
	}

	[[nodiscard]] bool empty() const {
		return x == 0 || y == 0 || z == 0;
	}
};