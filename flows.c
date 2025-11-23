#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define IN 1
#define OUT 0

typedef struct{

	/*vstupni soubor s definici toku*/
	char* soubor;
	/*pozadovanz pocet vzslednych shl*/	
	unsigned int N;
	/*Vaha pro total_bytes*/
	unsigned int WB;
	/*Vaha pro flow_duration*/
	unsigned int WT;
	/*vaha pro avg_interarrival_time*/
	unsigned int WD;
	/*vaha pro prumernou delku paketu*/
	unsigned int WS;

} vstupni_data;

typedef struct{

	/*pocet toku*/
	unsigned int count; /*zda se, ze pocet toku byt zaponym nemuze*/
	/*cislo klastera*/
	unsigned int* flowid;
	/*obsah dat*/
	int* total_bytes;
	/*IP odesilatele*/
	int src_ip;
	/*IP prijemce*/
	int dst_ip;
	/*delka toku dat*/
	int* flow_dur;
	/*kolik paketu se poslalu/dostalo*/
	int* packet_count;
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

int *resize(int *arr, unsigned int new_size){
	
	int* new_arr = realloc(arr, new_size * sizeof(int));
	
	if(new_arr == NULL){
	exit(1);
	}

	return new_arr;
}

int mocnina(int x, int y){

int nove_x = x;

for(int i = 0; i < y - 1; i++){
	x*= nove_x;
	}
if(y == 0){
x = 1;
}

	return x;
}

int int_z_stringu(char *c){
int result = 0;
int len = strlen(c);
c[len] = '\0';
/*treba chci dostat int 10 z c[0] = 1, c[1] = 0
 *
 *int 1 = c[0] - '0'; 
 *int 0 = c[1] - '0';
 *
 *int 10 = 1 * 10 + 0 * 1;
 *for(int i = 0; i < strlen(c); i++)
 * int 10 = (c[i] - '0') * 10^len - 1 - i
 *
 * */
	for(int i = 0; i < len; i++){
	result += ((c[i] - '0') * mocnina (10, len -1 -i));
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
		data->soubor = realloc(data->soubor, sizeof(char)+1);
		data->soubor[i] = argv[1][i];
		}
	}else{
	printf("Argumenty nejsou zadane\n");
	/*exit(1) pro me je zpusob ukonceni programu bez segmt. fault*/
	exit(1);
	}
	return 1;
}


int nacitani_vstupnich_dat(vstupni_data *data, data_z_souboru *sou_data ){

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

	sou_data->flowid=resize(sou_data->flowid, sou_data->count);
	sou_data->total_bytes=resize(sou_data->total_bytes,sou_data->count);
	sou_data->flow_dur=resize(sou_data->flow_dur,sou_data->count);
	sou_data->packet_count=resize(sou_data->packet_count, sou_data->count);

	sou_data->avg_int=resize(sou_data->flowid, sou_data->count);
	
	char *c = malloc(sizeof(char));

	int stav = OUT;
	int slovo = 0;
	int i = 0;
	int g = 0;
	int result = 0;
	int radek = 0;
	while((g = fgetc(soubor)) != EOF){
	putchar(g);/*PAK ODSTANIT*/
		
		if(g == ' ' || g == '\t' || g == '\n'){
		stav = OUT;
		i = 0;
		c = malloc(sizeof(char));
		}
		else if(stav == OUT){
		stav = IN;
		++slovo;
		}

		if(stav == IN && slovo == 1){
		c = realloc(c, sizeof(char) +i +1);
		c[i] = g;
		sou_data->flowid[radek] = int_z_stringu(c);
		i++;
		}
		
		if(stav == IN && slovo == 4){
		c = realloc(c, sizeof(char) + i + 1);
		c[i] = g;
		sou_data->total_bytes[radek] = int_z_stringu(c);
		i++;
		}
		
		if(stav == IN && slovo == 5){
		c = realloc(c, sizeof(char) + i + 1);
		c[i] = g;
		sou_data->flow_dur[radek] = int_z_stringu(c);
		i++;
		}
	
		if(stav == IN && slovo == 6){
		c = realloc(c, sizeof(char) + i + 1);
		c[i] = g;
		sou_data->packet_count[radek] = int_z_stringu(c);
		i++;
		}

		if(stav == IN && slovo == 7){
		c = realloc(c, sizeof(char) + i + 1);
		c[i] = g;
		sou_data->avg_int[radek] = int_z_stringu(c);
		i++;
		}

		if(g == '\n'){
		slovo = 0;
		radek++;
		}
	}

	printf("\nFlowid: ");
	for(int j = 0; j < sou_data->count; j++){
	printf("%d ", sou_data->flowid[j]);
	}
	printf("\nTotal_bytes: ");
	for(int j = 0; j < sou_data->count; j++){
	printf("%d ", sou_data->total_bytes[j]);
	}
	printf("\nFlow_duration: ");
	for(int j = 0; j < sou_data->count; j++){
	printf("%d ", sou_data->flow_dur[j]);
	}
	printf("\nPacket_count: ");
	for(int j = 0; j < sou_data->count; j++){
	printf("%d ", sou_data->packet_count[j]);
	}
	printf("\nPacket_count: ");
	for(int j = 0; j < sou_data->count; j++){
	printf("%d ", sou_data->avg_int[j]);
	}

	printf("\n");
	fclose(soubor);
}

/*data kaput*/
void data_dtor(vstupni_data *data){
	free(data->soubor);
	free(data);
}


int main(int argc, char** argv){

	vstupni_data *data = data_ctor();

	data_z_souboru *sou_data = soubor_data_ctor();

	nacitani_nazvu_souboru(argc,argv,data);

	nacitani_vstupnich_dat(data, sou_data);

	data_dtor(data);

	return 0; 
}


