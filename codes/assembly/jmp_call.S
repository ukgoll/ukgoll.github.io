_start:
 jmp _Print;

_start_label:

 jmp _Exit;

_Exit:
 ; exit stuff goes here

 ret;

_Print:

;print stuff goes here

jmp _start_label;
