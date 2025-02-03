; ** por compatibilidad se omiten tildes **
; ==============================================================================
; TALLER System Programming - Arquitectura y Organizacion de Computadoras - FCEN
; ==============================================================================

%include "print.mac"

global start



; COMPLETAR - Agreguen declaraciones extern según vayan necesitando
extern print_text_pm
extern print_text_rm
extern A20_enable
extern C_FG_RED
extern A20_disable
extern GDT_DESC
extern screen_draw_layout


; Interrupciones

extern idt_init
extern IDT_DESC
extern pic_reset
extern pic_enable


; Paginacion
extern KERNEL_PAGE_DIR
extern mmu_init_kernel_dir
extern mmu_init_task_dir
extern mmu_init


; Tareas
extern tasks_screen_draw
extern tss_init
extern tss_initial
extern GDT_IDX_TASK_INITIAL
extern sched_init
extern tasks_init




; COMPLETAR - Definan correctamente estas constantes cuando las necesiten
%define CS_RING_0_SEL 0x8  
%define DS_RING_0_SEL 0x18   
%define GDT_SEG_TASK_IDLE (12 << 3)

BITS 16
;; Saltear seccion de datos
jmp start

;;
;; Seccion de datos.
;; -------------------------------------------------------------------------- ;;
start_rm_msg db     'Iniciando kernel en Modo Real'
start_rm_len equ    $ - start_rm_msg

start_pm_msg db     'Iniciando kernel en Modo Protegido'
start_pm_len equ    $ - start_pm_msg

;;
;; Seccion de código.
;; -------------------------------------------------------------------------- ;;

;; Punto de entrada del kernel.
BITS 16
start:
    ; COMPLETAR - Deshabilitar interrupciones
    cli 
    
    ; Cambiar modo de video a 80 X 50
    mov ax, 0003h
    int 10h ; set mode 03h
    xor bx, bx
    mov ax, 1112h
    int 10h ; load 8x8 font

    ; COMPLETAR - Imprimir mensaje de bienvenida - MODO REAL
    ; (revisar las funciones definidas en print.mac y los mensajes se encuentran en la
    ; sección de datos)

    
    print_text_rm start_rm_msg, start_rm_len, 0x3, 0, 0 
        

    ; COMPLETAR - Habilitar A20
    ; (revisar las funciones definidas en a20.asm)
    call A20_enable
    ; COMPLETAR - Cargar la GDT
    
    lgdt [GDT_DESC] 

; TAMOS PARA TESTEAR. AQUI VA UN BREAKPOINT 


    ; COMPLETAR - Setear el bit PE del registro CR0
    mov edx, cr0
    or edx, 1 
    mov cr0, edx 
    ; COMPLETAR - Saltar a modo protegido (far jump)
    ; (recuerden que un far jmp se especifica como jmp CS_selector:address)
    ; Pueden usar la constante CS_RING_0_SEL definida en este archivo
    jmp CS_RING_0_SEL:modo_protegido 


BITS 32
modo_protegido:
    ; COMPLETAR - A partir de aca, todo el codigo se va a ejectutar en modo protegido
    ; Establecer selectores de segmentos DS, ES, GS, FS y SS en el segmento de datos de nivel 0
    ; Pueden usar la constante DS_RING_0_SEL definida en este archivo
    mov di, DS_RING_0_SEL
    mov ds, di
    mov ss, di
    mov es, di
    mov gs, di
    mov fs, di
    ; COMPLETAR - Establecer el tope y la base de la pila
    mov ebp, 0x25000 
    mov esp, 0x25000
    
    
    ; COMPLETAR - Imprimir mensaje de bienvenida - MODO PROTEGIDO
    print_text_pm start_pm_msg, start_pm_len, 0x5, 0, 0
        
    
    ; COMPLETAR - Inicializar pantalla
    call screen_draw_layout


    ;       Paginacion

    ; Cargar directorio de paginas
    call mmu_init_kernel_dir 
    mov cr3, eax
    


    ; Habilitamos paginacion 
    mov edx, cr0
    or edx, 0x80000000
    mov cr0, edx


    ; Inicializar tss
    call tss_init
    call tasks_screen_draw
    xor eax, eax
    mov ax, (11 << 3)  ;|Index|TI|RPL| Armamos el selector de segmento | GDT_IDX_TASK_INITIAL = 11
    ltr ax
            
    
    ; Inicializar el scheduler

    call sched_init

    ; Inicializar las tareas

    call tasks_init

    
    ; COMPLETAR - Inicializar y cargar la IDT
    call idt_init
    lidt [IDT_DESC]


    ;agregamos pit
    mov ax, 1500

    out 0x40, al

    rol ax, 8

    out 0x40, al



    ; COMPLETAR - Reiniciar y habilitar el controlador de interrupciones
    call pic_reset
    call pic_enable

    ; COMPLETAR - Habilitar interrupciones
    ;sti ;habilito interrupciones  






    ; NOTA: Pueden chequear que las interrupciones funcionen forzando a que se
    ;       dispare alguna excepción (lo más sencillo es usar la instrucción
    ;       `int3`)
    ; int3

    ; Probar Sys_call
    ; int 3
    ; Probar generar una excepción
    ; int 32


    ; Inicializar el directorio de paginas de la tarea de prueba
    ;    TESTS : PAGINACION EJERCICIO F TERCERA PARTE

    ;push 0x18000
    ;call mmu_init_task_dir
    ;mov cr3, eax
    ;mov word[0x07000010], 2
    ;mov [0x07000002], ecx
    ;intentamos escribir en la pila
 

    ; Cargar directorio de paginas de la tarea



    ; Restaurar directorio de paginas del kernel    


    
    ; Saltar a la primera tarea: Idle

    jmp (12<<3):0

   
    ;Ciclar infinitamente 
    mov eax, 0xFFFF
    mov ebx, 0xFFFF
    mov ecx, 0xFFFF
    mov edx, 0xFFFF
    jmp $

;; -------------------------------------------------------------------------- ;;

%include "a20.asm"
