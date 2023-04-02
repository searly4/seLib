#pragma once

//#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <map>
//#include <unordered_map>
#include <vector>
#include <string_view>

#if defined(TARGET_GL_GL)
#ifdef _WIN32
#include <Windows.h>
#endif
//#include <gl/GL.h>
#define GLEW_STATIC
#include <gl/glew.h>
//#include <gl/GLU.h>
#elif defined(TARGET_GL_GLES2)
#include <GLES3/gl2.h>
#include <GLES2/gl2ext.h>
#elif defined(TARGET_GL_GLES3)
#include <GLES3/gl3.h>
#else
#error OpenGL target not defined.
#endif

#include <seLib/RefHolder.h>
#include <seLib/Vector.h>
#include <seLib/BasicGeometry.h>
#include <seLib/experimental/Matrix.h>
#include <seLib/experimental/CG3D.h>
#include <seLib/ConstMap.h>
#include <seLib/Iterators.h>
#include <seLib/ArrayAccessor.h>

namespace seLib { namespace CG {

using Matrix4glf_t = seLib::SquareMatrix_t<float, 4>; // fixme: needs to be GLfloat?
using Vector4glf_t = seLib::Vector_t<float, 4>;
using Vector3glf_t = seLib::Vector_t<float, 3>;
using Vector2glf_t = seLib::Vector_t<float, 2>;


#pragma region // Exceptions ======================================================

struct GLException_t : std::exception {
	GLException_t() {}
	virtual std::string_view Message() const noexcept { return "" ; }
};

template <typename Message_T>
struct GLExceptionString_t : GLException_t {
	Message_T MessageValue;
	GLExceptionString_t(Message_T message) : MessageValue(message) {}
	virtual std::string_view Message() const noexcept { return MessageValue ; }
};

#pragma endregion


#pragma region // Shader Attributes ======================================================

struct ShaderAttributeType_t {
	unsigned Type;
	uint8_t ElementSize;
	uint8_t ElementCount;

	constexpr ShaderAttributeType_t(unsigned type, uint8_t element_size, uint8_t element_count)
		: Type(type),
		ElementSize(element_size),
		ElementCount(element_count)
	{}

	explicit constexpr ShaderAttributeType_t(ShaderAttributeType_t const &) = default;
	constexpr bool operator==(ShaderAttributeType_t const & b) const noexcept { return this == &b; }
};

template <typename T>
struct ShaderAttributeType2_t : public ShaderAttributeType_t {
	using DataType_t = T;

	static constexpr size_t TypeSize(unsigned type) {
		switch (type) {
			case GL_BYTE: return sizeof(GLbyte);
			case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
			case GL_SHORT: return sizeof(GLshort);
			case GL_UNSIGNED_SHORT: return sizeof(GLushort);
			case GL_INT: return sizeof(GLint);
			case GL_UNSIGNED_INT: return sizeof(GLuint);
			case GL_FLOAT: return sizeof(GLfloat);

			#ifdef TARGET_GL_GL
			case GL_2_BYTES: return sizeof(int8_t)*2;
			case GL_3_BYTES: return sizeof(int8_t)*3;
			case GL_4_BYTES: return sizeof(int8_t)*4;
			case GL_DOUBLE: return sizeof(sizeof(GLdouble));
			#endif

			default:
				#if defined(__cpp_exceptions) && __cpp_exceptions==199711
					throw GLExceptionString_t<std::string_view>("Invalid GL type");
				#else
					abort();//return Identity<Return_T>();
				#endif
		};
	}

	constexpr ShaderAttributeType2_t(unsigned type)
		: ShaderAttributeType_t(type, (uint8_t)TypeSize(type), (uint8_t)(sizeof(T) / TypeSize(type)))
	{}

