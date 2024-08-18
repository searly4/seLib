#pragma once

#include <map>

#include <seLib/OGLWrapper.h>
#include <seLib/Sort.h>
#include <seLib/ConstMap.h>
#include <seLib/RefHolder.h>
#include <seLib/Debug.h>

namespace seLib {
namespace CG {


inline constexpr char const* GLAssertString(GLuint err) {
	switch (err) {
	case GL_NO_ERROR:
		return "OpenGL error: GL_NO_ERROR";
	case GL_INVALID_ENUM:
		return "OpenGL error: GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "OpenGL error: GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "OpenGL error: GL_INVALID_OPERATION";
	  //case GL_STACK_OVERFLOW:
		//msg = "OpenGL error: GL_STACK_OVERFLOW";
	  //case GL_STACK_UNDERFLOW:
		//msg = "OpenGL error: GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY:
		return "OpenGL error: GL_OUT_OF_MEMORY";
	  //case GL_TABLE_TOO_LARGE:
		//return "OpenGL error: GL_TABLE_TOO_LARGE";
	default:
		return "OpenGL error: unknown";
	}
	//throw new MessageException(msg, SE::Debug::Trace::Error);
}

inline void GLAssert() {
#ifdef _DEBUG
	GLuint err = glGetError();
	if (err != GL_NO_ERROR) seLibTrace(GLAssertString(err), seLib::Debug::Trace::Error);
#endif
}


inline static void FBO_Assert() {
	auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status == GL_FRAMEBUFFER_COMPLETE)
		return;

	if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
		seLib_THROW_ASSERT(false, GLExceptionString_t(std::string("ERROR: a necessary attachment is uninitialized")))

	if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
		seLib_THROW_ASSERT(false,  GLExceptionString_t(std::string("ERROR: no attachments")))

//if (status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)
  //throw "ERROR: incomplete draw buffer";
//if (status == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER)
  //throw "ERROR: incomplete read buffer";

	if (status == GL_FRAMEBUFFER_UNSUPPORTED)
		seLib_THROW_ASSERT(false, GLExceptionString_t(std::string("ERROR: combination of attachments is not supported")))

//if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE)
  //throw "ERROR: number if samples for all attachments does not match";

	if (status == 0)
		status = 0;
};

inline static std::string GetLinkInfoLog(unsigned programObject) {
  //  Get the info log length.
	GLint infoLen = 0;
	glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

	//  Get the compile info.
	std::string infoLog(infoLen, 0);
	glGetProgramInfoLog(programObject, infoLen, NULL, (char*)infoLog.data());

	return infoLog;
}

inline static std::string GetCompileInfoLog(unsigned shaderObject) {
  //  Get the info log length.
	GLint infoLen = 0;
	glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &infoLen);

	//  Get the compile info.
	std::string infoLog(infoLen, 0);
	glGetShaderInfoLog(shaderObject, infoLen, NULL, (char*)infoLog.data());

	return infoLog;
}

inline static void AssertShader(unsigned shader) {
	GLint parameter;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &parameter);
	seLib_THROW_ASSERT(parameter != 0, GLExceptionString_t(std::string("Error compiling shader:\n%s\n") + GetCompileInfoLog(shader)))
}

inline static void AssertLink(unsigned programObject) {
	GLint parameter;
	glGetProgramiv(programObject, GL_LINK_STATUS, &parameter);
	seLib_THROW_ASSERT(parameter != 0, GLExceptionString_t(std::string("Error linking program:\n%s\n") + GetLinkInfoLog(programObject)))
}

//using GLAttribMap_t = std::pair<seLib::CG::ShaderAttribute_t, char const *>;
template <size_t Attribs_N>
using GLAttribNameMap_t = seLib::ConstMap_t<seLib::CG::ShaderAttributeRef_t, char const*, Attribs_N>;



#pragma region // GLES3VAOInstance_t ======================================================

class GLES3VAOInstance_t : public VAOInstance_t {
public:
	GLuint VAOIndex{ GL_INVALID_VALUE };
	GLuint ElementsBuffer = GL_INVALID_INDEX;

	inline GLES3VAOInstance_t(VAODef_t const& def) : VAOInstance_t(def) {}
	~GLES3VAOInstance_t();
};

#pragma endregion


#pragma region //  ======================================================
#pragma endregion

