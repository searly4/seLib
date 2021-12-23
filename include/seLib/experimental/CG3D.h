#pragma once

//#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <type_traits>
//#include <GLES2/gl2.h>

#include <seLib/Time.h>
#include <seLib/RefHolder.h>
#include <seLib/Vector.h>
#include <seLib/BasicGeometry.h>
#include <seLib/experimental/Matrix.h>

namespace seLib { namespace CG {

// RenderObject_t: renderer-agnostic data source type
// Renderer_t: renderer
// ClassHandler_t: renderer-specific handler for a RenderObject_t derived type
// ObjectHandler_t: ClassHandler_t-specific state tracking object for instances of a RenderObject_t derived type

using Time_t = seLib::TimeValue_t<int32_t, 15>;
using Interval_t = Time_t;

class RenderObject_t {
public:
	virtual ~RenderObject_t() noexcept {}
};


class DisplayList_t {
public:
	std::vector<RenderObject_t const *> Objects;
};


class Renderer_t {
public:
	using Time_t = seLib::CG::Time_t;
	using Interval_t = Time_t;

	virtual ~Renderer_t() {}

	virtual bool Init(Time_t timestamp) noexcept = 0;
	virtual void Resize(uint16_t width, uint16_t height, float xresolution, float yresolution) noexcept { }
	virtual void Render(Time_t timestamp) noexcept = 0;
	virtual void UpdateCamera(seLib::Matrix4f_t const & display_transform) noexcept = 0;
	virtual bool Add(RenderObject_t& obj) = 0;
	virtual bool Remove(RenderObject_t& obj) = 0;
	virtual void Set(DisplayList_t const & list) = 0;
};

}} // namespace seLib::CG
