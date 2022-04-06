#include <asf.h>

#include "oled/gfx_mono_ug_2832hsweg04.h"
#include "oled/gfx_mono_text.h"
#include "oled/sysfont.h"
#include "PIO_FUNCTIONS.h"
#include "TC-RTT-RTC.h"

#define LED_1_PIO PIOA
#define LED_1_PIO_ID ID_PIOA
#define LED_1_IDX 0
#define LED_1_IDX_MASK (1 << LED_1_IDX)

#define LED_2_PIO PIOC
#define LED_2_PIO_ID ID_PIOC
#define LED_2_IDX 30
#define LED_2_IDX_MASK (1 << LED_2_IDX)

#define LED_3_PIO PIOB
#define LED_3_PIO_ID ID_PIOB
#define LED_3_IDX 2
#define LED_3_IDX_MASK (1 << LED_3_IDX)

#define BUT_1_PIO PIOD
#define BUT_1_PIO_ID ID_PIOD
#define BUT_1_IDX 28
#define BUT_1_IDX_MASK (1u << BUT_1_IDX)

#define BUT_2_PIO PIOC
#define BUT_2_PIO_ID ID_PIOC
#define BUT_2_IDX 31
#define BUT_2_IDX_MASK (1u << BUT_2_IDX)

#define BUT_3_PIO PIOA
#define BUT_3_PIO_ID ID_PIOA
#define BUT_3_IDX 19
#define BUT_3_IDX_MASK (1u << BUT_3_IDX)

// setando as variaveis do projeto

volatile char flag_but1 = 0;
volatile char flag_but2 = 0;
volatile char flag_but3 = 0;
volatile char rtt_flag = 0;
volatile char cofre_aberto = 0;
volatile char flag_tc_led = 0;
volatile int conta_erros = 0;
volatile char senha_criada = 0;

int senha[] = {0, 0, 0, 0, 0, 0};
int gabarito[] = {1, 1, 2, 2, 3, 1};
	

int contador = 0;

void but1_callback(void) {
	flag_but1 = 1;
}

void but2_callback(void) {
	flag_but2 = 1;
}

void but3_callback(void) {
	flag_but3 = 1;
}

void RTT_Handler(void) {
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		rtt_flag = 0;
		gfx_mono_draw_string("Cofre fechado", 0,0, &sysfont);
		gfx_mono_draw_string("         ", 0,18, &sysfont);
		contador = 0;	
	}
}

void TC3_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrupção foi satisfeita.
	* Isso é realizado pela leitura do status do periférico
	**/
	volatile uint32_t status = tc_get_status(TC1, 0);

	/** Muda o estado do LED (pisca) **/
	flag_tc_led = 1;
}

void verifica_senhas(void) {
	for (int i = 0; i < 6; i++){
		if (senha[i] != gabarito[i]) {
			if (conta_erros > 2) {
				RTT_init(4, 40, RTT_MR_ALMIEN);	
			} else {
				RTT_init(4, 16, RTT_MR_ALMIEN);
			}
			rtt_flag = 1;
			gfx_mono_draw_string("Senha errada ", 0,0, &sysfont);
			gfx_mono_draw_string("Bloqueado", 0,18, &sysfont);
			conta_erros += 1;
			return;
		}
	}
	gfx_mono_draw_string("Cofre aberto ", 0,0, &sysfont);
	gfx_mono_draw_string("         ", 0,18, &sysfont);
	pin_toggle(LED_1_PIO, LED_1_IDX_MASK);
	pin_toggle(LED_2_PIO, LED_2_IDX_MASK);
	pin_toggle(LED_3_PIO, LED_3_IDX_MASK);
	contador = 0;
	cofre_aberto = 1;
	conta_erros = 0;
	
}

void desenha_asterisco(void) {
	if (senha_criada) {
		gfx_mono_draw_string("Cofre fechado", 0,0, &sysfont);
	}
	
	if (contador == 0) {
		gfx_mono_draw_string("      ", 0,18, &sysfont);
	} else if (contador == 1) {
		 gfx_mono_draw_string("*     ", 0,18, &sysfont);
	} else if (contador == 2) {
		 gfx_mono_draw_string("**      ", 0,18, &sysfont);
	} else if (contador == 3) {
		 gfx_mono_draw_string("***     ", 0,18, &sysfont);
	} else if (contador == 4) {
		 gfx_mono_draw_string("****    ", 0,18, &sysfont);
	} else if (contador == 5) {
		gfx_mono_draw_string("*****   ", 0,18, &sysfont);
	} else if (contador == 6) {
		gfx_mono_draw_string("******  ", 0,18, &sysfont);
	}
	
}

