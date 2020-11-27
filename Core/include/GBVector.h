#pragma once

namespace GenericBoson
{
	template<typename T>
	class GBVector3
	{
	public: T x, y, z;
	public: GBVector3(T px, T py, T pz) : x(px), y(py), z(pz) {}
	};
}