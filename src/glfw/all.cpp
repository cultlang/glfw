#include "glfw/common.h"

#include "all.h"

#include "lisp/library/system/prelude.h"
#include "lisp/semantics/cult/calling.h"

using namespace craft;
using namespace craft::lisp;
using namespace craft::types;
using namespace cultlang::glfw;

#include "typedefs.h"

#define lMM sem->builtin_implementMultiMethod
#define g "glfw"

CRAFT_DEFINE(Rect)
{
	_.defaults();
}

CRAFT_DEFINE(Window)
{
	_.defaults();
}

CRAFT_TYPE_DEFINE(GLFWmonitor)
{
	_.add<GraphPropertyName>("glfw/GLFWmonitor");
	_.add<GraphPropertyCppName>("::GLFWmonitor");
	_.use<PStringer>().singleton<FunctionalStringer>([](::craft::instance<GLFWmonitor> _this) {
		return "GLFWmonitor";
	});
	_.use<PRepr>().singleton<FunctionalRepr>([](::craft::instance<GLFWmonitor> _this) {
		return "GLFWmonitor";
	});
}

CRAFT_TYPE_DEFINE(GLFWvidmode)
{
	_.add<GraphPropertyName>("glfw/GLFWvidmode");
	_.add<GraphPropertyCppName>("::GLFWvidmode");
	_.use<PStringer>().singleton<FunctionalStringer>([](::craft::instance<GLFWvidmode> _this) {
		return "GLFWvidmode";
	});
	_.use<PRepr>().singleton<FunctionalRepr>([](::craft::instance<GLFWvidmode> _this) {
		return "GLFWvidmode";
	});
}

Window::~Window()
{
    if(window)
    {
        glfwDestroyWindow(window);
    }
}

namespace _impl {
    p_sub glfwErrorCallback;
    p_sub glfwKeyCallback;
}

