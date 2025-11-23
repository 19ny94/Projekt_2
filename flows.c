#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	unsigned int flowid;
	/*obsah dat*/
	int total_bytes;
	/*IP odesilatele*/
	int src_ip;
	/*IP prijemce*/
	int dst_ip;
	/*delka toku dat*/
	int flow_duration;
	/*kolik paketu se poslalu/dostalo*/
	int packet_count;
	/*prumerna delka*/
	double avg_interrival;
	
} data_z_souboru;

data_z_souboru *soubor_data_ctor(){

	data_z_souboru *s_data= malloc(sizeof(data_z_souboru));
	
	s_data->count = 0;
	s_data->src_ip = 0;
	s_data->dst_ip = 0;
	s_data->total_bytes = 0;
	s_data->flow_duration = 0;
	s_data->packet_count = 0;
	s_data->avg_interrival = 0.0;
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
	char *c = malloc(100);

	int stav = OUT;
	int slovo = 0;
	int i = 0;
	int g = 0;
	while((g = fgetc(soubor)) != EOF){
	putchar(g);/*PAK ODSTANIT*/
		
		if(g == ' ' || g == '\t' || g == '\n'){
		stav = OUT;
		}
		else if(stav == OUT){
		stav = IN;
		++slovo;
		i = 0;
		}
			

		if(stav == IN && slovo == 1){
		c[i] == g;
		i++;
		}
	}

	printf("Pocet slov: %d\n", slovo);
	printf("Flowid: %c%c\n",c[0], c[1]);
	printf("%d\n", sou_data->count);
	printf("%d\n", sou_data->flowid);
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