	//constexpr ShaderAttributeType2_t(unsigned type, uint8_t element_size)
	//	: ShaderAttributeType_t(type, element_size, sizeof(T) / element_size)
	//{}
};

using ShaderAttributePtr_t = ShaderAttributeType_t const *;
using ShaderAttributeRef_t = ShaderAttributeType_t const &;
/*struct ShaderAttributeRef_t {
	ShaderAttributeType_t const * ref;
	constexpr ShaderAttributeRef_t(ShaderAttributeType_t const & ref) : ref(&ref) {}
	constexpr ShaderAttributeRef_t(ShaderAttributeRef_t const & b) : ref(b.ref) {}
	ShaderAttributeRef_t& operator=(ShaderAttributeRef_t const &) = default;
	constexpr ShaderAttributeType_t const * operator*() const noexcept { return ref; }
	constexpr bool operator==(ShaderAttributeRef_t const & b) const noexcept { return ref == b.ref; }
	constexpr bool operator!=(ShaderAttributeRef_t const & b) const noexcept { return !(*this == b); }
};// */

namespace ShaderAttributes {
inline constexpr ShaderAttributeType2_t<Matrix4glf_t> ViewTransform { GL_FLOAT };
inline constexpr ShaderAttributeType2_t<Matrix4glf_t> ModelTransform { GL_FLOAT };
inline constexpr ShaderAttributeType2_t<Vector3glf_t> Position { GL_FLOAT };
inline constexpr ShaderAttributeType2_t<Vector3glf_t> Position2 { GL_FLOAT };
inline constexpr ShaderAttributeType2_t<Vector3glf_t> ModelScale { GL_FLOAT };
inline constexpr ShaderAttributeType2_t<Vector3glf_t> Color { GL_FLOAT };
inline constexpr ShaderAttributeType2_t<Vector3glf_t> Color2 { GL_FLOAT };
inline constexpr ShaderAttributeType2_t<Vector3glf_t> HighlightColor { GL_FLOAT };
}

/*enum class ShaderAttributes_e {
	Invalid,
	//Position,
	//Position2,
	//Color,
	//HighlightColor,
	//ModelTransform,
	ViewTransform,
	//ProjectionTransform,
	//ModelScale,
	//NodeIndex,
	//PositionOffset,
	//InterpolateFraction,
};// */

//std::map<ShaderAttributes, string>
/*seLib::ConstMap_t attribNames = {
  { ShaderAttributes::Position, "in_Position" },
  { ShaderAttributes::Position2, "in_Position2" },
  { ShaderAttributes::Color, "in_Color" },
  { ShaderAttributes::HighlightColor, "in_Highlight" },
  { ShaderAttributes::NodeIndex, "in_NodeIndex" },
  { ShaderAttributes::ModelTransform, "modelMatrix" },
  { ShaderAttributes::ProjectionTransform, "projectionMatrix" },
  { ShaderAttributes::ViewTransform, "viewMatrix" },
  { ShaderAttributes::ModelScale, "Scale" },
  { ShaderAttributes::PositionOffset, "positionOffset" },
  { ShaderAttributes::InterpolateFraction, "interpolate_fraction" },
};// */

#pragma endregion


struct InterleavedDataBase_t {
	struct Element_t {
		ShaderAttributePtr_t Attrib;
		size_t Offset;
	};

	size_t Stride;

	virtual ~InterleavedDataBase_t() {}

	virtual size_t ElementCount() const noexcept { return 0; }
	virtual Element_t const * GetElement(size_t index) const noexcept {
		(void)index;
		return nullptr;
	}
	virtual Element_t const * begin() const noexcept { return nullptr; }
	virtual Element_t const * end() const noexcept { return nullptr; }
	virtual Element_t const * GetElement(ShaderAttributeRef_t attrib) const noexcept {
		for (auto& elem : *this) {
			if (elem.Attrib == &attrib)
				return &elem;
		}
		return nullptr;
	}
};


template <size_t Attribs_N>
struct InterleavedDataArray_t : InterleavedDataBase_t {
	inline static constexpr size_t nAttribs { Attribs_N };
	std::array<Element_t, Attribs_N> Elements;

	constexpr InterleavedDataArray_t(std::array<Element_t, Attribs_N> const & elements)
		: Elements(elements)
	{
		size_t stride { 0 };
		for (auto& elem : elements) {
			stride += elem.Attrib->ElementSize * elem.Attrib->ElementCount;
		}
		Stride = stride;
	}

