#include <stdio.h>  // mainly just printf 
#include <stdlib.h> // size_t 

void list_color_options(void){
	for(size_t i = 30; i <= 37; i++){
		(void)printf("Color #%zu \033[%zum\n", i-30, i);	
	}
}

void change_color(int color){
	(void)printf("\033[%im", color); 
}
