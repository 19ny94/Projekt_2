#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define IN 1 /*stav slova = pozice indexu je na slove*/
#define OUT 0 /*stav slova = pozice indexu je mimo slova*/

typedef struct{

	/*vstupni soubor s definici toku*/
	char* soubor;
	/*pozadovanz pocet vzslednych shl*/	
	unsigned int N;
	/*Vaha pro total_bytes*/
	double WB;
	/*Vaha pro flow_duration*/
	double WT;
	/*vaha pro avg_interarrival_time*/
	double WD;
	/*vaha pro prumernou delku paketu*/
	double WS;

} vstupni_data;

typedef struct{

	/*pocet toku*/
	unsigned int count; /*zda se, ze pocet toku byt zaponym nemuze*/
	/*cislo klastera*/
	unsigned int* flowid;
	/*obsah dat(total_bytes)*/
	double* total_b;
	/*delka toku dat*/
	double* flow_dur;
	/*kolik paketu se poslalu/dostalo*/
	double* pac_c;
	/*prumerna delka*/
	double* d_avg;
	/*prumerna delka packetu*/
	double* s_avg;
	
} data_z_souboru;

typedef struct{
	unsigned int* flows;
	int pocet;

} Flows;

typedef struct{
	Flows* data;
	int size;
} Clusters;



Flows *flows_ctor(){

	Flows *f=malloc(sizeof(Flows));
	f->flows = NULL;
	f->pocet = 0;

	return f;
}

Clusters *clusters_ctor(){

	Clusters *c=malloc(sizeof(Clusters));
	c->data = NULL;
	c->size = 0;

	return c;
}

data_z_souboru *soubor_data_ctor(){

	data_z_souboru *s_data= malloc(sizeof(data_z_souboru));
	
	s_data->count = 0;
	s_data->flowid = NULL;
	s_data->total_b = NULL;
	s_data->flow_dur = NULL;
	s_data->pac_c = NULL;
	s_data->d_avg = NULL;
	s_data->s_avg = NULL;

	return s_data;
}

double *resize(double *arr, unsigned int new_size){
	
	double* new_arr = realloc(arr, new_size * sizeof(double));

	if(new_arr == NULL){
	free(arr);
	exit(1);
	}

	return new_arr;
}



double double_z_stringu(char *c){

double result = 0;
int len = strlen(c);

/*nastavim na -1 protoze index cary muze 
 * byt jakymkoliv od 0 do nekonecna*/	
int pozice_cary = -1;

/*treba chci dostat int 10 z c[0] = 1, c[1] = 0
 *
 *int 1 = c[0] - '0'; 
 *int 0 = c[1] - '0';
 *
 *int 10 = 1 * 10 + 0 * 1;
 *for(int i = 0; i < strlen(c); i++)
 * int 10 = (c[i] - '0') * 10^len - 1 - i
 */
	/*hleda se cara*/
	for(int i = 0; i < len; i++){
		if(c[i] == '.'){
		pozice_cary = i;
		}
	}
	/*pokud neni - provede se bezne pretypovani*/
	if(pozice_cary == -1){
	for(int i = 0; i < len; i++){
		result += ((c[i] - '0') * pow(10, len -1 -i));
		}
	}
	else{

		/*bezne pretypovani*/
	for(int i = 0; i < pozice_cary; i++){
	result += ((c[i] - '0') * pow(10, pozice_cary - 1 -i));
		}

	/*v podstate to stejny ale zaporna mocnina*/
	for(int i = pozice_cary; i < len; i++){
	result += ((c[i] - '0') * pow(10, -1 -i));
		}
	}
	return result;
}

/*inicialzice stuktury */
vstupni_data *data_ctor(){

	vstupni_data *data = malloc(sizeof(vstupni_data));
	
	if(data == NULL){
	return NULL;
	}

	data->soubor = NULL;
	data->WB = 0;
	data->WT = 0;
	data->WD = 0;
	data->WS = 0;

	return data;
}

