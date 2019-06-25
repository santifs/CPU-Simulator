#include <stdio.h>

#define TMEMORIA 1000
#define NUMREGS 8
#define STOP 0x3800

unsigned short mem[TMEMORIA];  /* la memoria del simulador */
unsigned short R[NUMREGS];     /* los registros de la CPU del simulador */
unsigned short pc, pc_inicial, pos_origen, pos_final;
int zf,sf,cf,of;               /* los flags de la ALU */

void InicializarMemoria (unsigned short m[], int tmem);
int  CargarMemoria      (unsigned short m[]);
void GrabarMemoria      (unsigned short m[], unsigned short pc, 
                         unsigned short origen, unsigned short fin);
int  Simulador          (void);
void MostrarRegistros   (void);
void ActualizaALU       (unsigned short op1, unsigned short op2, 
                         unsigned short r16, unsigned int r32);
int  obtclase           (unsigned short ins);
int  obtcodigo          (unsigned short ins, int clase);

int main (void)
{
	int excepcion;

	printf ("SIMULADOR DE CPU. E.P.S. CURSO 2014-2015\n");
	printf ("Santiago Fernandez Scagliusi.\n\n");
	
	InicializarMemoria (mem, TMEMORIA);
	if (CargarMemoria(mem))
	{
		excepcion = Simulador();
		MostrarRegistros(); 
		if (excepcion == 0) 
		{
			GrabarMemoria(mem, pc_inicial, pos_origen, pos_final);
		}
	}
	else
	{
		printf ("ERROR! No encuentro el fichero memoria.txt\n");
	}
	return 0;
}

void MostrarRegistros (void)
{
	int i;

	for (i=0;i<NUMREGS;i++)
	{
    	printf ("R%d = %04.4X\n", i, R[i]);
	}
  	printf ("ZF=%d  SF=%d  CF=%d  OF=%d\n\n", zf, sf, cf, of);
}

void InicializarMemoria (unsigned short m[], int tmem){

	for (tmem=0 ; tmem<TMEMORIA ; tmem++){
		m[tmem]=0;
	}
}

int CargarMemoria (unsigned short m[]){
	
	FILE *memoria;
	int i;
	
	memoria = fopen("memoria.txt", "rt");
	if (memoria == NULL){
		printf("ERROR! No puedo abrir memoria.txt\n");
		return 0;
	}else {
		fscanf(memoria, "%X", &pos_origen);
		fscanf(memoria, "%X", &pc_inicial);
		
		i=pos_origen;
		fscanf(memoria, "%X", &m[i]);
		while (!feof(memoria)){
			i++;
			fscanf(memoria, "%X", &m[i]);
		}
		
		pos_final=i;

		fclose( memoria );
		return 1;
	
	}

}

void GrabarMemoria (unsigned short m[], unsigned short pc, unsigned short origen, unsigned short fin){
	
	int i;
	FILE *resultados;
	
	resultados=fopen("resultados.txt", "wt");
	
	
		if (!resultados){
		printf("ERROR! No puedo crear resultados.txt.\n");
	}else {
		
		fprintf(resultados, "%4.4X\n", pos_origen);
		fprintf(resultados, "%4.4X\n", pc_inicial);
		for (i=origen ; i<fin ; i++){
			fprintf(resultados, "%4.4X\n", m[i]);
		}
	}
	fclose(resultados);
	printf("Programa terminado. ");
}