	size_t ElementCount() const noexcept override { return Attribs_N; }
	Element_t const * GetElement(size_t index) const noexcept override { return &Elements[index]; }
	Element_t const * begin() const noexcept override { return Elements.data(); }
	Element_t const * end() const noexcept override { return Elements.data() + Attribs_N; }
};

struct InterleavedDataVector_t : InterleavedDataBase_t {
	std::vector<Element_t> Elements;

	size_t ElementCount() const noexcept override { return Elements.size(); }
	Element_t const * GetElement(size_t index) const noexcept override { return &Elements[index]; }
	Element_t const * begin() const noexcept override { return Elements.data(); }
	Element_t const * end() const noexcept override { return Elements.data() + Elements.size(); }
};


template <typename Data_T = uint32_t>
struct InterleavedData_t {
	InterleavedDataBase_t const * Format;
	size_t ElementCount;
	Data_T * Data;
};

template <size_t Align = sizeof(uintptr_t), typename T = unsigned char>
class AlignedHeap_t {
protected:
	static T * Alloc(size_t size) {
		return new(std::align_val_t{ Align }) T[size];
	}

	static void Dealloc(T* ptr) {
		::operator delete(ptr, std::align_val_t{ Align });
	}

public:
	T * Data { nullptr };
	size_t Count { 0 };

	AlignedHeap_t() {};
	AlignedHeap_t(size_t count) : Data(Alloc(count)), Count(count) {}
	AlignedHeap_t(AlignedHeap_t const & b)
		: Data(Alloc(b.Count)), Count(b.Count)
	{
		std::memcpy(Data, b.Data, Count);
	}
	AlignedHeap_t(AlignedHeap_t && b)
		: Data(b.Data), Count(b.Count)
	{
		b.Data = nullptr;
		b.Count = 0;
	}
	~AlignedHeap_t() {
		if (Data)
			Dealloc(Data);
	}

	AlignedHeap_t& operator=(AlignedHeap_t const & b) {
		if (Data)
			Dealloc(Data);
		Data = Alloc(b.Count);
		Count = b.Count;
		std::memcpy(Data, b.Data, Count);
	}

	AlignedHeap_t& operator=(AlignedHeap_t && b) {
		if (Data)
			Dealloc(Data);
		Data = b.Data;
		Count = b.Count;
		std::memcpy(Data, b.Data, Count);
		b.Data = nullptr;
		b.Count = 0;
	}

	T* data() noexcept { return Data; }
	T const * data() const noexcept { return Data; }

	size_t size() const noexcept { return Count; }

	T* begin() noexcept { return Data; }
	T* end() noexcept { return Data + Count; }
	T const * begin() const noexcept { return Data; }
	T const * end() const noexcept { return Data + Count; }

	AlignedHeap_t& resize(size_t count) {
		if (Data)
			Dealloc(Data);
		Data = Alloc(count);
		Count = count;
	}
};


struct InterleavedArrayFormat_t {
	// Size of vector as number of elements
	int32_t VectorSize;
	// Interval between vector starts in bytes
	int32_t Stride;
	// Buffer position of the first vector in bytes
	size_t Offset;
	// Size of element type in bytes
	int8_t ElementSize;
};


template <typename T>
struct InterleavedArrayData_t {
	InterleavedArrayFormat_t Format;
	T const * Data { nullptr };
	size_t Count {};
};


class AttribListBase_t {
public:
	using Attrib_t = ShaderAttributeRef_t;
	using AttribName_t = char const*;
	using InitPair_t = std::pair<Attrib_t, AttribName_t>;

	virtual Attrib_t GetAttrib(size_t index) const = 0;
	virtual char const * GetName(size_t index) const = 0;
	virtual size_t size() const noexcept = 0;

	class iterator_t : public seLib::Iterators::IndexIterator_t<AttribListBase_t const, void> {
	public:
		using ref_t = AttribListBase_t const;
		using index_iterator_t = seLib::Iterators::IndexIterator_t<ref_t, void>;
		constexpr iterator_t(ref_t & instance, size_t index) : index_iterator_t(instance, index) {}
		constexpr iterator_t(iterator_t const &) = default;
		//constexpr iterator_t(iterator_t<Instance_T> &&) = delete;

