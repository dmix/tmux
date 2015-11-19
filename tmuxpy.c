#include <Python.h>
#include "tmux.h"
    
static PyObject*
say_hello(PyObject* self, PyObject* args)
{
    const char* name;

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    printf("Hello %s!\n", name);

    Py_RETURN_NONE;
}

static PyObject*
tmuxgetshell(PyObject* self)
{

    const char* shell;

    shell = getshell();

    printf("your shell is %s!\n", shell);

    Py_RETURN_NONE;
}

static PyMethodDef HelloMethods[] =
{
     {"say_hello", say_hello, METH_VARARGS, "Greet somebody."},
     {"tmuxgetshell", tmuxgetshell, METH_NOARGS, "Get user's shell"},
     {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
inittmuxpy(void)
{
     (void) Py_InitModule("tmuxpy", HelloMethods);
}
