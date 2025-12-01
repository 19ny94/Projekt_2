#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	/*obsah dat*/
	double* total_bytes;
	/*IP odesilatele*/
	int src_ip;
	/*IP prijemce*/
	int dst_ip;
	/*delka toku dat*/
	double* flow_dur;
	/*kolik paketu se poslalu/dostalo*/
	double* packet_count;
	/*prumerna delka*/
	double* avg_int;
	
} data_z_souboru;

data_z_souboru *soubor_data_ctor(){

	data_z_souboru *s_data= malloc(sizeof(data_z_souboru));
	
	s_data->count = 0;
	s_data->flowid = NULL;
	s_data->src_ip = 0;
	s_data->dst_ip = 0;
	s_data->total_bytes = NULL;
	s_data->flow_dur = NULL;
	s_data->packet_count = NULL;
	s_data->avg_int = NULL;

	return s_data;
}

double *resize(double *arr, unsigned int new_size){
	
	double* new_arr = realloc(arr, new_size * sizeof(double));
	
	if(new_arr == NULL){
	free(new_arr);
	exit(1);
	}

	return new_arr;
}

double mocnina(int x, int y){

double nove_x = x;

for(int i = 0; i < y - 1; i++){
	x*= nove_x;
	}
	
	if(y == 0){
		return 1;
	}
	
	if(y < 0){
		/*Jedine mozne reseni jak udelat zapornou mocninu
		 * funkce se zase vola z duvodu, protoze v tom cyklu
		 * nemuzu jako by vzat 10 -1krat*/
		return 1/mocnina(x, -y);
	}

	return x;
}

double double_z_stringu(char *c){

double result = 0;
int len = strlen(c);
/*jsou to dva pole, prvni ulozi cisla pred carou
 * druha ulozi po care, rozdil v tom,
 * ze cast po care budu mocnit na zapornou*/	
char *pred_carou = malloc(len + 1);
char *po_care = malloc(len + 1);
int pozice_cary = 0;

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
	if(pozice_cary == 0){
	for(int i = 0; i < len; i++){
		result += ((c[i] - '0') * mocnina (10, len -1 -i));
		}
	}
	/*do "po_care" se zapisou desetinna cisla, do "pred_carou" - cela*/	
	if(pozice_cary != 0){
	for(int i = pozice_cary; c[i] != '\0'; i++){
		po_care[i - pozice_cary] = c[i + 1];
		}
	for(int i = 0; i < pozice_cary; i++){
		pred_carou[i] = c[i];
		}

		pred_carou[pozice_cary] = '\0';
		po_care[len - pozice_cary - 1] = '\0';
int len1 = strlen(pred_carou);
		/*v podstate to stejny ale zaporna mocnina*/
	for(int i = 0; po_care[i] != '\0'; i++){
		result += ((po_care[i] - '0') * mocnina(10, -1 -i));
		}
		/*bezne pretypovani*/
	for(int i = 0; pred_carou[i] != '\0'; i++){
		result += ((pred_carou[i] - '0') * mocnina(10,len1 -1 -i));
		}
	}
	free(pred_carou);
	free(po_care);
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

double *pole_pro_kontrolu[4] = {&data->WB, &data->WD, &data->WD, &data->WS};
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
		else{
		printf("%.1lf ", *pole_pro_kontrolu[i]);
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
	
	printf("%s\n", buffer);
	
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
	sou_data->total_bytes=resize(sou_data->total_bytes,sou_data->count);
	sou_data->flow_dur=resize(sou_data->flow_dur,sou_data->count);
	sou_data->packet_count=resize(sou_data->packet_count, sou_data->count);

	sou_data->avg_int=resize(sou_data->avg_int, sou_data->count);
	
	char *c = malloc(sizeof(char));

	int stav = OUT;
	int slovo = 0;
	int i = 0;
	int g = 0;
	int radek = 0;
	printf("\n");/*PAK ODSTANIT*/
	while((g = fgetc(soubor)) != EOF){
	putchar(g);/*PAK ODSTANIT*/
		
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
nacitani_jednotlyvych_dat(&c,&i,g,radek,sou_data->total_bytes);
		}
		if(stav == IN && slovo == 5){
nacitani_jednotlyvych_dat(&c,&i,g,radek,sou_data->flow_dur);
		}
		if(stav == IN && slovo == 6){
nacitani_jednotlyvych_dat(&c,&i,g,radek,sou_data->packet_count);
		}
		if(stav == IN && slovo == 7){
nacitani_jednotlyvych_dat(&c,&i,g,radek,sou_data->avg_int);
		}

		if(g == '\n'){
		slovo = 0;
		radek++;
		}
	}
	/*PAK ODSTANIT*/
	printf("\nFlowid: ");
	for(int j = 0; j < sou_data->count; j++){
	printf("%d ", sou_data->flowid[j]);
	}
	printf("\nTotal_bytes: ");
	for(int j = 0; j < sou_data->count; j++){
	printf("%.0f ", sou_data->total_bytes[j]);
	}
	printf("\nFlow_duration: ");
	for(int j = 0; j < sou_data->count; j++){
	printf("%.0f ", sou_data->flow_dur[j]);
	}
	printf("\nPacket_count: ");
	for(int j = 0; j < sou_data->count; j++){
	printf("%.0f ", sou_data->packet_count[j]);
	}
	printf("\nAvg_int: ");
	for(int j = 0; j < sou_data->count; j++){
	printf("%.2f ", sou_data->avg_int[j]);
	}
	free(c);
	printf("\n");
	fclose(soubor);
}

/*data kaput*/
void data_dtor(vstupni_data *data){
	free(data->soubor);
	free(data);
}

void soubor_data_dtor(data_z_souboru *sou_data){
free(sou_data->flowid);
free(sou_data->total_bytes);
free(sou_data->flow_dur);
free(sou_data->packet_count);
free(sou_data->avg_int);
free(sou_data);
}

int main(int argc, char** argv){

	vstupni_data *data = data_ctor();

	data_z_souboru *sou_data = soubor_data_ctor();

	nacitani_nazvu_souboru(argc,argv,data);
	
	nacitani_argumentu(argv,data);

	nacitani_vstupnich_dat(data, sou_data);

	data_dtor(data);
	soubor_data_dtor(sou_data);

	return 0; 
}