		Attrib_t GetAttrib() const { return mInstance->GetAttrib(mIndex); }
		char const * GetName() const { return mInstance->GetName(mIndex); }
		size_t size() const noexcept { return mInstance->size(); }
		
	};// */
		
	auto begin() const noexcept { return iterator_t(*this, 0); }
	auto end() const noexcept { return iterator_t(*this, size()); }
};


template <typename First_T, typename...Second_T>
constexpr std::array<First_T, sizeof...(Second_T)> ExtractFirst(std::pair<First_T, Second_T>... pairs) {
	return { (pairs.first)... };
}

template <size_t Count_N>
class AttribList_t : public AttribListBase_t {
public:
	std::array<ShaderAttributePtr_t, Count_N> Attribs;
	std::array<AttribName_t, Count_N> Names;

	template <typename... AttribName_T>
	constexpr AttribList_t(std::pair<Attrib_t, AttribName_T>... attribs)
		: Attribs(std::array<ShaderAttributePtr_t, Count_N> {(&attribs.first)...}),
		Names(std::array<AttribName_t, Count_N> {(attribs.second)...})
	{}

	constexpr AttribList_t(Attrib_t key, AttribName_t value) : Attribs({&key}), Names({value}) {}

	constexpr AttribList_t(AttribList_t<Count_N-1> const & b, Attrib_t key, AttribName_t value)
		: Attribs({}),
		Names({})
	{
		for (size_t i = 0; i < (Count_N - 1); i++) {
			Attribs[i] = b.Attribs[i];
			Names[i] = b.Names[i];
		}
		Attribs[Count_N-1] = &key;
		Names[Count_N-1] = value;
	}

	constexpr auto operator()(Attrib_t key, AttribName_t value) const noexcept {
		return AttribList_t<Count_N+1>(*this, key, value);
	}

	/*AttribList_t(std::pair<Attrib_t, AttribName_t> attrib1)
		: Attribs({attrib1.first}),
		Names({attrib1.second})
	{}
	AttribList_t(std::pair<Attrib_t, AttribName_t> attrib1, std::pair<Attrib_t, AttribName_t> attrib2)
		: Attribs({attrib1.first}),
		Names({attrib1.second})
	{}// */

	ShaderAttributeRef_t GetAttrib(size_t index) const override { return *Attribs[index]; }
	char const * GetName(size_t index) const override { return Names[index]; }
	size_t size() const noexcept override { return Count_N; }
	//constexpr auto begin() const noexcept { return iterator_t(*this, 0); }
	//constexpr auto end() const noexcept { return iterator_t(*this, N); }
};

//template <typename Key_T, typename Value_T>
//AttribList_t(std::pair<Key_T, Value_T>) -> AttribList_t<1>;

template <typename... Value_T2>
AttribList_t(std::pair<AttribListBase_t::Attrib_t, Value_T2>...) -> AttribList_t<sizeof...(Value_T2)>;

template <size_t N=1>
AttribList_t(AttribListBase_t::Attrib_t, AttribListBase_t::AttribName_t) -> AttribList_t<N>;

template <size_t N>
AttribList_t(AttribList_t<N> const &, AttribListBase_t::Attrib_t, AttribListBase_t::AttribName_t) -> AttribList_t<N + 1>;


#pragma region // GLShaderConfig_t ======================================================

template <size_t Attribs_N = 0, size_t Uniforms_N = 0> class GLShaderConfig_t;

class GLShaderConfigBase_t {
protected:
	inline static constexpr std::array<ShaderAttributePtr_t, 0> NullAttribs {};

public:

	//using AttribNamesIterator_t = std::array<ShaderAttributeRef_t, 0>::const_iterator;
	GLenum Type;
	std::string_view const * ShaderSource;

