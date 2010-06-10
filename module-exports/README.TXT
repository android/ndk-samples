This sample is used to demonstrate the usage of module exports
(i.e. LOCAL_EXPORT_CFLAGS and similar other variables).

Here, three modules are defined: foo, bar, zoo

'foo' exports its include directory and a linker flag
bar simply uses 'foo', as a static library
zoo uses bar, is a shared library.

