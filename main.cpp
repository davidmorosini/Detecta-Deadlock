#include "globais.h"
#include "parser.cpp"
#include "deadlock.cpp"

/**
* Arquivo com funções relacionadas ao parser da entrada para as estruturas de dados do programa.
*
* @date 05.07.2017
* @authors Caio Vianna, David Morosini
*/


 /** @brief Função imprimir um comando na saida padrão (P, V, IF, ELSE)
* @param comando c: estrutura de dados Comando
* @return void, apenas imprime os comandos e não retirna nada
*/
void imprime_comando(comando c){
	for(int k = 0; k < c.tabs; k++){
		cout << "\t";
	}
	switch (c.comando){
		case IF:
			printf("if %s", c.condicao);
			break;
		case ELSE:
			printf("else");
			break;
		case P:
			printf("p %d", c.valor);
			break;
		case V:
			printf("v %d", c.valor);
			break;
	}

}

 /** @brief Função que inicializa o leitor
* @param void
* @return leitor *: Um ponteiro para a estrutura de dados Leitor
*/
leitor *iniLeitor (void){
	leitor *novo = (leitor*)malloc(sizeof(leitor));
	if (novo != NULL){
		novo->a = EMPILHA;
		novo->index = 0;
		novo->flagV = 0;
		novo->comandos = new vector<comando>();
	}
	return novo;
}

/** @brief Funcao de inicializacao do vetor de leitores
* @param int qtd: tamanho do vector criado
* @return vector<leitor>: vector da estrutura de dados Leitor
*/
vector<leitor> iniVetLeitor (int qtd){
	int i;
	vector<leitor> novo;
	for(i = 0; i < qtd; i++){
		leitor *l = iniLeitor();
		novo.push_back(*l);
	}
	return novo;
}

/** @brief Funcao que varre o vetor de comandos a partir de uma determinada posicao do ELSE em determinada tabulacao
* @param ivector<comando> v: vector da estrutura de dados Comando
* @return int index
* @return int qtdTabs
*/
int procuraElse (vector<comando> v, int index, int qtdTabs){
	int resp = -1, i;
	for (i = index; i < v.size(); i++){
		/// Verifica se e um comando else e se este e o correto pela tabulacao
		if (v[i].comando == ELSE && v[i].tabs == qtdTabs)
			resp = i;
	}
	return resp;
}

/// Funcao principal que le os comandos da estrutura de dados e testa varias combinacoes
bool leComandos (data d){
	int i,j,k;
	int tabAux = MAX;
	int termina = 0;
	int desemp = 0;
	int flag = 0;
	vector<pt> deadlock;
	bool flagDeadlock = false;
	matriz indices;

	/// Vetor contendo uma pilha para cada thread do vetor de threads
	vector<leitor> leitores = iniVetLeitor(d.threads.size());

	for(k = 0;  termina < d.threads.size() && !flagDeadlock; k++){

		// Faz uma verificaçao se todas as threads ja terminaram seus comandos
		if (desemp >= leitores.size())
				flag = 1;
			else
				desemp = 0;

		/// Percorre os vector para empilhar os processos		
		for (i = 0; i < leitores.size(); i++){
			
			//printf("\nTHREAD %d\n", i+1);
			/// Verifica 
			if (leitores[i].index != MAX)
				j = leitores[i].index;
			else
				j = 0;

			for ( ; j < d.threads[i].comandos.size(); j++){
				// 2 casos principais, acao == empilha ou acao == desempilha
				if (leitores[i].a == EMPILHA && tabAux > d.threads[i].comandos[j].tabs){
					if (d.threads[i].comandos[j].comando == IF || d.threads[i].comandos[j].comando == P){ 
						leitores[i].comandos->push_back(d.threads[i].comandos[j]);
						leitores[i].index = j;
						tabAux = MAX;
					}
					// Se for V
					else if (d.threads[i].comandos[j].comando == V){
						// Flag que indica se a pilha parou neste V
						// Se nao, entao seta o flag para 1
						if (leitores[i].flagV == 0){
							leitores[i].index = j;
							leitores[i].flagV = 1;
							break;
						}
						// Se sim, faz o ajuste no vector dos semaforos
						else{
							d.sem[d.threads[i].comandos[j].valor]++;
							leitores[i].flagV = 0;
						}
					}
					// Ignora os elses
					else{
						tabAux = d.threads[i].comandos[j].tabs+1;
					}
					// Verifica se os comandos acabaram
					if (j == d.threads[i].comandos.size()-1 && leitores[i].flagV == 0){
						leitores[i].a = DESEMPILHA;
						desemp++;
						break;
					}
				}
				/// Quando a funcao esta em modo desempilha
				else if ((leitores[i].a == DESEMPILHA) && flag == 1){
					while ( 0 < leitores[i].comandos->size()){
						/// Desempilha ate encontrar um comando IF
						if (leitores[i].comandos->back().comando == IF){
							/// Se tem IF procura o ELSE
							int indElse = procuraElse(d.threads[i].comandos, leitores[i].index, leitores[i].comandos->back().tabs);
							if (indElse != -1){
								j = indElse;
								leitores[i].a = EMPILHA;
								leitores[i].index = j+1;
								tabAux = MAX;
							}
							else{
								j = d.threads[i].comandos.size()+1;
							}
							leitores[i].comandos->pop_back();
							break;
						}
						else{
							leitores[i].comandos->pop_back();
						}
					}
					/// Verifica se a pilha ja acabou, incrementando o termina em caso positivo
					if (leitores[i].comandos->size() == 0)
						termina++;
				}
				
				if (j == d.threads[i].comandos.size()-1 && leitores[i].flagV == 0){
					/// Verifica se nao esta ignorando ELSES
					if (tabAux != MAX){
						leitores[i].a = DESEMPILHA;
					}
					if (leitores[i].a == DESEMPILHA)
						desemp++;
					break;
				}

			
			}
		}
	 
	//##################################################################################	
		deadlock = percorre_matriz(leitores, d.sem, &indices);

		if(!deadlock.empty()){
			flagDeadlock = true;
		}
	//#################################################################################
	}


	//#################################################################################
	if(flagDeadlock){
		int i, j;
		for(i = 0; i < deadlock.size(); i += 2){
			int x = deadlock[i].x;
			int y = deadlock[i].y; 
			if(i % 2 != 0){
				int aux = y; 
				y = x;
				x = aux; 
			}

			comando c = d.threads[x].comandos[indices[x][y]];
			cout << d.threads[x].nome << ":";
			imprime_comando(c);
			cout << endl;
		}

		for(i = 1; i < deadlock.size(); i += 2){
			int x = deadlock[i].x;
			int y = deadlock[i].y;
			if(i % 2 != 0){
				int aux = y;
				y = x;
				x = aux; 
			}

	
			comando c = d.threads[x].comandos[indices[x][y]];
			cout << d.threads[x].nome << ":";
			imprime_comando(c);
			cout << endl;
		}

	}else{
		cout << "programa livre de impasses." << endl;	
	}
	/// A intenção é fazer um while enquanto a função de deadlock der false ou acabar o vetor de comandos
	//#################################################################################
	
	return true;
}

int main(void){
	data d = get_data();
	leComandos(d);

	return 0;
}