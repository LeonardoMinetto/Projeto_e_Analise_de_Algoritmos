#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#define MAXN 6000

typedef struct{
    int n;
    double x[MAXN], y[MAXN];
    double reward[MAXN];
    double dist[MAXN][MAXN];
    int depot;
    double custo_max;
} Problema;

typedef struct{
    int caminho[MAXN];
    int tam;
    double custo;
    double recompensa_total;
} Solucao;

double calcular_distancia(Problema *P, int i, int j) {
    double dx = P->x[i] - P->x[j];
    double dy = P->y[i] - P->y[j];
    return sqrt(dx*dx + dy*dy);
}

void ler_oplib(Problema *P, const char *nome_arquivo) {
    FILE *f = fopen(nome_arquivo, "r");
    if (!f) {
        printf("Erro ao abrir arquivo!\n");
        exit(1);
    }

    char linha[256];

    while (fgets(linha, sizeof(linha), f)) {

        if (strstr(linha, "DIMENSION")) {
            sscanf(linha, "DIMENSION : %d", &P->n);
        }

        if (strstr(linha, "COST_LIMIT")) {
            sscanf(linha, "COST_LIMIT : %lf", &P->custo_max);
        }

        if (strstr(linha, "NODE_COORD_SECTION")) {
            for (int i = 0; i < P->n; i++) {
                int id;
                fscanf(f, "%d %lf %lf", &id, &P->x[i], &P->y[i]);
            }
        }

        if (strstr(linha, "NODE_SCORE_SECTION")) {
            for (int i = 0; i < P->n; i++) {
                int id;
                fscanf(f, "%d %lf", &id, &P->reward[i]);
            }
        }

        if (strstr(linha, "DEPOT_SECTION")) {
            fscanf(f, "%d", &P->depot);
            P->depot = 0; 
        }
    }

    for(int i = 0; i < P->n; i++){
        for(int j = 0; j < P->n; j++){

            double dx = P->x[i] - P->x[j];
            double dy = P->y[i] - P->y[j];

            P->dist[i][j] = calcular_distancia(P, i, j);
        }
    }

    fclose(f);
}

double custo_rota(Problema *P, Solucao *S) {
    double custo = 0.0;
    for (int i = 0; i < S->tam - 1; i++) {
        custo += P->dist[S->caminho[i]][S->caminho[i + 1]];
    }
    return custo;
}

double recompensa_total(Problema *P, Solucao *S) {
    double recompensa = 0.0;
    for (int i = 1; i < S->tam; i++) {
        recompensa += P->reward[S->caminho[i]];
    }
    return recompensa;
}

void gulosoV2(Problema *P, Solucao *S) {

    bool visitado[MAXN];
    memset(visitado, 0, sizeof(bool) * P->n);

    int atual = P->depot;
    double tempo_restante = P->custo_max;

    S->tam = 0;
    S->caminho[S->tam++] = atual;
    visitado[atual] = true;

    while(1){
        int melhor_no = -1;
        double melhor_score = 0.0;
        for(int i = 0; i < P->n; i++){
            if(!visitado[i]){
                double custo = P->dist[atual][i];
                if(custo <= 0) continue;

                double score = P->reward[i] / custo;

                if(custo <= tempo_restante && score > melhor_score){
                    melhor_score = score;
                    melhor_no = i;
                }
            }
        }

        if( melhor_no == -1)
            break;
        
        tempo_restante -= P->dist[atual][melhor_no];
        atual = melhor_no;
        visitado[atual] = true;
        S->caminho[S->tam++] = atual;
    }
    S->custo = custo_rota(P, S);
    S->recompensa_total = recompensa_total(P, S);
}

int two_opt(Problema *P, Solucao *S) {

    int melhor_i = -1;
    int melhor_j = -1;
    double melhor_delta = 0.0;

    for (int i = 1; i < S->tam - 1; i++) {

        for (int j = i + 1; j < S->tam; j++) {

            int a = S->caminho[i - 1];
            int b = S->caminho[i];
            int c = S->caminho[j];

            double custo_atual;
            double custo_novo;

            if (j == S->tam - 1) {
                custo_atual = P->dist[a][b];
                custo_novo = P->dist[a][c];
            } else {
                int d = S->caminho[j + 1];

                custo_atual = P->dist[a][b] + P->dist[c][d];
                custo_novo = P->dist[a][c] + P->dist[b][d];
            }

            double delta = custo_novo - custo_atual;

            if (delta < melhor_delta) {
                melhor_delta = delta;
                melhor_i = i;
                melhor_j = j;
            }
        }
    }

    if (melhor_i != -1) {

        int k = melhor_i;
        int l = melhor_j;

        while (k < l) {
            int temp = S->caminho[k];
            S->caminho[k] = S->caminho[l];
            S->caminho[l] = temp;

            k++;
            l--;
        }

        S->custo += melhor_delta;

        return 1;
    }

    return 0;
}


