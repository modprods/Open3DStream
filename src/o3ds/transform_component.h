#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include "o3ds/math.h"

namespace O3DS
{
	enum ComponentType { TTranslation, TRotation, TOrientation, TScale, TMatrix };

	class TransformComponent
	{
	public:
		virtual ~TransformComponent() {};

		virtual Matrixd asMatrix() = 0;

		Matrixd operator *(Matrixd &other)
		{
			return this->asMatrix() * other;
		}

		virtual enum ComponentType transformType() = 0;
	};

	class TransformTranslation : public TransformComponent
	{
	public:
		TransformTranslation()
			: value(0.0, 0.0, 0.0) {}

		TransformTranslation(Vector3d v)
			: value(v) {}

		TransformTranslation(double x, double y, double z) 
			: value(x, y, z) {}

		virtual ~TransformTranslation() {};

		Matrixd asMatrix() override
		{
			return Matrixd::TranslateXYZ(value[0], value[1], value[2]);
		}

		enum ComponentType transformType() override { return TTranslation;  }

		double delta() { return dist(value, lastSentValue); }

		void sent() { lastSentValue = value; }

		Vector3d value;
		Vector3d lastSentValue;
	};

	class TransformRotation : public TransformComponent
	{
	public:
		TransformRotation()
			: value(0.0, 0.0, 0.0, 1.0) {}

		TransformRotation(Vector4d v)
			:value(v) {}

		TransformRotation(double x, double y, double z, double w)
			: value(x, y, z, w) {}

		virtual ~TransformRotation() {};

		Matrixd asMatrix() override
		{
			return Matrixd::Quaternion(value);
		}

		enum ComponentType transformType() override { return TRotation; }

		double delta() { return dist(value, lastSentValue); }

		void sent() { lastSentValue = value; }

		Vector4d value;
		Vector4d lastSentValue;
	};

	class TransformScale : public TransformComponent
	{
	public:
		TransformScale()
			: value(1.0, 1.0, 1.0) {}

		TransformScale(Vector3d v)
			: value(v) {}

		TransformScale(double x, double y, double z)
			: value(x, y, z) {}

		virtual ~TransformScale() {};

		Matrixd asMatrix() override
		{
			return Matrixd::Scale(value);
		}

		enum ComponentType transformType() override { return TScale; }

		double delta() { return dist(value, lastSentValue); }

		void sent() { lastSentValue = value; }

		Vector3d value;
		Vector3d lastSentValue;
	};

	class TransformMatrix : public TransformComponent
	{
	public:
		TransformMatrix(O3DS::Matrixd m)
			: value(m) {}

		TransformMatrix(double m[])
			: value(m) {}

		TransformMatrix()
			: value(1.0, 0.0, 0.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0,
				0.0, 0.0, 0.0, 1.0) {}

		virtual ~TransformMatrix() {};

		Matrixd asMatrix() override
		{
			return value;
		}

		enum ComponentType transformType() override { return TMatrix; }

		Matrixd value;
	};

}

#endif