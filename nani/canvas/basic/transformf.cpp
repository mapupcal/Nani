#include "transformf.h"
namespace nani::canvas::basic
{
	namespace
	{
		const TransformF ApplyTransformInOrder(const TransformF& current, const TransformF& next)
		{
			TransformF result;

			result.m11 = next.m11 * current.m11 + next.m12 * current.m21;
			result.m12 = next.m11 * current.m12 + next.m12 * current.m22;
			result.dx = next.m11 * current.dx + next.m12 * current.dy + next.dx;

			result.m21 = next.m21 * current.m11 + next.m22 * current.m21;
			result.m22 = next.m21 * current.m12 + next.m22 * current.m22;
			result.dy = next.m21 * current.dx + next.m22 * current.dy + next.dy;

			return result;
		}
	}

	TransformF::TransformF()
		: m11(1.0f), m12(0.0f), dx(0.0f)
		, m21(0.0f), m22(1.0f), dy(0.0f)
	{

	}

	TransformF::~TransformF()
	{

	};

	const TransformF TransformF::Translation(scalar dx, scalar dy)
	{
		TransformF t;
		t.dx = dx;
		t.dy = dy;
		return t;
	}

	const TransformF TransformF::Rotation(scalar angle)
	{
		TransformF r;
		scalar cosA = std::cos(angle);
		scalar sinA = std::sin(angle);
		r.m11 = cosA;
		r.m12 = -sinA;
		r.m21 = sinA;
		r.m22 = cosA;
		return r;
	}

	const TransformF TransformF::Scaling(scalar kx, scalar ky)
	{
		TransformF s;
		s.m11 = kx;
		s.m22 = ky;
		return s;
	}

	const TransformF TransformF::Shearing(scalar khx, scalar khy)
	{
		TransformF sh;
		sh.m12 = khx;
		sh.m21 = khy;
		return sh;
	}

	const TransformF TransformF::RotationAround(const PointF& anchor, scalar angle)
	{
		scalar cosA = std::cos(angle);
		scalar sinA = std::sin(angle);

		TransformF result;
		result.m11 = cosA;
		result.m12 = -sinA;
		result.m21 = sinA;
		result.m22 = cosA;

		result.dx = anchor.x - cosA * anchor.x + sinA * anchor.y;
		result.dy = anchor.y - sinA * anchor.x - cosA * anchor.y;

		return result;
	}

	const TransformF TransformF::ScalingAround(const PointF& anchor, scalar kx, scalar ky)
	{
		TransformF result;
		result.m11 = kx;
		result.m12 = 0.0f;
		result.m21 = 0.0f;
		result.m22 = ky;

		result.dx = anchor.x * (1.0f - kx);
		result.dy = anchor.y * (1.0f - ky);

		return result;
	}

	const TransformF TransformF::ShearingAround(const PointF& anchor, scalar khx, scalar khy)
	{
		TransformF result;
		result.m11 = 1.0f;
		result.m12 = khx;
		result.m21 = khy;
		result.m22 = 1.0f;

		result.dx = -khx * anchor.y;
		result.dy = -khy * anchor.x;

		return result;
	}

	const TransformF TransformF::Compose(std::initializer_list<TransformF> transforms)
	{
		TransformF result;
		for (const auto& t : transforms)
		{
			result.Apply(t);
		}
		return result;
	}

	TransformF& TransformF::Apply(const TransformF& transform)
	{
		TransformF result = ApplyTransformInOrder(*this, transform);
		*this = result;
		return *this;
	}

	const TransformF TransformF::Reversed() const
	{
		scalar det = m11 * m22 - m12 * m21;
		if (IsScalarEqual(det, 0))
		{
			return TransformF();
		}

		TransformF inv;
		scalar invDet = 1.0f / det;

		inv.m11 = m22 * invDet;
		inv.m12 = -m12 * invDet;
		inv.m21 = -m21 * invDet;
		inv.m22 = m11 * invDet;

		inv.dx = (m12 * dy - dx * m22) * invDet;
		inv.dy = (dx * m21 - m11 * dy) * invDet;

		return inv;
	}

	const TransformF TransformF::Then(const TransformF& transform) const
	{
		TransformF result = *this;
		result.Apply(transform);
		return result;
	}

	const PointF TransformF::ApplyTo(const PointF& pos) const
	{
		PointF result;
		result.x = m11 * pos.x + m12 * pos.y + dx;
		result.y = m21 * pos.x + m22 * pos.y + dy;
		return result;
	}

	const PolygonF TransformF::ApplyTo(const PolygonF& polygon) const
	{
		PolygonF result;
		result.vertices.reserve(polygon.vertices.size());

		for (const auto& vertex : polygon.vertices)
		{
			result.vertices.push_back(ApplyTo(vertex));
		}

		return result;
	}

	TransformF& TransformF::operator=(const TransformF& other)
	{
		m11 = other.m11;
		m12 = other.m12;
		dx = other.dx;
		m21 = other.m21;
		m22 = other.m22;
		dy = other.dy;
		return *this;
	}
}