int add_nodes(Problema *P, Solucao *S) {

    bool inRota[MAXN];
    memset(inRota, 0, sizeof(bool) * P->n);

    for (int i = 0; i < S->tam; i++) {
        inRota[S->caminho[i]] = true;
    }

    int melhor_no = -1;
    double melhor_score = -1.0;

    int ultimo = S->caminho[S->tam - 1];

    for (int no = 0; no < P->n; no++) {

        if (inRota[no])
            continue;

        double custo_extra = P->dist[ultimo][no];

        if (S->custo + custo_extra <= P->custo_max) {

            double score = P->reward[no] / custo_extra;

            if (score > melhor_score) {
                melhor_score = score;
                melhor_no = no;
            }
        }
    }

    if (melhor_no != -1) {

        double custo_extra =
            P->dist[ultimo][melhor_no];

        S->caminho[S->tam++] = melhor_no;

        S->custo += custo_extra;

        S->recompensa_total += P->reward[melhor_no];

        return 1;
    }

    return 0;
}


void busca_local(Problema *P, Solucao *S){
    int melhorou = 1;
    while(melhorou){
        melhorou = 0;
        if(two_opt(P, S)){
            melhorou = 1;
        }
        if(add_nodes(P, S)){
            melhorou = 1;
        }
    
    }

    S->custo = custo_rota(P, S);
    S->recompensa_total = recompensa_total(P, S);
}



int main() {
    static Problema P;
    static Solucao S;
    char *arquivos[] = {
        "InstanciasOPLIB/rat99-gen2-50.oplib",
        "InstanciasOPLIB/rat195-gen2-50.oplib",
        "InstanciasOPLIB/rat575-gen2-50.oplib",
        "InstanciasOPLIB/rat783-gen2-50.oplib",
        "InstanciasOPLIB/rl1889-gen2-50.oplib",
        "InstanciasOPLIB/fnl4461-gen3-50.oplib",
        "InstanciasOPLIB/rl5934-gen2-50.oplib"
    };

    int num_instancias = 7;
    double rec_guloso[7];
    double rec_local[7];
    double rec_otimo[7] = { 2944.0, 5703.0, 17705.0, 24781.0, 60084.0, 164201.0, 176678.0};
    double temp[7];
    double custo_guloso[7];
    double custo_local[7];
    int tam_guloso[7];
    int tam_local[7];
    double folga_local[7];

    for (int k = 0; k < num_instancias; k++) {

        printf("\n--------------------------------\n");
        printf("Instância: %s\n", arquivos[k]);
        printf("--------------------------------\n");

        ler_oplib(&P, arquivos[k]);

        printf("Nós: %d\n", P.n);
        printf("Depot: %d\n", P.depot);
        printf("Tempo máximo: %.2f\n", P.custo_max);

        clock_t inicio = clock();

        gulosoV2(&P, &S);
        rec_guloso[k] = S.recompensa_total;
        custo_guloso[k] = S.custo;
        tam_guloso[k] = S.tam;
        busca_local(&P, &S);
        rec_local[k] = S.recompensa_total;
        custo_local[k] = S.custo;
        tam_local[k] = S.tam;
        folga_local[k] = P.custo_max - S.custo;
        clock_t fim = clock();

        temp[k] = (double)(fim - inicio) / CLOCKS_PER_SEC;

        printf("Recompensa gulosa: %.2f\n", rec_guloso[k]);
        printf("Recompensa com busca local: %.2f\n", rec_local[k]);
        printf("custo final: %.2f\n", S.custo);
        printf("Tempo: %f segundos\n", temp[k]);
    }

    printf("\n-------------------------------------------------------------------------------------------------------------------------------\n");
    printf("| %-24s | %10s | %10s | %10s | %12s | %10s | %10s |\n",
        "Instancia", "Otimo", "Guloso", "Q.Guloso", "BuscaLocal", "Q.Local", "Tempo(s)");
    printf("-------------------------------------------------------------------------------------------------------------------------------\n");

    for (int k = 0; k < num_instancias; k++) {
        printf("| %-24s | %10.1f | %10.1f | %9.2f%% | %12.1f | %9.2f%% | %10.6f |\n",
            arquivos[k],
            rec_otimo[k],
            rec_guloso[k],
            (rec_guloso[k] / rec_otimo[k]) * 100.0,
            rec_local[k],
            (rec_local[k] / rec_otimo[k]) * 100.0,
            temp[k]);
    }

    printf("-------------------------------------------------------------------------------------------------------------------------------\n");

    printf("\nResultados detalhados de custo e tamanho:\n");
    for(int k = 0; k < num_instancias; k++){
        printf("\nInstância: %s\n", arquivos[k]);
        printf("Custo guloso: %.2f\n", custo_guloso[k]);
        printf("Custo local: %.2f\n", custo_local[k]);
        printf("Tam guloso: %d\n", tam_guloso[k]);
        printf("Tam local: %d\n", tam_local[k]);
        printf("folga após 2-opt: %.2f\n", folga_local[k]);
    }
    return 0;
}