/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  Definicion de funciones del manejador de memoria
*/

#include "mmu.h"
#include "i386.h"

#include "kassert.h"

static pd_entry_t* kpd = (pd_entry_t*)KERNEL_PAGE_DIR;
static pt_entry_t* kpt = (pt_entry_t*)KERNEL_PAGE_TABLE_0;

static const uint32_t identity_mapping_end = 0x003FFFFF;
static const uint32_t user_memory_pool_end = 0x02FFFFFF;

static paddr_t next_free_kernel_page = 0x100000;
static paddr_t next_free_user_page = 0x400000;

/**
 * kmemset asigna el valor c a un rango de memoria interpretado
 * como un rango de bytes de largo n que comienza en s
 * @param s es el puntero al comienzo del rango de memoria
 * @param c es el valor a asignar en cada byte de s[0..n-1]
 * @param n es el tamaño en bytes a asignar
 * @return devuelve el puntero al rango modificado (alias de s)
*/
static inline void* kmemset(void* s, int c, size_t n) {
  uint8_t* dst = (uint8_t*)s;
  for (size_t i = 0; i < n; i++) {
    dst[i] = c;
  }
  return dst;
}

/**
 * zero_page limpia el contenido de una página que comienza en addr
 * @param addr es la dirección del comienzo de la página a limpiar
*/
static inline void zero_page(paddr_t addr) {
  kmemset((void*)addr, 0x00, PAGE_SIZE);
}


void mmu_init(void) {}


/**
 * mmu_next_free_kernel_page devuelve la dirección física de la próxima página de kernel disponible. 
 * Las páginas se obtienen en forma incremental, siendo la primera: next_free_kernel_page
 * @return devuelve la dirección de memoria de comienzo de la próxima página libre de kernel
 */
paddr_t mmu_next_free_kernel_page(void) {
      paddr_t ant = next_free_kernel_page;
      next_free_kernel_page += PAGE_SIZE;
      return ant;
}

/**
 * mmu_next_free_user_page devuelve la dirección de la próxima página de usuarix disponible
 * @return devuelve la dirección de memoria de comienzo de la próxima página libre de usuarix
 */
paddr_t mmu_next_free_user_page(void) {
      paddr_t ant = next_free_user_page;
      next_free_user_page += PAGE_SIZE;
      return ant;
  
}

/**
 * mmu_init_kernel_dir inicializa las estructuras de paginación vinculadas al kernel y
 * realiza el identity mapping
 * @return devuelve la dirección de memoria de la página donde se encuentra el directorio
 * de páginas usado por el kernel
 */


paddr_t mmu_init_kernel_dir(void) {
  // Atributos (por bit): 
  // 0: P   -> 1
  // 1: R/W -> 1
  // 2: U/S -> 0 (supervisor)
  // 3: PWT -> 0
  // 4: PCD -> 0
  // 5: A   -> 0
  // 6-11:  -> 0
  // Asignamos primer entrada del directorio de páginas de kernel
  zero_page(KERNEL_PAGE_DIR);
  zero_page(KERNEL_PAGE_TABLE_0);
  
  kpd->attrs = 0x3;
  kpd->pt = ((paddr_t)kpt >> 12); //paddr_t es 32 bits entonces nos quedamos con la parte de los 20
                                  // que es la parte de la direccion

  // Mapeamos la primer tabla de páginas del 0x000000 al 0x3FFFFF
    // 1023 * 4096 = 0x400000
  for(int i = 0; i < 1024; i++){
    kpt[i].attrs = 0x3;
    kpt[i].page = i; // entonces el i en este corresponde al indice de la pagina fisica
  }

  return KERNEL_PAGE_DIR ;
}

// CONSULTAR: Necesitamos que se nos explique en profundidad esta función. ¿Retornamos todo paddr_t?? En cr3 guardamos todo paddr_t ???  

/**
 * mmu_map_page agrega las entradas necesarias a las estructuras de paginación de modo de que
 * la dirección virtual virt se traduzca en la dirección física phy con los atributos definidos en attrs
 * @param cr3 el contenido que se ha de cargar en un registro CR3 al realizar la traducción
 * @param virt la dirección virtual que se ha de traducir en phy
 * @param phy la dirección física que debe ser accedida (dirección de destino)
 * @param attrs los atributos a asignar en la entrada de la tabla de páginas
 */
void mmu_map_page(uint32_t cr3, vaddr_t virt, paddr_t phy, uint32_t attrs) {
  uint32_t pd_index = VIRT_PAGE_DIR(virt);
  uint32_t pt_index = VIRT_PAGE_TABLE(virt);
  uint32_t offset = VIRT_PAGE_OFFSET(virt);

  pd_entry_t* PD_base = CR3_TO_PAGE_DIR(cr3);


  if(!(PD_base[pd_index].attrs & MMU_P )){ // vemos si el page directory entry esta 
    paddr_t new_PTaddr = mmu_next_free_kernel_page();
    zero_page(new_PTaddr);
    PD_base[pd_index].pt = new_PTaddr >> 12; //Guardamos los 20bits mas significativos de la direccion fisica.
  }
    PD_base[pd_index].attrs |= MMU_P | attrs; 

  pt_entry_t* PT_base = MMU_ENTRY_PADDR(PD_base[pd_index].pt); 

  PT_base[pt_index].page = phy >> 12;
  PT_base[pt_index].attrs |= MMU_P | attrs;

  tlbflush();
}