	virtual AttribListBase_t::iterator_t attribs_begin() const noexcept = 0;
	virtual AttribListBase_t::iterator_t attribs_end() const noexcept = 0;
	virtual AttribListBase_t::iterator_t uniforms_begin() const noexcept = 0;
	virtual AttribListBase_t::iterator_t uniforms_end() const noexcept = 0;

protected:
	constexpr GLShaderConfigBase_t(GLenum shader_type, std::string_view const & shaderSource)
		: Type(shader_type), ShaderSource(&shaderSource)
	{}
};

template <size_t Attribs_N, size_t Uniforms_N>
class GLShaderConfig_t : public GLShaderConfigBase_t {
public:
	friend class GLShaderConfigBase_t;
	template <size_t N>
	using AttribNameMap_t = seLib::ConstMap_t<seLib::CG::ShaderAttributeRef_t, char const *, N>;
	AttribList_t<Attribs_N> AttribNames {};
	AttribList_t<Uniforms_N> UniformNames {};

	constexpr GLShaderConfig_t(GLenum shader_type, std::string_view const & shaderSource)
		: GLShaderConfigBase_t(shader_type, shaderSource)
	{}

	constexpr GLShaderConfig_t(
			GLenum shader_type,
			AttribList_t<Attribs_N> const & attribNames,
			AttribList_t<Uniforms_N> const & uniformNames,
			std::string_view const & shaderSource
		)
		: GLShaderConfigBase_t(shader_type, shaderSource),
		AttribNames(attribNames),
		UniformNames(uniformNames)
	{}

	AttribListBase_t::iterator_t attribs_begin() const noexcept override { return AttribNames.begin(); }
	AttribListBase_t::iterator_t attribs_end() const noexcept override { return AttribNames.end(); }
	AttribListBase_t::iterator_t uniforms_begin() const noexcept override { return UniformNames.begin(); }
	AttribListBase_t::iterator_t uniforms_end() const noexcept override { return UniformNames.end(); }
};

class GLShaderInstanceBase_t {
public:
	GLuint ShaderIndex = 0;
	//map<ShaderAttribute_t, int> Uniforms;

	~GLShaderInstanceBase_t() {}
	GLShaderInstanceBase_t() {}

	virtual GLShaderConfigBase_t const & Config() const noexcept = 0;

	void Init();
};

template <size_t Attribs_N, size_t Uniforms_N>
class GLShaderInstance_t : public GLShaderInstanceBase_t {
public:
	friend class GLShaderInstance_t;
	GLShaderConfig_t<Attribs_N, Uniforms_N> const & ConfigRef;

	GLShaderInstance_t(GLShaderConfig_t<Attribs_N, Uniforms_N> const & config)
		: ConfigRef(config)
	{}

	GLShaderConfigBase_t const & Config() const noexcept override {
		return ConfigRef;
	}

};

#pragma endregion


#pragma region // GLProgConfig_t ======================================================

template <size_t VertexAttr_N, size_t VertexUni_N, size_t FragAttr_N, size_t FragUni_N> class GLProgInstance_t;

class GLProgInstanceBase_t {
public:
	GLuint ProgIndex = 0;
	std::map<ShaderAttributePtr_t, GLint> AttribIndexes;

	~GLProgInstanceBase_t() {}
	GLProgInstanceBase_t() {
	}

	void Init();

	void UseProgram();

	GLint GetUniform(ShaderAttributeRef_t uniform);

	virtual GLShaderInstanceBase_t & VertexShaderInstace() noexcept = 0;
	virtual GLShaderInstanceBase_t & FragShaderInstace() noexcept = 0;
	virtual GLShaderInstanceBase_t const& VertexShaderInstace() const noexcept = 0;
	virtual GLShaderInstanceBase_t const& FragShaderInstace() const noexcept = 0;
	virtual GLuint GetVertexShaderIndex() const = 0;
	virtual GLuint GetFragShaderIndex() const = 0;
	GLint GetAttribIndex(ShaderAttributeRef_t ptr) const {
		auto iter = AttribIndexes.find(&ptr);
		if (iter == AttribIndexes.end())
			return -1;
		return iter->second;
	}

protected:

};

using GLProgInstanceRef_t = seLib::RefAccessor_t<GLProgInstanceBase_t>;

template <size_t VertexAttr_N, size_t VertexUni_N, size_t FragAttr_N, size_t FragUni_N>
class GLProgInstance_t : public GLProgInstanceBase_t {
public:
	friend class GLProgInstance_t;
	GLShaderInstance_t<VertexAttr_N, VertexUni_N> & VertexShader;
	GLShaderInstance_t<FragAttr_N, FragUni_N> & FragShader;