/*prvni argument pri spusteni programu - nazev souboru s daty*/
int nacitani_nazvu_souboru(int argc, char** argv, vstupni_data *data){

	
	if(argc >= 2){
		/*dalo by se mozna udelat pres
		 * strcpy, ale mozna tenhle zpusob bezpecnejsi*/
		for(int i = 0; argv[1][i] != '\0'; i++){
		data->soubor = realloc(data->soubor, sizeof(char) +i +1);
		data->soubor[i] = argv[1][i];
		}
		data->soubor[strlen(argv[1])] = '\0';
	}else{
	printf("Argumenty nejsou zadane\n");
	/*exit(1) pro me je zpusob ukonceni programu bez segmt. fault*/
	free(data->soubor);
	exit(1);
	}
	return 1;
}

void nacitani_argumentu(char** argv, vstupni_data *data){

double *pole_pro_kontrolu[4] = {&data->WB, &data->WT, &data->WD, &data->WS};
	data->N = atoi(argv[2]);
	data->WB = atoi(argv[3]);
	data->WT = atoi(argv[4]);
	data->WD = atoi(argv[5]);
	data->WS = atoi(argv[6]);
		
	for(int i = 0; i < 4; i++){
		if(*pole_pro_kontrolu[i] < 0){
		printf("Argumenty jsou realna nezaporna cisla\n");
		exit(1);
			}
	}
}

void nacitani_jednotlyvych_dat(char**c, int*i, int g, int radek, double*arr){
	*c = realloc(*c, sizeof(char) +(*i) + 1);
	(*c)[*i] = g;
	(*i)++;
	(*c)[*i] = '\0';
	arr[radek] = double_z_stringu(*c);
}

void nacitani_vstupnich_dat(vstupni_data *data, data_z_souboru *sou_data ){

	FILE *soubor;

	soubor = fopen(data->soubor, "r");

	char buffer[sizeof(soubor) * 100];

	if(soubor == NULL){
	exit(1);
	}

	/*pri prvni iteraci dostavam count, cili pocet vstpnich klasteru*/
	fgets(buffer, sizeof(buffer), soubor);
	
	int len = strlen(buffer);

	for(int i = 0; i < len - 1; i++){

		if(buffer[i] == '='){
		/*ja nevim proc nefunguje atoi, ale jinak
		 * dostavam char '4', ktere v ASCII dava 52
		 * abych z toho dostal 4 jako int 
		 * tak udelam '4' - '0' == 52 - 48*/
		sou_data->count = buffer[i+1] - '0';
		}
	}

	sou_data->flowid=realloc(sou_data->flowid, sizeof(int) * sou_data->count);
	sou_data->total_b=resize(sou_data->total_b,sou_data->count);
	sou_data->flow_dur=resize(sou_data->flow_dur,sou_data->count);
	sou_data->pac_c=resize(sou_data->pac_c, sou_data->count);
	sou_data->d_avg=resize(sou_data->d_avg, sou_data->count);
	sou_data->s_avg=resize(sou_data->s_avg, sou_data->count);

	char *c = malloc(sizeof(char));

	int stav = OUT;
	int slovo = 0;
	int i = 0;
	int g = 0;
	int radek = 0;
	while((g = fgetc(soubor)) != EOF){
		
		if(g == ' ' || g == '\t' || g == '\n'){
		stav = OUT;
		i = 0;
		free(c);
		c = malloc(sizeof(char));
		}
		else if(stav == OUT){
		stav = IN;
		++slovo;
		}

		if(stav == IN && slovo == 1){
		c = realloc(c, sizeof(char) +i + 1);
		c[i] = g;
		i++;
		c[i] = '\0';
		sou_data->flowid[radek] = double_z_stringu(c);
		}
		
		if(stav == IN && slovo == 4){
nacitani_jednotlyvych_dat(&c,&i,g,radek,sou_data->total_b);
		}
		if(stav == IN && slovo == 5){
nacitani_jednotlyvych_dat(&c,&i,g,radek,sou_data->flow_dur);
		}
		if(stav == IN && slovo == 6){
nacitani_jednotlyvych_dat(&c,&i,g,radek,sou_data->pac_c);
		}
		if(stav == IN && slovo == 7){
nacitani_jednotlyvych_dat(&c,&i,g,radek,sou_data->d_avg);
		}

		if(g == '\n'){
		slovo = 0;
		radek++;
		}
	}

	free(c);
	fclose(soubor);
}

/*data kaput*/
void data_dtor(vstupni_data *data){
	free(data->soubor);
	free(data);
}