//PREGUNTAS: Cuando hablamos de agregar las entradas.. las tenemos que armar desde 0? O podemos suponer que lo indices del directorio y tabla son validos?. 
//que pasa si me pasan una dirección virtual que no es válida. 
//Cada proceso tiene su propio directorio...Entonces como nos garantizamos que en la direccioón phy no hay info de otro proceso. 

/**
 * mmu_unmap_page elimina la entrada vinculada a la dirección virt en la tabla de páginas correspondiente
 * @param virt la dirección virtual que se ha de desvincular
 * @return la dirección física de la página desvinculada
 */
paddr_t mmu_unmap_page(uint32_t cr3, vaddr_t virt){
  uint32_t pd_index = VIRT_PAGE_DIR(virt);
  uint32_t pt_index = VIRT_PAGE_TABLE(virt);
  uint32_t offset = VIRT_PAGE_OFFSET(virt);

  pd_entry_t* PD_base = CR3_TO_PAGE_DIR(cr3);
  pt_entry_t* PT_base = PD_base[pd_index].pt;

  paddr_t phy = MMU_ENTRY_PADDR(PT_base[pt_index].page);
  PT_base[pt_index].attrs &= 0xFFE;
  tlbflush();
  return phy;
}


#define DST_VIRT_PAGE 0xA00000
#define SRC_VIRT_PAGE 0xB00000

/**
 * copy_page copia el contenido d;e la página física localizada en la dirección src_addr a la página física ubicada en dst_addr
 * @param dst_addr la dirección a cuya página queremos copiar el contenido
 * @param src_addr la dirección de la página cuyo contenido queremos copiar
 *
 * Esta función mapea ambas páginas a las direcciones SRC_VIRT_PAGE y DST_VIRT_PAGE, respectivamente, realiza
 * la copia y luego desmapea las páginas. Usar la función rcr3 definida en i386.h para obtener el cr3 actual
 */
void copy_page(paddr_t dst_addr, paddr_t src_addr) {
  uint32_t cr3 = rcr3();
  mmu_map_page(cr3,SRC_VIRT_PAGE,src_addr,MMU_P);
  mmu_map_page(cr3,DST_VIRT_PAGE,dst_addr,MMU_P | MMU_W); //aca mapeamos ambas páginas. 
  
  //ahora hacemos la copia
  uint32_t* dst = (uint32_t*)DST_VIRT_PAGE;
  uint32_t* src = (uint32_t*)SRC_VIRT_PAGE;

  for(uint32_t i = 0; i < 1024; i++){
    dst[i] = src[i];
  }
  mmu_unmap_page(cr3,DST_VIRT_PAGE);
  mmu_unmap_page(cr3,SRC_VIRT_PAGE);
  
}



  #define VIRT_CODE 0x08000000
  #define VIRT_STACK 0x08003000
 /**
 * mmu_init_task_dir inicializa las estructuras de paginación vinculadas a una tarea cuyo código se encuentra en la dirección phy_start
 * @pararm phy_start es la dirección donde comienzan las dos páginas de código de la tarea asociada a esta llamada
 * @return el contenido que se ha de cargar en un registro CR3 para la tarea asociada a esta llamada
 */
paddr_t mmu_init_task_dir(paddr_t phy_start) {

  pd_entry_t* pdt_b = (pd_entry_t*)mmu_next_free_kernel_page();
  pt_entry_t* pt_b = (pt_entry_t*)mmu_next_free_kernel_page();

  zero_page(pdt_b);
  zero_page(pt_b);

  pdt_b->attrs = 0x3;
  pdt_b->pt = ((paddr_t)pt_b >> 12);

  //mapeamos el table

  for(int i = 0; i < 1024; i++){
    pt_b[i].attrs = 0x3;
    pt_b[i].page = i;
  }

  paddr_t cr3 = (paddr_t)pdt_b;
  // mapeamos el de codigo
  mmu_map_page(cr3,VIRT_CODE,phy_start,MMU_P|MMU_U);
  mmu_map_page(cr3,VIRT_CODE+PAGE_SIZE,phy_start+PAGE_SIZE, MMU_P| MMU_U);

  //mapeo el de pila
  paddr_t pila_addr = mmu_next_free_user_page();

  mmu_map_page(cr3,0x8002000, pila_addr, MMU_P|MMU_W|MMU_U);

  // mapeo el compartido 
  mmu_map_page(cr3,VIRT_STACK, SHARED, MMU_P|MMU_U);

  return cr3;
}

// COMPLETAR: devuelve true si se atendió el page fault y puede continuar la ejecución 
// y false si no se pudo atender
bool page_fault_handler(vaddr_t virt) {
  print("Atendiendo page fault...", 0, 0, C_FG_WHITE | C_BG_BLACK);
  // Chequeemos si el acceso fue dentro del area on-demand
  // En caso de que si, mapear la pagina
  
  
  if(virt >= 0x07000000 & virt <= 0x07000FFF){ //
    uint32_t cr3 = rcr3();
    //paddr_t new_addr = mmu_next_free_user_page();
    mmu_map_page(cr3,virt,0x3000000,MMU_W| MMU_P| MMU_U);
    return true;
  }

  return false;
  
}
// 