	GLProgInstance_t(
			GLShaderInstance_t<VertexAttr_N, VertexUni_N> & vertex_shader,
			GLShaderInstance_t<FragAttr_N, FragUni_N> & frag_shader
		)
		: VertexShader(vertex_shader),
		FragShader(frag_shader)
	{
	}

	//void GetShaderIndexes() override { }

public:
	GLShaderInstanceBase_t & VertexShaderInstace() noexcept override { return VertexShader; }
	GLShaderInstanceBase_t & FragShaderInstace() noexcept override { return FragShader; }
	GLShaderInstanceBase_t const& VertexShaderInstace() const noexcept override { return VertexShader; }
	GLShaderInstanceBase_t const& FragShaderInstace() const noexcept override { return FragShader; }

	GLuint GetVertexShaderIndex() const override {
		return VertexShader.ShaderIndex;
	}

	GLuint GetFragShaderIndex() const override {
		return FragShader.ShaderIndex;
	}
};

#pragma endregion


#pragma region // VAO ======================================================

class VAODef_t {
public:
	GLuint GLType = GL_INVALID_VALUE;
};

class VAOInstance_t {
public:
	VAODef_t const * Def;
	GLProgInstanceRef_t ShaderProg;
	//GLAttribMap AttribMap;
	GLuint VAOIndex = 0;
	GLuint VertexIndex = 0;
	GLuint ColorIndex = 0;
	//std::vector<RenderView> Arrays;
	//public uint[] Usage;
	//public uint[] Types;
	std::vector<GLuint> BufferIndexes;
	std::vector<GLuint> BufferFlags;
	//GCHandle[] Pinned;
	GLuint Count;

	~VAOInstance_t() {}

//protected:
	inline VAOInstance_t(VAODef_t const & def) : Def(&def) {}

public:
	inline bool IsValid() {
		return VAOIndex != GL_INVALID_INDEX;
	}

