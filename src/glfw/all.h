#pragma once
#include "glfw/common.h"


namespace cultlang {
namespace glfw 
{
	class Rect
		: public virtual craft::types::Object
	{
		CULTLANG_GLFW_EXPORTED CRAFT_OBJECT_DECLARE(cultlang::glfw::Rect)

	public:
		craft::types::instance<int32_t> x;
		craft::types::instance<int32_t> y;
	};

	class Window 
		: public virtual craft::types::Object
	{
		CULTLANG_GLFW_EXPORTED CRAFT_OBJECT_DECLARE(cultlang::glfw::Window)

	public:
		GLFWwindow* window;
		
		// Window Callbacks
		craft::types::instance<craft::lisp::PSubroutine> onClose;
		craft::types::instance<craft::lisp::PSubroutine> onMove;
		craft::types::instance<craft::lisp::PSubroutine> onResize;
		craft::types::instance<craft::lisp::PSubroutine> onFramebufferResize;
		craft::types::instance<craft::lisp::PSubroutine> onRefresh;
		craft::types::instance<craft::lisp::PSubroutine> onFocus;
		craft::types::instance<craft::lisp::PSubroutine> onIconify;
		

		// Input Callbacks
		craft::types::instance<craft::lisp::PSubroutine> onKey;
		craft::types::instance<craft::lisp::PSubroutine> onChar;

		craft::types::instance<craft::lisp::PSubroutine> onMousePosition;
		craft::types::instance<craft::lisp::PSubroutine> onMouseEnter;

		craft::types::instance<craft::lisp::PSubroutine> onMouseClick;
		craft::types::instance<craft::lisp::PSubroutine> onMouseScroll;

		craft::types::instance<craft::lisp::PSubroutine> onFileDrop;

		~Window();
	};

	extern craft::lisp::BuiltinModuleDescription BuiltinGlfw;

	CULTLANG_GLFW_EXPORTED void make_bindings(craft::instance<craft::lisp::Module> ret);
}}

CRAFT_TYPE_DECLARE(CULTLANG_GLFW_EXPORTED, GLFWmonitor)
CRAFT_TYPE_DECLARE(CULTLANG_GLFW_EXPORTED, GLFWvidmode)