void soubor_data_dtor(data_z_souboru *sou_data){
free(sou_data->flowid);
free(sou_data->total_b);
free(sou_data->flow_dur);
free(sou_data->pac_c);
free(sou_data->d_avg);
free(sou_data->s_avg);
free(sou_data);
}

int pokud_je_prvkem(data_z_souboru *s_data, unsigned potencialni_flowid){
	
	for(int i = 0; i < s_data->count; i++){
		if(s_data->flowid[i] == potencialni_flowid){
			return i;
			}
		}
	return -1;
}

double dist(vstupni_data *data, data_z_souboru *s_data, int A, int B){

double result = 0;

	result += sqrt(data->WB * pow(s_data->total_b[A] - s_data->total_b[B], 2)+
		       data->WT * pow(s_data->flow_dur[A] - s_data->flow_dur[B],2)+
		       data->WD * pow(s_data->d_avg[A] - s_data->d_avg[B], 2)+
		       data->WS * pow(s_data->s_avg[A] - s_data->s_avg[B], 2));
	return result;
}

void inicializace_pole_clusteru(Clusters *c, data_z_souboru *s_data){

	c->data = malloc(sizeof(Flows) * s_data->count);
	c->size = s_data->count;
	
	for(int i = 0; i < s_data->count; i++){
	c->data[i].flows = malloc(sizeof(unsigned int));
	c->data[i].flows[0] = s_data->flowid[i];
	c->data[i].pocet = 1;
	}
}

void hledani_minima(Clusters *c, data_z_souboru *s_data, vstupni_data *data, int *idx_A, int *idx_B){
	
double min = 999999999;
int d = 0;

	for(int i = 0; i < c->size; i++){
		for(int j = i + 1; j < c->size; j++){

			for(int a = 0; a < c->data[i].pocet; a++){
		int x = pokud_je_prvkem(s_data, c->data[i].flows[a]);	
				for(int b = 0; b < c->data[j].pocet; b++){
		int y = pokud_je_prvkem(s_data, c->data[j].flows[b]);

		if(x == -1 || y == -1){
		printf("Nevalidni data\n");
		exit(1);	
		}
		
			d = dist(data, s_data, x, y);

			if(min > d){
			min = dist(data, s_data, x, y);
			*idx_A = i;
			*idx_B = j;
					}
				}
			}
     		}
	}
}


void shlukovaci_analyza(Clusters *c, data_z_souboru *s_data, vstupni_data *data){

int idx_A = 0;
int idx_B = 0;

	while(c->size > data->N){
	hledani_minima(c, s_data, data, &idx_A, &idx_B);

	int stary_pocet = c->data[idx_A].pocet;
	int new_pocet = c->data[idx_A].pocet + c->data[idx_B].pocet;
	c->data[idx_A].pocet = new_pocet;	
	c->data[idx_A].flows = realloc(c->data[idx_A].flows, sizeof(int) * new_pocet);

	for(int i = 0 ; i < c->data[idx_B].pocet; i++){
	c->data[idx_A].flows[stary_pocet + i] = c->data[idx_B].flows[i];
	}

	/*prednaska IZP 7*/
	for(int i = idx_B; i < c->size - 1; i++){
	c->data[i] = c->data[i + 1];
	}
	c->size--;
	}
}

void print_vysledku(Clusters *c){

	printf("Clusters: \n");
	for(int i = 0; i < c->size; i++){
	printf("cluster %d: ", i);
		for(int j = 0; j < c->data[i].pocet; j++){
		printf("%d ", c->data[i].flows[j]);
		}
		printf("\n");
	}
}

void clusters_dtor(Clusters *c){

	for(int i = 0; i < c->size; i++){
		free(c->data[i].flows);
		}
	free(c->data);
	free(c);
}

int main(int argc, char** argv){

	vstupni_data *data = data_ctor();
	data_z_souboru *sou_data = soubor_data_ctor();
	Flows *f = flows_ctor();
	Clusters *c = clusters_ctor();

	nacitani_nazvu_souboru(argc,argv,data);
	
	nacitani_argumentu(argv,data);

	nacitani_vstupnich_dat(data, sou_data);

	inicializace_pole_clusteru(c, sou_data);

	shlukovaci_analyza(c, sou_data, data);

	print_vysledku(c);

	data_dtor(data);
	soubor_data_dtor(sou_data);
	clusters_dtor(c);

	return 0; 
}