void io_init(void) {
  pmc_enable_periph_clk(LED_1_PIO_ID);
  pmc_enable_periph_clk(LED_2_PIO_ID);
  pmc_enable_periph_clk(LED_3_PIO_ID);
  pmc_enable_periph_clk(BUT_1_PIO_ID);
  pmc_enable_periph_clk(BUT_2_PIO_ID);
  pmc_enable_periph_clk(BUT_3_PIO_ID);

  pio_configure(LED_1_PIO, PIO_OUTPUT_0, LED_1_IDX_MASK, PIO_DEFAULT);
  pio_configure(LED_2_PIO, PIO_OUTPUT_0, LED_2_IDX_MASK, PIO_DEFAULT);
  pio_configure(LED_3_PIO, PIO_OUTPUT_0, LED_3_IDX_MASK, PIO_DEFAULT);

  pio_configure(BUT_1_PIO, PIO_INPUT, BUT_1_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
  pio_configure(BUT_2_PIO, PIO_INPUT, BUT_2_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
  pio_configure(BUT_3_PIO, PIO_INPUT, BUT_3_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);

  pio_handler_set(BUT_1_PIO, BUT_1_PIO_ID, BUT_1_IDX_MASK, PIO_IT_FALL_EDGE,
  but1_callback);
  pio_handler_set(BUT_2_PIO, BUT_2_PIO_ID, BUT_2_IDX_MASK, PIO_IT_FALL_EDGE,
  but2_callback);
  pio_handler_set(BUT_3_PIO, BUT_3_PIO_ID, BUT_3_IDX_MASK, PIO_IT_FALL_EDGE,
  but3_callback);

  pio_enable_interrupt(BUT_1_PIO, BUT_1_IDX_MASK);
  pio_enable_interrupt(BUT_2_PIO, BUT_2_IDX_MASK);
  pio_enable_interrupt(BUT_3_PIO, BUT_3_IDX_MASK);

  pio_get_interrupt_status(BUT_1_PIO);
  pio_get_interrupt_status(BUT_2_PIO);
  pio_get_interrupt_status(BUT_3_PIO);

  NVIC_EnableIRQ(BUT_1_PIO_ID);
  NVIC_SetPriority(BUT_1_PIO_ID, 4);

  NVIC_EnableIRQ(BUT_2_PIO_ID);
  NVIC_SetPriority(BUT_2_PIO_ID, 4);

  NVIC_EnableIRQ(BUT_3_PIO_ID);
  NVIC_SetPriority(BUT_3_PIO_ID, 4);
}

int main(void) {
  board_init();
  sysclk_init();
  delay_init();
  io_init();
  gfx_mono_ssd1306_init();
  gfx_mono_draw_string("Crie a senha", 0,0, &sysfont);
  TC_init(TC1, ID_TC3, 0, 4);
  tc_start(TC1, 0);
  while (1) {
	  
	  
	  if (!rtt_flag && !cofre_aberto && senha_criada) {
		  gfx_mono_draw_string("Cofre fechado", 0,0, &sysfont);
		  pio_clear(LED_1_PIO, LED_1_IDX_MASK);
		  pio_clear(LED_2_PIO, LED_2_IDX_MASK);
		  pio_clear(LED_3_PIO, LED_3_IDX_MASK);
		  
		  if (flag_but1) {
			  senha[contador] = 1;
			  contador += 1;
			  
			  flag_but1 = 0;
			  
		  }
		  
		  if (flag_but2) {
			  senha[contador] = 2;
			  contador += 1;
			  
			  flag_but2 = 0;
		  }
		  
		  if (flag_but3) {
			  senha[contador] = 3;
			  contador += 1;
			  
			  flag_but3 = 0;
		  }
		  
		  desenha_asterisco();
		  if (contador == 6) {
			  verifica_senhas();
		  }
		  
	  }
	  
	  if (!senha_criada) {
		  
		  if (flag_tc_led) {
			  pin_toggle(LED_1_PIO, LED_1_IDX_MASK);
			  pin_toggle(LED_3_PIO, LED_3_IDX_MASK);
			  flag_tc_led = 0;
		  }
		  
		  if (flag_but1) {
			  gabarito[contador] = 1;
			  contador += 1;
			  
			  flag_but1 = 0;
			  
		  }
		  
		  if (flag_but2) {
			  gabarito[contador] = 2;
			  contador += 1;
			  
			  flag_but2 = 0;
		  }
		  
		  if (flag_but3) {
			  gabarito[contador] = 3;
			  contador += 1;
			  
			  flag_but3 = 0;
		  }
		  
		  desenha_asterisco();
		  if (contador == 6) {
			  senha_criada = 1;
		  }
		  
		  
		  
	  }
	  
	  if (rtt_flag) {
		  if (flag_tc_led) {
			  pin_toggle(LED_1_PIO, LED_1_IDX_MASK);
			  pin_toggle(LED_2_PIO, LED_2_IDX_MASK);
			  pin_toggle(LED_3_PIO, LED_3_IDX_MASK);
			  flag_tc_led = 0;
		  }
	  }
	  
	  if (cofre_aberto && senha_criada) {
		  if (flag_but1) {
			  cofre_aberto = 0;		
			  pio_set(LED_1_PIO, LED_1_IDX_MASK);
			  pio_set(LED_2_PIO, LED_2_IDX_MASK);
			  pio_set(LED_3_PIO, LED_3_IDX_MASK);
			  gfx_mono_draw_string("Cofre fechado", 0,0, &sysfont);	  
			  flag_but1 = 0;
		  }
	  }
	  pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	  
  }
}