void cultlang::glfw::make_bindings(craft::instance<craft::lisp::Module> ret)
{
    
	auto sem = ret->require<lisp::CultSemantics>();

    //Utility 
    lMM(g"/rect", [](t_i32 x, t_i32 y) { auto r = instance<Rect>::make(); r->x = x; r->y = y; return r;});
    lMM(g"/rect/x", [](instance<Rect> r) { return r->x;});
    lMM(g"/rect/x", [](instance<Rect> r, t_i32 x) { r->x = x;});
    lMM(g"/rect/y", [](instance<Rect> r) { return r->y;});
    lMM(g"/rect/y", [](instance<Rect> r, t_i32 y) { r->y = y;});
    

    // Initialization
    glfwSetErrorCallback([](int err, const char* msg) {
        throw stdext::exception("Error {}: {}", err, msg);
    });


	lMM(g"/init", []() { glfwInit(); return t_bool::make(true);});
    lMM(g"/terminate", []() { glfwTerminate();});
    lMM(g"/error/callback", [](instance<PSubroutine> p){ _impl::glfwErrorCallback = p; return p;});
    lMM(g"/version", []() {return t_str::make(glfwGetVersionString());});

    // Monitors
    lMM(g"/monitors", []() { 
        auto res = instance<craft::lisp::library::List>::make();
        int count;
        auto monitors = glfwGetMonitors(&count);
        for(size_t i =0; i < count; i++)
        {
            res->push(instance<GLFWmonitor>::makeFromPointerAndMemoryManager(
                monitors[i], 
                monitors[i]));
        }

        return res;
    });
    lMM(g"/monitors/primary", []() {
        auto p = glfwGetPrimaryMonitor();
        return instance<GLFWmonitor>::makeFromPointerAndMemoryManager(p, p);
    });
    lMM(g"/monitors/position", [](instance<GLFWmonitor> f) {
        int32_t x, y;
        auto res = instance<Rect>::make();
        glfwGetMonitorPos(f.get(), &x, &y);
        res->x = t_i32::make(x);
        res->y = t_i32::make(y);

        return res;
    });
    lMM(g"/monitors/physicalSize", [](instance<GLFWmonitor> f) {
        int32_t x, y;
        auto res = instance<Rect>::make();
        glfwGetMonitorPhysicalSize(f.get(), &x, &y);
        res->x = t_i32::make(x);
        res->y = t_i32::make(y);

        return res;
    });
    lMM(g"/monitors/name", [](instance<GLFWmonitor> f) {return t_str::make(glfwGetMonitorName(f.get()));});
    //TODO video modes http://www.glfw.org/docs/latest/group__monitor.html

    // Windows
    lMM(g"/window/create", [](t_i64 w, t_i64 h, t_str n)  { 
        auto i = glfwCreateWindow(*w, *h, n->c_str(), nullptr, nullptr);
        auto res = instance<Window>::make();
        res->window = i;

        glfwSetWindowUserPointer(i, res.asInternalPointer());

        // WindowEvents
        glfwSetWindowSizeCallback(i,  [](GLFWwindow* window, int xpos, int ypos) {
            auto win = instance<Window>::fromInternalPointer(glfwGetWindowUserPointer(window));
            if(win->onResize)
                auto res = win->onResize->execute(win->onResize, {win, t_i32::make(xpos), t_i32::make(ypos)});
        });
        glfwSetWindowPosCallback(i,  [](GLFWwindow* window, int xpos, int ypos) {
            auto win = instance<Window>::fromInternalPointer(glfwGetWindowUserPointer(window));
            if(win->onMove)
                auto res = win->onMove->execute(win->onMove, {win, t_i32::make(xpos), t_i32::make(ypos)});
        });
        glfwSetFramebufferSizeCallback(i,  [](GLFWwindow* window, int width, int height) {
            auto win = instance<Window>::fromInternalPointer(glfwGetWindowUserPointer(window));
            if(win->onFramebufferResize)
                auto res = win->onFramebufferResize->execute(win->onFramebufferResize, {win, t_i32::make(width), t_i32::make(height)});
        });
        glfwSetWindowCloseCallback(i,  [](GLFWwindow* window) {
            auto win = instance<Window>::fromInternalPointer(glfwGetWindowUserPointer(window));
            if(win->onClose)
            {
                auto res = win->onClose->execute(win->onClose, {win});
                if(res) glfwSetWindowShouldClose(window, GLFW_FALSE);
            }
            else
            {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        });
        glfwSetWindowRefreshCallback(i,  [](GLFWwindow* window) {
            auto win = instance<Window>::fromInternalPointer(glfwGetWindowUserPointer(window));
            if(win->onRefresh)
                auto res = win->onRefresh->execute(win->onRefresh, {win});
        });
        glfwSetWindowFocusCallback(i,  [](GLFWwindow* window, int i) {
            auto win = instance<Window>::fromInternalPointer(glfwGetWindowUserPointer(window));
            if(win->onFocus)
                auto res = win->onFocus->execute(win->onFocus, {win, t_bool::make(i)});
        });
        glfwSetWindowIconifyCallback(i,  [](GLFWwindow* window, int i) {
            auto win = instance<Window>::fromInternalPointer(glfwGetWindowUserPointer(window));
            if(win->onIconify)
                auto res = win->onIconify->execute(win->onIconify, {win, t_bool::make(i)});
        });
        

        // Input Events
        glfwSetKeyCallback(i, [](GLFWwindow* window, int key, int scancode, int action, int mods){
            auto win = instance<Window>::fromInternalPointer(glfwGetWindowUserPointer(window));
            if(win->onKey)
                win->onKey->execute(win->onKey, { 
                    win,
                    t_i32::make(key), 
                    t_i32::make(scancode),
                    t_i32::make(action),
                    t_i32::make(mods),
                });
        });
        glfwSetCharCallback(i, [](GLFWwindow* window, unsigned int codepoint){
            auto win = instance<Window>::fromInternalPointer(glfwGetWindowUserPointer(window));
            if(win->onChar)
                win->onChar->execute(win->onChar, { 
                    win,
                    t_i32::make(codepoint), 
                });
        });
        glfwSetCursorPosCallback(i, [](GLFWwindow* window, double xpos, double ypos){
            auto win = instance<Window>::fromInternalPointer(glfwGetWindowUserPointer(window));
            if(win->onMousePosition)
                win->onMousePosition->execute(win->onMousePosition, { 
                    win,
                    t_f64::make(xpos), 
                    t_f64::make(ypos), 
                });
        });
        glfwSetCursorEnterCallback(i, [](GLFWwindow* window, int entered){
            auto win = instance<Window>::fromInternalPointer(glfwGetWindowUserPointer(window));
            if(win->onMouseEnter)
                win->onMouseEnter->execute(win->onMouseEnter, { 
                    win,
                    t_bool::make(entered),
                });
        });
        glfwSetMouseButtonCallback(i, [](GLFWwindow* window, int button, int action, int mods){
            auto win = instance<Window>::fromInternalPointer(glfwGetWindowUserPointer(window));
            if(win->onMouseClick)
                win->onMouseClick->execute(win->onMouseClick, { 
                    win,
                    t_i32::make(button),
                    t_i32::make(action),
                    t_i32::make(mods),
                });
        });
        glfwSetScrollCallback(i, [](GLFWwindow* window, double xpos, double ypos){
            auto win = instance<Window>::fromInternalPointer(glfwGetWindowUserPointer(window));
            if(win->onMouseScroll)
                win->onMouseScroll->execute(win->onMouseScroll, { 
                    win,
                    t_f64::make(xpos), 
                    t_f64::make(ypos), 
                });
        });
        glfwSetDropCallback(i, [](GLFWwindow* window, int count, const char** paths){
            auto win = instance<Window>::fromInternalPointer(glfwGetWindowUserPointer(window));
            if(win->onFileDrop)
            {
                auto lst = instance<craft::lisp::library::List>::make();

                for(size_t i =0; i < count; i++)
                {
                    lst->push(t_str::make(paths[i]));
                }
                win->onFileDrop->execute(win->onFileDrop, { 
                    win,
                    lst
                });
            }    
        });

        
        return res;       
    });
    lMM(g"/window/context", [](instance<Window> w) {glfwMakeContextCurrent(w->window);});
    lMM(g"/window/swap", [](instance<Window> w) {glfwSwapBuffers(w->window);});

    lMM(g"/window/onClose", [](instance<Window> w, instance<PSubroutine> p) {w->onClose=p;});
    lMM(g"/window/onMove", [](instance<Window> w, instance<PSubroutine> p) {w->onMove=p;});
    lMM(g"/window/onResize", [](instance<Window> w, instance<PSubroutine> p) {w->onResize=p;});
    lMM(g"/window/onFramebufferResize", [](instance<Window> w, instance<PSubroutine> p) {w->onFramebufferResize=p;});
    lMM(g"/window/onRefresh", [](instance<Window> w, instance<PSubroutine> p) {w->onRefresh=p;});
    lMM(g"/window/onFocus", [](instance<Window> w, instance<PSubroutine> p) {w->onFocus=p;});
    lMM(g"/window/onIconify", [](instance<Window> w, instance<PSubroutine> p) {w->onIconify=p;});

    lMM(g"/window/onKey", [](instance<Window> w, instance<PSubroutine> p) {w->onKey=p;});
    lMM(g"/window/onChar", [](instance<Window> w, instance<PSubroutine> p) {w->onChar=p;});
    lMM(g"/window/onMousePosition", [](instance<Window> w, instance<PSubroutine> p) {w->onMousePosition=p;});
    lMM(g"/window/onMouseEnter", [](instance<Window> w, instance<PSubroutine> p) {w->onMouseEnter=p;});
    lMM(g"/window/onMouseClick", [](instance<Window> w, instance<PSubroutine> p) {w->onMouseClick=p;});
    lMM(g"/window/onMouseScroll", [](instance<Window> w, instance<PSubroutine> p) {w->onMouseScroll=p;});
    lMM(g"/window/onFileDrop", [](instance<Window> w, instance<PSubroutine> p) {w->onFileDrop=p;});


    lMM(g"/window/destroy", [](instance<Window> w) {glfwDestroyWindow(w->window);w->window = nullptr;});
    lMM(g"/window/hint", [](t_i32 h, t_i32 v) {glfwWindowHint(*h, *v);});
    lMM(g"/window/hint/reset", []() {glfwDefaultWindowHints();});

    lMM(g"/window/shouldClose", [](instance<Window> w) {return t_bool::make(glfwWindowShouldClose(w->window));});
    lMM(g"/window/close", [](instance<Window> w, t_bool v) {glfwSetWindowShouldClose(w->window, *v);});
    
    lMM(g"/window/title", [](instance<Window> w, t_str s) {glfwSetWindowTitle(w->window, s->c_str());});
    
    //TODO glfwSetWindowIcon 
    
    lMM(g"/window/pos", [](instance<Window> w) {
        auto res = instance<Rect>::make();
        int x, y;
        glfwGetWindowPos(w->window, &x, &y);
        res->x = t_i32::make(x);
        res->y = t_i32::make(y);

        return res;
    });
    lMM(g"/window/pos", [](instance<Window> w, instance<Rect> r) {glfwSetWindowPos(w->window, *r->x, *r->y);});
    lMM(g"/window/pos", [](instance<Window> w, t_i32 x, t_i32 y) {glfwSetWindowPos(w->window, *x, *y);});

    lMM(g"/window/size", [](instance<Window> w) {
        auto res = instance<Rect>::make();
        int x, y;
        glfwGetWindowSize(w->window, &x, &y);
        res->x = t_i32::make(x);
        res->y = t_i32::make(y);

        return res;
    });
    lMM(g"/window/size", [](instance<Window> w, instance<Rect> r) {glfwSetWindowSize(w->window, *r->x, *r->y);});
    lMM(g"/window/size", [](instance<Window> w, t_i32 x, t_i32 y) {glfwSetWindowSize(w->window, *x, *y);});

    lMM(g"/window/framebuffersize", [](instance<Window> w) {
        auto res = instance<Rect>::make();
        int x, y;
        glfwGetFramebufferSize(w->window, &x, &y);
        res->x = t_i32::make(x);
        res->y = t_i32::make(y);

        return res;
    });
    lMM(g"/window/framebuffersize", [](instance<Window> w, instance<Rect> r) {glfwSetWindowSize(w->window, *r->x, *r->y);});
    lMM(g"/window/framebuffersize", [](instance<Window> w, t_i32 x, t_i32 y) {glfwSetWindowSize(w->window, *x, *y);});

    //TODO lMM(g"/window/framesize"

    lMM(g"/window/size/limits", [](instance<Window> w, instance<Rect> minr, instance<Rect> maxr) {glfwSetWindowSizeLimits(w->window, *minr->x, *minr->y, *maxr->x, *maxr->y);});
    lMM(g"/window/size/limits", [](instance<Window> w, t_i32 minx, t_i32 miny, t_i32 maxx, t_i32 maxy) {glfwSetWindowSizeLimits(w->window, *minx, *miny, *maxx, *maxy);});
    
    lMM(g"/window/aspect", [](instance<Window> w, t_i32 num, t_i32 dem) {glfwSetWindowAspectRatio(w->window, *num, *dem);});

    lMM(g"/window/iconify", [](instance<Window> w) {glfwIconifyWindow (w->window);});
    lMM(g"/window/restore", [](instance<Window> w) {glfwRestoreWindow (w->window);});
    lMM(g"/window/maximize", [](instance<Window> w) {glfwMaximizeWindow (w->window);});
    lMM(g"/window/show", [](instance<Window> w) {glfwShowWindow (w->window);});
    lMM(g"/window/hide", [](instance<Window> w) {glfwHideWindow  (w->window);});
    lMM(g"/window/focus", [](instance<Window> w) {glfwFocusWindow  (w->window);});

    // Keyboard
    lMM(g"/key/name", [](t_i32 k, t_i32 s)  { return t_str::make(glfwGetKeyName(*k, *s));});
    // Events
    lMM(g"/event/poll", []()  { glfwPollEvents();});
    lMM(g"/event/wait", []()  { glfwPollEvents();});
    lMM(g"/event/wait", [](t_f64 w)  { glfwWaitEventsTimeout(*w);});
    lMM(g"/event/post", []()  { glfwPostEmptyEvent();});

}

BuiltinModuleDescription cultlang::glfw::BuiltinGlfw("extensions/glfw", cultlang::glfw::make_bindings);


#include "types/dll_entry.inc"
