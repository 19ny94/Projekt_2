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
	unsigned int count;
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
	/*V jednotlyvem clusteru se muze nachazet vice toku
	 * ruzych poctu. Treba cluster 10 12 obsahuje flows[2]*/ 
	unsigned int* flows;
	int pocet;

} Flows;

typedef struct{
	Flows* data;
	int size;
} Clusters;

	/*podminka pro usporadani*/
int compare(const void *a, const void *b){

int x = *(int*)a;
int y = *(int*)b;

	return x - y;
}
	/*usporadani clusteru*/
int compare_struktur(const void *a, const void *b){
	
Flows *flow_a = (Flows *)a;
Flows *flow_b = (Flows *)b;

	return(flow_a->flows[0] - flow_b->flows[0]);
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

double double_z_stringu(char *c){

double result = 0;
int len = strlen(c);

/*nastavim na -1 protoze index cary muze 
 * byt jakymkoliv od 0 do nekonecna*/	
int pozice_cary = -1;

/*treba chci dostat int 10 z c[0] = 1, c[1] = 0
 *
 *int 0 = c[0] - '0'; 
 *int 1 = c[1] - '0';
 *
 *int 10 = 1 * 10^1 + 0 * 10^0;
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

	/*v podstate to stejny ale zaporna mocnina, slouzi pro desetinna cisla*/
	for(int i = pozice_cary + 1; i < len; i++){
	result += ((c[i] - '0') * pow(10, -(i - pozice_cary)));
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
	data->WB = 1;
	data->WT = 1;
	data->WD = 1;
	data->WS = 1;

	return data;
}

/*prvni argument pri spusteni programu - nazev souboru s daty*/
int nacitani_nazvu_souboru(int argc, char** argv, vstupni_data *data){

	
	if(argc > 1){
		/*dalo by se mozna udelat pres
		 * strcpy, ale mozna tenhle zpusob bezpecnejsi*/
		for(int i = 0; argv[1][i] != '\0'; i++){
		data->soubor = realloc(data->soubor, sizeof(char) +i +1);
		data->soubor[i] = argv[1][i];
		}
		data->soubor[strlen(argv[1])] = '\0';
	}else{
	printf("Argumenty nejsou zadane(zejmena nazev souboru)\n");
	/*exit(1) pro me je zpusob ukonceni programu bez segmt. fault*/
	free(data->soubor);
	exit(1);
	}
	return 1;
}

void nacitani_argumentu(int argc, char** argv, vstupni_data *data){

double *pole_pro_kontrolu[4] = {&data->WB, &data->WT, &data->WD, &data->WS};
	if(argc > 2){
	data->N = atoi(argv[2]);
	}
	if(argc > 3){
	data->WB = atoi(argv[3]);
	}
	if(argc > 4){
	data->WT = atoi(argv[4]);
	}
	if(argc > 5){
	data->WD = atoi(argv[5]);
	}
	if(argc > 6){
	data->WS = atoi(argv[6]);
	}
		
	for(int i = 0; i < 4; i++){
		if(*pole_pro_kontrolu[i] < 0){
		printf("Argumenty jsou realna nezaporna cisla\n");
		exit(1);
			}
	}
}
/*muze se zdat, ze je to nejaka kouzelna prace s ukazateli, ale slouzi pouze pro
 * prehlednost. Vzal jsem jenom kus kodu, ktery btw tam zustal pro flowid, 
 * a dal jsem ukazateli na promenne, ktere se pri behu meni*/
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

	/*zda se mi, ze staci pro projekt udelat pamet jednou pro 
	 * sou_data->count prvku. Calloc jsem pouzil z duvodu,
	 * protoze valgrind si stezoval, ze pokud alokovanou 
	 * pamet nepouziji - zustane alokovana, ale nepouzita*/
	sou_data->flowid=calloc(sou_data->count, sizeof(unsigned int));
	sou_data->total_b=calloc(sou_data->count, sizeof(double));
	sou_data->flow_dur=calloc(sou_data->count, sizeof(double));
	sou_data->pac_c=calloc(sou_data->count, sizeof(double));
	sou_data->d_avg=calloc(sou_data->count, sizeof(double));
	sou_data->s_avg=calloc(sou_data->count, sizeof(double));

	char *c = malloc(sizeof(char));

	int stav = OUT;
	int slovo = 0;
	int i = 0;
	int g = 0;
	int radek = 0;
	while((g = fgetc(soubor)) != EOF){
		

		/*Prikral poctani slov jsem vzal z knihy "The C Programming Language"*/
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
		sou_data->flowid[radek] = (unsigned int)double_z_stringu(c);
		}
	/*slovo 3 a 4 s ip prijemce a odesilatele jsem nepouzil,
	 * protoze neprisel jsem na to, jakzym zpusobem*/

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

	/*mozna da se najit chytrejsi reseni*/
	/*Controla, ze packet_count <D-z>*/
for(unsigned int i = 0; i < sou_data->count; i++){
	if(sou_data->pac_c[i] != 0){
	sou_data->s_avg[i] = sou_data->total_b[i]/sou_data->pac_c[i];
	}
	else{
	printf("Pozor!\nPacket_count != 0\n");
	exit(1);
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


/*Jelikoz funkce dist pracuje s indexy, a struktura clusteru predava
 * flowid, tak najdu ten flowid a vratim index.*/
int pokud_je_prvkem(data_z_souboru *s_data, unsigned potencialni_flowid){
	
	for(unsigned int i = 0; i < s_data->count; i++){
		if(s_data->flowid[i] == potencialni_flowid){
			return i;
			}
		}
	return -1;
}

double dist(vstupni_data *data, data_z_souboru *s_data, int A, int B){

double result = 0;
			/*Euklidovsta vzdalenost*/
	result += sqrt(data->WB * pow(s_data->total_b[A] - s_data->total_b[B], 2)+
		       data->WT * pow(s_data->flow_dur[A] - s_data->flow_dur[B],2)+
		       data->WD * pow(s_data->d_avg[A] - s_data->d_avg[B], 2)+
		       data->WS * pow(s_data->s_avg[A] - s_data->s_avg[B], 2));
	return result;
}

void inicializace_pole_clusteru(Clusters *c, data_z_souboru *s_data){

	c->data = malloc(sizeof(Flows) * s_data->count);
	c->size = s_data->count;
	
	for(unsigned int i = 0; i < s_data->count; i++){
	c->data[i].flows = malloc(sizeof(unsigned int));
	c->data[i].flows[0] = s_data->flowid[i];
	c->data[i].pocet = 1;
	}
}

void hledani_minima(Clusters *c, data_z_souboru *s_data, vstupni_data *data, int *idx_A, int *idx_B){

/*do minu ja pridam prvni porovnani, jelikoz kdybych
 * zadal min = 0 -> tak porovnani nedavaji smysl
 * v tomto pripade nebo tato distance bude nejmensi
 * nebo se najde mensi*/
double min = dist(data, s_data, 0, 1);
int d = 0;

	/*V te analize single linkage se pocita minimalni delka
	 * mezi toky vsech clusteru, teda pro kazde x, ktere
	 * nalezi X, a pro kazde y z Y, kde X a Y jsou clustery*/

	for(int i = 0; i < c->size; i++){
		for(int j = i + 1; j < c->size; j++){

			for(int a = 0; a < c->data[i].pocet; a++){
		/*x dostava ne cislo, ale index toku, jelikoz funkce dist pracuje s indexy*/
		int x = pokud_je_prvkem(s_data, c->data[i].flows[a]);	
				for(int b = 0; b < c->data[j].pocet; b++){
		/*y to stejne*/			
		int y = pokud_je_prvkem(s_data, c->data[j].flows[b]);

		if(x == -1 || y == -1){
		printf("Nevalidni data\n");
		exit(1);	
		}
		
			d = dist(data, s_data, x, y);
			
			if(min > d){
	/*opravdu nevim proc, ale nufunguje mi to kdyt zadam min = d*/
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

	/*Opakuje se, pokud se nedostane do pozadovaneho poctu clusteru*/
	while(c->size > (int)data->N){
	hledani_minima(c, s_data, data, &idx_A, &idx_B);

	/*Stary_pocet - delka flows na indexu ind_A pred popovanim*/
	int stary_pocet = c->data[idx_A].pocet;
	/*Novy_pocet - delka flows, aby ten novy prvek mohla prijmout*/
	int new_pocet = c->data[idx_A].pocet + c->data[idx_B].pocet;
	c->data[idx_A].pocet = new_pocet;	
	c->data[idx_A].flows = realloc(c->data[idx_A].flows, sizeof(int) * new_pocet);

	
	for(int i = 0 ; i < c->data[idx_B].pocet; i++){
	c->data[idx_A].flows[stary_pocet + i] = c->data[idx_B].flows[i];
	}

	/*Jelikoz uz nepotrebujeme - uvolnime*/
	free(c->data[idx_B].flows);
	
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

		/*Na zacatku se provede sortovani toku v jednotlyvych kasterach*/
	qsort(c->data[i].flows, c->data[i].pocet, sizeof(signed int), compare);
		/*Pak se provede sortovani clusteru po prvnimu toku, jelikoz pole uz serazene*/
	qsort(c->data, c->size, sizeof(Flows), compare_struktur);

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
	Clusters *c = clusters_ctor();

	nacitani_nazvu_souboru(argc,argv,data);
	
	nacitani_argumentu(argc, argv, data);

	nacitani_vstupnich_dat(data, sou_data);

	if(argc < 3){
	data->N = sou_data->count;
	}

	inicializace_pole_clusteru(c, sou_data);

	shlukovaci_analyza(c, sou_data, data);

	print_vysledku(c);

	data_dtor(data);
	soubor_data_dtor(sou_data);
	clusters_dtor(c);

	return 0; 
}