class GraphRendererWeb_t : public OGLRenderer_t {
public:
	using GLProgInstanceRef_t = seLib::RefAccessor_t<GLProgInstanceBase_t>;
	using GLShaderInstanceRef_t = seLib::RefAccessor_t<GLShaderInstanceBase_t>;
	//using ObjectHandlerRef_t = seLib::RefAccessor_t<ObjectHandlerBase_t>;

	class ClassHandlerBase_t {
	public:
		virtual void InitClass() = 0;
		virtual void UpdateClass() = 0;
		virtual void DeinitClass() = 0;
		virtual void Draw() = 0;
		virtual void DrawStart() = 0;
		virtual void DrawEnd() = 0;
		virtual bool CanHandle(RenderObject_t& obj) = 0;
		virtual bool TryAdd(RenderObject_t& obj) = 0;
		virtual bool TryRemove(RenderObject_t& obj) = 0;
		template <typename Derived_T> class ObjectSlice_t;
	};

	using ClassHandlerRef_t = seLib::RefAccessor_t<ClassHandlerBase_t>;


	template <typename Object_T>
	class ClassHandler_t : public ClassHandlerBase_t {
	public:
		GraphRendererWeb_t& Renderer;
		ClassHandler_t(GraphRendererWeb_t* renderer) : Renderer(*renderer) {}
		virtual bool Add(Object_T& obj) = 0;
		virtual bool Remove(Object_T& obj) = 0;
		bool CanHandle(RenderObject_t& obj) override {
			return dynamic_cast<Object_T*>(&obj) != nullptr;
		}

		bool TryAdd(RenderObject_t& obj) override {
			auto* obj2 = dynamic_cast<Object_T*>(&obj);
			if (obj2 == nullptr)
				return false;

			return Add(*obj2);
		}

		bool TryRemove(RenderObject_t& obj) override {
			auto* obj2 = dynamic_cast<Object_T*>(&obj);
			if (obj2 == nullptr)
				return false;

			return Remove(*obj2);
		}
	};

	class ObjectHandlerBase_t {
	public:
		virtual void InitObject() = 0;
		virtual void UpdateObject() = 0;
		virtual void DeinitObject() = 0;
		virtual void DrawObject() = 0;
	};

	using ObjectHandlerRef_t = seLib::RefAccessor_t<ObjectHandlerBase_t>;

	std::vector<ClassHandlerRef_t> ClassHandlers{};
	std::vector<ObjectHandlerRef_t> ObjectHandlers{};
	std::vector<GLShaderInstanceRef_t> ShaderInstances{};
	std::vector<GLProgInstanceRef_t> ProgInstances{};

	template <typename ClassHandler_T>
	class ObjectHandler_t : public ObjectHandlerBase_t {
	public:
		ClassHandler_T& ClassHandler;
		ObjectHandler_t(ClassHandler_T& handler) : ClassHandler(handler) {}
		ObjectHandler_t(ObjectHandler_t const&) = default;
	};


public:

protected:
	//vector<VAO> VBAList;
	//uint32_t displayFB = GL_INVALID_INDEX;
	//uint32_t selectFB = GL_INVALID_INDEX;
	//uint32_t displayRB = GL_INVALID_INDEX;
	//uint32_t selectRB = GL_INVALID_INDEX;

	Matrix4f_t GridScale;

	double _LastFrameTime;
	int _LastFrameInterval;


public:
	GraphRendererWeb_t() {}
	virtual ~GraphRendererWeb_t() {}

	void Register(ClassHandlerRef_t handler) {
		for (auto iter : ClassHandlers)
			if (iter == handler)
				return;
		ClassHandlers.push_back(handler);
	}

	void Register(ObjectHandlerRef_t handler) {
		for (auto iter : ObjectHandlers)
			if (iter == handler)
				return;
		ObjectHandlers.push_back(handler);
	}

	void Register(GLShaderInstanceRef_t instance) {
		for (auto iter : ShaderInstances)
			if (iter == instance)
				return;
		ShaderInstances.push_back(instance);
	}

	void Register(GLProgInstanceRef_t instance) {
		for (auto iter : ProgInstances)
			if (iter == instance)
				return;
		ProgInstances.push_back(instance);
	}

	template <typename T, typename... Args_T>
	ClassHandlerRef_t Create(Args_T...args) {
		ClassHandlers.push_back(seLib::RefAccessor_t<T>::InstantiateShared());
	}

	// Inherited via GraphRenderer
	void Resize(uint16_t width, uint16_t height, float xresolution, float yresolution) noexcept override;
	bool Init(Renderer_t::Time_t timestamp) noexcept override;


protected:
};

}
}
