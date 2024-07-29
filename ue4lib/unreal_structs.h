#pragma once
#include <iostream>

struct FVectorA {
	double x = 0, y = 0, z = 0;
	FVectorA() {}
	inline FVectorA(double xx, double yy, double zz) : x(xx), y(yy), z(zz) {}

	inline FVectorA operator + (const FVectorA& other) const { return FVectorA(x + other.x, y + other.y, z + other.z); }

	inline FVectorA operator - (const FVectorA& other) const { return FVectorA(x - other.x, y - other.y, z - other.z); }

	inline FVectorA operator * (double scalar) const { return FVectorA(x * scalar, y * scalar, z * scalar); }

	inline FVectorA operator * (const FVectorA& other) const { return FVectorA(x * other.x, y * other.y, z * other.z); }

	inline FVectorA operator / (double scalar) const { return FVectorA(x / scalar, y / scalar, z / scalar); }

	inline FVectorA operator / (const FVectorA& other) const { return FVectorA(x / other.x, y / other.y, z / other.z); }

	inline FVectorA& operator=  (const FVectorA& other) { x = other.x; y = other.y; z = other.z; return *this; }

	inline FVectorA& operator+= (const FVectorA& other) { x += other.x; y += other.y; z += other.z; return *this; }

	inline FVectorA& operator-= (const FVectorA& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }

	inline FVectorA& operator*= (const double other) { x *= other; y *= other; z *= other; return *this; }


	inline bool operator !=(const FVectorA& other) { return (!(x == other.x && y == other.y && z == other.z)); };

	inline double Dot(const FVectorA& b) const { return (x * b.x) + (y * b.y) + (z * b.z); }

	inline float MagnitudeSqr() const { return Dot(*this); }

	inline double Magnitude() const { return sqrtf(MagnitudeSqr()); }

	double Distance(FVectorA src) {
		FVectorA delta;
		delta.x = x - src.x;
		delta.y = y - src.y;
		delta.z = z - src.z;

		return sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z) / 10.0f;
	}

	inline FVectorA Unit() const
	{
		const double fMagnitude = Magnitude();
		return FVectorA(x / fMagnitude, y / fMagnitude, z / fMagnitude);
	}

	friend bool operator==(const FVectorA& first, const FVectorA& second) { return first.x == second.x && first.y == second.y && first.z == second.z; }

	friend bool operator!=(const FVectorA& first, const FVectorA& second) { return !(first == second); }
};
struct FQuat
{
	float                                              X;                                                         // 0x0000(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                              Y;                                                         // 0x0004(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                              Z;                                                         // 0x0008(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                              W;                                                         // 0x000C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

};
struct FRotator {
	double Pitch = 0, Yaw = 0, Roll = 0;
	inline FRotator() : Pitch(0), Yaw(0), Roll(0) {}

	inline FRotator(double spitch, double syaw, double sroll) : Pitch(spitch), Yaw(syaw), Roll(sroll) {}

	inline double Size() { return sqrt(Pitch * Pitch + Yaw * Yaw + Roll * Roll); }

	inline FRotator Clamp()
	{
		FRotator r = { Pitch, Yaw, Roll };
		if (r.Yaw > 180.f)
			r.Yaw -= 360.f;
		else if (r.Yaw < -180.f)
			r.Yaw += 360.f;

		if (r.Pitch > 180.f)
			r.Pitch -= 360.f;
		else if (r.Pitch < -180.f)
			r.Pitch += 360.f;

		if (r.Pitch < -89.f)
			r.Pitch = -89.f;
		else if (r.Pitch > 89.f)
			r.Pitch = 89.f;

		r.Roll = 0.f;
		return r;
	}

	inline FRotator operator + (const FRotator& other) const { return FRotator(Pitch + other.Pitch, Yaw + other.Yaw, Roll + other.Roll); }

	inline FRotator operator - (const FRotator& other) const { return FRotator(Pitch - other.Pitch, Yaw - other.Yaw, Roll - other.Roll); }

	inline FRotator operator * (double scalar) const { return FRotator(Pitch * scalar, Yaw * scalar, Roll * scalar); }

	inline FRotator operator * (const FRotator& other) const { return FRotator(Pitch * other.Pitch, Yaw * other.Yaw, Roll * other.Roll); }

	inline FRotator operator / (double scalar) const { return FRotator(Pitch / scalar, Yaw / scalar, Roll / scalar); }

	inline FRotator operator / (const FRotator& other) const { return FRotator(Pitch / other.Pitch, Yaw / other.Yaw, Roll / other.Roll); }

	inline FRotator& operator=  (const FRotator& other) { Pitch = other.Pitch; Yaw = other.Yaw; Roll = other.Roll; return *this; }

	inline FRotator& operator+= (const FRotator& other) { Pitch += other.Pitch; Yaw += other.Yaw; Roll += other.Roll; return *this; }

	inline FRotator& operator-= (const FRotator& other) { Pitch -= other.Pitch; Yaw -= other.Yaw; Roll -= other.Roll; return *this; }

	inline FRotator& operator*= (const double other) { Pitch *= other; Yaw *= other; Roll *= other; return *this; }

	inline bool operator !=(const FRotator& other) { return (!(Pitch == other.Pitch && Yaw == other.Yaw && Roll == other.Roll)); };

	inline bool operator ==(const FRotator& other) { return ((Pitch == other.Pitch && Yaw == other.Yaw && Roll == other.Roll)); };



	double Distance2D(const FRotator vec) {
		float diffY = Pitch - vec.Pitch;
		float diffX = Yaw - vec.Yaw;
		return sqrt((diffY * diffY) + (diffX * diffX));
	}

};

struct Vector2 {
	double x = 0, y = 0;

	inline Vector2& operator=  (const Vector2& other) { x = other.x; y = other.y;  return *this; }

	inline Vector2& operator/  (const double other) { x /= other; y /= other;  return *this; }


	double Distance(const Vector2 vec) {
		double diffY = y - vec.y;
		double diffX = x - vec.x;
		return sqrt((diffY * diffY) + (diffX * diffX));
	}

};

struct FPlane : public FVectorA
{
	float                                              W;                                                         // 0x000C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

};

struct FMatrix
{
	struct FPlane                                      XPlane;                                                    // 0x0000(0x0010) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	struct FPlane                                      YPlane;                                                    // 0x0010(0x0010) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	struct FPlane                                      ZPlane;                                                    // 0x0020(0x0010) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	struct FPlane                                      WPlane;                                                    // 0x0030(0x0010) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

};

struct FTransform
{
	struct FQuat                                       Rotation;                                                  // 0x0000(0x0010) (Edit, BlueprintVisible, SaveGame, IsPlainOldData, NoDestructor, NativeAccessSpecifierPublic)
	struct FVectorA                                     Translation;                                               // 0x0010(0x000C) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData_UZUZ[0x4];                                     // 0x001C(0x0004) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	struct FVectorA                                     Scale3D;                                                   // 0x0020(0x000C) (Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	unsigned char                                      UnknownData_RRCR[0x4];                                     // 0x002C(0x0004) MISSED OFFSET (PADDING)

};

class FLinearColor {
public:
	float r, g, b, a;

	FLinearColor() : r{ 0.f }, g{ 0.f }, b{ 0.f }, a{ 0.f } { }
	FLinearColor(const float r, const float g, const float b, const float a) : r{ r }, g{ g }, b{ b }, a{ a } { }
};