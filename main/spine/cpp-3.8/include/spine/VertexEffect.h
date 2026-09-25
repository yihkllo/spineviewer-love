
#ifndef Spine_VertexEffect_h
#define Spine_VertexEffect_h

#include <spine/SpineObject.h>
#include <spine/MathUtil.h>

namespace spine {

class Skeleton;
class Color;

class SP_API VertexEffect: public SpineObject {
public:
	virtual void begin(Skeleton& skeleton) = 0;
	virtual void transform(float& x, float& y, float &u, float &v, Color &light, Color &dark) = 0;
	virtual void end() = 0;
};

class SP_API JitterVertexEffect: public VertexEffect {
public:
	JitterVertexEffect(float jitterX, float jitterY);

	void begin(Skeleton& skeleton);
	void transform(float& x, float& y, float &u, float &v, Color &light, Color &dark);
	void end();

	void setJitterX(float jitterX);
	float getJitterX();

	void setJitterY(float jitterY);
	float getJitterY();

protected:
	float _jitterX;
	float _jitterY;
};

class SP_API SwirlVertexEffect: public VertexEffect {
public:
	SwirlVertexEffect(float radius, Interpolation &interpolation);

	void begin(Skeleton& skeleton);
	void transform(float& x, float& y, float &u, float &v, Color &light, Color &dark);
	void end();

	void setCenterX(float centerX);
	float getCenterX();

	void setCenterY(float centerY);
	float getCenterY();

	void setRadius(float radius);
	float getRadius();

	void setAngle(float angle);
	float getAngle();

	void setWorldX(float worldX);
	float getWorldX();

	void setWorldY(float worldY);
	float getWorldY();

protected:
	float _centerX;
	float _centerY;
	float _radius;
	float _angle;
	float _worldX;
	float _worldY;

	Interpolation& _interpolation;
};
}

#endif
