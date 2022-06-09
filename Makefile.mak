.c.o:
        gcc -fPIC -c $<

# all: uart_messages.o bar.o main.o
#         gcc -shared uart_messages.o -o libUartMsg.dll -Wl,--out-implib,libuart_messages.a
#         gcc -shared bar.o -o libbar.dll -Wl,--out-implib,libbar.a
#         gcc main.o  -Wl,-rpath=. -L . -lbar -luart_messages -o main

all: uart_messages.o
        gcc -shared uart_messages.o -o libUartMsg.dll -Wl,--out-implib,libuart_messages.a