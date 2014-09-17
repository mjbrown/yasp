yasp
====

Yet Another Serial Protocol

Because all the world needs is another new protocol that is nearly the same as every other one.  I am sick of inventing a new one every time I start a new firmware project.

PIC Example Usage
====
1. Open the Microchip "CDC Basic" demo project from the Microchip Application Library (or whatever they are calling it these days).
2. Make sure it works on your target platform without modification.
3. Drop in yasp.c
4. Add yasp_Init() prior to the main loop
4. Add the yasp service to the main loop and disable "app_basic_cdc"
5. Add the CDCTxService to the main loop
6. Run the yasp.py script to see that it works.