	/*
	void Disable(ShaderAttributes attribID);

	void Set(ShaderAttributes attribID, const RenderView& view);
	//void Rebuffer(ShaderAttributes attribID);

	inline void SetFlags(uint bufferindex, uint flags) {
		BufferFlags[bufferindex] = flags;
	}

	void SetElements(const RenderView& view);

	void Bind();

	inline GLuint GetAttribIndex(ShaderAttributes attribID) {
		return AttribMap[attribID].Index;
	}

	inline void EnableAttribArray(ShaderAttributes attribID) {
		glEnableVertexAttribArray(AttribMap[attribID].Index);
	}

	inline void DisableAttribArray(ShaderAttributes attribID) {
		glDisableVertexAttribArray(AttribMap[attribID].Index);
	}
	// */
};

#pragma endregion

/*
struct RenderData {
	const uint8_t* Data = nullptr;
	uint32_t Offset = 0; // Offset into buffer in bytes
	uint32_t Length = 0; // Length of buffer in bytes
	uint8_t ElementSize = 0; // Element size in bytes

	RenderData() { }

	RenderData(const uint8_t* data, uint32_t offset, uint32_t length, uint8_t elementsize) :
		Data(data), Offset(offset), Length(length), ElementSize(elementsize)
	{ }

	template <typename T>
	RenderData(const T* data, uint32_t offset, uint32_t length) :
		Data((const uint8_t*)data), Offset(offset), Length(length), ElementSize(sizeof(T))
	{ }

	template <typename T>
	RenderData(const std::vector<T>& data, uint32_t offset) :
		Data((const uint8_t*)data.data()), Offset(offset), Length((data.size() * sizeof(T)) - offset), ElementSize(sizeof(T))
	{ }

	RenderData(const RenderData&) = default;
	RenderData(RenderData&&) = default;

	RenderData& operator=(const RenderData&) = default;
	RenderData& operator=(RenderData&&) = default;

	template <typename Scalar_T, int Cols>
	inline static RenderData FromVectorList(const std::vector<Vector<Scalar_T, Cols>>& data) {
		return RenderData(
			(const uint8_t*)data.data(),
			0,
			data.size() * Cols * sizeof(Scalar_T),
			sizeof(Scalar_T)
		);
	}

};

class RenderView {
public:
	RenderData Data;
	uint32_t Offset = 0; // Starting offset in bytes.
	uint8_t ElementSize = 0; // Size in bytes of each element.
	uint8_t VectorSize = 0; // Number of elements in each vector.
	uint32_t Stride = 0; // Increment in bytes between the start of each vector.
	uint32_t Count = 0;

	RenderView() {}
	RenderView(const RenderData& renderdata, uint8_t vectorSize, uint32_t offset = 0, uint8_t elementSize = 0, uint32_t stride = 0, uint32_t count = 0) {
		Data = renderdata;
		Offset = offset;

		if (elementSize == 0) {
			elementSize = renderdata.ElementSize;
		}
		ElementSize = elementSize;
		VectorSize = vectorSize;

		if (stride == 0) {
			stride = elementSize * vectorSize;
		}
		Stride = stride;

		if (count == 0) {
			count = (uint32_t)((renderdata.Length - offset) / stride);
		}
		Count = count;
	}

	RenderView(const RenderView&) = default;
	RenderView(RenderView&&) = default;

	RenderView& operator=(const RenderView&) = default;
	RenderView& operator=(RenderView&&) = default;

	template <typename T>
	inline const T& Get(size_t index) {
		if (sizeof(T) != VectorSize * ElementSize)
			throw MessageException("Attempted cast to type with different size.");
		return *(const T*)(Data.Data + Data.Offset + Offset + (Stride * index));
	}

	template <typename T>
	inline void Set(size_t index, const T& value) {
		if (sizeof(T) != VectorSize * ElementSize)
			throw MessageException("Attempted cast to type with different size.");
		*(T*)(Data.Data + Data.Offset + Offset + (Stride * index)) = value;
	}

	template <typename Scalar_T, int Cols>
	inline static RenderView FromVectorList(const std::vector<Vector<Scalar_T, Cols>>& data) {
		return RenderView(
			RenderData((const uint8_t*)data.data(), 0, data.size() * Cols * sizeof(Scalar_T), sizeof(Scalar_T)),
			0, sizeof(Scalar_T), Cols, Cols * sizeof(Scalar_T), data.size()
		);
	}
};
// */

class OGLRenderer_t : public virtual Renderer_t {
public:
	//using RenderView_t = RenderView;


	Vector2glf_t GLWindowSize;
	Vector2glf_t GLWindowResolution;
	Matrix4glf_t DisplayTransform;
	float InterpolationFraction;

	DisplayList_t const * SceneObjects { nullptr };

public:
protected:
	double _LastFrameTime;
	int _LastFrameInterval;

	unsigned WidthPx;
	unsigned HeightPx;

public:
	OGLRenderer_t() {}
	virtual ~OGLRenderer_t() {}

	// Inherited via GraphRenderer
	//virtual int InitGL() { return 0; }

	virtual int SetScene(DisplayList_t const & objs) {
		SceneObjects = &objs;
		return 0;
	}

	virtual int BeginDrawScene() { return 0; }
	virtual int EndDrawScene() { return 0; }
	virtual int DrawSceneObjects(DisplayList_t const & objs) { return 0; }

	virtual int DrawScene(DisplayList_t const & objs) {
		BeginDrawScene();
		DrawSceneObjects(objs);
		EndDrawScene();
		return 0;
	}

	virtual int LastRenderTime() { return 0; }
	virtual void ResetRenderTime() { }
	//virtual int32_t GetNodeAtPixel(int x, int y);
	virtual void UpdateData() {}

	// empty overrides for unit tests
	virtual int QueueRedraw() { return 0; }

protected:
	void CreateVerticesForSquare();
};

}}