int Simulador(void){
	
	int aux, aux1;
	unsigned short int ins;
	unsigned short int clase;
	unsigned short int codins;
	int excepcion=0;
	int ri, rs, rs1, rs2, rd, rds, inm8, cond, temp;
	unsigned short pc, mar, op1, op2;
	short desplaz, aux2, aux3;
	
	zf=0;
	sf=0;
	of=0;
	cf=0;
	
	pc=pc_inicial;
	while ( ins!=STOP && excepcion==0 ){
		
		ins = mem[pc];
		pc++;
		
		clase=obtclase(ins);
		codins=obtcodigo(ins, clase);
		
		if (clase==0){
			if (codins==0){
				
				//printf("MOV Rd,Rs\n");
				rd = (ins>>8)&7;
				rs = (ins>>5)&7;
				R[rd] = R[rs];
				
				
			}else if (codins==1){
				
				//printf("MOV Rd,[Ri]\n");
				rd = (ins>>8)&7;
				ri = (ins>>5)&7;
				mar = R[ri];
				if (mar < TMEMORIA){
					R[rd] = mem[mar];
				}else {
					excepcion = 2;
				}
				
			}else if (codins==2){
				
				//printf("MOV [Ri],Rs\n");
				ri = (ins>>8)&7;
				rs = (ins>>5)&7;
				mar = R[ri];
				if (mar < TMEMORIA){
					mem[mar] = R[rs];
				}else {
					excepcion = 3;
				}
				
			}else if (codins==4){
				
				//printf("MOVL Rd,inm8\n");
				rd = (ins>>8)&7;
				inm8 = ins&255;
				R[rd] = (R[rd]&0xFF00) | inm8;
				
			}else if (codins==5){
				
				//printf("MOVH Rd,inm8\n");
				rd = (ins>>8)&7;
				inm8 = ins&255;
				R[rd] = (R[rd]&0x00FF) | (inm8<<8);
				
			}else if (codins==7){
				//printf("STOP\n");
				
			}else {
				printf("EXCEPCION\n");
				excepcion=1;
			}
		}else if (clase==1){
			if (codins==0){
				
				//printf("ADD Rd,Rs1,Rs2\n");
				rd = (ins>>6)&7;
				rs1 = (ins>>3)&7;
				rs2 = ins&7;
				op1 = R[rs1];
				op2 = R[rs2];
				temp = op1 + op2;
				R[rd] = temp;
				ActualizaALU (op1, op2, R[rd], temp);
				
			}else if (codins==1){
				
				//printf("SUB Rd,Rs1,Rs2\n");
				rd = (ins>>6)&7;
				rs1 = (ins>>3)&7;
				rs2 = ins&7;
				op1 = R[rs1];
				op2 = (~R[rs2])+1;
				temp = op1 + op2;
				R[rd] = temp;
				ActualizaALU (op1, op2, R[rd], temp);
				
			}else if (codins==2){
				
				//printf("OR Rd,Rs1,Rs2\n");
				rd = (ins>>6)&7;
				rs1 = (ins>>3)&7;
				rs2 = ins&7;
				op1 = R[rs1]; 
				op2 = R[rs2];
				temp = op1 | op2;
				R[rd] = temp;
				ActualizaALU (op1, op2, R[rd], temp);
				of = 0;
				cf = 0;
				
			}else if (codins==3){
				
				//printf("AND Rd,Rs1,Rs2\n");
				rd = (ins>>6)&7;
				rs1 = (ins>>3)&7;
				rs2 = ins&7;
				op1 = R[rs1];
				op2 = R[rs2];
				temp = op1 & op2;
				R[rd] = temp;
				ActualizaALU (op1, op2, R[rd], temp);
				of = 0;
				cf = 0;

			}else if (codins==4){
				
				//printf("XOR Rd,Rs1,Rs2\n");
				rd = (ins>>6)&7;
				rs1 = (ins>>3)&7;
				rs2 = ins&7;
				op1 = R[rs1];
				op2 = R[rs2];
				temp = op1^op2;
				R[rd] = temp;
				ActualizaALU (op1, op2, R[rd], temp);
				of = 0;
				cf = 0;
				
			}else if (codins==7){
				
				//printf("COMP Rs1,Rs2\n");
				rs1 = (ins>>6)&7;
				rs2 = (ins>>3)&7;
				op1 = R[rs1];
				op2 = (~R[rs2])+1;
				temp = op1 + op2;
				ActualizaALU (op1, op2, temp, temp); //CAMBIAAAAAAAAAR
				
			}else if (codins==8){
				
				//printf("NOT Rds\n");
				rds = (ins>>6)&7;
				R[rds] = (~R[rds]);
				ActualizaALU (0, 0, R[rds], 0);
				of = 0;
				cf = 0;
				
			}else if (codins==9){
				
				//printf("INC Rds\n");
				rds = (ins>>6)&7;
				op1 = R[rds];
				op2 = 1;
				temp = op1 + op2;
				R[rds] = temp;
				ActualizaALU (op1, op2, R[rds], temp);
				
			}else if (codins==10){
				
				//printf("DEC Rds\n");
				rds = (ins>>6)&7;
				op1 = R[rds];
				op2 = (~1)+1;
				temp = op1 + op2;
				R[rds] = temp;
				ActualizaALU (op1, op2, R[rds], temp);
				
			}else if (codins==11){
				
				//printf("NEG Rds\n");
				rds = (ins>>6)&7;
				op1 = (~R[rds]);
				op2 = 1;
				temp = op1 + op2;
				R[rds] = temp;
				ActualizaALU (op1, op2, R[rds], temp);
				of = 0;
				cf = 0;
				
			}else if (codins==12){
				
				//printf("CLR Rds\n");
				rds = (ins>>6)&7;
				R[rds] = 0;
				ActualizaALU (0, 0, 0, 0);
				sf = 0;
				of = 0;
				cf = 0;
				
			}else {
				printf("EXCEPCION\n");
				excepcion=1;
			}
		}else if (clase==2){
				
				//printf("JMP desplaz\n");
				desplaz = ins&16383;
				aux2 = (desplaz>>13)&1;	
				if (aux2==1){
					desplaz = desplaz | 0xC000;
				}
				mar = pc + desplaz;
					if (mar < TMEMORIA){
						pc = mar;
						
				}else {
					excepcion = 4;
				}
				
		}else if (clase==3){
			if (codins==0){
				
				//printf("BR C desplaz\n");
				cond = (ins>>11)&7;
				desplaz = ins&2047;
				aux2 = (desplaz>>10)&1;	
				if (aux2==1){
					desplaz = desplaz | 0xF800;
				}
				if (cf == 1){
					mar = pc + desplaz;
					if (mar < TMEMORIA){
						pc = mar;
						
					}else {
						excepcion = 4;
					}
				}
				
			}else if (codins==1){
				
				//printf("BR NC desplaz\n");
				cond = (ins>>11)&7;
				desplaz = ins&2047;
				aux2 = (desplaz>>10)&1;	
				if (aux2==1){
					desplaz = desplaz | 0xF800;
				}
				if (cf == 0){
					mar = pc + desplaz;
					if (mar < TMEMORIA){
						pc = mar;
						
					}else {
						excepcion = 4;
					}
				}
				
			}else if (codins==2){
				
				//printf("BR O desplaz\n");
				cond = (ins>>11)&7;
				desplaz = ins&2047;
				aux2 = (desplaz>>10)&1;	
				if (aux2==1){
					desplaz = desplaz | 0xF800;
				}
				if (of == 1){
					mar = pc + desplaz;
					if (mar < TMEMORIA){
						pc = mar;
						
					}else {
						excepcion = 4;
					}
				}
				
			}else if (codins==3){
				
				//printf("BR NO desplaz\n");
				cond = (ins>>11)&7;
				desplaz = ins&2047;
				aux2 = (desplaz>>10)&1;	
				if (aux2==1){
					desplaz = desplaz | 0xF800;
				}
				if (of == 0){
					mar = pc + desplaz;
					if (mar < TMEMORIA){
						pc = mar;
						
					}else {
						excepcion = 4;
					}
				}
				
			}else if (codins==4){
				
				//printf("BR Z desplaz\n");
				cond = (ins>>11)&7;
				desplaz = ins&2047;
				aux2 = (desplaz>>10)&1;	
				if (aux2==1){
					desplaz = desplaz | 0xF800;
				}
				if (zf == 1){
					mar = pc + desplaz;
					if (mar < TMEMORIA){
						pc = mar;
						
					}else {
						excepcion = 4;
					}
				}
				
			}else if (codins==5){
				
				//printf("BR NZ desplaz\n");
				cond = (ins>>11)&7;
				desplaz = ins&2047;
				aux2 = (desplaz>>10)&1;	
				if (aux2==1){
					desplaz = desplaz | 0xF800;
				}
				if (zf == 0){
					mar = pc + desplaz;
					if (mar < TMEMORIA){
						pc = mar;
						
					}else {
						excepcion = 4;
					}
				}
				
			}else if (codins==6){
				
				//printf("BR S desplaz\n");
				cond = (ins>>11)&7;
				desplaz = ins&2047;
				aux2 = (desplaz>>10)&1;	
				if (aux2==1){
					desplaz = desplaz | 0xF800;
				}
				if (sf == 1){
					mar = pc + desplaz;
					if (mar < TMEMORIA){
						pc = mar;
						
					}else {
						excepcion = 4;
					}
				}
				
			}else if (codins==7){
				
				//printf("BR NS desplaz\n");
				cond = (ins>>11)&7;
				desplaz = ins&2047;
				aux2 = (desplaz>>10)&1;	
				if (aux2==1){
					desplaz = desplaz | 0xF800;
				}
				if (sf == 0){
					mar = pc + desplaz;
					if (mar < TMEMORIA){
						pc = mar;
						
					}else {
						excepcion = 4;
					}
				}
				
			}
		}
	}
	
	switch (excepcion){
		case 0:
		printf ("Programa terminado con normalidad. Se encontro STOP en la direccion %4.4X\n", pc-1);
		break;
		case 1:
		printf ("EXCEPCION 1. Instruccion (%4.4X) desconocida en la direccion %4.4X\n", ins, pc-1);
		break;
		case 2:
		printf ("EXCEPCION 2. La instruccion (%4.4X), en la direccion %4.4X intento leer la direccion fuera de rango %4.4X\n", ins, pc-1, mar);
		break;
		case 3:
		printf ("EXCEPCION 3. La instruccion (%4.4X), en la direccion %4.4X intento escribir en la direccion fuera de rango %4.4X\n", ins, pc-1, mar);
		break;
		case 4:
		printf ("EXCEPCION 4. La instruccion (%4.4X), en la direccion %4.4X intento saltar a la direccion fuera de rango %4.4X\n", ins, pc-1, mar);
		break;
	}
	
	return (excepcion);
}

int obtclase (unsigned short ins){
	unsigned short clase;
	
	clase = (ins>>14)&3;
	return (clase);
}

int obtcodigo (unsigned short ins, int clase){
	unsigned short codins;
	if (clase==1){
		codins = (ins>>9)&31;
	}else if (clase==2){
		codins = 0;
	}else {
		codins = (ins>>11)&7;
	}
	return (codins);
}

void ActualizaALU (unsigned short op1, unsigned short op2, unsigned short r16, unsigned int r32){
	
	int aux1, aux2, aux3; 
	
	//FLAG DE ACARREO CF
	cf = (r32>>16)&1;
	
	//FLAG DE OVERFLOW OF
	aux1 = (op1>>15)&1;
	aux2 = (op2>>15)&1;
	aux3 = (r16>>15)&1;
	
	if (aux1==aux2 && aux1!=aux3 && aux2!=aux3){
		of = 1;
	}else {
		of = 0;
	}
	
	//FLAG DE SIGNO SF
	sf = (r16>>15)&1;
	
	//FLAG DE CERO ZF
	if (r16==0){
		zf = 1;
	}else {
		zf = 0;
	}
}

