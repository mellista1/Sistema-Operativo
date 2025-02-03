#include "task_lib.h"

#define WIDTH TASK_VIEWPORT_WIDTH
#define HEIGHT TASK_VIEWPORT_HEIGHT
#define TOTALHEIGHT 5

#define SHARED_SCORE_BASE_VADDR (PAGE_ON_DEMAND_BASE_VADDR + 0xF00)
#define CANT_PONGS 3


void task(void) {
	screen pantalla;
	// ¿Una tarea debe terminar en nuestro sistema?
	while (true)
	{
	// Completar:
	// - Pueden definir funciones auxiliares para imprimir en pantalla
	// - Pueden usar `task_print`, `task_print_dec`, etc.
		uint32_t y = 5;
		for(uint8_t i = 0; i < CANT_PONGS; i++)
		{
			struct puntaje
			{
				uint32_t puntaje1;
				uint32_t puntaje2;
			};

			struct puntaje* scores = (struct puntaje*) SHARED_SCORE_BASE_VADDR;

			
			task_print_dec(pantalla,scores[i].puntaje1,6,(WIDTH/2)-5,y, C_BG_RED|C_BG_CYAN);
			task_print_dec(pantalla,scores[i].puntaje2,6,(WIDTH/2)-5,y+2, C_BG_BLUE|C_BG_CYAN);
			y += 5;
		} 
		syscall_draw(pantalla);
	}
}
